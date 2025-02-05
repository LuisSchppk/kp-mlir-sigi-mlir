#include "Utils.h"
#include "sigi-mlir/Conversion/ClosurePasses.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <cassert>
#include <cstddef>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/EpochTracker.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Dialect/Bufferization/Transforms/OneShotAnalysis.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/IR/Attributes.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/Location.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/OpDefinition.h>
#include <mlir/IR/Operation.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/Value.h>
#include <mlir/IR/ValueRange.h>
#include <mlir/Interfaces/ControlFlowInterfaces.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Support/LLVM.h>
#include <mlir/Transforms/Passes.h>

#define GEN_PASS_DEF_STACKELIMINATIONPASS
namespace mlir::sigi {
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace mlir::sigi

namespace mlir::closure {}

#define DEBUG_TYPE "stack-elim"
using namespace mlir;
namespace {

std::string has_global_stack = "has-global-stack";

struct StackEliminationPass
        : public mlir::sigi::impl::StackEliminationPassBase<StackEliminationPass> {

    struct EliminateStack : OpRewritePattern<func::FuncOp> {
        using OpRewritePattern<func::FuncOp>::OpRewritePattern;

        template<typename R>
        SmallVector<Value> getStackFrom(R &&Range) const
        {
            SmallVector<Value> result;
            for (auto value : Range)
                if (llvm::isa<sigi::StackType>(value.getType())) result.push_back(value);
            return result;
        }

        LogicalResult getReturnOp(func::FuncOp funcOp, func::ReturnOp &returnOp) const
        {
            int count = 0;
            for (auto op : funcOp.getOps<func::ReturnOp>()) {
                returnOp = op;
                count++;
            }

            if (count != 1) return llvm::failure();
            return llvm::success();
        }

        SmallVector<Operation*> mergeAndSort(DenseMap<Value, SmallVector<Operation*>> map) const
        {
            SmallVector<Operation*> merged;
            for (auto [val, vec] : map) merged.append(vec);

            llvm::sort(merged, [&](Operation* a, Operation* b) {
                if (isBeforeInOp(a, b))
                    return b < a;
                else
                    return a < b;
            });

            return merged;
        }

        bool isGlobalStackNecessary(
            func::FuncOp funcOp,
            SmallVector<Operation*> inParamDefs,
            SmallVector<Operation*> outParamDefs) const
        {
            bool result = llvm::is_contained(
                funcOp.getFunctionType().getInputs(),
                sigi::StackType::get(funcOp->getContext()));
            if (inParamDefs.size() > 0 && outParamDefs.size() > 0) {
                auto lastPop = cast<sigi::PopOp>(inParamDefs.back());
                auto firstPush = cast<sigi::PushOp>(outParamDefs.front());
                result = lastPop.getOutStack().hasOneUse()
                         && lastPop.getOutStack() != firstPush.getInStack();
            } else if (inParamDefs.size() > 0) {
                auto lastPop = cast<sigi::PopOp>(inParamDefs.back());
                result = !lastPop->use_empty();
            } else {
                funcOp->walk([&](sigi::GetGlobalStack) { result = false; });
            }
            return result;

            // DEFAULT TO
            /*
             * Case: inParamsDefs.size() == 0 -> 1. no input stack 2. No Pops
             *      1. No input stack; if there is an output stack -> has to be init in this func,
                                so we do nothing
                    2. input stack but no pops: provide global stack to store pushes.
             */
        }

        LogicalResult matchAndRewrite(func::FuncOp funcOp, PatternRewriter &rewriter) const override
        {
            LLVM_DEBUG(llvm::errs() << " ENTERED REWRITE ON " << funcOp.getSymName() << "\n");
            if (!funcOp->hasAttr("sigi.stackType")
                || !(
                    llvm::is_contained(
                        funcOp.getFunctionType().getInputs(),
                        sigi::StackType::get(funcOp->getContext()))
                    || llvm::is_contained(
                        funcOp.getFunctionType().getResults(),
                        sigi::StackType::get(funcOp->getContext())))
                || funcOp.getBlocks().empty())
                return llvm::failure();

            auto moduleOp = funcOp->getParentOfType<ModuleOp>();
            Block* entryBlock = &funcOp.getBlocks().front();
            func::ReturnOp returnOp;
            if (getReturnOp(funcOp, returnOp).failed()) return llvm::failure();

            auto stackTypeAttr = llvm::cast<TypeAttr>(funcOp->getAttr("sigi.stackType"));
            auto stackFuncType = cast<FunctionType>(stackTypeAttr.getValue());

            // Stack -> reverts order
            SmallVector<Type> inputs{llvm::reverse(stackFuncType.getInputs())};
            SmallVector<Type> outputs{llvm::reverse(stackFuncType.getResults())};
            DenseMap<Value, SmallVector<Operation*>> stackParameter;
            DenseMap<Value, SmallVector<Operation*>> stackResults;
            auto inStacks = getStackFrom(entryBlock->getArguments());
            auto outStacks = getStackFrom(returnOp->getOperands());

            if (inStacks.size() > 1 || outStacks.size() > 1) return llvm::failure();
            Value inStack = inStacks.empty() ? NULL : inStacks.front();
            Value outStack = outStacks.empty() ? NULL :outStacks.front();

            Operation* op_getGlobalStack = nullptr;
            funcOp->walk([&](sigi::GetGlobalStack op) { op_getGlobalStack = op.getOperation(); });

            LLVM_DEBUG(llvm::errs() << " MATCH SUCCESS \n");

            if (!op_getGlobalStack) {
                rewriter.setInsertionPointToStart(entryBlock);
                auto globalStack = rewriter.create<sigi::GetGlobalStack>(
                    funcOp->getLoc(),
                    sigi::StackType::get(funcOp->getContext()));

                if (inStack) rewriter.replaceAllUsesWith(inStack, globalStack.getGlobalStack());
                op_getGlobalStack = globalStack.getOperation();
            }

            assert(op_getGlobalStack && "GLOBAL STACK WAS NOT SET!");
            sigi::GetGlobalStack globalStackOp = cast<sigi::GetGlobalStack>(op_getGlobalStack);
            if (inStack)
                assert(
                    inStack.use_empty()
                    && "USE OF STACK ARG WAS NOT REPLACED IN THIS OR PREV. RUNS OF THE PATTERN.");

            SmallVector<size_t> markForErasure;
            if (inStack) {
                for (size_t i = 0; i < entryBlock->getNumArguments(); i++) {
                    if (entryBlock->getArgument(i) == inStack) {
                        LLVM_DEBUG(llvm::errs() << "ERASE " << entryBlock->getArgument(i) << "\n");
                        markForErasure.push_back(i);
                    }
                }

                assert(
                    markForErasure.size() <= 1
                    && "Can not erase multiple Stacks. Only one Stack Supported.");
                for (auto eraseIdx : markForErasure) {
                    // DOES THIS CIRCUMVENT LISTENERS ATTACHED TO REWRITER??
                    entryBlock->eraseArgument(eraseIdx);
                }

                SmallVector<Type> newInTypes;
                SmallVector<Location> newInLocs;
                for (auto oldIn : funcOp.getFunctionType().getInputs()) {
                    if (llvm::isa<sigi::StackType>(oldIn))
                        for (auto newIn : stackFuncType.getInputs()) {
                            newInLocs.push_back(funcOp->getLoc());
                            newInTypes.push_back(newIn);
                        }
                    else {
                        newInTypes.push_back(oldIn);
                        newInLocs.push_back(funcOp->getLoc());
                    }
                }

                rewriter.setInsertionPointAfter(globalStackOp);
                Value currentStack = globalStackOp.getResult();
                entryBlock->addArguments(newInTypes, newInLocs);
                for (auto arg : entryBlock->getArguments()) {
                    auto paramPush = rewriter.create<sigi::PushOp>(
                        funcOp->getLoc(),
                        sigi::StackType::get(funcOp->getContext()),
                        currentStack,
                        arg);
                    currentStack = paramPush.getOutStack();
                }

                rewriter.replaceAllUsesExcept(
                    globalStackOp.getResult(),
                    currentStack,
                    globalStackOp->getNextNode());
            }

            // i32 i32 i32 -> i32 i1 closure
            // push i32 
            // push i1
            // push closure
            // pop closure
            // pop i1
            // pop i32 
            if(outStack) {
                rewriter.setInsertionPoint(returnOp);
                Value currentStack = outStack;
                SmallVector<Value> newResults;
                for(auto results : outputs) {
                    auto resultPop  = rewriter.create<sigi::PopOp>(funcOp.getLoc(), sigi::StackType::get(funcOp->getContext()), results, currentStack);
                    currentStack = resultPop.getOutStack();
                    newResults.push_back(resultPop.getValue());
                }
                newResults = {llvm::reverse(newResults).begin(), llvm::reverse(newResults).end()};
                auto newReturn = rewriter.create<func::ReturnOp>(funcOp->getLoc(), newResults);
                rewriter.replaceOp(returnOp, newReturn);
                returnOp = newReturn;
            }

            SmallVector<Type> newInTypes{entryBlock->getArgumentTypes()};
            SmallVector<Type> newOutTypes{returnOp->getOperandTypes()};
            FunctionType newFunctionType =
                FunctionType::get(funcOp->getContext(), newInTypes, newOutTypes);
            rewriter.modifyOpInPlace(funcOp, [&]() { funcOp.setFunctionType(newFunctionType); });

            return llvm::success();
        }
    };

    void runOnOperation() override
    {
        RewritePatternSet patterns(&getContext());
        GreedyRewriteConfig greedyConf;
        greedyConf.strictMode = GreedyRewriteStrictness::ExistingOps;
        patterns.add<EliminateStack>(&getContext());
        (void)applyOpPatternsAndFold({getOperation()}, std::move(patterns), greedyConf);
    }
};
} // namespace