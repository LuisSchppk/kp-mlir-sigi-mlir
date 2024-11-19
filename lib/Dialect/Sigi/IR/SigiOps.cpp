/// Implements the Sigi dialect ops.
///
/// @file

#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureBase.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Transforms/DialectConversion.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include "llvm/ADT/APFloat.h"
#include <asm-generic/errno.h>
#include <iostream>
#include <llvm/Support/Casting.h>
#include <llvm/Support/LogicalResult.h>
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

LogicalResult verifySigiOperandType(Value element) {
    bool valid = false;
    if(element.getType().isInteger(32) || element.getType().isInteger(1)) {
        valid = true;
    } else if (isa<closure::BoxedClosureType>(element.getType())) {
        valid = true;
    }
    return valid ? LogicalResult::success() : LogicalResult::failure();
}


LogicalResult PopOp::verify() {
    return verifySigiOperandType(PopOp::getElement());
}

LogicalResult PushOp::verify() {
    return verifySigiOperandType(PushOp::getElement());
}