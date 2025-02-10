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
  let dependentDialects = ["mlir::scf::SCFDialect"];
}

def ClosureInline : Pass<"closure-inline", "mlir::ModuleOp"> {
  let summary = "Run Closure Inlining with the pipeline --canonicalize --inline --canonicalize --prep-closure-inline --canonicalize --inlines";
  let dependentDialects = ["mlir::scf::SCFDialect"];
  // let options = [
  //   Option<"maxIterationsNestedInline", "max-iterations", "int",
  //     "10",
  //     "Maximum number of fixed-point iterations for repeated inlining of nested boxOps.">
  // ];
}

#endif // CLOSURE_CONVERSION_PASSES