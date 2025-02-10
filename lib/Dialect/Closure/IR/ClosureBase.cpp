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

namespace mlir::closure {

struct CanonicalizeClosure : OpRewritePattern<CallOp> {
public:
    using OpRewritePattern<CallOp>::OpRewritePattern;

    LogicalResult matchAndRewrite(CallOp op, PatternRewriter &rewriter) const override
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
                llvm::errs() << "Cast to boxop failed. " << callee->getLoc() << " at "
                             << callee->getName() << "\n");
        }
        return failure();
    }
};

class ParameterlessClosureInliner : public DialectInlinerInterface {
public:
    using DialectInlinerInterface::DialectInlinerInterface;

    bool debugIncludeMain = false;

    bool isLegalToInline(Operation* call, Operation*, bool) const final override { return true; }

    bool isLegalToInline(Region* region, Region*, bool, IRMapping &) const final override
    {
        return true;
    }

    bool isLegalToInline(Operation* op, Region*, bool, IRMapping &) const final override
    {  
        bool isABoxOp = llvm::isa<closure::BoxOp>(op);
        return !isABoxOp;
    }

    void handleTerminator(Operation* op, ValueRange valuesToReplace) const final override
    {
        LLVM_DEBUG(llvm::errs() << "[OP TO RANGE] ENTERED \n");
        if (auto returnOp = llvm::dyn_cast<ReturnOp>(*op)) {
            for (auto [value, operand] : llvm::zip_equal(valuesToReplace, returnOp->getOperands()))
                value.replaceAllUsesWith(operand);
        } else {
            LLVM_DEBUG(
                llvm::errs()
                << "[OP TO RANGE] Return op not from closure at" << op->getLoc() << "\n");
        }
    }

    void handleTerminator(Operation* op, Block* newDest) const final override
    {
        LLVM_DEBUG(llvm::errs() << "[OP TO BLOCK] ENTERED \n");
        auto returnOp = dyn_cast<ReturnOp>(op);
        if (!returnOp) {
            LLVM_DEBUG(
                llvm::errs()
                << "[OP TO BLOCK] Return op not from closure at" << op->getLoc() << "\n");
            return;
        }

        OpBuilder builder(op);
        builder.create<cf::BranchOp>(op->getLoc(), newDest, returnOp.getOperands());
        // if (op->use_empty())
        // op->erase();
        // else
        //     LLVM_DEBUG(llvm::errs() << "Uses for " << returnOp << " are not empty!");
    }
};

} // namespace mlir::closure

void ClosureDialect::getCanonicalizationPatterns(RewritePatternSet &result) const
{
    result.add<::closure::CanonicalizeClosure>(result.getContext());
}

void ClosureDialect::initialize()
{
    registerOps();
    registerTypes();
    addInterface<closure::ParameterlessClosureInliner>();
}