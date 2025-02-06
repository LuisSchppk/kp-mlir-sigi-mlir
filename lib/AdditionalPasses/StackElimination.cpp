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
#include <mlir/IR/IRMapping.h>
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
#include <mlir/Transforms/GreedyPatternRewriteDriver.h>
#include <mlir/Transforms/Passes.h>
#include <sys/types.h>
#include <utility>

#define GEN_PASS_DEF_STACKELIMINATIONPASS
namespace mlir::sigi {
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace mlir::sigi

namespace mlir::closure {}

#define DEBUG_TYPE "stack-elim"
using namespace mlir;
namespace {

std::string has_global_stack = "has-global-stack";
std::string stack_type_attr = "sigi.stackType";
std::string sigi_builtin = "sigi.builtinfunc";

    struct StackEliminationPass
        : public mlir::sigi::impl::StackEliminationPassBase<StackEliminationPass> {

    template<typename R>
    static SmallVector<Value> getStackFrom(R &&Range)
    {
        SmallVector<Value> result;
        for (auto value : Range)
            if (llvm::isa<sigi::StackType>(value.getType())) result.push_back(value);
        return result;
    }

    static LogicalResult getReturnOp(func::FuncOp funcOp, func::ReturnOp &returnOp)
    {
        int count = 0;
        for (auto op : funcOp.getOps<func::ReturnOp>()) {
            returnOp = op;
            count++;
        }

        if (count != 1) return llvm::failure();
        return llvm::success();
    }

    struct AdaptCallSite : OpRewritePattern<func::CallOp> {
        using OpRewritePattern<func::CallOp>::OpRewritePattern;

        LogicalResult matchAndRewrite(func::CallOp callOp, PatternRewriter &rewriter) const override
        {
            LLVM_DEBUG(llvm::errs() << "ENTERED CLEAN UP CALLSITE AT " << callOp->getLoc() << "\n");

            // FAILURE 1
            if (!callOp->hasAttr(stack_type_attr) 
                || !(
                    llvm::is_contained(
                        callOp.getCalleeType().getInputs(),
                        sigi::StackType::get(callOp->getContext()))
                    || llvm::is_contained(
                        callOp.getCalleeType().getResults(),
                        sigi::StackType::get(callOp->getContext()))))
                return llvm::failure();

            // FAILUTRE 2: I HAVE TO ASSUME THIS ALWAYS HOLDS, same as !funcOp.getBlocks.empty
            // FAILURE 3:
            auto inStacks = getStackFrom(callOp.getOperands());
            auto outStacks = getStackFrom(callOp->getResults());
            if (inStacks.size() > 1 || outStacks.size() > 1) return llvm::failure();

            auto callee = SymbolTable::lookupSymbolIn(callOp->getParentOfType<ModuleOp>(), callOp.getCalleeAttr());
            if(auto funcOp = dyn_cast<func::FuncOp>(callee)) {
                if(funcOp->hasAttr(sigi_builtin)) return llvm::failure();
            } else {
                return llvm::failure();
            }

            auto stackTypeAttr = llvm::cast<TypeAttr>(callOp->getAttr(stack_type_attr));
            auto stackFuncType = cast<FunctionType>(stackTypeAttr.getValue());
            SmallVector<Type> inputs{llvm::reverse(stackFuncType.getInputs())};
            SmallVector<Type> outputs{llvm::reverse(stackFuncType.getResults())};
            func::FuncOp encompassingFunc = callOp->getParentOfType<func::FuncOp>();

            LLVM_DEBUG(llvm::errs() << "CALL SITE MATCH DONE\n");

            // i32 i1 closure -> i1 closure i32
            //  stack closure i1 i32
            rewriter.setInsertionPoint(callOp);
            SmallVector<Value> newOperands = callOp->getOperands();
            Value lastStack = NULL;
            if (!inStacks.empty()) {
                Value inStack = inStacks.front();
                auto currentStack = inStack;
                SmallVector<Value> stackOperands;
                newOperands.clear();
                for (auto input : inputs) {
                    auto popParam = rewriter.create<sigi::PopOp>(
                        encompassingFunc->getLoc(),
                        sigi::StackType::get(callOp->getContext()),
                        input,
                        currentStack);
                    stackOperands.push_back(popParam.getValue());
                    currentStack = popParam.getOutStack();
                }
                lastStack = currentStack;

                for (auto operand : callOp->getOperands()) {
                    if (operand == inStack)
                        for (auto stackOperand : stackOperands) newOperands.push_back(stackOperand);
                    else
                        newOperands.push_back(operand);
                }
            }

            if (!outStacks.empty()) {
                Value outStack = outStacks.front();
                Value currentStack;
                if (inStacks.empty()) {
                    /*
                     * In this case we have a function that takes no input stack, but still returns
                     * a stack. So the stack has to be initalized within that function.
                     * e.g. Main
                     * So here we just get the global stack and push the results?
                     * ATTENTION: THIS SHOULD NEVER BE THE CASE IN SIGI: ONLY ONE STACK!!!
                     *          If parentFunc already contains a global stack -> we just reload that
                     *          [lets hope that lowered to LLVM this works]
                     * If parentFunc has no global stack -> initialize
                     */
                    auto getGlobal = rewriter.create<sigi::GetGlobalStack>(
                        encompassingFunc->getLoc(),
                        sigi::StackType::get(encompassingFunc->getContext()));
                    currentStack = getGlobal.getGlobalStack();
                } else {
                    assert(lastStack && "LAST STACK HAS TO BE SET IF THERE IS AN INPUT STACK!\n");
                    currentStack = lastStack;
                }

                SmallVector<Type> newResultType;
                for (auto resultType : callOp->getResultTypes()) {
                    if (llvm::isa<sigi::StackType>(resultType))
                        for (auto stackType : stackFuncType.getResults())
                            newResultType.push_back(stackType);
                    else
                        newResultType.push_back(resultType);
                }

                auto newCall = rewriter.create<func::CallOp>(
                    encompassingFunc->getLoc(),
                    callOp.getCalleeAttr(),
                    newResultType,
                    newOperands);

                for (auto result : newCall->getResults()) {
                    auto resultPush = rewriter.create<sigi::PushOp>(
                        encompassingFunc->getLoc(),
                        sigi::StackType::get(encompassingFunc->getContext()),
                        currentStack,
                        result);
                    currentStack = resultPush.getOutStack();
                }

                SmallVector<Value> oldResults = {callOp->getResults()};
                SmallVector<Value> newResults = {newCall->getResults()};
                IRMapping oldToNew;
                int newIdx = 0;
                for (auto result : oldResults) {
                    if (result == outStack) {
                        newIdx += outputs.size();
                    } else {
                        assert(
                            (size_t)newIdx < newResults.size() - 1
                            && "\nINDEX OVERFLOW AT OLD TO NEW MAPPING\n");
                        rewriter.replaceAllUsesWith(result, newResults[newIdx]);
                    }
                }
                rewriter.replaceAllUsesWith(outStack, currentStack);
                rewriter.eraseOp(callOp);
            } else {

                // if only the operands changed, there is no need to replace call, just replace
                // operands
                rewriter.modifyOpInPlace(callOp, [&]() {
                    callOp->eraseOperands(0, callOp->getNumOperands());
                    callOp->setOperands(newOperands);
                });
            }

            LLVM_DEBUG(llvm::errs() << encompassingFunc << "\n");
            return llvm::success();
        };
    };

    struct EliminateStack : OpRewritePattern<func::FuncOp> {
        using OpRewritePattern<func::FuncOp>::OpRewritePattern;

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
                return llvm::failure(); // FAILURE 1

            Block* entryBlock = &funcOp.getBlocks().front();
            func::ReturnOp returnOp;
            if (getReturnOp(funcOp, returnOp).failed()) return llvm::failure(); // FAILURE 2

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
            Value outStack = outStacks.empty() ? NULL : outStacks.front();

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
            if (outStack) {
                rewriter.setInsertionPoint(returnOp);
                Value currentStack = outStack;
                SmallVector<Value> newResults;
                for (auto results : outputs) {
                    auto resultPop = rewriter.create<sigi::PopOp>(
                        funcOp.getLoc(),
                        sigi::StackType::get(funcOp->getContext()),
                        results,
                        currentStack);
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
        RewritePatternSet callPatterns(&getContext());
        GreedyRewriteConfig greedyConf;
        greedyConf.strictMode = GreedyRewriteStrictness::ExistingOps;
        patterns.add<EliminateStack>(&getContext());
        callPatterns.add<AdaptCallSite>(&getContext());
        SmallVector<Operation*> callOps;
        getOperation().walk([&](func::CallOp callOp) { callOps.push_back(callOp.getOperation()); });
        (void)applyOpPatternsAndFold({getOperation()}, std::move(patterns), greedyConf);
        (void)applyOpPatternsAndFold(callOps, std::move(callPatterns), greedyConf);
    }
};
} // namespace