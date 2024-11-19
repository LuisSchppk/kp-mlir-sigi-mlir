//===- Passes.td - Sigi dialect passes ---------------------*- tablegen -*-===//
//
// This is the definitions file for the Sigi dialect conversion passes.
//
//===----------------------------------------------------------------------===//

#ifndef SIGI_CONVERSION_PASSES
#define SIGI_CONVERSION_PASSES

include "PassUtil.td"

def ConvertSigiToLLVM : ConvertToLlvm<"Sigi", "sigi">;

#endif // SIGI_CONVERSION_PASSES