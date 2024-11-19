//===- Types.td - Sigi dialect types ------------------*- tablegen -*-===//
//
// This is the definitions file for the Sigi dialect types.
//
//===----------------------------------------------------------------------===//

#ifndef SIGI_TYPES
#define SIGI_TYPES

include "mlir/IR/BuiltinTypeInterfaces.td"
include "SigiBase.td"

def Sigi_StackType : Sigi_Type<"Stack"> {
    let mnemonic="stack";
    let summary="Opaque type for representation of Sigi global stack.";
    let description = "";
}

#endif
