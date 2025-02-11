/// Implements the Sigi dialect ops.
///
/// @file

#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Transforms/DialectConversion.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureBase.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include "llvm/ADT/APFloat.h"

#include <asm-generic/errno.h>
#include <cassert>
#include <iostream>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/IR/Block.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/Diagnostics.h>
#include <mlir/IR/Location.h>
#include <mlir/IR/Value.h>
#include <mlir/Interfaces/ControlFlowInterfaces.h>
#include <mlir/Support/LLVM.h>

#define DEBUG_TYPE "sigi-ops"

using namespace mlir;
using namespace mlir::sigi;
//===- Generated implementation -------------------------------------------===//

#define GET_OP_CLASSES
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.cpp.inc"

//===----------------------------------------------------------------------===//
// SigiDialect
//===----------------------------------------------------------------------===//

using namespace mlir;
using namespace mlir::sigi;

void SigiDialect::registerOps()
{
    addOperations<
#define GET_OP_LIST
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.cpp.inc"
        >();
}

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

bool containsReachablePop(Value startStack, Block* block)
{
    auto* next = startStack.getDefiningOp();
    bool popReachable = false;
    while (next && next->getBlock() == block)
        if (auto pop = llvm::dyn_cast<PopOp>(next))
            return true;
        else if (auto push = llvm::dyn_cast<PushOp>(next))
            next = push.getInStack().getDefiningOp();
        else
            return false;
    return popReachable;
}

LogicalResult verifySigiOperandType(Value element)
{
    bool valid = false;
    if (element.getType().isInteger(32) || element.getType().isInteger(1))
        valid = true;
    else if (isa<closure::BoxedClosureType>(element.getType()))
        valid = true;
    return valid ? LogicalResult::success() : LogicalResult::failure();
}

LogicalResult PopOp::verify() { return verifySigiOperandType(PopOp::getValue()); }

LogicalResult PushOp::verify() { return verifySigiOperandType(PushOp::getValue()); }

LogicalResult PopOp::canonicalize(PopOp op, ::mlir::PatternRewriter &rewriter)
{
    LLVM_DEBUG(llvm::errs() << "Entered Pop canonicalization. \n");
    auto in = op.getInStack();
    auto definingOp = in.getDefiningOp();
    if (definingOp == nullptr) {
        LLVM_DEBUG(llvm::errs() << "No defining push found. \n");
    } else if (auto definingPush = llvm::dyn_cast<PushOp>(definingOp)) {
        LLVM_DEBUG(llvm::errs() << "Found defining push. \n");
        auto value = definingPush.getValue();
        auto originalStack = definingPush.getInStack();

        // only replace pop -> if push is dead it will get folded by mlir
        LLVM_DEBUG(llvm::errs() << "\n REPLACING " << op << " WITH " << definingPush << "\n");
        rewriter.replaceOp(op, {originalStack, value});

        bool allUsesAreStores = true;
        LLVM_DEBUG(llvm::errs() << "CHECK ALL USES ARE STORE\n");
        for (auto &use : definingPush.getOutStack().getUses())
            allUsesAreStores &= llvm::isa<StoreGlobalStackOp>(use.getOwner());
        if (allUsesAreStores || definingPush.getOutStack().use_empty())
            rewriter.replaceOp(definingPush, definingPush.getInStack());

        return llvm::success();
    }
    return llvm::failure();
}

LogicalResult PushOp::canonicalize(PushOp op, ::mlir::PatternRewriter &rewriter)
{
    if (op.getOutStack().hasOneUse()
        && llvm::isa<scf::YieldOp>(op.getOutStack().getUses().begin()->getOwner())
        && llvm::isa<scf::IfOp>(op->getParentOp())
        && !containsReachablePop(op.getInStack(), op->getBlock())) {
        auto ifOp = cast<scf::IfOp>(op->getParentOp());

        LLVM_DEBUG(llvm::errs() << "BEGIN ON \n" << ifOp << "\n");
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
            if (stackCount != 1) return llvm::failure();
        }

        if (otherOutStack.hasOneUse() && llvm::isa<sigi::PushOp>(otherOutStack.getDefiningOp())
            && (otherOutStack.getDefiningOp()->getParentOp() != ifOp
                || !containsReachablePop(
                    otherOutStack,
                    otherOutStack.getDefiningOp()->getBlock()))) {
            auto otherPush = cast<PushOp>(otherOutStack.getDefiningOp());
            SmallVector<Value> newOperandsFirst = firstYield->getOperands();
            SmallVector<Value> newOperandsOther = otherYield->getOperands();

            newOperandsFirst.push_back(op.getValue());
            llvm::transform(newOperandsFirst, newOperandsFirst.begin(), [&](Value v) {
                return (v == op.getOutStack()) ? op.getInStack() : v;
            });

            newOperandsOther.push_back(otherPush.getValue());
            llvm::transform(newOperandsOther, newOperandsOther.begin(), [&](Value v) {
                return (v == otherPush.getOutStack()) ? otherPush.getInStack() : v;
            });

            rewriter.modifyOpInPlace(firstYield, [&]() {
                firstYield->setOperands(newOperandsFirst);
            });

            rewriter.modifyOpInPlace(otherYield, [&]() {
                otherYield->setOperands(newOperandsOther);
            });

            LLVM_DEBUG(llvm::errs() << "INTERMEDIARY 1. \n" << ifOp << "\n");

            assert(
                firstYield->getOperandTypes() == otherYield->getOperandTypes()
                && "\n Operand Types do not match!\n");

            rewriter.setInsertionPointAfter(ifOp);
            scf::IfOp newIfOp = cast<scf::IfOp>(buildNewIf(rewriter, ifOp, op->getLoc()));

            LLVM_DEBUG(llvm::errs() << newIfOp << "\n");

            Value newStack;
            Value newPushValue = newIfOp->getResults().back();
            int stackCount = 0;
            for (auto result : newIfOp->getResults()) {
                if (llvm::isa<sigi::StackType>(result.getType())) {
                    newStack = result;
                    ++stackCount;
                }
            }

            assert(stackCount == 1 && "\n STACK COUNT NOT ONE\n");

            rewriter.setInsertionPointAfter(newIfOp);
            auto extractedPush = rewriter.create<PushOp>(
                op->getLoc(),
                sigi::StackType::get(op->getContext()),
                newStack,
                newPushValue);

            Value oldOutStack;
            for (auto [oldResult, newResult] :
                 llvm::zip_first(ifOp->getResults(), newIfOp->getResults())) {
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
            return llvm::success();
        }
    }
    return llvm::failure();
}

LogicalResult
LoadGlobalStackOp::canonicalize(LoadGlobalStackOp op, ::mlir::PatternRewriter &rewriter)
{
    return llvm::failure();
}

LogicalResult
StoreGlobalStackOp::canonicalize(StoreGlobalStackOp op, ::mlir::PatternRewriter &rewriter)
{
    bool allUsesAreStores = true;

    for (auto &use : op.getInStack().getUses())
        allUsesAreStores &= llvm::isa<StoreGlobalStackOp>(use.getOwner());

    if (allUsesAreStores && llvm::isa<LoadGlobalStackOp>(op.getInStack().getDefiningOp())) {
        rewriter.eraseOp(op);
        return llvm::success();
    } else if (allUsesAreStores && llvm::isa<scf::IfOp>(op.getInStack().getDefiningOp())) {

        // NOT SURE IF THIS IS A CANONICALIZATION PATTERN -> NON MONOTON.
        auto ifOp = cast<scf::IfOp>(op.getInStack().getDefiningOp());
        auto thenYield = ifOp.thenYield();
        auto elseYield = ifOp.elseYield();
        IRMapping ifToThen;
        IRMapping ifToElse;

        ifToThen.map(ifOp->getResults(), thenYield->getOperands());
        ifToElse.map(ifOp->getResults(), elseYield->getOperands());

        rewriter.setInsertionPoint(thenYield);
        rewriter.clone(*op.getOperation(), ifToThen);

        rewriter.setInsertionPoint(elseYield);
        rewriter.clone(*op.getOperation(), ifToElse);

        rewriter.eraseOp(op);
        return llvm::success();
    }
    return llvm::failure();
}