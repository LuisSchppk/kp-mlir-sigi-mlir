/// Declaration of the conversion passes for the Closure dialect.
///
/// @file

#pragma once

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "sigi-mlir/Conversion/ClosureToLLVM/ClosureToLLVM.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureOps.h"

#include <mlir/Dialect/Func/IR/FuncOps.h>

namespace mlir {
namespace closure {
#define GEN_PASS_DECL
#define GEN_PASS_REGISTRATION
#include "sigi-mlir/Conversion/ClosurePasses.h.inc"
} // namespace closure
} // namespace mlir