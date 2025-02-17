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