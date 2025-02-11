//===- Ops.td - Sigi dialect ops ----------------------*- tablegen -*-===//
//
// This is the definitions file for the Sigi dialect ops.
//
//===----------------------------------------------------------------------===//

#ifndef SIGI_OPS
#define SIGI_OPS


include "sigi-mlir/Dialect/Sigi/IR/SigiBase.td"
include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.td"

include "mlir/IR/EnumAttr.td"
include "mlir/IR/OpAsmInterface.td"
include "mlir/IR/SymbolInterfaces.td"
include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Bytecode/BytecodeOpInterface.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/IR/BuiltinTypes.td"


// These ops are pure because they consume a stack and return a new one.
// The implementation is not pure though.

def PopOp : Sigi_Op<"pop"> {
    let summary = "";
    let description  = "";
    let arguments = (ins Sigi_StackType:$in_stack);

    // check that value has correct type in verifier ?
    let results = (outs Sigi_StackType:$out_stack, AnyType:$value);
    let assemblyFormat = "$in_stack attr-dict `:` type($value)";
    let hasVerifier=1;
    let hasCanonicalizeMethod = 1;
    // let hasCustomAssemblyFormat = 1;
}

def PushOp : Sigi_Op<"push"> {
    let summary = "";
    let description  = "";
    let arguments = (ins Sigi_StackType:$in_stack, AnyType:$value);

    // check that value has correct type in verifier ? 
    let results = (outs Sigi_StackType:$out_stack);
    let assemblyFormat = "$in_stack `,` $value attr-dict `:` type($value)";
    let hasVerifier=1;
    let hasCanonicalizeMethod = 1;
    // let hasCustomAssemblyFormat = 1; 
}

def LoadGlobalStackOp : Sigi_Op<"load.global", [Pure]> {
    let summary = "";
    let description  = "";

    // check that value has correct type in verifier ? 
    let results = (outs Sigi_StackType:$global_stack);
    let hasCanonicalizeMethod = 1;
    // let hasCanonicalizeMethod = 1;
    // let hasCustomAssemblyFormat = 1; 
}

def StoreGlobalStackOp : Sigi_Op<"store.global"> {
    let summary = "";
    let description  = "";
    let arguments = (ins Sigi_StackType:$in_stack);
    let hasCanonicalizeMethod = 1;
}

#endif
