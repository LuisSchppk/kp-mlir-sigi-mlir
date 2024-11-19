//===- Base.td - Sigi dialect base ------------------------*- tablegen -*-===//
//
// This is the definitions file for the Sigi dialect base.
//
//===----------------------------------------------------------------------===//

#ifndef SIGI_BASE
#define SIGI_BASE

include "mlir/IR/BuiltinAttributes.td"
include "mlir/IR/BuiltinTypes.td"
include "mlir/IR/OpBase.td"

def Sigi_Dialect : Dialect {
    let name = "sigi";
    let cppNamespace = "::mlir::sigi";

    let summary = "TODO";
    let description = [{
        TODO
    }];

    let useDefaultTypePrinterParser = 1;


    code extraClassDeclaration = [{
    private:
        void registerOps();
        void registerTypes();
    }];
}

// Template for attributes.
class Sigi_Attr<string name, list<Trait> traits = []>
         : AttrDef<Sigi_Dialect, name, traits>;
// Template for ops.
class Sigi_Op<string mnemonic, list<Trait> traits = []>
        : Op<Sigi_Dialect, mnemonic, traits>;
// Template for types.
class Sigi_Type<string name, list<Trait> traits = []>
        : TypeDef<Sigi_Dialect, name, traits>;

#endif
