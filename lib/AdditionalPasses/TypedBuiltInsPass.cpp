#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <cassert>
#include <llvm/ADT/MapVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/SymbolTable.h>
#include <mlir/IR/Types.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Rewrite/FrozenRewritePatternSet.h>
#include <mlir/Transforms/GreedyPatternRewriteDriver.h>

#define GEN_PASS_DEF_TYPEDBUILTINSPASS
#define GEN_PASS_DEF_TYPEDBUILTINSINNERPASS
namespace mlir::sigi {
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace mlir::sigi

namespace mlir::closure {}
#define DEBUG_TYPE "typed-builtins"
using namespace mlir;

namespace {

std::string stack_type_attr = "sigi.stackType";
std::string sigi_builtin = "sigi.builtinfunc";
std::string sigi_pp = "sigi::pp";
std::string sigi_pp_i32 = "sigi::pp_i32";
std::string sigi_pp_i1 = "sigi::pp_i1";
std::string sigi_pp_closure = "sigi::pp_closure";

struct TypedBuiltInsPass : public mlir::sigi::impl::TypedBuiltInsPassBase<TypedBuiltInsPass> {

    struct ReplacePP : OpRewritePattern<func::CallOp> {
        using OpRewritePattern<func::CallOp>::OpRewritePattern;

        LogicalResult getPrintFunction(
            func::CallOp callOp,
            Type printType,
            PatternRewriter &rewriter,
            ModuleOp moduleOp,
            func::FuncOp &printFunc) const
        {
            rewriter.setInsertionPointToStart(moduleOp.getBody());
            auto printFunctionType = FunctionType::get(moduleOp->getContext(), printType, {});
            if (printType == rewriter.getIntegerType(32)) {
                if (auto op = SymbolTable::lookupSymbolIn(moduleOp, sigi_pp_i32)) {
                    if (auto pp_i32 = llvm::dyn_cast<func::FuncOp>(op)) printFunc = pp_i32;
                } else {
                    printFunc = rewriter.create<func::FuncOp>(
                        callOp->getLoc(),
                        sigi_pp_i32,
                        printFunctionType);
                    rewriter.modifyOpInPlace(printFunc, [&]() {
                        printFunc->setAttr(sigi_builtin, rewriter.getUnitAttr());
                        printFunc.setVisibility(mlir::SymbolTable::Visibility::Private);
                    });
                }
            } else if (printType == rewriter.getIntegerType(1)) {
                if (auto op = SymbolTable::lookupSymbolIn(moduleOp, sigi_pp_i1)) {
                    if (auto pp_i1 = llvm::dyn_cast<func::FuncOp>(op)) printFunc = pp_i1;
                } else {
                    printFunc = rewriter.create<func::FuncOp>(
                        callOp->getLoc(),
                        sigi_pp_i1,
                        printFunctionType);
                    rewriter.modifyOpInPlace(printFunc, [&]() {
                        printFunc->setAttr(sigi_builtin, rewriter.getUnitAttr());
                        printFunc.setVisibility(mlir::SymbolTable::Visibility::Private);
                    });
                }
            } else if (llvm::isa<closure::BoxedClosureType>(printType)) {
                if (auto op = SymbolTable::lookupSymbolIn(moduleOp, sigi_pp_closure)) {
                    if (auto pp_closure = llvm::dyn_cast<func::FuncOp>(op)) printFunc = pp_closure;
                } else {
                    printFunc = rewriter.create<func::FuncOp>(
                        callOp->getLoc(),
                        sigi_pp_closure,
                        printFunctionType);
                    rewriter.modifyOpInPlace(printFunc, [&]() {
                        printFunc->setAttr(sigi_builtin, rewriter.getUnitAttr());
                        printFunc.setVisibility(mlir::SymbolTable::Visibility::Private);
                    });
                }
            } else {
                return llvm::failure();
            }
            return llvm::success();
        }

        LogicalResult matchAndRewrite(func::CallOp callOp, PatternRewriter &rewriter) const override
        {
            LLVM_DEBUG(
                llvm::errs()
                << "ENTERED REWRITE ON " << callOp << " FOR " << callOp.getCallee() << "\n");
            ModuleOp moduleOp = callOp->getParentOfType<ModuleOp>();
            Operation* callee = SymbolTable::lookupSymbolIn(moduleOp, callOp.getCalleeAttr());

            if (!callee) return llvm::failure();
            if (auto funcOp = llvm::dyn_cast<func::FuncOp>(callee)) {
                if (!funcOp.isExternal()) return llvm::failure();
            } else {
                return llvm::failure();
            }

            if (!llvm::is_contained(
                    callOp.getOperandTypes(),
                    sigi::StackType::get(callOp->getContext())))
                return llvm::failure();

            sigi::PushOp pushBefore;
            sigi::PopOp popAfter;

            if (!callOp->getOperand(0).getDefiningOp()) {
                LLVM_DEBUG(
                    llvm::errs() << "FOR: " << callOp << " " << callOp->getOperand(0)
                                 << "HAS NO DEFINING OP!\n");
                return llvm::failure();
            }

            if ((pushBefore =
                     llvm::dyn_cast<sigi::PushOp>(callOp->getOperand(0).getDefiningOp()))) {
                if (!pushBefore.getOutStack().hasOneUse()) return llvm::failure();
            } else {
                return llvm::failure();
            }

            if (callOp->getResult(0).hasOneUse()) {
                if ((popAfter = llvm::dyn_cast<sigi::PopOp>(
                         callOp->getResult(0).getUses().begin()->getOwner())))
                    ;
            } else if (!callOp->getResult(0).use_empty()) {
                return llvm::failure();
            }

            assert(pushBefore && "THERE HAS TO BE A PUSH BEFORE PP FOR THIS REWRITE PATTERN.");
            assert(
                popAfter
                || callOp->getResult(0).use_empty()
                       && "Result has either to be popped or never used.");

            auto stackTypeAttr = llvm::cast<TypeAttr>(callOp->getAttr(stack_type_attr));
            auto stackFuncType = cast<FunctionType>(stackTypeAttr.getValue());

            if (stackFuncType.getNumInputs() != 1) return llvm::failure();

            auto printType = stackFuncType.getResult(0);
            func::FuncOp printFunc;
            if (getPrintFunction(callOp, printType, rewriter, moduleOp, printFunc).failed())
                return llvm::failure();
            assert(printFunc && "printFunc not set -> Should have resulted in a failure earlier.");
            rewriter.setInsertionPointAfter(callOp);
            rewriter.create<func::CallOp>(callOp->getLoc(), printFunc, pushBefore.getValue());

            if (popAfter) {
                rewriter.replaceAllUsesWith(popAfter.getOutStack(), pushBefore.getInStack());
                rewriter.replaceAllUsesWith(popAfter.getValue(), pushBefore.getValue());
                rewriter.eraseOp(popAfter);
            }
            rewriter.eraseOp(callOp);
            rewriter.eraseOp(pushBefore);
            return llvm::success();
        }
    };

    void runOnOperation() override
    {
        RewritePatternSet patterns(&getContext());
        patterns.add<ReplacePP>(&getContext());
        GreedyRewriteConfig greedyConf;
        greedyConf.strictMode = GreedyRewriteStrictness::ExistingOps;
        SmallVector<Operation*> callOps;
        getOperation()->walk([&](func::CallOp callOp) {
            Operation* callee = SymbolTable::lookupSymbolIn(getOperation(), callOp.getCalleeAttr());
            if (!callee) return;
            if (!llvm::isa<func::FuncOp>(callee)) return;
            if (!callee->hasAttr(sigi_builtin)) return;
            if (callOp.getCallee() != "sigi::pp") return;
            if (callOp.getNumOperands() != 1) return;
            if (callOp.getNumResults() != 1) return;
            

            LLVM_DEBUG(llvm::errs() << "REGISTER " << callOp << "\n");
            callOps.push_back(callOp.getOperation());
        });

        FrozenRewritePatternSet frozen(std::move(patterns));
        for (auto printCall : callOps) (void)applyOpPatternsAndFold({printCall}, frozen, greedyConf);
        callOps.clear();
    }
};
} // namespace