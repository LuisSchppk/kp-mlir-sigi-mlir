/// Declaration of the conversion pass within Sigi dialect.
///
/// @file

#pragma once

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "sigi-mlir/Conversion/SigiToLLVM/SigiToLLVM.h"

#include <memory>
#include <mlir/Dialect/Func/IR/FuncOps.h>

namespace mlir {
namespace sigi {
#define GEN_PASS_DECL
#define GEN_PASS_REGISTRATION
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace sigi
} // namespace mlir