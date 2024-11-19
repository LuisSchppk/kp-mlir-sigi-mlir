/// Declaration of the Sigi dialect ops.
///
/// @file

#pragma once

#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/IR/PatternMatch.h"



//===- Generated includes -------------------------------------------------===//

#define GET_OP_CLASSES
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h.inc"

//===----------------------------------------------------------------------===//
