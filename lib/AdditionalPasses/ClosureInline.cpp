#include "sigi-mlir/Dialect/Closure/IR/ClosureBase.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureOps.h"

#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Transforms/GreedyPatternRewriteDriver.h>
#include <mlir/Transforms/Passes.h>
#include <utility>

#define GEN_PASS_DEF_CLOSUREINLINE
#define GEN_PASS_DEF_PREPARECLOSUREINLINE
namespace mlir::closure {
#include "sigi-mlir/Conversion/ClosurePasses.h.inc"
} // namespace mlir::closure

#define DEBUG_TYPE "closure-inline"

using namespace mlir;
struct CalledBoxOpSelectToIf : OpRewritePattern<closure::CallOp> {
public:
    using OpRewritePattern<closure::CallOp>::OpRewritePattern;

    LogicalResult matchAndRewrite(closure::CallOp op, PatternRewriter &rewriter)
        const override
    {
        auto definingOp = op.getCallee().getDefiningOp();
        if (definingOp == nullptr) {
            LLVM_DEBUG(llvm::errs() << "No defining op found.\n");
        } else if (
            // Parse SelectOp & Deconstruct.
            auto selectOp = llvm::dyn_cast<arith::SelectOp>(definingOp)) {
            auto condition = selectOp.getCondition();

            // Construct skeleton for IfOp.
            auto thenOp = selectOp.getTrueValue().getDefiningOp();
            auto elseOp = selectOp.getFalseValue().getDefiningOp();
            auto ifOp = rewriter.create<scf::IfOp>(
                selectOp->getLoc(),
                op.getResultTypes(),
                condition,
                true,
                true);
            auto &thenBlock = ifOp.getThenRegion();
            auto &elseBlock = ifOp.getElseRegion();

            // Fill IfBlock.
            rewriter.setInsertionPointToStart(&thenBlock.front());
            // auto clonedThenop = rewriter.clone(*thenOp);
            auto castThenOp = cast<closure::BoxOp>(thenOp);
            auto thenCall = rewriter.create<closure::CallOp>(
                op->getLoc(),
                op.getResultTypes(),
                castThenOp,
                op.getCalleeOperands());
            rewriter.create<scf::YieldOp>(op->getLoc(), thenCall->getResults());

            // Fill ElseBlock.
            rewriter.setInsertionPointToStart(&elseBlock.front());
            // auto clonedElseOp = rewriter.clone(*elseOp);
            auto castElseOp = cast<closure::BoxOp>(elseOp);
            auto elseCall = rewriter.create<closure::CallOp>(
                op->getLoc(),
                op.getResultTypes(),
                castElseOp,
                op.getCalleeOperands());
            rewriter.create<scf::YieldOp>(op->getLoc(), elseCall->getResults());

            // Replace selectOp and callOp with if op.
            rewriter.replaceOp(selectOp, ifOp);
            rewriter.replaceOp(op, ifOp);

            return llvm::success();
        } else {
            auto parentOp = definingOp->getParentOfType<func::FuncOp>();
            LLVM_DEBUG(
                llvm::errs() << "Cast to select failed on "
                             << definingOp->getName()
                             // << " in " << parentOp
                             << "\n");
        }

        return llvm::failure();
    }
};

namespace {
struct PrepareClosureInline
        : public mlir::closure::impl::PrepareClosureInlineBase<
              PrepareClosureInline> {

    void runOnOperation() override
    {
        LLVM_DEBUG(llvm::errs() << "Entered Prep Closure Inline\n");
        RewritePatternSet patterns(&getContext());
        patterns.add<CalledBoxOpSelectToIf>(&getContext());
        (void)applyPatternsAndFoldGreedily(getOperation(), std::move(patterns));
    }
};

struct ClosureInline : public closure::impl::ClosureInlineBase<ClosureInline> {
    void runOnOperation() override
    {
        ModuleOp moduleOp = getOperation();
        mlir::OpPassManager closureInlinePM(moduleOp.getOperationName());
        closureInlinePM.addPass(mlir::createInlinerPass());
        closureInlinePM.addPass(mlir::createCanonicalizerPass());
        closureInlinePM.addNestedPass<func::FuncOp>(
            closure::createPrepareClosureInline());
        closureInlinePM.addPass(mlir::createCanonicalizerPass());
        closureInlinePM.addPass(mlir::createInlinerPass());
        if (mlir::failed(runPipeline(closureInlinePM, moduleOp))) {
            moduleOp.emitError("Failed to run ClosureInline Pipeline.");
            signalPassFailure();
        }
    }
};
} // namespace