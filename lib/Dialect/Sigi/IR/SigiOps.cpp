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
#include <iostream>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/IR/Diagnostics.h>
#include <mlir/IR/Location.h>
#include <mlir/IR/Value.h>

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

LogicalResult PopOp::verify()
{
    return verifySigiOperandType(PopOp::getValue());
}

LogicalResult PushOp::verify()
{
    return verifySigiOperandType(PushOp::getValue());
}

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
        rewriter.replaceOp(op, {originalStack, value});
        return llvm::success();
    }

    return llvm::failure();
}

// LogicalResult PushOp::canonicalize(PushOp op, ::mlir::PatternRewriter &rewriter)
// {

//     return llvm::success();
// }