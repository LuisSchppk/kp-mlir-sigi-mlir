#include <mlir/Pass/Pass.h>
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"
#include <mlir/Dialect/Func/IR/FuncOps.h>

#define GEN_PASS_DEF_TYPEDBUILTINSPASS
namespace mlir::sigi {
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace mlir::sigi

namespace mlir::closure {}
#define DEBUG_TYPE "typed-builtins"
using namespace mlir;

namespace {

struct TypedBuiltInsPass : public mlir::sigi::impl::TypedBuiltInsPassBase<TypedBuiltInsPass> {

    struct ReplacePP : OpRewritePattern<func::CallOp> {
    using OpRewritePattern<func::CallOp>::OpRewritePattern;

    };

    void runOnOperation() override
    {
        
    }
};
} // namespace