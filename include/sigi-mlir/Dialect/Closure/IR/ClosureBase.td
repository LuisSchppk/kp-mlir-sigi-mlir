//===- Base.td - Closure dialect base ------------------------*- tablegen -*-===//
//
// This is the definitions file for the Closure dialect base.
//
//===----------------------------------------------------------------------===//

#ifndef Closure_BASE
#define Closure_BASE

include "mlir/IR/BuiltinAttributes.td"
include "mlir/IR/BuiltinTypes.td"
include "mlir/IR/OpBase.td"

def Closure_Dialect : Dialect {
    let name = "closure";
    let cppNamespace = "::mlir::closure";

    let summary = "An implementation of closures.";
    let description = [{
        A minimal implementation of closures.
    }];

    let useDefaultTypePrinterParser = 1;

    code extraClassDeclaration = [{
    private:
        void registerOps();
        void registerTypes();
        void getCanonicalizationPatterns(RewritePatternSet &result) const override;
    }];
}



// Template for attributes.
// class Closure_Attr<string name, list<Trait> traits = []>
//         : AttrDef<Closure_Dialect, name, traits>;
// Template for ops.
class Closure_Op<string mnemonic, list<Trait> traits = []>
        : Op<Closure_Dialect, mnemonic, traits>;
// Template for types.
class Closure_Type<string name, list<Trait> traits = []>
        : TypeDef<Closure_Dialect, name, traits>;

#endif
