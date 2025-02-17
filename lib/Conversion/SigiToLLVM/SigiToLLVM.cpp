#include "sigi-mlir/Conversion/SigiToLLVM/SigiToLLVM.h"

#include "../PassDetails.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "sigi-mlir/Conversion/ClosureToLLVM/ClosureToLLVM.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"
#include "sigi-mlir/Dialect/Closure/Transforms/ClosureConversionUtil.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"

#include <cassert>
#include <cstddef>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/LogicalResult.h>
#include <llvm/Support/raw_ostream.h>
#include <mlir/Conversion/ArithToLLVM/ArithToLLVM.h>
#include <mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h>
#include <mlir/Conversion/GPUToNVVM/GPUToNVVMPass.h>
#include <mlir/Conversion/LLVMCommon/Pattern.h>
#include <mlir/Conversion/LLVMCommon/TypeConverter.h>
#include <mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/Func/Transforms/FuncConversions.h>
#include <mlir/Dialect/LLVMIR/FunctionCallUtils.h>
#include <mlir/Dialect/LLVMIR/LLVMAttrs.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinAttributeInterfaces.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/Diagnostics.h>
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/Operation.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/Value.h>
#include <mlir/IR/ValueRange.h>
#include <mlir/IR/Visitors.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Transforms/DialectConversion.h>
#include <stdbool.h>
#include <string>

using namespace mlir;
using namespace mlir::sigi;

#define DEBUG_TYPE "SigiToLLVM"

namespace {
std::string sigi_global_stack = "sigi.global.stack";
static LLVM::LLVMPointerType ptrType(Type ty)
{
    return LLVM::LLVMPointerType::get(ty.getContext());
}

static LLVM::LLVMPointerType llvmStackStructType(MLIRContext* ctx)
{
    // Opaque Struct instead of simply identified -> Body should not be changed.
    return ptrType(LLVM::LLVMStructType::getOpaque("sigi_stack_impl", ctx));
}

static LLVM::LLVMFuncOp getSigiInitStack(ModuleOp moduleOp)
{
    ::mlir::MLIRContext* context = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "sigi_init_stack",
        llvmStackStructType(context),
        LLVM::LLVMVoidType::get(context));
}

static LLVM::LLVMFuncOp getSigiFreeStack(ModuleOp moduleOp)
{
    ::mlir::MLIRContext* context = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "sigi_free_stack",
        llvmStackStructType(context),
        LLVM::LLVMVoidType::get(context));
}

static LLVM::LLVMFuncOp getSigiPopBool(ModuleOp moduleOp)
{
    ::mlir::MLIRContext* context = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "sigi_pop_bool",
        llvmStackStructType(context),
        IntegerType::get(context, 1));
}

static LLVM::LLVMFuncOp getSigiPopI32(ModuleOp moduleOp)
{
    ::mlir::MLIRContext* context = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "sigi_pop_i32",
        llvmStackStructType(context),
        IntegerType::get(context, 32));
}

static LLVM::LLVMFuncOp getSigiPopClosure(ModuleOp moduleOp, Type resultType)
{
    mlir::MLIRContext* context = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "sigi_pop_closure",
        llvmStackStructType(context),
        ptrType(resultType));
}

static LLVM::LLVMFuncOp getSigiPushBool(ModuleOp moduleOp)
{
    mlir::MLIRContext* context = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "sigi_push_bool",
        {llvmStackStructType(context), IntegerType::get(context, 1)},
        LLVM::LLVMVoidType::get(context));
}

static LLVM::LLVMFuncOp getSigiPushI32(ModuleOp moduleOp)
{
    mlir::MLIRContext* context = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "sigi_push_i32",
        {llvmStackStructType(context), IntegerType::get(context, 32)},
        LLVM::LLVMVoidType::get(context));
}

static LLVM::LLVMFuncOp getSigiPushClosure(ModuleOp moduleOp, Type resultType)
{
    mlir::MLIRContext* context = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "sigi_push_closure",
        {llvmStackStructType(context), ptrType(resultType)},
        LLVM::LLVMVoidType::get(context));
}

struct ConvertSigiPopOpToLLVM : public ConvertOpToLLVMPattern<sigi::PopOp> {
    using ConvertOpToLLVMPattern<sigi::PopOp>::ConvertOpToLLVMPattern;

    LogicalResult matchAndRewrite(
        sigi::PopOp op,
        PopOpAdaptor adaptor,
        ConversionPatternRewriter &rewriter) const override
    {

        LLVM_DEBUG(llvm::errs() << "Convert PopOp " << op << "\n");
        ModuleOp moduleOp = op->getParentOfType<ModuleOp>();

        // Error might lie here? op.getValue is not yet converted -> for push
        // i can use adaptor, but here not
        auto valueType = op.getValue().getType();
        LLVM::LLVMFuncOp pop;
        if (valueType.isInteger(1)) {
            pop = getSigiPopBool(moduleOp);
        } else if (valueType.isInteger(32)) {
            pop = getSigiPopI32(moduleOp);
        } else if (
            llvm::isa<closure::BoxedClosureType>(valueType)
            || llvm::isa<LLVM::LLVMPointerType>(valueType)) {

            // if verifier for popOp is correct, then any invalid argument
            // should get marked down during parsing? check for pointer enough?
            pop = getSigiPopClosure(moduleOp, valueType);
        } else {
            return rewriter.notifyMatchFailure(op->getLoc(), [&](Diagnostic &diag) {
                diag << "Encountered element with invalid type " << valueType
                     << "during PopOp conversion.";
            });
        }

        LLVM::CallOp pop_call =
            rewriter.create<LLVM::CallOp>(op->getLoc(), pop, adaptor.getInStack());

        rewriter.replaceOp(op, {adaptor.getInStack(), pop_call.getResult()});
        return success();
    };
};

struct ConvertSigiPushOpToLLVM : public ConvertOpToLLVMPattern<sigi::PushOp> {
    using ConvertOpToLLVMPattern<sigi::PushOp>::ConvertOpToLLVMPattern;

    LogicalResult matchAndRewrite(
        sigi::PushOp op,
        PushOpAdaptor adaptor,
        ConversionPatternRewriter &rewriter) const override
    {
        LLVM_DEBUG(llvm::errs() << "Convert PushOp " << op << "\n");
        ModuleOp moduleOp = op->getParentOfType<ModuleOp>();
        auto valueType = adaptor.getValue().getType();
        LLVM::LLVMFuncOp push;
        if (valueType.isInteger(1)) {
            push = getSigiPushBool(moduleOp);
        } else if (valueType.isInteger(32)) {
            push = getSigiPushI32(moduleOp);
        } else if (
            llvm::isa<closure::BoxedClosureType>(valueType)
            || llvm::isa<LLVM::LLVMPointerType>(valueType)) {

            push = getSigiPushClosure(moduleOp, valueType);
        } else {
            return rewriter.notifyMatchFailure(op->getLoc(), [&](Diagnostic &diag) {
                diag << "Encountered element with invalid type " << valueType
                     << "during PopOp conversion.";
            });
        }

        rewriter.create<LLVM::CallOp>(op->getLoc(), push, adaptor.getOperands());
        rewriter.replaceOp(op, adaptor.getInStack());
        return success();
    };
};

LLVM::GlobalOp lookupOrCreateGlobalStack(Operation* op, ConversionPatternRewriter &rewriter)
{
    OpBuilder::InsertionGuard insertionGuard(rewriter);
    ModuleOp moduleOp = op->getParentOfType<ModuleOp>();
    auto context = op->getContext();
    LLVM::GlobalOp globalStackOp;
    if (!(globalStackOp = moduleOp.lookupSymbol<LLVM::GlobalOp>(sigi_global_stack))) {
        LLVM_DEBUG(llvm::errs() << "CREATE NEW GLOBAL \n");
        rewriter.setInsertionPointToStart(moduleOp.getBody(0));
        globalStackOp = rewriter.create<LLVM::GlobalOp>(
            op->getLoc(),
            LLVM::LLVMPointerType::get(context),
            false,
            LLVM::Linkage::Linkonce,
            sigi_global_stack,
            Attribute());
    }
    assert(globalStackOp && "Global Stack still not found.");
    return globalStackOp;
}

struct ConvertSigiLoadGlobalOpToLLVM : public ConvertOpToLLVMPattern<sigi::LoadGlobalStackOp> {
    using ConvertOpToLLVMPattern<sigi::LoadGlobalStackOp>::ConvertOpToLLVMPattern;

    LogicalResult matchAndRewrite(
        sigi::LoadGlobalStackOp op,
        LoadGlobalStackOpAdaptor adaptor,
        ConversionPatternRewriter &rewriter) const override
    {
        LLVM_DEBUG(llvm::errs() << "Convert LoadGlobalOp " << op << "\n");
        auto context = op->getContext();
        lookupOrCreateGlobalStack(op, rewriter);
        rewriter.setInsertionPoint(op);
        auto globalStackPtrPtr = rewriter.create<LLVM::AddressOfOp>(
            op->getLoc(),
            LLVM::LLVMPointerType::get(context),
            sigi_global_stack);
        auto globalStackPtr = rewriter.create<LLVM::LoadOp>(
            op->getLoc(),
            LLVM::LLVMPointerType::get(context),
            globalStackPtrPtr.getResult());
        rewriter.replaceOp(op, globalStackPtr);
        return llvm::success();
    }
};

struct ConvertSigiStoreGlobalOpToLLVM : public ConvertOpToLLVMPattern<sigi::StoreGlobalStackOp> {
    using ConvertOpToLLVMPattern<sigi::StoreGlobalStackOp>::ConvertOpToLLVMPattern;

    LogicalResult matchAndRewrite(
        sigi::StoreGlobalStackOp op,
        StoreGlobalStackOpAdaptor adaptor,
        ConversionPatternRewriter &rewriter) const override
    {
        LLVM_DEBUG(llvm::errs() << "Convert StoreGlobalOp " << op << "\n");
        auto context = op->getContext();
        lookupOrCreateGlobalStack(op, rewriter);
        rewriter.setInsertionPoint(op);
        auto globalStackPtrPtr = rewriter.create<LLVM::AddressOfOp>(
            op->getLoc(),
            LLVM::LLVMPointerType::get(context),
            sigi_global_stack);
        rewriter.create<LLVM::StoreOp>(
            op->getLoc(),
            adaptor.getInStack(),
            globalStackPtrPtr.getResult());
        rewriter.eraseOp(op);
        return llvm::success();
    }
};

struct CreateSigiMain : public ConvertOpToLLVMPattern<LLVM::LLVMFuncOp> {
    using ConvertOpToLLVMPattern<LLVM::LLVMFuncOp>::ConvertOpToLLVMPattern;

    LogicalResult matchAndRewrite(
        LLVM::LLVMFuncOp op,
        LLVM::LLVMFuncOpAdaptor,
        ConversionPatternRewriter &rewriter) const override
    {
        LLVM_DEBUG(llvm::errs() << "SEARCH FOR MAIN\n");
        if (op->hasAttr("sigi.main")) {

            LLVM_DEBUG(llvm::errs() << "FOUND MAIN\n");
            ModuleOp moduleOp = op->getParentOfType<ModuleOp>();
            MLIRContext* context = op->getContext();
            rewriter.setInsertionPointToEnd(moduleOp.getBody());
            auto newMainFunc = rewriter.create<LLVM::LLVMFuncOp>(
                op->getLoc(),
                "main",
                LLVM::LLVMFunctionType::get(rewriter.getI1Type(), {}));
            auto mainBody = rewriter.createBlock(&newMainFunc.getBody());
            rewriter.setInsertionPointToStart(mainBody);

            if (llvm::is_contained(op->getOperandTypes(), sigi::StackType::get(context))) {
                auto mallocSignature =
                    LLVM::lookupOrCreateMallocFn(moduleOp, rewriter.getI64Type());
                auto initStack = getSigiInitStack(moduleOp);
                auto freeStack = getSigiFreeStack(moduleOp);
                auto size =
                    rewriter.create<LLVM::ConstantOp>(op.getLoc(), rewriter.getI64Type(), 128);
                auto c_malloc =
                    rewriter.create<LLVM::CallOp>(op.getLoc(), mallocSignature, size.getResult());
                rewriter.create<LLVM::CallOp>(op.getLoc(), initStack, c_malloc.getResult());
                auto castToLLVMStruct = rewriter.create<LLVM::BitcastOp>(
                    op.getLoc(),
                    llvmStackStructType(context),
                    c_malloc.getResult());
                rewriter.create<LLVM::CallOp>(op.getLoc(), op, castToLLVMStruct.getResult());
                rewriter.create<LLVM::CallOp>(op.getLoc(), freeStack, c_malloc.getResult());
            } else {
                auto mallocSignature =
                    LLVM::lookupOrCreateMallocFn(moduleOp, rewriter.getI64Type());
                auto initStack = getSigiInitStack(moduleOp);
                auto freeStack = getSigiFreeStack(moduleOp);
                bool containsGlobalStack = false;
                bool constainsGlobalStackSym = moduleOp.lookupSymbol(sigi_global_stack) != nullptr;

                moduleOp->walk([&](LoadGlobalStackOp) {
                    containsGlobalStack = true;
                    WalkResult::interrupt();
                });
                moduleOp->walk([&](StoreGlobalStackOp) {
                    containsGlobalStack = true;
                    WalkResult::interrupt();
                });

                if (containsGlobalStack || constainsGlobalStackSym) {
                    lookupOrCreateGlobalStack(op.getOperation(), rewriter);
                    auto size =
                        rewriter.create<LLVM::ConstantOp>(op.getLoc(), rewriter.getI64Type(), 128);
                    auto c_malloc = rewriter.create<LLVM::CallOp>(
                        op.getLoc(),
                        mallocSignature,
                        size.getResult());
                    rewriter.create<LLVM::CallOp>(op.getLoc(), initStack, c_malloc.getResult());
                    auto castToLLVMStruct = rewriter.create<LLVM::BitcastOp>(
                        op.getLoc(),
                        llvmStackStructType(context),
                        c_malloc.getResult());
                    auto globalStackPtrPtr = rewriter.create<LLVM::AddressOfOp>(
                        op->getLoc(),
                        LLVM::LLVMPointerType::get(context),
                        sigi_global_stack);
                    rewriter.create<LLVM::StoreOp>(
                        op->getLoc(),
                        castToLLVMStruct.getResult(),
                        globalStackPtrPtr.getResult());
                    rewriter.create<LLVM::CallOp>(op.getLoc(), op, ValueRange{});
                    rewriter.create<LLVM::CallOp>(op.getLoc(), freeStack, c_malloc.getResult());
                } else {
                    LLVM_DEBUG(
                        llvm::errs() << "NO STACK containsglobal: " << containsGlobalStack
                                     << " SYM " << constainsGlobalStackSym << "\n");
                    rewriter.create<LLVM::CallOp>(op.getLoc(), op, ValueRange{});
                }
            }

            auto exitCode =
                rewriter.create<LLVM::ConstantOp>(op->getLoc(), rewriter.getI1Type(), 0);
            rewriter.create<LLVM::ReturnOp>(op.getLoc(), exitCode.getResult());
            rewriter.modifyOpInPlace(op, [&]() { op->removeAttr("sigi.main"); });
            return llvm::success();
        }
        LLVM_DEBUG(llvm::errs() << "Matched op is not main: " << op << "\n");
        return failure();
    }
};

struct convertPrintFuncToFunc : public ConvertOpToLLVMPattern<LLVM::LLVMFuncOp> {
    using ConvertOpToLLVMPattern<LLVM::LLVMFuncOp>::ConvertOpToLLVMPattern;

    LogicalResult replaceWithRuntimeBuiltin(
        LLVM::LLVMFuncOp op,
        ConversionPatternRewriter &rewriter,
        std::string new_name) const
    {
        LLVM_DEBUG(llvm::errs() << "Convert PrintFunc " << op << "\n");
        rewriter.startOpModification(op);
        if (failed(op.replaceAllSymbolUses(
                rewriter.getStringAttr(new_name),
                op->getParentOfType<ModuleOp>()))) {
            rewriter.cancelOpModification(op);
            return llvm::failure();
        } else {
            op.setName(new_name);
            op->removeAttr("sigi.builtinfunc");
            rewriter.finalizeOpModification(op);
        }
        return success();
    }

    LogicalResult matchAndRewrite(
        LLVM::LLVMFuncOp op,
        OpAdaptor,
        ConversionPatternRewriter &rewriter) const override
    {
        if (op.isExternal() && op.getName() == "sigi::pp") {
            std::string new_name = "sigi_builtin__pp";
            return replaceWithRuntimeBuiltin(op, rewriter, new_name);
        } else if (op.isExternal() && op.getName() == "sigi::pp_i32") {
            std::string new_name = "sigi_builtin__pp_i32";
            return replaceWithRuntimeBuiltin(op, rewriter, new_name);
        } else if (op.isExternal() && op.getName() == "sigi::pp_i1") {
            std::string new_name = "sigi_builtin__pp_i1";
            return replaceWithRuntimeBuiltin(op, rewriter, new_name);
        } else if (op.isExternal() && op.getName() == "sigi::pp_closure") {
            std::string new_name = "sigi_builtin__pp_closure";
            return replaceWithRuntimeBuiltin(op, rewriter, new_name);
        }
        return failure();
    }
};
}; // namespace

struct ConvertSigiToLLVMPass : public impl::ConvertSigiToLLVMBase<ConvertSigiToLLVMPass> {
    void runOnOperation() final;
};

void ConvertSigiToLLVMPass::runOnOperation()
{
    LLVM_DEBUG(llvm::errs() << "START SIGI_TO_LLVM\n");
    LLVMTypeConverter converter(&getContext());

    mlir::ConversionTarget target(getContext());
    target.addIllegalDialect<
        arith::ArithDialect,
        sigi::SigiDialect,
        closure::ClosureDialect,
        func::FuncDialect,
        scf::SCFDialect,
        mlir::cf::ControlFlowDialect>();

    RewritePatternSet patterns(&getContext());

    const auto addUnrealizedCast =
        [](OpBuilder &builder, Type type, ValueRange inputs, Location loc) {
            return builder.create<UnrealizedConversionCastOp>(loc, type, inputs).getResult(0);
        };

    mlir::closure::populateClosureGenericTypeConversions(converter);
    populateSigiToLLVMFinalTypeConversions(converter);
    closure::populateClosureToLLVMFinalTypeConversions(converter);

    converter.addSourceMaterialization(addUnrealizedCast);
    converter.addTargetMaterialization(addUnrealizedCast);

    populateFunctionOpInterfaceTypeConversionPattern<func::FuncOp>(patterns, converter);
    populateCallOpTypeConversionPattern(patterns, converter);
    populateReturnOpTypeConversionPattern(patterns, converter);
    populateSigiToLLVMConversionPatterns(converter, patterns);

    populateSCFToControlFlowConversionPatterns(patterns);
    arith::populateArithToLLVMConversionPatterns(converter, patterns);
    closure::populateClosureToLLVMConversionPatterns(converter, patterns);
    cf::populateControlFlowToLLVMConversionPatterns(converter, patterns);
    sigi::populateSigiToLLVMConversionPatterns(converter, patterns);
    mlir::populateFuncToLLVMConversionPatterns(converter, patterns);

    target.markUnknownOpDynamicallyLegal([](Operation*) { return true; });

    // make sure our rewrite pattern is applied -> from Clemént's solution
    target.addDynamicallyLegalOp<LLVM::LLVMFuncOp>([&](LLVM::LLVMFuncOp op) {
        return !op->hasAttr("sigi.main") && !op->hasAttr("sigi.builtinfunc")
               && converter.isLegal(&op.getBody());
    });

    if (failed(applyFullConversion(
            getOperation(), // Walk IR
            target,
            std::move(patterns))))
        signalPassFailure();
}

void mlir::sigi::populateSigiToLLVMConversionPatterns(
    LLVMTypeConverter &typeConverter,
    RewritePatternSet &patterns)
{
    patterns.add<
        ConvertSigiPopOpToLLVM,
        ConvertSigiPushOpToLLVM,
        CreateSigiMain,
        convertPrintFuncToFunc,
        ConvertSigiLoadGlobalOpToLLVM,
        ConvertSigiStoreGlobalOpToLLVM>(typeConverter);
}

void mlir::sigi::populateSigiToLLVMFinalTypeConversions(LLVMTypeConverter &typeConverter)
{
    typeConverter.addConversion(
        [&](sigi::StackType stackType) { return llvmStackStructType(stackType.getContext()); });
}

std::unique_ptr<Pass> mlir::sigi::createConvertSigiToLLVMPass()
{
    return std::make_unique<ConvertSigiToLLVMPass>();
}