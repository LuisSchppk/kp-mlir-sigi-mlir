#include "sigi-mlir/Dialect/Closure/IR/ClosureDialect.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureOps.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <list>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/MapVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/IntrinsicsNVPTX.h>
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
#include <mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlow.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/Dialect/LLVMIR/Transforms/AddComdats.h>
#include <mlir/Dialect/MemRef/IR/MemRef.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/IR/AsmState.h>
#include <mlir/IR/Block.h>
#include <mlir/IR/BlockSupport.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/Diagnostics.h>
#include <mlir/IR/Dominance.h>
#include <mlir/IR/IRMapping.h>
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include <mlir/IR/Location.h>
#include <mlir/IR/OpDefinition.h>
#include <mlir/IR/Operation.h>
#include <mlir/IR/OperationSupport.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/Region.h>
#include <mlir/IR/SymbolTable.h>
#include <mlir/IR/TypeRange.h>
#include <mlir/IR/TypeUtilities.h>
#include <mlir/IR/Types.h>
#include <mlir/IR/Value.h>
#include <mlir/IR/ValueRange.h>
#include <mlir/IR/Visitors.h>
#include <mlir/Interfaces/ControlFlowInterfaces.h>
#include <mlir/Interfaces/LoopLikeInterface.h>
#include <mlir/Interfaces/SideEffectInterfaces.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Support/LLVM.h>
#include <mlir/Transforms/GreedyPatternRewriteDriver.h>
#include <ostream>
#include <set>
#include <strings.h>
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

LogicalResult isTailCall(Operation* op)
{
    auto lastResult = op->getResults();
    auto next = op->getNextNode();
    while (!llvm::isa<func::ReturnOp>(next)) {
        if ((next->hasTrait<OpTrait::ReturnLike>() || next->hasTrait<OpTrait::IsTerminator>())
            && !llvm::isa<LoopLikeOpInterface>(next->getParentOp())
            && llvm::isa<scf::SCFDialect>(next->getParentOp()->getDialect())
            && llvm::equal(next->getOperands(), lastResult)) {
            auto parentOp = next->getParentOp();
            lastResult = parentOp->getResults();
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
    LLVM_DEBUG(
        llvm::errs() << "Tail Call analysis done: Op at " << op->getLoc() << " is a Tail Call\n");
    return llvm::success();
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

namespace {

LogicalResult checkInputFunc(func::FuncOp &funcOp, SmallVector<Operation*> recursiveCalls)
{
    // Check input funcOp for validity.
    // 1. Was not already transformed to iterative version
    if (funcOp->hasAttr("transformed_to_iterative")) return llvm::failure();

    // Is actual recursive function and was categorized as tail call recursion in previous step.
    ModuleOp moduleOp = funcOp->getParentOfType<ModuleOp>();
    auto callOp = llvm::cast<func::CallOp>(recursiveCalls.front());
    auto calleeAttr = callOp.getCalleeAttr();
    SmallVector<Operation*> sym;
    if (!moduleOp) {
        emitError(funcOp->getLoc(), "Could not find moduleOp during ConstructIterativeVersion.");
        return llvm::failure();
    } else if (SymbolTable::lookupSymbolIn(moduleOp, calleeAttr, sym).failed()) {
        emitError(funcOp->getLoc(), "Could not find funcOp during ConstructIterativeVersion.");
        return llvm::failure();
    } else if (moduleOp.lookupSymbol(calleeAttr) != funcOp) {
        // Not a recursive call.
        return llvm::failure();
    }
    return llvm::success();
}

struct CFConstructIterativeVersion : OpRewritePattern<func::FuncOp> {

public:
    using OpRewritePattern<func::FuncOp>::OpRewritePattern;

    LogicalResult matchAndRewrite(func::FuncOp funcOp, PatternRewriter &rewriter) const override
    {

        SmallVector<Operation*> recursiveCalls;
        ModuleOp moduleOp = funcOp->getParentOfType<ModuleOp>();

        if (getRecursiveCalls(funcOp, recursiveCalls).failed()) return llvm::failure();
        if (checkInputFunc(funcOp, recursiveCalls).failed()) return llvm::failure();
        auto* entryBlock = &funcOp.getBlocks().front();
        auto* newEntry = new Block;
        TypeRange argType = entryBlock->getArgumentTypes();
        SmallVector<Location> argLoc;
        for (auto arg : entryBlock->getArguments()) argLoc.push_back(arg.getLoc());
        newEntry->addArguments(argType, argLoc);
        newEntry->insertBefore(entryBlock);
        rewriter.setInsertionPointToStart(newEntry);
        rewriter.create<cf::BranchOp>(funcOp->getLoc(), entryBlock, newEntry->getArguments());
        assert(newEntry->isEntryBlock() && "ADDING NEW ENTRY FAILED.");

        if (checkInputFunc(funcOp, recursiveCalls).failed()) return llvm::failure();
        for (auto call : recursiveCalls) {
            auto callOp = cast<func::CallOp>(call);
            auto oldBr = call->getNextNode();
            assert(
                isa<cf::BranchOp>(oldBr)
                && "Node directly after rec. call is not a branchOp. I.e. no tail call.");

            rewriter.setInsertionPoint(call);
            auto newBr =
                rewriter.create<cf::BranchOp>(funcOp->getLoc(), entryBlock, callOp.getOperands());
            rewriter.replaceOp(oldBr, newBr);
            rewriter.eraseOp(callOp);
        }

        rewriter.modifyOpInPlace(funcOp, [&]() {
            funcOp->setAttr("transformed_to_iterative", rewriter.getUnitAttr());
        });

        if (failed(moduleOp.verify())) {
            moduleOp.emitError("Verification failed");
            return llvm::failure();
        };
        return llvm::success();
    }

private:
};

struct ConstructIterativeVersion : OpRewritePattern<func::FuncOp> {
private:
    std::string terminationYieldAttr = "terminationYield";
    std::string callYieldAttr = "CallYield";

public:
    using OpRewritePattern<func::FuncOp>::OpRewritePattern;

    bool isInvariant(func::FuncOp funcOp, Operation* op) const
    {
        bool constant = op->hasTrait<OpTrait::ConstantLike>();
        // TODO case reaching Def outside loop
        // TODO recursive: at most one reaching def from inside that loop, that also is invariant
        return constant;
    }

    LogicalResult getTopMostIfAndProtoBody(
        func::FuncOp funcOp,
        func::CallOp callOp,
        scf::IfOp &topMostIf,
        Region*&proto_body,
        bool &hasNestedIfs) const
    {
        topMostIf = callOp->getParentOfType<scf::IfOp>();
        bool isTopMost = topMostIf->getParentOp() == funcOp;
        proto_body = callOp->getParentRegion();
        hasNestedIfs = topMostIf && !isTopMost;
        while (topMostIf && !isTopMost) {
            topMostIf = topMostIf->getParentOfType<scf::IfOp>();
            proto_body = &topMostIf.getThenRegion() == proto_body->getParentRegion()
                             ? &topMostIf.getThenRegion()
                             : &topMostIf.getElseRegion();
            isTopMost = topMostIf->getParentOp() == funcOp;
        }

        if (topMostIf->isAncestor(callOp) && proto_body)
            return llvm::success();
        else
            return llvm::failure();
    }

    LogicalResult
    findLCVs(func::FuncOp funcOp, Operation* border, Region* proto_body, SmallVector<Value> &lcvs)
        const
    {
        SmallVector<Value> defsHead;
        SmallVector<Value> usesBody;

        proto_body->walk([&](Operation* op) {
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

    LogicalResult constructSimpleTermination(
        Region* terminationBranch,
        scf::IfOp topMostIf,
        IRMapping &resultMapping,
        PatternRewriter &rewriter) const
    {
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
                    auto mappedValue = resultMapping.lookup(yieldRes);

                    // replace a possible return r1,...,ifRes,..,r_n with y1,...,mappedValue,..,y_n
                    // or rather just provide the mapping
                    map_transitive(resultMapping, ifRes, mappedValue);
                }
            } else {
                /*
                 *  In case the termination yield is not yet reached just clone the remaining ops
                 *  and provide the corresponding map.
                 */
                auto clonedOp = rewriter.clone(*op, resultMapping);
                resultMapping.map(op->getResults(), clonedOp->getResults());
            }
        });

        return llvm::success();
    }

    LogicalResult eliminateRecursiveCalls(
        Block* afterBody,
        PatternRewriter &rewriter,
        func::FuncOp &funcOp,
        bool hasNestedIfs,
        ArrayRef<BlockArgument> blockArgs,
        IRMapping &toAfter,
        SmallVector<Operation*> &newYields) const
    {
        SmallVector<std::pair<Operation*, Operation*>> eliminate;
        ModuleOp moduleOp = funcOp->getParentOfType<ModuleOp>();
        afterBody->walk([&](func::CallOp callOp) {
            // in isTail we checked, that the next node is a Returnlike or terminatorLike.
            // HERE WE JUST ASSUME THAT THIS TERMINATOR/RETURNLIKE IS A YIELDOP BY VIRTUE OF
            // ASSUMPTION ALL CONTROLFLOW IS SCF.
            if (moduleOp.lookupSymbol(callOp.getCalleeAttr()) == funcOp) {
                auto yield = callOp->getNextNode();
                assert(llvm::isa<scf::YieldOp>(yield) && "CallOp not followed by yieldOp!");
                eliminate.push_back({callOp, yield});
            }
        });

        for (auto [recCall, yield] : eliminate) {
            auto callOp = cast<func::CallOp>(recCall);
            toAfter.map(blockArgs, callOp.getOperands());
            rewriter.setInsertionPoint(yield);
            SmallVector<Value> newResults = {
                callOp.getOperands().begin(),
                callOp.getOperands().end()};

            // todo do this with idx.
            if (hasNestedIfs) {
                auto lastVal = afterBody->getArguments().back();
                newResults.push_back(lastVal);
            }
            auto newYield = rewriter.create<scf::YieldOp>(funcOp->getLoc(), newResults);
            rewriter.replaceOp(yield, newYield);
            newYields.push_back(newYield);
            rewriter.eraseOp(recCall);
        }
        return llvm::success();
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

    void adjustIfOpResultType(
        func::FuncOp funcOp,
        scf::YieldOp &callYield,
        IRMapping toAfter,
        PatternRewriter &rewriter) const
    {
        IRMapping yieldToIf;
        scf::IfOp newIf = NULL;
        if (auto ifOp = llvm::dyn_cast<scf::IfOp>(callYield->getParentOp())) {
            rewriter.modifyOpInPlace(callYield, [&]() {
                if (callYield->hasAttr(terminationYieldAttr))
                    callYield->removeAttr(terminationYieldAttr);
                callYield->setAttr(callYieldAttr, rewriter.getUnitAttr());
            });

            auto other = ifOp.thenYield() == callYield ? ifOp.elseYield() : ifOp.thenYield();
            rewriter.modifyOpInPlace(other, [&]() {
                if (!other->hasAttr(callYieldAttr))
                    other->setAttr(terminationYieldAttr, rewriter.getUnitAttr());
            });
            auto callIdx = ifOp.thenYield() == callYield ? 0 : 1;
            if (!llvm::equal(callYield->getOperandTypes(), ifOp->getOperandTypes())) {
                rewriter.setInsertionPointAfter(ifOp);
                newIf = rewriter.create<scf::IfOp>(
                    funcOp->getLoc(),
                    callYield->getOperandTypes(),
                    ifOp.getCondition(),
                    true,
                    true);

                callYield->getParentRegion()->cloneInto(&newIf->getRegion(callIdx), toAfter);
                other->getParentRegion()->cloneInto(&newIf->getRegion((callIdx + 1) % 2), toAfter);
                rewriter.eraseBlock(&newIf->getRegion(callIdx).front());
                rewriter.eraseBlock(&newIf->getRegion((callIdx + 1) % 2).front());
                rewriter.eraseBlock(other->getBlock());
                rewriter.eraseBlock(callYield->getBlock());
            }
            auto yield = ifOp->getNextNode();
            while (!llvm::isa<scf::YieldOp>(yield)) yield = yield->getNextNode();
            rewriter.setInsertionPointAfter(yield);

            if (newIf) {
                auto newYield =
                    rewriter.create<scf::YieldOp>(funcOp->getLoc(), newIf->getResults());
                rewriter.replaceOp(yield, newYield);
                callYield = newYield;
                rewriter.eraseOp(ifOp);
            } else {
                callYield = llvm::cast<scf::YieldOp>(yield);
            }
        }
    }

    LogicalResult resultAllocations(
        func::FuncOp iterativeFunc,
        PatternRewriter &rewriter,
        SmallVector<Operation*> &allocations,
        SmallVector<unsigned> &order,
        DenseMap<unsigned, Type> &idxToType) const
    {
        order.clear();
        allocations.clear();
        SetVector<Type> distinctTypes = {
            iterativeFunc.getFunctionType().getResults().begin(),
            iterativeFunc.getFunctionType().getResults().end()};
        DenseMap<Type, unsigned> buckets;
        DenseMap<Type, unsigned> indices;
        ;
        int counter = 0;
        for (auto type : distinctTypes) {
            buckets.insert({type, 0});
            indices.insert({type, counter});
            idxToType.insert({counter, type});
            ++counter;
        }

        for (auto type : iterativeFunc.getFunctionType().getResults()) {
            buckets[type]++;
            order.push_back(indices[type]);
        }

        rewriter.setInsertionPointToStart(&iterativeFunc.getBlocks().front());
        for (auto [type, size] : buckets) {
            auto memRefType = MemRefType::get(size, type);
            auto allocation = rewriter.create<memref::AllocOp>(iterativeFunc->getLoc(), memRefType);
            allocations.push_back(allocation);
        }
        return llvm::success();
    }

    LogicalResult constructWhileOp(
        func::FuncOp iterativeFunc,
        PatternRewriter &rewriter,
        SmallVector<Value> &lcvs,
        scf::IfOp topMostIf,
        IRMapping &toPreheader,
        SmallVector<Value> &initValues,
        bool hasNestedIfs,
        std::pair<int, int> &idx_termination_flags,
        scf::WhileOp &whileOp) const
    {
        initValues.clear();

        // Always pass updated condition to next iteration/ check before first iteration.
        auto cond = topMostIf.getCondition();
        idx_termination_flags.first = lcvs.size();
        lcvs.push_back(toPreheader.lookup(cond));

        if (hasNestedIfs) {
            auto boolType = rewriter.getIntegerType(1);
            auto break_flag = rewriter.create<arith::ConstantOp>(
                iterativeFunc->getLoc(),
                boolType,
                rewriter.getIntegerAttr(boolType, 0));
            idx_termination_flags.second = lcvs.size();
            lcvs.push_back(break_flag.getResult());
        } else {
            idx_termination_flags.second = -1;
        }

        /* Calculate initial values that will be passed to the whileOp. These are either block args
         * or results from the prehead.
         */
        SmallVector<Type> initTypes;
        SmallVector<Location> initLocs;
        for (auto value : lcvs) {
            auto mappedValue = toPreheader.lookupOrDefault(value);
            initValues.push_back(mappedValue);
            initTypes.push_back(mappedValue.getType());
            initLocs.push_back(mappedValue.getLoc());
        }

        whileOp = rewriter.create<scf::WhileOp>(iterativeFunc->getLoc(), initTypes, initValues);

        // Create Bodies for Before And After Region.
        rewriter
            .createBlock(&whileOp.getBefore(), whileOp.getBefore().begin(), initTypes, initLocs);
        rewriter.createBlock(&whileOp.getAfter(), whileOp.getAfter().begin(), initTypes, initLocs);
        return llvm::success();
    }

    LogicalResult writePostbody(
        scf::WhileOp &whileOp,
        SmallVector<Value> &initValues,
        SmallVector<Operation*> postbodyOps,
        IRMapping &toAfter,
        PatternRewriter &rewriter,
        func::FuncOp funcOp,
        scf::YieldOp &whileYield,
        std::pair<int, int> idx_termination_flags) const
    {

        // "POSTBODY"
        rewriter.setInsertionPointToEnd(whileOp.getAfterBody());

        SmallVector<Value> postbodyLCVs;
        Operation* finalYield;
        postbodyLCVs.resize(initValues.size());

        if (getFinalYield(whileOp.getAfterBody(), funcOp, finalYield).failed())
            return llvm::failure();
        for (size_t i = 0; i < finalYield->getOperands().size(); i++) {
            auto yieldOperand = finalYield->getOperands()[i];
            auto mappedValue = (yieldOperand);
            postbodyLCVs[i] = mappedValue;
        }

        postbodyLCVs.back() = toAfter.lookupOrDefault(finalYield->getOperands().back());

        for (auto op : postbodyOps) {
            auto* clone = rewriter.clone(*op, toAfter);
            toAfter.map(op->getResults(), clone->getResults());

            for (auto result : clone->getResults()) {
                for (size_t i = 0; i < initValues.size(); i++) {
                    auto val = initValues[i];
                    auto mappedLCV = toAfter.lookupOrDefault(val);
                    if (mappedLCV == result) {
                        postbodyLCVs[i] = result;
                    }
                }
            }
        }

        if (idx_termination_flags.second != -1) {
            arith::CmpIOp termination = rewriter.create<arith::CmpIOp>(
                funcOp->getLoc(),
                rewriter.getIntegerType(1),
                arith::CmpIPredicate::sgt,
                postbodyLCVs[idx_termination_flags.first],
                postbodyLCVs[idx_termination_flags.second]);
            postbodyLCVs[idx_termination_flags.first] = termination.getResult();
        }

        whileYield = rewriter.create<scf::YieldOp>(funcOp->getLoc(), postbodyLCVs);
        rewriter.modifyOpInPlace(whileOp, [&]() {
            whileYield->setAttr(callYieldAttr, rewriter.getUnitAttr());
        });
        rewriter.eraseOp(finalYield);
        return llvm::success();
    }

    LogicalResult determineMultiTerminationBehaviour(
        func::FuncOp funcOp,
        TypeRange availableTypes,
        SetVector<unsigned> &resultIdxs,
        bool &hijackOperands) const
    {
        hijackOperands = true;
        resultIdxs.clear();
        SmallVector<bool> occupied;
        for (size_t i = 0; i < availableTypes.size() - 2; i++) occupied.push_back(false);

        for (auto res : funcOp.getFunctionType().getResults()) {
            bool next = false;
            for (size_t i = 0; i < availableTypes.size() - 2;
                 i++) // disallow last two indices -> flag.
                if (!next && res == availableTypes[i] && !occupied[i]) {
                    hijackOperands = resultIdxs.insert(i);
                    occupied[i] = true;
                    next = true;
                };
        }
        if (!hijackOperands)
            resultIdxs.clear();
        else
            assert(
                resultIdxs.size() == funcOp.getFunctionType().getNumResults()
                && "Can not fit result into return types.");
        return llvm::success();
    }

    LogicalResult matchAndRewrite(func::FuncOp funcOp, PatternRewriter &rewriter) const override
    {

        SmallVector<Operation*> recursiveCalls;
        ModuleOp moduleOp = funcOp->getParentOfType<ModuleOp>();
        if (getRecursiveCalls(funcOp, recursiveCalls).failed()) return llvm::failure();
        if (checkInputFunc(funcOp, recursiveCalls).failed()) return llvm::failure();
        IRMapping resultMapping;
        IRMapping toPreheader;
        IRMapping toAfter;

        // Create new Function Skeleton
        rewriter.setInsertionPointAfter(funcOp);
        auto iterativeFunc = rewriter.create<func::FuncOp>(
            funcOp->getLoc(),
            funcOp.getNameAttr().str() + "It",
            funcOp.getFunctionType());
        auto entryBlock = iterativeFunc.addEntryBlock();

        // !!! FOR NOW RECURSION WITH ONE CALL.
        auto call = recursiveCalls.front();
        auto callOp = cast<func::CallOp>(call);
        scf::IfOp topMostIf;
        Region* proto_body = nullptr;
        bool hasNestedIfs;
        if (getTopMostIfAndProtoBody(funcOp, callOp, topMostIf, proto_body, hasNestedIfs).failed())
            return llvm::failure();

        SmallVector<Value> lcvs;
        if (findLCVs(funcOp, topMostIf, proto_body, lcvs).failed()) return llvm::failure();

        // Map from old BlockArgs to new BlockArgs
        for (auto [from, to] : llvm::zip_equal(
                 funcOp.getBlocks().front().getArguments(),
                 entryBlock->getArguments())) {
            toPreheader.map(from, to);
        }

        // Write Preheader and prepare same set of operations for postbody.
        SmallVector<Operation*> postbodyOps;

        rewriter.setInsertionPointToStart(entryBlock);
        funcOp->walk([&](Operation* op) {
            bool isBefore = isBeforeInOp(op, topMostIf.getOperation());
            if (isBefore) {
                auto* clone = rewriter.clone(*op, toPreheader);
                toPreheader.map(op->getResults(), clone->getResults());
                if (!isInvariant(funcOp, clone)) {
                    postbodyOps.push_back(clone);
                }
            }
        });

        // Construct WhileOp
        scf::WhileOp whileOp;
        SmallVector<Value> initValues;
        std::pair<int, int> idx_termination_flags;
        if (constructWhileOp(
                iterativeFunc,
                rewriter,
                lcvs,
                topMostIf,
                toPreheader,
                initValues,
                hasNestedIfs,
                idx_termination_flags,
                whileOp)
                .failed())
            return llvm::failure();

        /*
         *  Fill BEFORE block
         */
        rewriter.setInsertionPointToStart(whileOp.getBeforeBody());
        rewriter.create<scf::ConditionOp>(
            funcOp->getLoc(),
            whileOp.getBeforeBody()->getArgument(idx_termination_flags.first),
            whileOp.getBeforeBody()->getArguments());

        // AFTER BLOCK:

        // Map from old lcv onto After-BlockArgs
        for (auto [from, to] : llvm::zip_equal(initValues, whileOp.getAfterBody()->getArguments()))
            toAfter.map(from, to);
        toAfter = merge_value_map(toPreheader, toAfter);

        scf::YieldOp whileYield;

        // clone protobody to while body.
        rewriter.setInsertionPointToStart(whileOp.getAfterBody());
        for (auto &op : proto_body->getOps()) {
            auto clone = rewriter.clone(op, toAfter);
            toAfter.map(op.getResults(), clone->getResults());
        }

        // Eliminate recursive calls in cloned operations.
        SmallVector<Operation*> newYields;
        if (eliminateRecursiveCalls(
                whileOp.getAfterBody(),
                rewriter,
                funcOp,
                hasNestedIfs,
                entryBlock->getArguments(),
                toAfter,
                newYields)
                .failed())
            return llvm::failure();

        // Adjust scf:IfOps to match new return types-> yield operands instead of call results.
        scf::YieldOp current = llvm::cast<scf::YieldOp>(newYields[0]);
        while (current.getOperation()->getParentOp() != whileOp)
            adjustIfOpResultType(funcOp, current, toAfter, rewriter);

        if (writePostbody(
                whileOp,
                initValues,
                postbodyOps,
                toAfter,
                rewriter,
                funcOp,
                whileYield,
                idx_termination_flags)
                .failed())
            return llvm::failure();

        // HANDLE MULTITERMINATION
        SetVector<unsigned> resultIdxs;
        SmallVector<Operation*> allocations;
        SmallVector<unsigned> resultOrder;
        DenseMap<unsigned, Type> idxToType;

        bool hijackOperands;

        if (hasNestedIfs) {
            if (determineMultiTerminationBehaviour(
                    iterativeFunc,
                    iterativeFunc.getFunctionType().getInputs(),
                    resultIdxs,
                    hijackOperands)
                    .failed())
                return llvm::failure();
            // hijackOperands = false;
            if (!hijackOperands) {
                if (resultAllocations(iterativeFunc, rewriter, allocations, resultOrder, idxToType)
                        .failed())
                    return llvm::failure();
            }
        }

        LogicalResult success = llvm::success();
        SmallVector<Operation*> markForErasure;
        whileOp.getAfterBody()->walk([&](scf::YieldOp yield) {
            if (yield->hasAttr(terminationYieldAttr)) {
                if (llvm::equal(yield->getResultTypes(), iterativeFunc.getResultTypes())) {
                    emitError(
                        funcOp->getLoc(),
                        "Terminating yield found that does not return parent funcs return types.");
                    success = llvm::failure();
                }

                SmallVector<Value> hijackedResults;
                for (auto val : recursiveCalls.front()->getOperands()) {
                    bool next = false;
                    for (auto arg : whileOp.getAfterBody()->getArguments()) {
                        if (!next && arg.getType() == val.getType()) {
                            hijackedResults.push_back(arg);
                            next = true;
                        }
                    }
                }

                rewriter.setInsertionPoint(yield);
                if (hijackOperands) {
                    for (auto [operand, idx] : llvm::zip_equal(yield->getOperands(), resultIdxs))
                        hijackedResults[idx] = operand;
                } else {
                    SmallVector<Value> yieldOperands = yield->getOperands();
                    DenseMap<Type, unsigned> counters;
                    SetVector<Type> distinctTypes = {
                        iterativeFunc->result_type_begin(),
                        iterativeFunc->result_type_end()};

                    for (auto type : distinctTypes) counters.insert({type, 0});

                    for (auto [result, idx] : llvm::zip_equal(yield->getOperands(), resultOrder)) {
                        auto alloc = cast<memref::AllocOp>(allocations[idx]);
                        auto type = result.getType();
                        auto memref = alloc.getMemref();
                        auto index = counters[type]++;
                        auto indexOp =
                            rewriter.create<arith::ConstantIndexOp>(iterativeFunc->getLoc(), index);
                        rewriter.create<memref::StoreOp>(
                            iterativeFunc->getLoc(),
                            result,
                            memref,
                            indexOp.getResult());
                    }
                }

                auto boolType = rewriter.getIntegerType(1);
                auto break_flag = rewriter.create<arith::ConstantOp>(
                    iterativeFunc->getLoc(),
                    boolType,
                    rewriter.getIntegerAttr(boolType, 1));
                hijackedResults.push_back(break_flag.getResult());
                auto newYield = rewriter.create<scf::YieldOp>(yield->getLoc(), hijackedResults);
                rewriter.modifyOpInPlace(newYield, [&]() {
                    newYield->setAttr(terminationYieldAttr, rewriter.getUnitAttr());
                });
                markForErasure.push_back(yield);
            }
        });
        if (success.failed()) return llvm::failure();
        for (auto yield : markForErasure) rewriter.eraseOp(yield);

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
        for (auto [from, to] :
             llvm::zip_equal(whileOp.getAfterBody()->getArguments(), whileOp->getResults()))
            resultMapping.map(from, to);

        /*
         * Provide mapping afterblock results -> whileOp results.
         */
        // for (auto [from, to] : llvm::zip_equal(whileYield->getOperands(), whileOp->getResults()))
        //     resultMapping.map(from, to);

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

        /*
         * merge the remaining two maps in case a value from the prehead is part of the
         * return values.
         */
        resultMapping = merge_value_map(toAfter, resultMapping);

        Region* terminationBranch = &topMostIf.getThenRegion() == proto_body
                                        ? &topMostIf.getElseRegion()
                                        : &topMostIf.getThenRegion();

        rewriter.setInsertionPointAfter(whileOp);
        if (hasNestedIfs) {
            auto termination_if = rewriter.create<scf::IfOp>(
                iterativeFunc->getLoc(),
                iterativeFunc.getFunctionType().getResults(),
                whileOp->getResults()[idx_termination_flags.second],
                true,
                true);

            rewriter.setInsertionPointToStart(&termination_if.getElseRegion().front());
            SmallVector<Value> finalReturn;
            if (constructSimpleTermination(terminationBranch, topMostIf, resultMapping, rewriter)
                    .failed())
                return llvm::failure();

            for (auto returnVal : returnOp->getOperands())
                finalReturn.push_back(resultMapping.lookup(returnVal));
            rewriter.create<scf::YieldOp>(iterativeFunc->getLoc(), finalReturn);

            finalReturn.clear();
            rewriter.setInsertionPointToStart(&termination_if.getThenRegion().front());
            if (hijackOperands) {
                for (auto idx : resultIdxs) finalReturn.push_back(whileOp->getResults()[idx]);
            } else {

                DenseMap<unsigned, unsigned> counters;
                auto max = llvm::max_element(resultOrder);
                for (unsigned i = 0; i < *max; i++) counters[i] = 0;
                for (auto idx : resultOrder) {
                    auto alloc = cast<memref::AllocOp>(allocations[idx]);
                    auto memref = alloc.getMemref();
                    auto index = counters[idx]++;
                    auto indexOp =
                        rewriter.create<arith::ConstantIndexOp>(iterativeFunc->getLoc(), index);
                    auto result = rewriter.create<memref::LoadOp>(
                        iterativeFunc->getLoc(),
                        idxToType[idx],
                        memref,
                        indexOp.getResult());
                    finalReturn.push_back(result.getResult());
                }
            }
            rewriter.create<scf::YieldOp>(iterativeFunc->getLoc(), finalReturn);

            rewriter.setInsertionPointAfter(termination_if);
            rewriter.create<func::ReturnOp>(iterativeFunc->getLoc(), termination_if->getResults());
        } else {
            if (constructSimpleTermination(terminationBranch, topMostIf, resultMapping, rewriter)
                    .failed())
                return llvm::failure();
            rewriter.clone(*returnOp, resultMapping);
        }

        rewriter.setInsertionPoint(entryBlock->getTerminator());
        for (auto op : allocations) {
            auto allocation = cast<memref::AllocOp>(op);
            rewriter.create<memref::DeallocOp>(iterativeFunc->getLoc(), allocation.getResult());
        }

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
        LLVM_DEBUG(llvm::errs() << moduleOp);
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

        if (recCalls.size() > 1) {
            LLVM_DEBUG(
                llvm::errs() << "Currently only one recursive call is supported by this  Pass.\n");
            PassManager pm(&getContext());
            pm.addPass(createConvertSCFToCFPass());
            if (pm.run(operation).failed()) return;
            RewritePatternSet patterns(&getContext());
            GreedyRewriteConfig greedyConf;
            greedyConf.strictMode = GreedyRewriteStrictness::ExistingOps;
            patterns.add<CFConstructIterativeVersion>(&getContext());
            (void)applyOpPatternsAndFold({operation}, std::move(patterns), greedyConf);
            return;
        }

        for (auto call : recCalls)
            if (isTailCall(call).failed()) return;

        RewritePatternSet patterns(&getContext());
        GreedyRewriteConfig greedyConf;
        greedyConf.strictMode = GreedyRewriteStrictness::ExistingOps;
        patterns.add<ConstructIterativeVersion>(&getContext());
        (void)applyOpPatternsAndFold({operation}, std::move(patterns), greedyConf);
    }
};
} // namespace