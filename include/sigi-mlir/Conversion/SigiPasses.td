//===- Passes.td - Sigi dialect passes ---------------------*- tablegen -*-===//
//
// This is the definitions file for the Sigi dialect conversion passes.
//
//===----------------------------------------------------------------------===//

#ifndef SIGI_CONVERSION_PASSES
#define SIGI_CONVERSION_PASSES

include "PassUtil.td"

def ConvertSigiToLLVM : ConvertToLlvm<"Sigi", "sigi">;


def TailRecursionPass : Pass<"detect-tail-rec", "func::FuncOp"> {
  let summary = "Detect tail recursions";
   let dependentDialects = ["mlir::memref::MemRefDialect"];
//   let constructor = "mlir::sigi::createTailRecursionPass()";
}

def DestackPass : Pass<"destack", "func::FuncOp"> {
  let summary = "Tries to remove stack as far as possible.";

  // let constructor = "mlir::sigi::createDestackPass()";
}

#endif // SIGI_CONVERSION_PASSES