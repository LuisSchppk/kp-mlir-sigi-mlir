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
            auto inputs = stackFuncType.getInputs();
            auto outputs = stackFuncType.getResults();
            DenseMap<Value, SmallVector<Operation*>> stackParameter;
            DenseMap<Value, SmallVector<Operation*>> stackResults;
            auto inStacks = getStackFrom(entryBlock->getArguments());
            auto outStacks = getStackFrom(returnOp->getOperands());

            if (inStacks.size() > 1 || outStacks.size() > 1) return llvm::failure();
            auto inStack = inStacks.front();
            auto outStack = outStacks.front();

            SmallVector<Operation*> stack;

            for (auto &use : inStack.getUses())
                if (use.getOwner()) stack.push_back(use.getOwner());

            while (!stack.empty() && stackParameter.size() < inputs.size()) {
                auto current = stack.pop_back_val();
                if (auto pop = llvm::dyn_cast<sigi::PopOp>(current)) {
                    assert(
                        pop.getValue().getType() == inputs[stackParameter.size()]
                        && "Input Parameter has wrong type.");

                    if (stackParameter.contains(pop.getInStack())) {
                        auto ops = stackParameter[pop.getInStack()];
                        ops.push_back(pop.getOperation());
                        stackParameter.insert({pop.getInStack(), ops});
                    } else {
                        stackParameter.insert({pop.getInStack(), {pop.getOperation()}});
                    }

                    for (auto &use : pop.getOutStack().getUses())
                        if (use.getOwner()) stack.push_back(use.getOwner());

                } else if (auto ifOp = llvm::dyn_cast<scf::IfOp>(current)) {
                    // NEVER THE CASE? NO WAY TO EVALUATE stack to i1
                } else {
                    /*
                     * Assumption: Dialects SCF, CF, Arith, Func, Sigi, Closure are allowed.
                     * SCF -> Possible to be ARG to For, While,... sp we fail.
                     * CF -> No Possible Uses
                     * Arith -> No Possible Uses
                     * Func -> Return of Call -> Failure, try to convert other func first, what
                     *          happens on f -> g && g -> f ?
                     * Sigi -> only pop
                     * Closure -> inlined by now? otherwise fail this optimization.
                     */
                    return llvm::failure();
                }
            }

            if (stackParameter.size() != inputs.size()) return llvm::failure();

            stack.clear();
            stack = {outStack.getDefiningOp()};
            while (!stack.empty() && stackResults.size() < outputs.size()) {
                auto current = stack.pop_back_val();
                if (auto push = llvm::dyn_cast<sigi::PushOp>(current)) {
                    assert(
                        push.getValue().getType() == outputs[stackResults.size()]
                        && "Output Parameter has wrong type.");

                    if (stackResults.contains(push.getOutStack())) {
                        auto ops = stackResults[push.getOutStack()];
                        ops.push_back(push.getOperation());
                        stackResults.insert({push.getOutStack(), ops});
                    } else {
                        stackResults.insert({push.getInStack(), {push.getOperation()}});
                    }

                    stack.push_back(push.getInStack().getDefiningOp());
                    LLVM_DEBUG(llvm::errs() << current->getLoc() << "\n");
                } else if (auto ifOp = llvm::dyn_cast<scf::IfOp>(current)) {
                    for (auto sigiStack : getStackFrom(ifOp.elseYield()->getOperands()))
                        stack.push_back(sigiStack.getDefiningOp());

                    for (auto sigiStack : getStackFrom(ifOp.thenYield()->getOperands()))
                        stack.push_back(sigiStack.getDefiningOp());

                } else if (stack.empty()) {
                    return llvm::failure();
                }
            }

            if (stackResults.size() != outputs.size()) return llvm::failure();

            SmallVector<Operation*> allParamPops = mergeAndSort(stackParameter);
            SmallVector<Operation*> allResultPushes = mergeAndSort(stackResults);

            // bool globalStackNecessary = isGlobalStackNecessary(funcOp, inParamDefs,
            // outParamDefs);
            bool hasGlobalStack = false;
            funcOp->walk([&](sigi::GetGlobalStack) { hasGlobalStack = true; });

            LLVM_DEBUG(llvm::errs() << " MATCH SUCCESS \n");
            // MATCH DONE

            /*
             * 1. Adjust Call sites -> What happens to call sites outside of module?
             *          Trust Process: Additional Rewrite for all func.calls to external where the
             *                      Call gets adjusted to meet the stackTypes
             *          Safety: for funcOps within module where we did not achieve this fix with
             *                  global stack
             * 2. Erase old push & pop
             * 3. Insert global stack where needed
             * 4. Implement Safety for (1).
             */
            for (auto op : allParamPops) LLVM_DEBUG(llvm::errs() << op->getLoc() << "\n");
            if (!hasGlobalStack) {
                rewriter.setInsertionPointToStart(entryBlock);
                auto globalStack = rewriter.create<sigi::GetGlobalStack>(
                    funcOp->getLoc(),
                    sigi::StackType::get(funcOp->getContext()));

                if (stackParameter.empty()) {
                    rewriter.replaceAllUsesWith(inStack, globalStack);
                } else {
                    auto lastParamPop = cast<sigi::PopOp>(allParamPops.back());
                    rewriter.replaceAllUsesWith(lastParamPop.getOutStack(), globalStack);
                }
            }

            // SmallVector<Type> newInTypes;
            // for (auto oldIn : funcOp.getFunctionType().getInputs()) {
            //     if (llvm::isa<sigi::StackType>(oldIn))
            //         for (auto newIn : inputs) newInTypes.push_back(newIn);
            //     else
            //         newInTypes.push_back(oldIn);
            // }

            // SmallVector<Type> newOutTypes;
            // SmallVector<Value> newOuts;
            // for (auto oldOut : returnOp->getOperands()) {
            //     if (llvm::isa<sigi::StackType>(oldOut.getType())) {
            //         for (auto def : outParamDefs) {
            //             auto push = cast<sigi::PushOp>(def);
            //             newOuts.push_back(push.getValue());
            //             newOutTypes.push_back(push.getValue().getType());
            //         }
            //     }
            // }
            LLVM_DEBUG(llvm::errs() << moduleOp);
            return llvm::failure();
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