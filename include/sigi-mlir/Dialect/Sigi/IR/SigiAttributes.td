//===- Attributes.td - Sigi dialect attributes --------*- tablegen -*-===//
//
// This is the definitions file for the Sigi dialect attributes.
//
//===----------------------------------------------------------------------===//

#ifndef SIGI_ATTRIBUTES
#define SIGI_ATTRIBUTES

include "sigi-mlir/Dialect/Sigi/IR/SigiBase.td"

def Sigi_Main : Sigi_Attr<"main"> {
    let name = "main";
    let summary = "";
    let description = "";
}

def Sigi_Builtin : Sigi_Attr<"builtinfunc"> {
    let name = "builtinfunc";
    let summary ="";
    let description = "";
}

#endif
