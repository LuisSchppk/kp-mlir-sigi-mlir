#include "sigi-mlir/Dialect/Closure/IR/ClosureDialect.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureOps.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <list>
#include <llvm/ADT/MapVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Support/ModRef.h>
#include <llvm/Support/RWMutex.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <mlir/Analysis/DataFlowFramework.h>
#include <mlir/Analysis/Liveness.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/Dialect/LLVMIR/Transforms/AddComdats.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/IR/AsmState.h>
#include <mlir/IR/Block.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/Diagnostics.h>
#include <mlir/IR/Dominance.h>
#include <mlir/IR/IRMapping.h>
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include <mlir/IR/Location.h>
#include <mlir/IR/OpDefinition.h>
#include <mlir/IR/Operation.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/Region.h>
#include <mlir/IR/SymbolTable.h>
#include <mlir/IR/TypeUtilities.h>
#include <mlir/IR/Types.h>
#include <mlir/IR/Value.h>
#include <mlir/IR/ValueRange.h>
#include <mlir/IR/Visitors.h>
#include <mlir/Interfaces/ControlFlowInterfaces.h>
#include <mlir/Interfaces/SideEffectInterfaces.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Transforms/GreedyPatternRewriteDriver.h>
#include <ostream>
#include <set>
#include <thread>
#include <utility>
#include <vector>

#define GEN_PASS_DEF_TAILRECURSIONPASS
namespace mlir::sigi {
#include "sigi-mlir/Conversion/SigiPasses.h.inc"
} // namespace mlir::sigi

#define DEBUG_TYPE "tail-rec"

using namespace mlir;

bool isBeforeInOp(Operation* first, Operation* second)
{
    assert(first && "First is null.");
    assert(second && "Second is null.");
    auto firstBlock = first->getBlock();
    auto secondBlock = second->getBlock();
    bool isBefore = false;
    if (firstBlock == secondBlock) {
        isBefore = first->isBeforeInBlock(second);
    } else {
        llvm::SmallVector<Block*> queue{firstBlock};
        int i = 0;
        int size = queue.size();
        while (i < size && !isBefore) {
            auto current = queue[i++];
            isBefore = current == secondBlock;
            for (auto succ : current->getSuccessors())
                if (!isBefore && !llvm::is_contained(queue, succ)) queue.push_back(succ);
            size = queue.size();
        }
    }
    return isBefore;
}

llvm::SmallVector<Operation*> getRecursiveCalls(mlir::func::FuncOp op)
{
    llvm::SmallVector<Operation*> result;
    op->walk([&](mlir::func::CallOp nestedOp) {
        auto callee = nestedOp.getCalleeAttr();
        auto funcOp = mlir::SymbolTable::lookupNearestSymbolFrom(op, callee);
        if (op == funcOp) result.push_back(nestedOp.getOperation());
    });
    return result;
}

llvm::SmallVector<Operation*> getReturnOps(func::FuncOp funcOp)
{
    llvm::SmallVector<Operation*> result;
    funcOp.walk([&](func::ReturnOp returnOp) {
        auto closestParentFunc = returnOp->getParentOfType<func::FuncOp>();
        if (funcOp == closestParentFunc) {
            LLVM_DEBUG(
                llvm::errs() << " Found possible returnOp for " << funcOp.getNameAttr() << " at "
                             << returnOp->getLoc() << "\n");
            result.push_back(returnOp.getOperation());
        }
    });
    return result;
}

llvm::MapVector<func::CallOp, llvm::SmallVector<Operation*>>
getRecCallUses(llvm::SmallVector<Operation*> recCalls)
{
    thread_local llvm::MapVector<func::CallOp, llvm::SmallVector<Operation*>> recCallAndUses;
    bool valid = !recCalls.empty();

    for (auto op : recCalls) {
        auto callOp = llvm::cast<func::CallOp>(op);
        auto results = callOp->getResults();
        llvm::SmallVector<Operation*> queue;
        llvm::SmallVector<Operation*> visited;
        for (auto &use : results.getUses()) queue.push_back(use.getOwner());

        int i = 0;
        int size = queue.size();
        llvm::SmallVector<Operation*> entry = recCallAndUses.contains(callOp)
                                                  ? recCallAndUses.lookup(callOp)
                                                  : llvm::SmallVector<Operation*>{};

        while (valid && i < size) {
            auto owner = queue[i++];

            if (!llvm::is_contained(visited, owner)) {
                valid &= owner->hasTrait<OpTrait::ReturnLike>()
                         || owner->hasTrait<OpTrait::IsTerminator>();

                if (valid) entry.push_back(owner);

                for (auto &use : owner->getResults().getUses()) queue.push_back(use.getOwner());
                visited.push_back(owner);
            }
        }
        recCallAndUses.insert({callOp, entry});
    }

    if (!valid) recCallAndUses.clear();

    return recCallAndUses;
}

bool checkSuccessorOps(
    Operation* parent,
    llvm::MapVector<func::CallOp, llvm::SmallVector<Operation*>> recCallsAndUses)
{
    bool validSuccessorOp = true;
    // only for >= cpp 20
    // for (auto [callOp, uses] : recCallsAndUses) {
    for (auto entry : recCallsAndUses) {
        auto callOp = entry.first;
        auto uses = entry.second;
        if (validSuccessorOp) {
            parent->walk([&](Operation* op) {
                // op is after call
                // if (!op->isBeforeInBlock(callOp) && op != callOp) validSuccessorOp &= isPure(op);
                validSuccessorOp &=
                    !(op->getBlock() == callOp->getBlock()) // not in same block -> ignore
                    || op->isBeforeInBlock(callOp)          // or before call -> ignore
                    || op == callOp                         // is call -> ignore
                    || isPure(op); // if all previous false -> has to be pure.

                for (auto use : uses)
                    validSuccessorOp &= !(op->getBlock() == use->getBlock())
                                        || op->isBeforeInBlock(use) || op == use || isPure(op);
                // if (!op->isBeforeInBlock(use) && op != use) validSuccessorOp &= isPure(op);
                if (!validSuccessorOp) {
                    LLVM_DEBUG(
                        llvm::errs()
                        << "Op at " << op->getLoc() << " not a valid successor for tail call\n");
                    return;
                }
            });
        }
    }

    return validSuccessorOp;
}

namespace {

struct ConstructIterativeVersion : public OpRewritePattern<func::FuncOp> {
public:
    explicit ConstructIterativeVersion(
        MLIRContext* context,
        llvm::MapVector<func::CallOp, llvm::SmallVector<Operation*>>* recCallsAndUses)
            : OpRewritePattern<func::FuncOp>(context)
    {
        this->recCallsAndUses = recCallsAndUses;
    }

private:
    llvm::MapVector<func::CallOp, llvm::SmallVector<Operation*>>* recCallsAndUses;

    std::pair<scf::IfOp, Region*>
    getTopMostIfAndWhileBody(func::FuncOp funcOp, func::CallOp callOp) const
    {
        auto encompassingIf = callOp->getParentOfType<scf::IfOp>();
        bool isTopMost = encompassingIf->getParentOp() == funcOp;
        Region* whileBody = callOp->getParentRegion();

        LLVM_DEBUG(llvm::errs() << "Whilebody begins at " << whileBody->getLoc() << "\n");
        while (encompassingIf && !isTopMost) {
            encompassingIf = encompassingIf->getParentOfType<scf::IfOp>();
            whileBody = &encompassingIf.getThenRegion() == whileBody->getParentRegion()
                            ? &encompassingIf.getThenRegion()
                            : &encompassingIf.getElseRegion();
            isTopMost = encompassingIf->getParentOp() == funcOp;
            LLVM_DEBUG(llvm::errs() << "Whilebody begins at " << whileBody->getLoc() << "\n");
        }
        return {encompassingIf, whileBody};
    }

    bool isInvariant(func::FuncOp funcOp, Operation* op) const
    {
        bool constant = op->hasTrait<OpTrait::ConstantLike>();
        return constant;
    }

    bool containsCall(func::FuncOp funcOp, func::CallOp callOp) const
    {
        auto parent = callOp->getParentOfType<func::FuncOp>();
        bool isContained = false;
        while (!isContained && parent) {
            isContained = parent == funcOp;
            parent = parent->getParentOfType<func::FuncOp>();
        }
        return isContained;
    }

    Region* getWhileBody(scf::IfOp encompassingIf, func::CallOp callOp) const
    {
        if (&encompassingIf.getThenRegion() == callOp->getParentRegion()) {
            return &encompassingIf.getThenRegion();
        } else if (&encompassingIf.getElseRegion() == callOp->getParentRegion()) {
            return &encompassingIf.getElseRegion();
        } else {
            LLVM_DEBUG(llvm::errs() << "CALL OP IS NOT IN THEN OR ELSE REGION\n");
            return nullptr;
        }
    }

    // Sigi has support for types: Integer (32, 1); BoxType & StackType
    LogicalResult createDefaultSSA(
        Type type,
        PatternRewriter &rewriter,
        Location loc,
        Operation*&defaultAssignment) const
    {

        if (type.isInteger()) {
            defaultAssignment =
                rewriter.create<mlir::arith::ConstantOp>(loc, rewriter.getI32IntegerAttr(0))
                    .getOperation();
        } else if (llvm::isa<closure::BoxedClosureType>(type)) {
            auto functionType = FunctionType::get(type.getContext(), ValueRange{}, ValueRange{});
            defaultAssignment =
                rewriter.create<closure::BoxOp>(loc, rewriter.getIntegerType(1), ValueRange{})
                    .getOperation();
        } else if (llvm::isa<sigi::StackType>(type)) {
            // maybe implemement logic for this.
            emitError(
                loc,
                "StackType can never be part of extended LCVs as a Stack cannot be pushed on a "
                "stack.");
            return llvm::failure();
        } else {
            emitError(
                loc,
                "Could not match type from extenden LCVs to sigi compatible types {Integer, "
                "Boolean, BoxType and StackType}.\n");
            return llvm::failure();
        }

        return llvm::success();
    }

    LogicalResult constructPreHeader(
        SmallVector<Value> lcv,
        func::FuncOp funcOp,
        PatternRewriter &rewriter,
        IRMapping &oldToNew,
        SmallVector<Value> &initValues,
        SmallVector<Type> &initTypes,
        SmallVector<Location> &initLocs,
        SmallVector<Value> &verifyUnalive) const
    {
        auto blockArgs = funcOp.getBlocks().front().getArguments();
        BlockArgument stackArg;
        for (auto arg : blockArgs) {
            bool isStack = isa<sigi::StackType>(arg.getType());
            if (stackArg && isStack) {
                emitError(
                    funcOp->getLoc(),
                    "Currently only rec. Functions with one sigi-stack argument are supported.");
                return llvm::failure();
            } else if (isStack) {
                stackArg = arg;
            }
        }
        for (auto value : lcv) {
            if (llvm::is_contained(blockArgs, value)) {
                auto mappedValue = oldToNew.lookupOrDefault(value);
                initValues.push_back(mappedValue);
                initTypes.push_back(mappedValue.getType());
                initLocs.push_back(mappedValue.getLoc());
            } else if (llvm::isa<sigi::StackType>(value.getType())) {
                oldToNew.map(value, stackArg);
                auto mappedValue = oldToNew.lookupOrDefault(stackArg);
                initValues.push_back(mappedValue);
                initTypes.push_back(mappedValue.getType());
                initLocs.push_back(mappedValue.getLoc());
                verifyUnalive.push_back(value);
            } else {
                Operation* defaultAssignment = nullptr;
                auto succ = createDefaultSSA(
                    value.getType(),
                    rewriter,
                    funcOp->getLoc(),
                    defaultAssignment);

                if (succ.failed()) {
                    emitError(funcOp->getLoc(), "Could not construct Preheader.");
                    return llvm::failure();
                }

                if (defaultAssignment) {
                    for (auto default_value : defaultAssignment->getResults()) {
                        initValues.push_back(default_value);
                        initTypes.push_back(default_value.getType());
                        initLocs.push_back(default_value.getLoc());
                        oldToNew.map(value, default_value);
                    }
                } else {
                    emitError(
                        funcOp->getLoc(),
                        "Could not construct Preheader. Default assignment is null, eventhough "
                        "createDefaultSSA succeded.");
                    return llvm::failure();
                }
            }
        }
        return llvm::success();
    }

    LogicalResult getYield(Block* block, scf::YieldOp &yield, func::FuncOp &funcOp) const
    {
        SmallVector<scf::YieldOp> yields{
            block->getOps<scf::YieldOp>().begin(),
            block->getOps<scf::YieldOp>().end()};
        if (yields.empty()) {
            emitError(
                funcOp->getLoc(),
                "Passed block during eliminateRecursiveCalls did not contain any yieldOp.\n");
            return llvm::failure();
        } else if (yields.size() > 1) {
            emitError(
                funcOp->getLoc(),
                "Passed block during eliminateRecursiveCalls contains more than one yieldOp.\n");
            return llvm::failure();
        } else if (yields.front()->getBlock() == block) {
            yield = yields.front();
            return llvm::success();
        } else {
            emitError(
                funcOp->getLoc(),
                "Yield found during getYield for eliminateRecursiveCalls is not directly nested "
                "same block as call.\n");
            return llvm::failure();
        }
    }

    LogicalResult
    eliminateRecursiveCalls(Block* afterBody, PatternRewriter &rewriter, func::FuncOp &funcOp) const
    {
        bool success = true;
        SmallVector<std::pair<Operation*, Operation*>> eliminate;
        SmallVector<Operation*> convertYields;

        ModuleOp moduleOp = funcOp->getParentOfType<ModuleOp>();
        afterBody->walk([&](func::CallOp callOp) {
            if (success && moduleOp.lookupSymbol(callOp.getCalleeAttr()) == funcOp) {
                scf::YieldOp yield;
                if (getYield(callOp->getBlock(), yield, funcOp).failed()) {
                    success = false;
                    return;
                }
                eliminate.push_back({callOp.getOperation(), yield.getOperation()});
            }
        });

        if (success) {
            for (auto [call, yield] : eliminate) {
                rewriter.setInsertionPoint(yield);
                auto newYield =
                    rewriter.create<scf::YieldOp>(funcOp->getLoc(), call->getOperands());
                rewriter.replaceOp(yield, newYield);
                rewriter.eraseOp(call);
            }
            return llvm::success();
        } else
            return llvm::failure();
    }

    LogicalResult constructWhileYield(
        PatternRewriter &rewriter,
        Block* afterBody,
        IRMapping &oldToNew,
        SmallVector<Value> initValues,
        Block* entryBlock,
        func::FuncOp &funcOp) const
    {
        SmallVector<Value> loopResult;

        Operation* finalYield = nullptr;
        afterBody->walk([&](scf::YieldOp yield) {
            if (!finalYield && yield->getBlock() == afterBody) {
                finalYield = yield.getOperation();
                LLVM_DEBUG(
                    llvm::errs() << "FINAL YIELD: " << yield << " AT " << yield->getLoc() << "\n");
            }
        });

        if (!finalYield) {
            emitError(funcOp->getLoc(), "Could not identify final yield of recursive body.\n");
            return llvm::failure();
        }
        if (finalYield->getNumOperands() > initValues.size()) {
            emitError(funcOp->getLoc(), "Could not match up loop results with initValues\n");
            return llvm::failure();
        }

        for (auto operand : finalYield->getOperands()) loopResult.push_back(operand);

        for (size_t i = finalYield->getNumOperands(); i < initValues.size(); i++) {

            // Pass initial dummy values again.
            loopResult.push_back(initValues[i]);
        }

        for (auto value : loopResult) LLVM_DEBUG(llvm::errs() << "LOOP RESULT " << value << "\n");

        rewriter.setInsertionPoint(finalYield);
        auto newYield = rewriter.create<scf::YieldOp>(funcOp->getLoc(), loopResult);
        rewriter.eraseOp(finalYield);
        afterBody->print(llvm::errs());
        return llvm::success();
    }

    LogicalResult walkAndCloneRegion(
        Region* srcRegion,
        PatternRewriter &rewriter,
        IRMapping &oldToNew,
        func::FuncOp &funcOp,
        SmallVector<Operation*> &preHeader,
        SmallVector<Operation*> &preHeaderOriginal) const
    {
        ModuleOp moduleOp = funcOp->getParentOfType<ModuleOp>();
        for (auto &op : srcRegion->getOps()) {
            auto clone = rewriter.clone(op, oldToNew);
            oldToNew.map(op.getResults(), clone->getResults());
            if (isInvariant(funcOp, &op)) {
                auto castToCall = llvm::dyn_cast<func::CallOp>(op);
                if (castToCall && moduleOp.lookupSymbol(castToCall.getCalleeAttr()) == funcOp) {
                    emitError(
                        funcOp->getLoc(),
                        "Found Invariant Recursive Call in " + funcOp->getName().getStringRef()
                            + " " + funcOp.getNameAttr().str() + "\n");
                    return llvm::failure();
                } else {
                    preHeader.push_back(clone);
                    preHeaderOriginal.push_back(&op);
                }
            }
        }

        return llvm::success();
    }

    SmallVector<Value> findLCVs(func::FuncOp funcOp, Operation* border, Region* whileBody) const
    {
        SmallVector<Value> defsHead;
        SmallVector<Value> usesHead;
        SmallVector<Operation*> premptiveDefinitions;
        SmallVector<Value> usesBody;

        // Always contain all function Arguments as lcv
        SmallVector<Value> lcv;

        whileBody->walk([&](Operation* op) {
            if (!isInvariant(funcOp, op))
                for (auto operands : op->getOperands()) usesBody.push_back(operands);
        });

        funcOp->walk([&](Operation* op) {
            if (isBeforeInOp(op, border) && !isInvariant(funcOp, op)) {
                for (auto result : op->getResults()) defsHead.push_back(result);

                // for (auto operand : op->getOperands()) {
                //     if (llvm::is_contained(funcOp.getBlocks().front().getArguments(),
                //     operand)) {

                //     } else {
                //         usesHead.push_back(operand);
                //     }
                // }
            }
        });

        auto longer = usesBody.size() >= defsHead.size() ? usesBody : defsHead;
        auto shorter = usesBody.size() < defsHead.size() ? usesBody : defsHead;
        assert(shorter != longer && "USES MIXED UP");

        for (auto blockArg : funcOp.getBlocks().front().getArguments())
            if (llvm::is_contained(usesBody, blockArg) && !llvm::is_contained(lcv, blockArg))
                lcv.push_back(blockArg);
        for (auto value : longer)
            if (llvm::is_contained(shorter, value) && !llvm::is_contained(lcv, value))
                lcv.push_back(value);
        return lcv;
    }

    LogicalResult matchAndRewrite(func::FuncOp funcOp, PatternRewriter &rewriter) const override
    {
        if (funcOp->hasAttr("transformed_to_iterative")) return llvm::failure();
        ModuleOp moduleOp = funcOp->getParentOfType<ModuleOp>();
        auto calleeAttr = recCallsAndUses->front().first.getCalleeAttr();
        SmallVector<Operation*> sym;
        if (!moduleOp) {
            emitError(
                funcOp->getLoc(),
                "Could not find moduleOp during ConstructIterativeVersion.");
            return llvm::failure();
        } else if (SymbolTable::lookupSymbolIn(moduleOp, calleeAttr, sym).failed()) {
            emitError(funcOp->getLoc(), "Could not find funcOp during ConstructIterativeVersion.");
            return llvm::failure();
        } else if (moduleOp.lookupSymbol(calleeAttr) != funcOp) {
            // Not a recursive call.
            return llvm::failure();
        }

        IRMapping blockToResult;
        IRMapping oldToNew;

        // !!! FOR NOW RECURSION WITH ONE CALL.
        auto callOp = recCallsAndUses->front().first; // first recursive call.

        auto ifAndBody = getTopMostIfAndWhileBody(funcOp, callOp);
        auto topMostIf = ifAndBody.first;
        auto whileBody = ifAndBody.second;
        LLVM_DEBUG(llvm::errs() << "WHILE BODY " << whileBody->getLoc() << "\n");

        // Create new Function Skeleton
        rewriter.setInsertionPointAfter(funcOp);
        auto iterativeFunc = rewriter.create<func::FuncOp>(
            funcOp->getLoc(),
            funcOp.getNameAttr().str() + "It",
            funcOp.getFunctionType());
        auto entryBlock = iterativeFunc.addEntryBlock();

        // Map from old BlockArgs to new BlockArgs
        for (auto [from, to] : llvm::zip_equal(
                 funcOp.getBlocks().front().getArguments(),
                 entryBlock->getArguments())) {
            oldToNew.map(from, to);
            blockToResult.map(from, to);
        }

        SmallVector<Value> recLcv = findLCVs(funcOp, topMostIf, whileBody);
        SmallVector<Type> lcvTypes;
        SmallVector<Location> lcvLocs;

        for (auto value : recLcv) {
            LLVM_DEBUG(llvm::errs() << "LCV " << value << " TYPE " << value.getType() << "\n");
            lcvTypes.push_back(value.getType());
            lcvLocs.push_back(value.getLoc());
        }

        rewriter.setInsertionPointToStart(entryBlock);
        SmallVector<Value> initValues;
        SmallVector<Type> initTypes;
        SmallVector<Location> initLocs;
        SmallVector<Value> verifyUnaliveAtLoopFront;

        if (constructPreHeader(
                recLcv,
                funcOp,
                rewriter,
                blockToResult,
                initValues,
                initTypes,
                initLocs,
                verifyUnaliveAtLoopFront)
                .failed())
            return llvm::failure();

        // rewriter.setInsertionPointToStart(entryBlock);
        auto whileOp = rewriter.create<scf::WhileOp>(funcOp->getLoc(), initTypes, initValues);

        // Map from input to output.
        for (auto [from, to] : llvm::zip_first(entryBlock->getArguments(), whileOp->getResults()))
            blockToResult.map(from, to);

        // Create Bodies for Before And After Region.
        auto* beforeBlock = rewriter.createBlock(
            &whileOp.getBefore(),
            whileOp.getBefore().begin(),
            initTypes,
            initLocs);
        auto* afterBlock = rewriter.createBlock(
            &whileOp.getAfter(),
            whileOp.getAfter().begin(),
            initTypes,
            initLocs); // LOCS NOT CORRECT?

        // Map from Initial Values calculated in Preheader to beforeBlock Args.
        for (auto [from, to] : llvm::zip(initValues, beforeBlock->getArguments()))
            oldToNew.map(from, to);

        // Map from old BlockArgs directly to beforeBlockArgs if possible.
        // CAN BE REMOVED?
        for (auto [from, to] : llvm::zip_equal(
                 funcOp.getBlocks().front().getArguments(),
                 entryBlock->getArguments())) {
            oldToNew.map(from, oldToNew.lookupOrDefault(to));
        }

        // for (auto [from, to] :
        //      llvm::zip_first(beforeBlock->getArguments(), whileOp->getResults()))
        //     blockToResult.map(from, to);

        SmallVector<Operation*> preHeader;
        SmallVector<Operation*> preHeaderOriginal;

        // // BEFORE
        rewriter.setInsertionPointToStart(whileOp.getBeforeBody());
        funcOp->walk([&](Operation* op) {
            bool isBefore = isBeforeInOp(op, topMostIf.getOperation());
            if (isBefore) {
                auto clone = rewriter.clone(*op, oldToNew);
                oldToNew.map(op->getResults(), clone->getResults());
                if (isInvariant(funcOp, op)) {
                    preHeader.push_back(clone);
                    preHeaderOriginal.push_back(op);
                }
            }
        });

        Liveness liveness(iterativeFunc);
        for (auto blockArgs : beforeBlock->getArguments()) {}

        SmallVector<Value> lcv;
        for (auto var : recLcv) lcv.push_back(oldToNew.lookup(var));

        rewriter.create<scf::ConditionOp>(
            funcOp->getLoc(),
            oldToNew.lookup(topMostIf.getCondition()),
            lcv);

        // AFTER
        // Map from old lcv onto After-BlockArgs
        for (auto [from, to] : llvm::zip_first(recLcv, afterBlock->getArguments())) {
            LLVM_DEBUG(llvm::errs() << "MAP " << from << " TO " << to << "\n");
            oldToNew.map(from, to);
        }

        rewriter.setInsertionPointToStart(whileOp.getAfterBody());
        if (walkAndCloneRegion(whileBody, rewriter, oldToNew, funcOp, preHeader, preHeaderOriginal)
                .failed())
            return llvm::failure();

        if (eliminateRecursiveCalls(whileOp.getAfterBody(), rewriter, funcOp).failed())
            return llvm::failure();
        if (constructWhileYield(
                rewriter,
                whileOp.getAfterBody(),
                oldToNew,
                initValues,
                entryBlock,
                funcOp)
                .failed())
            return llvm::failure();

        // // Move invariant to preheader
        rewriter.setInsertionPointToStart(entryBlock);
        for (auto [opNew, opOriginal] : llvm::zip_equal(preHeader, preHeaderOriginal)) {
            // rewriter.moveOpBefore(op, whileOp);
            auto clone = rewriter.clone(*opNew, oldToNew);
            oldToNew.map(opNew->getResults(), clone->getResults());
            blockToResult.map(opOriginal->getResults(), clone->getResults());
            rewriter.replaceAllUsesWith(opNew->getResults(), clone->getResults());
            rewriter.eraseOp(opNew);
        }

        // Termination
        Operation* returnOp;
        funcOp->walk([&](mlir::func::ReturnOp op) {
            if (op->getParentOp() == funcOp) returnOp = op.getOperation();
        });

        if (!returnOp) {
            emitError(funcOp->getLoc(), "Could not find returnOp of funcOp");
            return llvm::failure();
        }

        rewriter.setInsertionPointAfter(whileOp);

        Region* terminationBranch = &topMostIf.getThenRegion() == whileBody
                                        ? &topMostIf.getElseRegion()
                                        : &topMostIf.getThenRegion();

        terminationBranch->walk([&](Operation* op) {
            if (isa<scf::YieldOp>(op) && op->getParentOp() == topMostIf) {
                auto yield = cast<scf::YieldOp>(op);
                for (auto [ifRes, yieldRes] :
                     llvm::zip_equal(topMostIf.getResults(), yield.getResults())) {

                    auto current = yieldRes;
                    auto next = blockToResult.lookup(current);
                    int i = 0;
                    while (current != next) {
                        current = next;
                        next = blockToResult.lookupOrDefault(current);
                        ++i;
                    }

                    blockToResult.map(ifRes, current);
                }
                rewriter.clone(*returnOp, blockToResult);
            } else {
                auto clonedOp = rewriter.clone(*op, blockToResult);
                blockToResult.map(op->getResults(), clonedOp->getResults());
            }
        });

        moduleOp->walk([&](func::CallOp callOp) {
            auto calleeAttr = callOp.getCalleeAttr();
            auto callee = moduleOp.lookupSymbol(calleeAttr);
            if (callee == funcOp && !containsCall(funcOp, callOp)) {
                rewriter.setInsertionPoint(callOp);
                auto itCall = rewriter.create<func::CallOp>(
                    funcOp->getLoc(),
                    iterativeFunc,
                    callOp.getOperands());
                rewriter.replaceOp(callOp, itCall);
            }
        });

        rewriter.modifyOpInPlace(funcOp, [&]() {
            funcOp->setAttr("transformed_to_iterative", rewriter.getUnitAttr());
        });

        rewriter.modifyOpInPlace(iterativeFunc, [&]() {
            iterativeFunc->setAttr("transformed_to_iterative", rewriter.getUnitAttr());
        });

        funcOp.eraseBody();

        auto newEntry = funcOp.addEntryBlock();
        rewriter.setInsertionPointToStart(newEntry);
        auto itCall = rewriter.create<func::CallOp>(
            funcOp->getLoc(),
            iterativeFunc,
            newEntry->getArguments());
        rewriter.create<func::ReturnOp>(funcOp->getLoc(), itCall->getResults());
        if (failed(moduleOp.verify())) moduleOp.emitError("Verification failed");
        return llvm::success();
    }
};

struct TailRecursionPass : public mlir::sigi::impl::TailRecursionPassBase<TailRecursionPass> {

    void runOnOperation() override
    {
        func::FuncOp operation = getOperation();
        ModuleOp moduleOp = operation->getParentOfType<ModuleOp>();
        auto recCalls = getRecursiveCalls(operation);

        if (recCalls.empty()) {
            LLVM_DEBUG(
                llvm::errs() << "######\n Function " << operation.getNameAttr()
                             << " contains no recursive calls\n");
            return;
        }

        auto recCallsAndUses = getRecCallUses(recCalls);
        bool validSuccessors = checkSuccessorOps(operation.getOperation(), recCallsAndUses);
        if (recCallsAndUses.empty()) {
            LLVM_DEBUG(
                llvm::errs() << "###### \n Function " << getOperation().getNameAttr()
                             << " is recursive but not tail recursive.\n");
            return;
        } else if (!validSuccessors) {
            LLVM_DEBUG(
                llvm::errs()
                << "###### \n Function " << getOperation().getNameAttr()
                << " is recursiv due to non-sideeffect free ops after recursive call.\n");
            return;
        } else {
            LLVM_DEBUG(
                llvm::errs() << "###### \n Function " << getOperation().getNameAttr()
                             << " is tail recursive.\n");
        }

        RewritePatternSet patterns(&getContext());
        GreedyRewriteConfig greedyConf;
        greedyConf.strictMode = GreedyRewriteStrictness::ExistingOps;
        patterns.add<ConstructIterativeVersion>(&getContext(), &recCallsAndUses);
        (void)applyOpPatternsAndFold(
            {operation},
            std::move(patterns), greedyConf);
        
        // if (failed(moduleOp.verify())) moduleOp.emitError("Verification failed");
    }
};
} // namespace
