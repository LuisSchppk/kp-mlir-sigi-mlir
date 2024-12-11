/// Implements the Closure dialect base.
///
/// @file

#include "sigi-mlir/Dialect/Closure/IR/ClosureBase.h"

#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/InliningUtils.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureDialect.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureOps.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"

#include <cerrno>
#include <cstdlib>
#include <iterator>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Arith/Utils/Utils.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/IR/Block.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/IRMapping.h>
#include <mlir/IR/Location.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/Operation.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/TypeRange.h>
#include <mlir/IR/Value.h>
#include <mlir/IR/ValueRange.h>
#include <mlir/Interfaces/DataLayoutInterfaces.h>
#include <string>

#define DEBUG_TYPE "closure-base"

using namespace mlir;
using namespace mlir::closure;

//===- Generated implementation -------------------------------------------===//

#include "sigi-mlir/Dialect/Closure/IR/ClosureBase.cpp.inc"

//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// ClosureDialect
//===----------------------------------------------------------------------===//

namespace closure {

struct CanonicalizeIf : OpRewritePattern<scf::IfOp> {
public:
    using OpRewritePattern<scf::IfOp>::OpRewritePattern;

    bool inlineIfNestedClosure(
        CallOp callOp,
        PatternRewriter &rewriter,
        Region &region) const
    {
        auto callee = callOp.getCallee().getDefiningOp();
        if (callee == nullptr) {
            LLVM_DEBUG(llvm::errs() << "No defining op found.\n");
        } else if (auto boxop = llvm::dyn_cast<BoxOp>(callee)) {
            Region &calleeRegion = boxop.getBody();
            Block &calleeBlock = calleeRegion.front();
            IRMapping mapping;

            for (auto [calleeOperand, blockOperand] : llvm::zip_equal(
                     callOp.getCalleeOperands(),
                     calleeRegion.getArguments())) {
                mapping.map(blockOperand, calleeOperand);
            }

            rewriter.setInsertionPointToStart(&region.front());
            for (auto &calleeOp : calleeBlock) {
                auto clone = rewriter.clone(calleeOp, mapping);
                if (auto returnOp = llvm::dyn_cast<ReturnOp>(clone)) {

                    for (auto [callOpResult, returnOperand] : llvm::zip_equal(
                             callOp.getResults(),
                             returnOp->getOperands())) {
                        callOpResult.replaceAllUsesWith(returnOperand);
                        rewriter.eraseOp(clone);
                    }
                }
            }

            rewriter.eraseOp(callOp);
            if (boxop->use_empty()) rewriter.eraseOp(boxop);
            return true;
        }
        return false;
    }

    LogicalResult
    matchAndRewrite(scf::IfOp op, PatternRewriter &rewriter) const override
    {
        bool changed = false;
        auto &thenRegion = op.getThenRegion();
        auto &elseRegion = op.getElseRegion();
        thenRegion.walk([&](::mlir::closure::CallOp callOp) {
            changed |= CanonicalizeIf::inlineIfNestedClosure(
                callOp,
                rewriter,
                thenRegion);
        });

        elseRegion.walk([&](::mlir::closure::CallOp callOp) {
            changed |= CanonicalizeIf::inlineIfNestedClosure(
                callOp,
                rewriter,
                elseRegion);
        });

        if (changed) {
            // LLVM_DEBUG(llvm::errs() << "\n inline done\n" << op << "\n");
            ModuleOp moduleOp = op->getParentOfType<ModuleOp>();
            while (!moduleOp) {
                auto parentOp = op->getParentOp();
                if (parentOp) {
                    moduleOp = parentOp->getParentOfType<ModuleOp>();
                } else {
                    LLVM_DEBUG(llvm::errs() << "No Parent no Print");
                    return llvm::success();
                }
            }

            LLVM_DEBUG(llvm::errs() << "\n Inline Done\n" << moduleOp);
            return llvm::success();
        } else
            return llvm::failure();
    }
};
struct CalledBoxOpSelectToIf : OpRewritePattern<CallOp> {
public:
    using OpRewritePattern<CallOp>::OpRewritePattern;

    LogicalResult
    matchAndRewrite(CallOp op, PatternRewriter &rewriter) const override
    {
        auto definingOp = op.getCallee().getDefiningOp();
        if (definingOp == nullptr) {
            LLVM_DEBUG(llvm::errs() << "No defining op found.\n");
        } else if (
            auto selectOp = llvm::dyn_cast<arith::SelectOp>(definingOp)) {
            auto condition = selectOp.getCondition();
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

            rewriter.setInsertionPointToStart(&thenBlock.front());
            auto castThenOp = cast<BoxOp>(thenOp);
            auto thenCall = rewriter.create<CallOp>(
                op->getLoc(),
                op.getResultTypes(),
                castThenOp,
                op.getCalleeOperands());
            rewriter.create<scf::YieldOp>(op->getLoc(), thenCall->getResults());

            rewriter.setInsertionPointToStart(&elseBlock.front());
            auto castElseOp = cast<BoxOp>(elseOp);
            auto elseCall = rewriter.create<CallOp>(
                op->getLoc(),
                op.getResultTypes(),
                castElseOp,
                op.getCalleeOperands());
            rewriter.create<scf::YieldOp>(op->getLoc(), elseCall->getResults());

            rewriter.replaceOp(selectOp, ifOp);
            rewriter.replaceOp(op, ifOp);

            ModuleOp moduleOp = ifOp->getParentOfType<ModuleOp>();
            while (!moduleOp) {
                auto parentOp = ifOp->getParentOp();
                if (parentOp) {
                    moduleOp = parentOp->getParentOfType<ModuleOp>();
                } else {
                    LLVM_DEBUG(llvm::errs() << "No Parent no Print");
                    return llvm::success();
                }
            }

            // LLVM_DEBUG(llvm::errs() << "\nAll Done\n" << moduleOp);
            return llvm::success();
        } else {
            LLVM_DEBUG(
                llvm::errs()
                << "Cast to select failed on " << definingOp << "\n");
        }

        return llvm::failure();
    }
};

struct CanonicalizeClosure : OpRewritePattern<CallOp> {
public:
    using OpRewritePattern<CallOp>::OpRewritePattern;

    LogicalResult
    matchAndRewrite(CallOp op, PatternRewriter &rewriter) const override
    {
        auto callee = op.getCallee().getDefiningOp();
        if (callee == nullptr) {
            LLVM_DEBUG(llvm::errs() << "No defining op found.\n");
        } else if (auto boxop = llvm::dyn_cast<BoxOp>(callee)) {
            if (boxop.getCaptureArgs().empty()) return llvm::failure();

            auto calleeOperands = op.getCalleeOperandsMutable();
            calleeOperands.slice(0, 0).assign(boxop.getCaptureArgs());

            rewriter.modifyOpInPlace(boxop, [&]() {
                boxop->eraseOperands(0, boxop.getCaptureArgs().size());
                boxop.setFunctionType(FunctionType::get(
                    op->getContext(),
                    op.getCalleeOperands().getTypes(),
                    op->getResultTypes()));
                boxop.getResult().setType(boxop.getClosureType());
            });
            return llvm::success();
        } else {
            LLVM_DEBUG(
                llvm::errs() << "Cast to boxop failed. " << callee << " at "
                             << callee->getLoc() << "\n");
        }

        return failure();
    }
};

class ParameterlessClosureInliner : public DialectInlinerInterface {
public:
    using DialectInlinerInterface::DialectInlinerInterface;

    bool debugIncludeMain = false;

    bool isLegalToInline(Operation* call, Operation*, bool) const override
    {
        // if (call->getParentOfType<func::FuncOp>()->hasAttr("sigi.main"))
        //     return debugIncludeMain;
        return llvm::isa<CallOp>(call);
    }

    bool
    isLegalToInline(Region* region, Region*, bool, IRMapping &) const override
    {
        // if (region->getParentOfType<func::FuncOp>()->hasAttr("sigi.main"))
        //     return debugIncludeMain;
        return false;
    }

    bool
    isLegalToInline(Operation* op, Region*, bool, IRMapping &) const override
    {
        // if (op->getParentOfType<func::FuncOp>()->hasAttr("sigi.main"))
        //     return debugIncludeMain;
        return llvm::isa<CallOp>(op);;
    }

    void
    handleTerminator(Operation* op, ValueRange valuesToReplace) const override
    {
        if (auto returnOp = llvm::dyn_cast<ReturnOp>(*op)) {
            for (auto [value, operand] :
                 llvm::zip_equal(valuesToReplace, returnOp->getOperands())) {
                value.replaceAllUsesWith(operand);
            }
        } else {
            LLVM_DEBUG(
                llvm::errs()
                << "Return op not from closure at" << op->getLoc() << "\n");
        }
    }
};

} // namespace closure

void ClosureDialect::getCanonicalizationPatterns(
    RewritePatternSet &result) const
{
    // result.add<
    //     ::closure::CanonicalizeClosure,
    //     ::closure::CalledBoxOpSelectToIf,
    //     ::closure::CanonicalizeIf>(result.getContext());

    // result.add<::closure::CanonicalizeClosure>(result.getContext());

    result.add<
    ::closure::CanonicalizeClosure,
    ::closure::CalledBoxOpSelectToIf>(result.getContext());
}

void ClosureDialect::initialize()
{
    registerOps();
    registerTypes();
    addInterfaces<::closure::ParameterlessClosureInliner>();
}