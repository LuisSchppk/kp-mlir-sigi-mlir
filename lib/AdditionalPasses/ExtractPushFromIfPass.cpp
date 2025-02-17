#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <llvm-c/ExternC.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/Visitors.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Rewrite/FrozenRewritePatternSet.h>
#include <mlir/Support/LLVM.h>
#include <mlir/Transforms/GreedyPatternRewriteDriver.h>
#include <mlir/Transforms/Passes.h>
#include <utility>

#define GEN_PASS_DEF_EXTRACTPUSHFROMIFPASS
namespace mlir::sigi {
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace mlir::sigi

#define DEBUG_TYPE "extract-push"
using namespace mlir;

namespace {

Operation* buildNewIf(::mlir::PatternRewriter &rewriter, scf::IfOp oldIfOp, Location loc)
{
    SmallVector<Type> newResultsTypes{oldIfOp.thenYield()->getOperandTypes()};
    scf::IfOp newIfOp =
        rewriter.create<scf::IfOp>(loc, newResultsTypes, oldIfOp.getCondition(), true, true);

    rewriter.moveBlockBefore(oldIfOp.thenBlock(), newIfOp.thenBlock());
    rewriter.moveBlockBefore(oldIfOp.elseBlock(), newIfOp.elseBlock());
    rewriter.eraseBlock(newIfOp.thenBlock());
    rewriter.eraseBlock(newIfOp.elseBlock());
    return newIfOp.getOperation();
}

struct ExtractPush : OpRewritePattern<sigi::PushOp> {
    using OpRewritePattern<sigi::PushOp>::OpRewritePattern;

    LogicalResult matchAndRewrite(sigi::PushOp op, ::mlir::PatternRewriter &rewriter) const override
    {
        if (op.getOutStack().hasOneUse()
            && llvm::isa<scf::YieldOp>(op.getOutStack().getUses().begin()->getOwner())
            && llvm::isa<scf::IfOp>(op->getParentOp())) {
            auto ifOp = cast<scf::IfOp>(op->getParentOp());
            if (ifOp->getNumRegions() != 2) return llvm::failure();

            auto firstYield = cast<scf::YieldOp>(op.getOutStack().getUses().begin()->getOwner());
            auto otherYield = firstYield == ifOp.thenYield() ? ifOp.elseYield() : ifOp.thenYield();
            Value otherOutStack;
            int stackCount = 0;
            for (auto operand : otherYield.getOperands()) {
                if (llvm::isa<sigi::StackType>(operand.getType())) {
                    otherOutStack = operand;
                    ++stackCount;
                }
            }
            if (stackCount != 1) return llvm::failure();

            if (otherOutStack.hasOneUse()
                && llvm::isa<sigi::PushOp>(otherOutStack.getDefiningOp())) {
                auto otherPush = cast<sigi::PushOp>(otherOutStack.getDefiningOp());
                if (op.getValue().getType() != otherPush.getValue().getType())
                    return llvm::failure();
                SmallVector<Value> newOperandsFirst{op.getValue()};
                SmallVector<Value> newOperandsOther{otherPush.getValue()};

                for (auto [first, other] :
                     llvm::zip_equal(firstYield->getOperands(), otherYield->getOperands())) {
                    newOperandsFirst.push_back(first);
                    newOperandsOther.push_back(other);
                }

                llvm::transform(newOperandsFirst, newOperandsFirst.begin(), [&](Value v) {
                    return (v == op.getOutStack()) ? op.getInStack() : v;
                });

                llvm::transform(newOperandsOther, newOperandsOther.begin(), [&](Value v) {
                    return (v == otherPush.getOutStack()) ? otherPush.getInStack() : v;
                });

                rewriter.modifyOpInPlace(firstYield, [&]() {
                    firstYield->setOperands(newOperandsFirst);
                });

                rewriter.modifyOpInPlace(otherYield, [&]() {
                    otherYield->setOperands(newOperandsOther);
                });

                assert(
                    firstYield->getOperandTypes() == otherYield->getOperandTypes()
                    && "\n Operand Types do not match!\n");

                rewriter.setInsertionPointAfter(ifOp);
                scf::IfOp newIfOp = cast<scf::IfOp>(buildNewIf(rewriter, ifOp, op->getLoc()));

                Value newStack;
                Value newPushValue = newIfOp->getResults().front();
                int stackCount = 0;
                for (auto result : newIfOp->getResults()) {
                    if (llvm::isa<sigi::StackType>(result.getType())) {
                        newStack = result;
                        ++stackCount;
                    }
                }

                assert(stackCount == 1 && "\n STACK COUNT NOT ONE\n");

                rewriter.setInsertionPointAfter(newIfOp);
                auto extractedPush = rewriter.create<sigi::PushOp>(
                    op->getLoc(),
                    sigi::StackType::get(op->getContext()),
                    newStack,
                    newPushValue);

                Value oldOutStack;
                for (auto [oldResult, newResult] :
                     llvm::zip_first(ifOp->getResults(), newIfOp->getResults().drop_front())) {
                    assert(oldResult.getType() == newResult.getType() && "\n TYPE MISMATCH \n");
                    if (!llvm::isa<sigi::StackType>(oldResult.getType()))
                        rewriter.replaceAllUsesWith(oldResult, newResult);
                    else
                        oldOutStack = oldResult;
                }
                assert(oldOutStack && "\n IfOp has to return a stack to get this far!.\n");

                rewriter.replaceAllUsesWith(oldOutStack, extractedPush.getOutStack());
                assert(ifOp->use_empty() && "\n OLD IF OP STILL HAS USES!\n");
                rewriter.eraseOp(op);
                rewriter.eraseOp(otherPush);
                rewriter.eraseOp(ifOp);
                LLVM_DEBUG(llvm::errs() << newIfOp->getParentOfType<func::FuncOp>() << "\n");
                return llvm::success();
            }
        }
        return llvm::failure();
    }
};

struct ExtractPushFromIfPass
        : public mlir::sigi::impl::ExtractPushFromIfPassBase<ExtractPushFromIfPass> {
    void runOnOperation() override
    {
        func::FuncOp funcOp = getOperation();
        RewritePatternSet pattern(&getContext());
        pattern.add<ExtractPush>(&getContext());
        FrozenRewritePatternSet frozen(std::move(pattern));
        SetVector<Operation*> pushOps;
        LLVM_DEBUG(llvm::errs() << "BEGIN\n");
        funcOp->walk([&](sigi::PushOp op) {
            if (op.getOutStack().hasOneUse()
                && llvm::isa<scf::YieldOp>(op.getOutStack().getUses().begin()->getOwner())
                && llvm::isa<scf::IfOp>(op->getParentOp())) {
                auto ifOp = cast<scf::IfOp>(op->getParentOp());

                if (ifOp->getNumRegions() != 2) return;
                auto firstYield =
                    cast<scf::YieldOp>(op.getOutStack().getUses().begin()->getOwner());
                auto otherYield =
                    firstYield == ifOp.thenYield() ? ifOp.elseYield() : ifOp.thenYield();
                Value otherOutStack;
                int stackCount = 0;
                for (auto operand : otherYield.getOperands()) {
                    if (llvm::isa<sigi::StackType>(operand.getType())) {
                        otherOutStack = operand;
                        ++stackCount;
                    }
                }
                if (stackCount != 1) return;

                if (otherOutStack.hasOneUse()
                    && llvm::isa<sigi::PushOp>(otherOutStack.getDefiningOp())) {
                    auto otherPush = cast<sigi::PushOp>(otherOutStack.getDefiningOp());
                    if (op.getValue().getType() != otherPush.getValue().getType()) {
                        return;
                    }
                    pushOps.insert(op.getOperation());
                    return;
                }
                return;
            }
        });

        if (pushOps.empty()) return;
        (void)applyOpPatternsAndFold(pushOps.getArrayRef(), frozen).succeeded();
    }
};

} // namespace