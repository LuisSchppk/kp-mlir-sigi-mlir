#include "sigi-mlir/Dialect/Closure/IR/ClosureDialect.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureOps.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <list>
#include <llvm/ADT/ArrayRef.h>
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
#include <mlir/Support/LLVM.h>
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

LogicalResult
getRecursiveCalls(mlir::func::FuncOp op, llvm::SmallVector<Operation*> &recursiveCalls)
{

    op->walk([&](mlir::func::CallOp nestedOp) {
        auto callee = nestedOp.getCalleeAttr();
        auto funcOp = mlir::SymbolTable::lookupNearestSymbolFrom(op, callee);
        if (op == funcOp) recursiveCalls.push_back(nestedOp.getOperation());
    });
    if (recursiveCalls.empty()) {
        LLVM_DEBUG(
            llvm::errs()
            << "FuncOp " << op.getSymNameAttr() << " at " << op->getLoc() << " is not recursive\n");
        return llvm::failure();
    } else {

        LLVM_DEBUG(
            llvm::errs()
            << "FuncOp " << op.getSymNameAttr() << " at " << op->getLoc()
            << " is recursive and has " << recursiveCalls.size() << " recursive call(s). \n");
        return llvm::success();
    }
}

LogicalResult isTailCall(Operation* funcOp, Operation* op)
{
    auto next = op->getNextNode();
    while (!llvm::isa<func::ReturnOp>(next)) {
        // LLVM_DEBUG(llvm::errs() << "Tail Call analysis at " << next->getLoc() << "\n");
        if (next->hasTrait<OpTrait::ReturnLike>() || next->hasTrait<OpTrait::IsTerminator>()) {
            auto parentOp = next->getParentOp();
            next = parentOp->getNextNode();
        } else {
            LLVM_DEBUG(
                llvm::errs()
                << "Encountered non-terminating operation on path form recursive call to "
                   "func.return. Operation at "
                << op->getLoc() << "\n");
            return llvm::failure();
        }
    }
    LLVM_DEBUG(llvm::errs() << "Tail Call analysis done: Op at " << op->getLoc() << " is a Tail Call\n");
    return llvm::success();
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
    llvm::MapVector<func::CallOp, llvm::SmallVector<Operation*>> recCallAndUses;
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

LogicalResult
topologicalSort(llvm::SmallVector<Operation*> &unsorted, llvm::SmallVector<Operation*> &sorted)
{
    // Underwhich circumstances do we have opSet != unsorted? only through faulty double inserted
    // value?
    SetVector<Operation*> opSet{unsorted.begin(), unsorted.end()};
    llvm::DenseMap<Operation*, llvm::SetVector<Operation*>> dependencies;

    // get dependencies.
    for (auto* op : unsorted) {
        for (auto operand : op->getOperands()) {
            if (auto* definingOp = operand.getDefiningOp()) {
                if (opSet.count(definingOp)) dependencies[op].insert(definingOp);
            }
        }
    }

    // begin with ops that have no dependency within unsorted. i.e. trivially ready.
    // Set to avoid duplicates? necessary?
    llvm::SetVector<Operation*> readyOps;
    for (auto* op : unsorted)
        if (dependencies[op].empty()) readyOps.insert(op);

    // Populate sorted.
    while (!readyOps.empty()) {
        auto current = readyOps.pop_back_val();
        sorted.push_back(current);

        for (auto &[op, definingOps] : dependencies) {

            // if current can be removed && definingOps is empty after -> new readyOp
            if (definingOps.remove(current) && definingOps.empty()) readyOps.insert(op);
        }
    }

    if (sorted.size() != opSet.size()) {
        llvm::errs() << "Something went wrong during Topological search. Debug for more info.\n";
        for (auto op : sorted) {
            if (llvm::is_contained(opSet, op)) {
                LLVM_DEBUG(
                    llvm::errs()
                    << "Op " << op << " at " << op->getLoc() << " not part of original opSet\n");
            }
        }
        for (auto op : opSet) {
            if (llvm::is_contained(sorted, op)) {
                LLVM_DEBUG(
                    llvm::errs()
                    << "Op " << op << " at " << op->getLoc()
                    << " not in sorted set. Not all dependencies could be resolved. (Cyclical?)\n");
                for (auto dep : dependencies[op]) {
                    LLVM_DEBUG(
                        llvm::errs()
                        << "still depends on op " << op << " at " << op->getLoc() << "\n");
                }
            }
        }

        return llvm::failure();
    }
    return llvm::success();
}

LogicalResult getDefinitions(
    Value value,
    ArrayRef<BlockArgument> blockArgs,
    SmallVector<Operation*> &definitions,
    SmallVector<Value> definedValues = {})
{
    SmallVector<Operation*> unsorted_defs = SmallVector<Operation*>{value.getDefiningOp()};

    int i = 0;
    int size = definitions.size();

    while (i < size) {
        auto current = definitions[i++];
        for (auto operand : current->getOperands()) {
            auto definingOp = operand.getDefiningOp();

            if (llvm::is_contained(blockArgs, operand)
                || llvm::is_contained(definedValues, operand)) {
                // reached leaf.
                continue;
            } else if (llvm::is_contained(definitions, definingOp)) {
                // already registered.
                continue;
            }

            definitions.push_back(definingOp);
        }
        size = definitions.size();
    }

    return topologicalSort(unsorted_defs, definitions);
}

void map_transitive(IRMapping &map, Value from, Value to)
{
    auto initial = from;
    auto current = to;
    auto next = map.lookupOrDefault(current);
    while (current != next && next != initial) {
        current = next;
        next = map.lookupOrDefault(current);
    }
    if (next == initial) {
        llvm::errs() << "Cycle during transitive map encountered. From " << from << " to " << to
                     << " Stop one entry before full cycle.\n";
    }
    map.map(from, current);
}

void map_transitive(IRMapping &map, ValueRange from, ValueRange to)
{
    for (auto [from_val, to_val] : llvm::zip_equal(from, to)) map_transitive(map, from_val, to_val);
}

IRMapping merge_value_map(IRMapping map1, IRMapping map2)
{
    IRMapping merged = map2;
    for (auto [from, to] : map1.getValueMap()) map_transitive(merged, from, to);
    return merged;
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

struct ConstructIterativeVersion : OpRewritePattern<func::FuncOp> {
public:
    using OpRewritePattern<func::FuncOp>::OpRewritePattern;

private:
    std::pair<scf::IfOp, Region*>
    getTopMostIfAndWhileBody(func::FuncOp funcOp, func::CallOp callOp) const
    {
        auto encompassingIf = callOp->getParentOfType<scf::IfOp>();
        bool isTopMost = encompassingIf->getParentOp() == funcOp;
        Region* whileBody = callOp->getParentRegion();

        while (encompassingIf && !isTopMost) {
            encompassingIf = encompassingIf->getParentOfType<scf::IfOp>();
            whileBody = &encompassingIf.getThenRegion() == whileBody->getParentRegion()
                            ? &encompassingIf.getThenRegion()
                            : &encompassingIf.getElseRegion();
            isTopMost = encompassingIf->getParentOp() == funcOp;
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

    LogicalResult getYield(Block* block, scf::YieldOp &yield, func::FuncOp &funcOp) const
    {
        SmallVector<Operation*> yields;
        for (auto yield : block->getOps<scf::YieldOp>()) yields.push_back(yield.getOperation());
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
            auto uncast_yield = yields[0];
            yield = llvm::cast<scf::YieldOp>(uncast_yield);
            return llvm::success();
        } else {
            emitError(
                funcOp->getLoc(),
                "Yield found during getYield for eliminateRecursiveCalls is not directly nested "
                "same block as call.\n");
            return llvm::failure();
        }
    }

    LogicalResult eliminateRecursiveCalls(
        Block* afterBody,
        PatternRewriter &rewriter,
        func::FuncOp &funcOp,
        ArrayRef<BlockArgument> blockArgs,
        IRMapping &toAfter) const
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
                toAfter.map(blockArgs, call->getOperands());
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

    LogicalResult getFinalYield(Block* afterBody, func::FuncOp &funcOp, Operation*&finalYield) const
    {
        bool found = false;
        for (auto yield : afterBody->getOps<scf::YieldOp>()) {
            if (!found && yield->getBlock() == afterBody) {
                finalYield = yield.getOperation();
                found = true;
            } else if (found && yield->getBlock() == afterBody) {
                emitError(funcOp->getLoc(), "Found multiple yields in top level after body.\n");
                return llvm::failure();
            }
        }

        if (!found) {
            emitError(funcOp->getLoc(), "Could not identify final yield of recursive body.\n");
            return llvm::failure();
        }
        return llvm::success();
    }

    LogicalResult
    findLCVs(func::FuncOp funcOp, Operation* border, Region* whileBody, SmallVector<Value> &lcvs)
        const
    {
        SmallVector<Value> defsHead;
        SmallVector<Value> usesHead;
        SmallVector<Operation*> premptiveDefinitions;
        SmallVector<Value> usesBody;

        whileBody->walk([&](Operation* op) {
            if (!isInvariant(funcOp, op))
                for (auto operands : op->getOperands()) usesBody.push_back(operands);
        });

        funcOp->walk([&](Operation* op) {
            if (isBeforeInOp(op, border) && !isInvariant(funcOp, op))
                for (auto result : op->getResults()) defsHead.push_back(result);
        });

        auto longer = usesBody.size() >= defsHead.size() ? usesBody : defsHead;
        auto shorter = usesBody.size() < defsHead.size() ? usesBody : defsHead;
        assert(shorter != longer && "USES MIXED UP");

        for (auto blockArg : funcOp.getBlocks().front().getArguments())
            if (llvm::is_contained(usesBody, blockArg) && !llvm::is_contained(lcvs, blockArg))
                lcvs.push_back(blockArg);
        for (auto value : longer)
            if (llvm::is_contained(shorter, value) && !llvm::is_contained(lcvs, value))
                lcvs.push_back(value);
        return llvm::success();
    }

    LogicalResult fillAfterBlock(
        Block* afterBlock,
        Block* entryBlock,
        Region* whileBody,
        scf::WhileOp &whileOp,
        SmallVector<Value> &lcvs,
        SmallVector<Operation*> postbody,
        SmallVector<Operation*> proto_head,
        IRMapping &toAfter,
        PatternRewriter &rewriter,
        func::FuncOp funcOp,
        scf::YieldOp &whileYield) const
    {

        rewriter.setInsertionPointToStart(whileOp.getAfterBody());
        for (auto &op : whileBody->getOps()) {
            auto clone = rewriter.clone(op, toAfter);
            toAfter.map(op.getResults(), clone->getResults());
        }

        if (eliminateRecursiveCalls(
                whileOp.getAfterBody(),
                rewriter,
                funcOp,
                entryBlock->getArguments(),
                toAfter)
                .failed())
            return llvm::failure();

        // "POSTBODY"
        rewriter.setInsertionPointToEnd(afterBlock);

        SmallVector<Value> postbodyLCVs;
        Operation* finalYield;
        if (getFinalYield(whileOp.getAfterBody(), funcOp, finalYield).failed())
            return llvm::failure();

        postbodyLCVs.resize(lcvs.size());
        for (size_t i = 0; i < finalYield->getOperands().size(); i++) {
            auto yieldOperand = finalYield->getOperands()[i];
            auto mappedValue = yieldOperand;
            postbodyLCVs[i] = mappedValue;
        }

        for (auto [op, original] : llvm::zip_equal(postbody, proto_head)) {
            auto* clone = rewriter.clone(*op, toAfter);
            toAfter.map(op->getResults(), clone->getResults());
            toAfter.map(original->getResults(), clone->getResults());

            for (auto result : clone->getResults()) {
                LLVM_DEBUG(llvm::errs());
                for (size_t i = 0; i < lcvs.size(); i++) {
                    auto lcv = lcvs[i];
                    auto mappedLCV = toAfter.lookupOrDefault(lcv);
                    if (mappedLCV == result) postbodyLCVs[i] = result;
                }
            }
        }

        rewriter.setInsertionPointToEnd(afterBlock);
        whileYield = rewriter.create<scf::YieldOp>(funcOp->getLoc(), postbodyLCVs);
        rewriter.eraseOp(finalYield);
        return llvm::success();
    }

    LogicalResult matchAndRewrite(func::FuncOp funcOp, PatternRewriter &rewriter) const override
    {

        // Check input funcOp for validity.
        // 1. Was not already transformed to iterative version
        if (funcOp->hasAttr("transformed_to_iterative")) return llvm::failure();

        // Is actual recursive function and was categorized as tail call recursion in previous step.
        ModuleOp moduleOp = funcOp->getParentOfType<ModuleOp>();
        SmallVector<Operation*> recursiveCalls;
        if(getRecursiveCalls(funcOp, recursiveCalls).failed()) return llvm::failure();
        auto callOp = llvm::cast<func::CallOp>(recursiveCalls.front());
        auto calleeAttr = callOp.getCalleeAttr();
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

        IRMapping resultMapping;
        IRMapping toPreheader;
        IRMapping toAfter;

        // !!! FOR NOW RECURSION WITH ONE CALL.
        auto ifAndBody = getTopMostIfAndWhileBody(funcOp, callOp);
        auto topMostIf = ifAndBody.first;
        auto whileBody = ifAndBody.second;

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
            toPreheader.map(from, to);
        }

        SmallVector<Operation*> postbody;
        SmallVector<Operation*> postbody_original;

        // Write Preheader and prepare same set of operations for postbody.
        rewriter.setInsertionPointToStart(entryBlock);
        funcOp->walk([&](Operation* op) {
            bool isBefore = isBeforeInOp(op, topMostIf.getOperation());
            if (isBefore) {
                auto* clone = rewriter.clone(*op, toPreheader);
                toPreheader.map(op->getResults(), clone->getResults());
                if (!isInvariant(funcOp, clone)) {
                    postbody_original.push_back(op);
                    postbody.push_back(clone);
                }
            }
        });

        SmallVector<Value> lcvs;
        if (findLCVs(funcOp, topMostIf, whileBody, lcvs).failed()) return llvm::failure();

        // Always pass updated condition to next iteration/ check before first iteration.
        auto cond = topMostIf.getCondition();
        lcvs.push_back(toPreheader.lookup(cond));

        /* Calculate initial values that will be passed to the whileOp. These are either block args
         * or results from the prehead.
         */
        SmallVector<Value> initValues;
        SmallVector<Type> initTypes;
        SmallVector<Location> initLocs;
        for (auto value : lcvs) {
            auto mappedValue = toPreheader.lookupOrDefault(value);
            initValues.push_back(mappedValue);
            initTypes.push_back(mappedValue.getType());
            initLocs.push_back(mappedValue.getLoc());
        }

        auto whileOp = rewriter.create<scf::WhileOp>(funcOp->getLoc(), initTypes, initValues);

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
            initLocs);

        /*
         *  Fill BEFORE block
         */
        rewriter.setInsertionPointToStart(whileOp.getBeforeBody());
        rewriter.create<scf::ConditionOp>(
            funcOp->getLoc(),
            beforeBlock->getArguments().back(),
            beforeBlock->getArguments());

        // Map from old lcv onto After-BlockArgs
        for (auto [from, to] : llvm::zip_equal(initValues, afterBlock->getArguments()))
            toAfter.map(from, to);
        toAfter = merge_value_map(toPreheader, toAfter);

        // AFTER
        scf::YieldOp whileYield;
        if (fillAfterBlock(
                afterBlock,
                entryBlock,
                whileBody,
                whileOp,
                lcvs,
                postbody,
                postbody_original,
                toAfter,
                rewriter,
                funcOp,
                whileYield)
                .failed())
            return llvm::failure(); // Termination

        /*
         * Provide mapping afterblock blockargs->whileOp results.
         * Why do we need?
         * after.blockArg -> whileOp.results
         * For the case that termination yield returns one of the LCVs we can use the mapping
         * yieldRes = oldLCV
         * toPrehead: oldLCV -> initValue
         * toAfter:             initValue -> afterBlockArgs (transitive mapping exists bc merge)
         * resultMapping:                    afterBlockArgs -> whileOpResult
         *
         * WhileOpResult ofc does not return afterBlockArgs but the PostBodyLCV, but postbodyLCVs
         * are mapped onto the blockargs manually in the block above.
         * HOW CAN I SHOW THIS FORMALLY? WHY IS whileYield -> whileop not enough?
         */
        for (auto [from, to] : llvm::zip_equal(afterBlock->getArguments(), whileOp->getResults()))
            resultMapping.map(from, to);

        /*
         * Provide mapping afterblock results -> whileOp results.
         */
        for (auto [from, to] : llvm::zip_equal(whileYield->getOperands(), whileOp->getResults()))
            resultMapping.map(from, to);

        Operation* returnOp = nullptr;
        for (auto op : funcOp.getOps<func::ReturnOp>()) {
            if (op->getParentOp() == funcOp && !returnOp)
                returnOp = op.getOperation();
            else if (op->getParentOp() == funcOp) {
                emitError(funcOp->getLoc(), "Two returns at in direct region of func.func.\n");
                return llvm::failure();
            }
        }

        if (!returnOp) {
            emitError(funcOp->getLoc(), "Could not find returnOp of funcOp");
            return llvm::failure();
        }

        // Copy termination branch to after whileop.
        rewriter.setInsertionPointAfter(whileOp);
        Region* terminationBranch = &topMostIf.getThenRegion() == whileBody
                                        ? &topMostIf.getElseRegion()
                                        : &topMostIf.getThenRegion();

        terminationBranch->walk([&](Operation* op) {
            if (isa<scf::YieldOp>(op) && op->getParentOp() == topMostIf) {

                /*
                 * If the walk encountered the final yieldOp of the termination branch. i.e. yieldOp
                 * nested directly in the region of topMostIf, the end of the function is reached.
                 * We still need to provide a mapping from ifRes, that might have been returned in\
                 * recursive version to the corresponding result of the whileOp.
                 */
                auto yield = cast<scf::YieldOp>(op);
                for (auto [ifRes, yieldRes] :
                     llvm::zip_equal(topMostIf.getResults(), yield.getResults())) {

                    // Lookup new "yieldRes" in iterative version
                    auto mappedValue = toAfter.lookup(yieldRes);

                    // replace a possible return r1,...,ifRes,..,r_n with y1,...,mappedValue,..,y_n
                    // or rather just provide the mapping
                    map_transitive(resultMapping, ifRes, mappedValue);
                }

                /*
                 * merge the remaining two maps in case a value from the prehead is part of the
                 * return values.
                 */
                resultMapping = merge_value_map(toAfter, resultMapping);
                rewriter.clone(*returnOp, resultMapping);
            } else {
                /*
                 *  In case the termination yield is not yet reached just clone the remaining ops
                 *  and provide the corresponding map.
                 */
                auto clonedOp = rewriter.clone(*op, resultMapping);
                resultMapping.map(op->getResults(), clonedOp->getResults());
            }
        });

        rewriter.modifyOpInPlace(funcOp, [&]() {
            funcOp->setAttr("transformed_to_iterative", rewriter.getUnitAttr());
        });

        IRMapping newToOld;
        funcOp.eraseBody();
        auto newEntry = funcOp.addEntryBlock();
        newToOld.map(entryBlock->getArguments(), newEntry->getArguments());
        rewriter.setInsertionPointToStart(newEntry);
        funcOp.getBody().takeBody(iterativeFunc.getBody());
        rewriter.eraseOp(iterativeFunc);
        if (failed(moduleOp.verify())) moduleOp.emitError("Verification failed");
        return llvm::success();
    }
};

struct TailRecursionPass : public mlir::sigi::impl::TailRecursionPassBase<TailRecursionPass> {

    void runOnOperation() override
    {
        func::FuncOp operation = getOperation();
        SmallVector<Operation*> recCalls;
        if (getRecursiveCalls(operation, recCalls).failed()) return;

        for(auto call : recCalls) {
            if(isTailCall(operation.getOperation(), call).failed()) return;
        }

        RewritePatternSet patterns(&getContext());
        GreedyRewriteConfig greedyConf;
        greedyConf.strictMode = GreedyRewriteStrictness::ExistingOps;
        patterns.add<ConstructIterativeVersion>(&getContext());
        (void)applyOpPatternsAndFold({operation}, std::move(patterns), greedyConf);
    }
};
} // namespace
