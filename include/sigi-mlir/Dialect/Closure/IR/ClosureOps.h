/// Declaration of the Closure dialect ops.
///
/// @file

#pragma once

#include "sigi-mlir/Dialect/Closure/IR/ClosureBase.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"



//===- Generated includes -------------------------------------------------===//

#define GET_OP_CLASSES
#include "sigi-mlir/Dialect/Closure/IR/ClosureOps.h.inc"

//===----------------------------------------------------------------------===//