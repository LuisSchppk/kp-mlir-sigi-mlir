#include "sigi-mlir/Conversion/ClosurePasses.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <cassert>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Dialect/Bufferization/Transforms/OneShotAnalysis.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/LLVMIR/LLVMAttrs.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/Dialect/MemRef/IR/MemRef.h>
#include <mlir/IR/Attributes.h>
#include <mlir/IR/Block.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/OpDefinition.h>
#include <mlir/IR/Operation.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/SymbolTable.h>
#include <mlir/IR/Value.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Transforms/Passes.h>
#include <string>

#define GEN_PASS_DEF_GLOBALSTACKPASS
namespace mlir::sigi {
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace mlir::sigi

namespace mlir::closure {}

#define DEBUG_TYPE "global-stack"
using namespace mlir;
namespace {

std::string has_global_stack = "has-global-stack";

struct InsertGlobalStack : OpRewritePattern<func::FuncOp> {
    using OpRewritePattern<func::FuncOp>::OpRewritePattern;

    LogicalResult matchAndRewrite(func::FuncOp funcOp, PatternRewriter &rewriter) const override
    {
        if (funcOp->hasAttr(has_global_stack)
            || !llvm::is_contained(
                funcOp.getFunctionType().getInputs(),
                sigi::StackType::get(funcOp->getContext()))
            || funcOp.getBlocks().empty())
            return llvm::failure();

        sigi::StackType stackType = sigi::StackType::get(funcOp->getContext());
        Block* entryBlock = &funcOp.getBlocks().front();

        BlockArgument argStack;
        for (auto blockArg : entryBlock->getArguments())
            if (argStack && llvm::isa<sigi::StackType>(blockArg.getType()))
                return llvm::failure();
            else if (llvm::isa<sigi::StackType>(blockArg.getType()))
                argStack = blockArg;

        rewriter.setInsertionPointToStart(&funcOp.getBlocks().front());
        auto globalStack = rewriter.create<sigi::GetGlobalStack>(funcOp->getLoc(), stackType);
        rewriter.replaceAllUsesWith(argStack, globalStack);

        rewriter.modifyOpInPlace(funcOp, [&]() {
            funcOp->setAttr(has_global_stack, rewriter.getUnitAttr());
        });

        // ACTUALLY IN THIS CASE FAILURE IS FORBIDDEN -> ALREADY
        // MODIFIED IR. SOMEHOW ROLLBACK? SIGNAL_PASS_FAILURE
        if (funcOp->getParentOfType<ModuleOp>().verify().failed()) return llvm::failure();
        LLVM_DEBUG(llvm::errs() << "REWRITE DONE\n");
        return llvm::success();
    }
};

struct GlobalStackPass : public mlir::sigi::impl::GlobalStackPassBase<GlobalStackPass> {

    void runOnOperation() override
    {
        RewritePatternSet patterns(&getContext());
        GreedyRewriteConfig greedyConf;
        greedyConf.strictMode = GreedyRewriteStrictness::ExistingOps;
        patterns.add<InsertGlobalStack>(&getContext());
        (void)applyOpPatternsAndFold({getOperation()}, std::move(patterns), greedyConf);
    }
};
} // namespace