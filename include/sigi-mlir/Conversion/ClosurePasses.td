//===- Passes.td - Closure dialect passes ---------------------*- tablegen -*-===//
//
// This is the definitions file for the Closure dialect conversion passes.
//
//===----------------------------------------------------------------------===//

#ifndef CLOSURE_CONVERSION_PASSES
#define CLOSURE_CONVERSION_PASSES

include "PassUtil.td"


def ConvertClosureToLLVM : ConvertToLlvm<"Closure", "closure">;

def PrepareClosureInline : Pass<"prep-closure-inline", "func::FuncOp"> {
  let summary = "Prepare Closure Inlining by making sure closure in if statements have the required pattern.";
}

def ClosureInline : Pass<"closure-inline", "mlir::ModuleOp"> {
  let summary = "Run Closure Inlining with the pipeline --canonicalize --inline --canonicalize --prep-closure-inline --canonicalize --inlines";
}

#endif // CLOSURE_CONVERSION_PASSES