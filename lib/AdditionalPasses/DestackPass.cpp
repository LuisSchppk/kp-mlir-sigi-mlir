#include "sigi-mlir/Conversion/ClosurePasses.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Dialect/Bufferization/Transforms/OneShotAnalysis.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/OpDefinition.h>
#include <mlir/IR/Operation.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/Value.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Transforms/Passes.h>
#define GEN_PASS_DEF_DESTACKPASS
namespace mlir::sigi {
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace mlir::sigi

namespace mlir::closure {}

#define DEBUG_TYPE "tail-rec"
using namespace mlir;
namespace {

struct DestackFunc : public OpRewritePattern<func::FuncOp> {
    using OpRewritePattern<func::FuncOp>::OpRewritePattern;

    SmallVector<Value> extractArgs(
        func::FuncOp funcOp,
        PatternRewriter &rewriter,
        SmallVector<sigi::PopOp> pops,
        SmallVector<sigi::PushOp> pushes,
        SmallVector<func::CallOp> builtins,
        SmallVector<int> order) const
    {
        SmallVector<Value> args;
        SmallVector<Type> argTypes;
        SmallVector<Location> argLocs;

        SmallVector<Value> results;
        SmallVector<Type> resultTypes;
        bool pushed;
        for (size_t i = 0; i < order.size(); i++) {
            int type = order[i];
            switch (type) {
            case 0:
            {
                if (!pushed) {
                    args.push_back(pops[i].getValue());
                    argTypes.push_back(pops[i].getValue().getType());
                    argLocs.push_back(funcOp->getLoc()); // Should be improved for better debugging.
                } else {
                    // Further handling for pop after push
                }
                break;
            }
            case 1:
            {
                if (!pushed) {
                    pushed = true;
                } else {
                    results.push_back(pushes[i].getValue());
                    resultTypes.push_back(pushes[i].getValue().getType());
                }
                break;
            }
            case 2: pushed = true; break;
            }
        }

        rewriter.modifyOpInPlace(funcOp, [&]() {
            funcOp.setFunctionType(FunctionType::get(funcOp->getContext(), argTypes, resultTypes));
        });

        auto entry = &funcOp.getBlocks().front();
        int numOldArgs = entry->getNumArguments();
        
        
        entry->addArguments(argTypes, argLocs);

        funcOp->print(llvm::errs());
        for (auto [popped, blockarg] : llvm::zip_equal(
                 args,
                 funcOp.getBlocks().front().getArguments().take_back(
                     entry->getNumArguments() - numOldArgs))) {
            rewriter.replaceAllUsesWith(popped, blockarg);
        }

        bool removedOnce = false;
        funcOp->walk([&](func::ReturnOp returnOp) {
            if (removedOnce) return;

            rewriter.setInsertionPoint(returnOp);
            auto newReturn = rewriter.create<func::ReturnOp>(funcOp->getLoc(), results);
            rewriter.replaceOp(returnOp, newReturn);
            removedOnce = true;
        });

        // entry->eraseArguments(0, numOldArgs);
        funcOp->print(llvm::errs());

        return args;
    }

    LogicalResult matchAndRewrite(func::FuncOp funcOp, PatternRewriter &rewriter) const override
    {
        if(funcOp->hasAttr("DESTACKED")) return llvm::failure();
        SmallVector<sigi::PopOp> pops;
        SmallVector<sigi::PushOp> pushes;
        SmallVector<func::CallOp> builtins;
        SmallVector<int> order;
        const int POP = 0;
        const int PUSH = 1;
        const int BUILTIN = 2;

        bool nonSigiAssignment = false;
        funcOp->walk([&](Operation* op) {
            if (auto callOp = llvm::dyn_cast<func::CallOp>(op)) {
                auto calleeAttr = callOp.getCalleeAttr();
                auto callee = SymbolTable::lookupNearestSymbolFrom(callOp, calleeAttr);
                if (callee->hasAttr("sigi.builtinfunc")) {
                    builtins.push_back(callOp);
                    order.push_back(BUILTIN);
                    return;
                }
            }

            if (auto pop = llvm::dyn_cast<sigi::PopOp>(op)) {
                pops.push_back(pop);
                order.push_back(POP);
            } else if (auto push = llvm::dyn_cast<sigi::PushOp>(op)) {
                pushes.push_back(push);
                order.push_back(PUSH);
            } else if (
                !op->hasTrait<OpTrait::ReturnLike>() && !op->hasTrait<OpTrait::IsTerminator>()
                && !llvm::isa<sigi::PopOp>(op) && !llvm::isa<sigi::PushOp>(op)
                && !nonSigiAssignment) {
                auto resultsTypes = op->getResultTypes();

                // at this point all closures are assumed to be inlined?
                if (llvm::is_contained(resultsTypes, sigi::StackType::get(funcOp->getContext()))
                    && isa<func::CallOp>(op)) {
                    auto callOp = llvm::cast<func::CallOp>(op);
                    auto calleeAttr = callOp.getCalleeAttr();
                    auto callee = SymbolTable::lookupNearestSymbolFrom(callOp, calleeAttr);

                    // only allow non-sigi stack assignment by recursive calls.
                    nonSigiAssignment = callee != funcOp;
                } else if (llvm::is_contained(
                               resultsTypes,
                               sigi::StackType::get(funcOp->getContext()))) {
                    nonSigiAssignment = true;
                }
            }
        });

        if (nonSigiAssignment) return llvm::failure();
        extractArgs(funcOp, rewriter, pops, pushes, builtins, order);

        rewriter.modifyOpInPlace(funcOp, [&](){
            funcOp->setAttr("DESTACKED", rewriter.getUnitAttr());
        });
        return llvm::success();
    }
};

struct DestackPass : public mlir::sigi::impl::DestackPassBase<DestackPass> {

    void runOnOperation() override
    {
        ModuleOp moduleOp = getOperation()->getParentOfType<ModuleOp>();
        mlir::OpPassManager preparationPM(moduleOp.getOperationName());

        RewritePatternSet patterns(&getContext());
        patterns.add<DestackFunc>(&getContext());
        (void)applyOpPatternsAndFold({getOperation()}, std::move(patterns));
    }
};
} // namespace