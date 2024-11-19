/// Implements the ClosureToLLVM pass.
///
/// @file
/// @author     Clément Fournier (clement.fournier@mailbox.tu-dresden.de)

#include "sigi-mlir/Conversion/ClosureToLLVM/ClosureToLLVM.h"

#include "../PassDetails.h"
#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Dialect/LLVMIR/FunctionCallUtils.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/IRMapping.h"
#include "mlir/IR/ImplicitLocOpBuilder.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/SymbolTable.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureDialect.h"

using namespace mlir;
using namespace mlir::closure;

namespace {

static LLVM::LLVMPointerType ptrType(Type ty)
{
    return LLVM::LLVMPointerType::get(ty.getContext());
}

static LLVM::LLVMPointerType untypedPtrType(MLIRContext* ctx)
{
    return LLVM::LLVMPointerType::get(ctx);
}

static LLVM::LLVMFunctionType
convertFunType(const LLVMTypeConverter &converter, FunctionType funTy)
{
    TypeConverter::SignatureConversion conversion(funTy.getInputs().size());
    Type res =
        converter.convertFunctionSignature(funTy, false, true, conversion);
    if (res) return mlir::cast<LLVM::LLVMFunctionType>(res);
    return nullptr;
}

static LLVM::LLVMFunctionType
insertClosureParameter(LLVM::LLVMFunctionType funTy)
{
    SmallVector<Type> argTypes;
    argTypes.reserve(funTy.getParams().size() + 1);
    argTypes.push_back(untypedPtrType(funTy.getContext()));
    for (auto ty : funTy.getParams()) argTypes.push_back(ty);

    return LLVM::LLVMFunctionType::get(
        funTy.getContext(),
        funTy.getReturnType(),
        argTypes,
        funTy.isVarArg());
}

/// Return the function that checks decrements the refcount and checks if it is
/// zero.
static LLVM::LLVMFuncOp getDecrOrDropFunc(ModuleOp moduleOp)
{
    auto ctx = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "closure_decr_then_drop",
        {untypedPtrType(ctx)},
        LLVM::LLVMVoidType::get(ctx));
}
/// Return the function that drops a closure if its refcount is zero. Does not
/// decrement the refcount.
static LLVM::LLVMFuncOp getCheckDropFunc(ModuleOp moduleOp)
{
    auto ctx = moduleOp->getContext();
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "closure_check_drop",
        {untypedPtrType(ctx)},
        LLVM::LLVMVoidType::get(ctx));
}

static LLVM::LLVMFuncOp getIncrRefCountFunc(ModuleOp moduleOp)
{
    return LLVM::lookupOrCreateFn(
        moduleOp,
        "closure_incr",
        {untypedPtrType(moduleOp.getContext())},
        LLVM::LLVMVoidType::get(moduleOp.getContext()));
}

static LLVM::LLVMFuncOp
getNoopDropFunc(ModuleOp moduleOp, ImplicitLocOpBuilder &rewriter)
{
    auto func = LLVM::lookupOrCreateFn(
        moduleOp,
        "closure_drop_nothing",
        {untypedPtrType(rewriter.getContext())},
        LLVM::LLVMVoidType::get(rewriter.getContext()));
    func.setLinkage(LLVM::Linkage::Private);

    if (func.empty()) {
        ConversionPatternRewriter::InsertionGuard guard(rewriter);
        auto block = func.addEntryBlock(rewriter);
        rewriter.setInsertionPointToStart(block);
        rewriter.create<LLVM::ReturnOp>(ValueRange{});
    }
    return func;
}

static SmallString<20>
getUniqueFunctionName(ModuleOp moduleOp, const char prefix[])
{
    // Get a unique global name.
    unsigned stringNumber = 0;
    size_t prefixLen = strlen(prefix);
    assert(20 > 3 + prefixLen); // make sure this is bigger than the prefix
                                // (prefixes are literals)
    SmallString<20> name(prefix);
    do {
        name.truncate(prefixLen);
        name.append(std::to_string(stringNumber++));
    } while (moduleOp.lookupSymbol(name));
    return name;
}

struct ConvertClosureBoxToLLVM : public ConvertOpToLLVMPattern<closure::BoxOp> {

    explicit ConvertClosureBoxToLLVM(
        LLVMTypeConverter &typeConverter,
        PatternBenefit benefit = 1)
            : ConvertOpToLLVMPattern(typeConverter, benefit)
    {
        // closures may be nested
        setHasBoundedRewriteRecursion();
    }

    static Value getSizeOfType(Type ty, ImplicitLocOpBuilder &rewriter)
    {
        //  https://stackoverflow.com/questions/14608250/how-can-i-find-the-size-of-a-type
        auto fakeArray = rewriter.create<LLVM::UndefOp>(ptrType(ty));

        auto gep = rewriter.create<LLVM::GEPOp>(
            ptrType(ty),
            ty,
            fakeArray.getResult(),
            ArrayRef<LLVM::GEPArg>{1});
        auto size =
            rewriter.create<LLVM::PtrToIntOp>(rewriter.getI64Type(), gep);
        return size.getResult();
    }

    static Value capture(
        Value captureArg,
        Type originalArgTy,
        ModuleOp moduleOp,
        ImplicitLocOpBuilder rewriter)
    {
        if (mlir::isa<closure::BoxedClosureType>(originalArgTy)) {
            auto cast = rewriter.create<LLVM::BitcastOp>(
                untypedPtrType(rewriter.getContext()),
                captureArg);
            rewriter.create<LLVM::CallOp>(
                getIncrRefCountFunc(moduleOp),
                ValueRange{cast.getResult()});
        }
        return captureArg;
    }

    LogicalResult matchAndRewrite(
        closure::BoxOp op,
        OpAdaptor adaptor,
        ConversionPatternRewriter &rewriter0) const override
    {

        auto loc = op.getLoc();
        ImplicitLocOpBuilder rewriter(loc, rewriter0);

        auto moduleOp = op->getParentOfType<ModuleOp>();
        auto funTy = convertFunType(*getTypeConverter(), op.getFunctionType());
        if (!funTy) {
            return rewriter0.notifyMatchFailure(
                op.getLoc(),
                "Closure signature cannot be converted to LLVM types.");
        }
        auto wrapperTy = insertClosureParameter(funTy);
        auto workerTy = convertFunType(
            *getTypeConverter(),
            FunctionType::get(
                getContext(),
                op.getRegion().front().getArgumentTypes(),
                wrapperTy.getReturnTypes()));
        // Drop function: void drop(void*);
        // This is the virtual drop function that recursively drops fields.
        auto dropFuncTy = LLVM::LLVMFunctionType::get(
            getContext(),
            getVoidType(),
            {untypedPtrType(getContext())},
            false);
        auto decOrDropFunc = getDecrOrDropFunc(moduleOp);
        auto refCountType = rewriter.getI32Type();

        LLVM::LLVMFuncOp workerFun;
        {
            // outline the body of the closure into a worker function

            ConversionPatternRewriter::InsertionGuard guard(rewriter);
            rewriter.setInsertionPointToStart(moduleOp.getBody());
            workerFun = rewriter.create<LLVM::LLVMFuncOp>(
                getUniqueFunctionName(moduleOp, "closure_worker_"),
                workerTy,
                LLVM::Linkage::Private);
            Block* entry = workerFun.addEntryBlock(rewriter);

            rewriter0.cloneRegionBefore(op.getBody(), entry);
            workerFun.getFunctionBody()
                .getBlocks()
                .pop_back(); // remove that fake block

            auto conversionResult = rewriter0.convertRegionTypes(
                &workerFun.getFunctionBody(),
                *getTypeConverter());
            if (failed(conversionResult))
                return rewriter0.notifyMatchFailure(loc, [](auto &diag) {
                    diag << "cannot convert block signature";
                });
        }

        // these are the converted types!
        SmallVector<Type> captureArgTypes(adaptor.getCaptureArgs().getTypes());

        // type of the struct that holds the captured parameters
        auto captureParamStructTy =
            LLVM::LLVMStructType::getLiteral(getContext(), captureArgTypes);

        // type of the struct that holds the entire closure (fptr + refcount +
        // drop fun + capture args)
        auto fullClosureTy = LLVM::LLVMStructType::getLiteral(
            getContext(),
            {ptrType(wrapperTy),  // invoke function
             refCountType,        // reference count
             ptrType(dropFuncTy), // drop function
             captureParamStructTy});

        // create a wrapper function, that has the type of the erased
        // function + 1 initial parameter for the closure itself
        LLVM::LLVMFuncOp wrapperFun;
        {
            ConversionPatternRewriter::InsertionGuard guard(rewriter);
            rewriter.setInsertionPointAfter(workerFun.getOperation());
            wrapperFun = rewriter.create<LLVM::LLVMFuncOp>(
                getUniqueFunctionName(moduleOp, "closure_wrapper_"),
                wrapperTy,
                LLVM::Linkage::Private);
            Block* entry = wrapperFun.addEntryBlock(rewriter);
            rewriter.setInsertionPointToStart(entry);

            // clang-format off
            // %argsPtr = llvm.getelementptr %closure[0, 1]: (!llvm.ptr<fullClosureTy>) -> !llvm.ptr<captureParamStructTy>
            // clang-format on
            auto argsPtr = rewriter.create<LLVM::GEPOp>(
                ptrType(captureParamStructTy),
                fullClosureTy,
                entry->getArgument(0),
                ArrayRef<LLVM::GEPArg>{0, 3});

            // clang-format off
            // %loadedArgs = llvm.load %argsPtr: !llvm.ptr<captureParamStructTy>
            // clang-format on
            auto loadedArgs = rewriter.create<LLVM::LoadOp>(
                loc,
                captureParamStructTy,
                argsPtr.getResult());

            // unpack each struct field
            SmallVector<Value> workerArgs;
            workerArgs.reserve(workerFun.getArguments().size());

            int64_t i = 0, numCaptureArgs = adaptor.getCaptureArgs().size();
            for (; i < numCaptureArgs; i++) {
                auto field = rewriter.create<LLVM::ExtractValueOp>(
                    loadedArgs,
                    ArrayRef<int64_t>{i});
                workerArgs.emplace_back(field.getResult());
            }
            i = 0;
            for (auto funArg : entry->getArguments()) {
                if (i++ == 0) continue; // skip the ptr arg
                workerArgs.emplace_back(funArg);
            }

            // finally call the worker
            auto result = rewriter.create<LLVM::CallOp>(workerFun, workerArgs);
            // and return
            rewriter.create<LLVM::ReturnOp>(result.getResults());
        }
        SmallVector<Type> originalCaptureArgTypes(
            op.getCaptureArgs().getTypes());

        // Create the virtual drop function.
        LLVM::LLVMFuncOp dropFun;
        {

            // If there are any closures to drop, generate the func.
            if (std::any_of(
                    originalCaptureArgTypes.begin(),
                    originalCaptureArgTypes.end(),
                    [](auto ty) {
                        return mlir::isa<closure::BoxedClosureType>(ty);
                    })) {
                ConversionPatternRewriter::InsertionGuard guard(rewriter);
                rewriter.setInsertionPointAfter(workerFun.getOperation());
                dropFun = rewriter.create<LLVM::LLVMFuncOp>(
                    getUniqueFunctionName(moduleOp, "closure_drop_"),
                    dropFuncTy,
                    LLVM::Linkage::Private);
                Block* entry = dropFun.addEntryBlock(rewriter);
                rewriter.setInsertionPointToStart(entry);

                // clang-format off
                // %argsPtr = llvm.getelementptr %closure[0, 1]: (!llvm.ptr<fullClosureTy>) -> !llvm.ptr<captureParamStructTy>
                // clang-format on
                auto argsPtr = rewriter.create<LLVM::GEPOp>(
                    ptrType(fullClosureTy),
                    fullClosureTy,
                    entry->getArgument(0),
                    ArrayRef<LLVM::GEPArg>{0, 3});

                // clang-format off
                // %loadedArgs = llvm.load %argsPtr: !llvm.ptr<captureParamStructTy>
                // clang-format on
                auto loadedArgs = rewriter.create<LLVM::LoadOp>(
                    loc,
                    captureParamStructTy,
                    argsPtr);

                int64_t i = 0, numCaptureArgs = adaptor.getCaptureArgs().size();
                for (; i < numCaptureArgs; i++) {
                    auto argTy = originalCaptureArgTypes[i];
                    if (mlir::isa<closure::BoxedClosureType>(argTy)) {
                        auto field = rewriter.create<LLVM::ExtractValueOp>(
                            loadedArgs,
                            ArrayRef<int64_t>{i});

                        auto castField = rewriter.create<LLVM::BitcastOp>(
                            decOrDropFunc.getArgumentTypes()[0],
                            field);
                        rewriter.create<LLVM::CallOp>(
                            decOrDropFunc,
                            ValueRange{castField.getResult()});
                    }
                }
                rewriter.create<LLVM::ReturnOp>(ValueRange{});
            } else {
                // there are no closures to drop. Just use the noop
                // implementation.
                dropFun = getNoopDropFunc(moduleOp, rewriter);
            }
        }

        // now for replacing the closure.box

        // first we allocate memory for it
        Value sizeOfClosure = getSizeOfType(fullClosureTy, rewriter);
        auto mallocFun =
            LLVM::lookupOrCreateMallocFn(moduleOp, sizeOfClosure.getType());
        auto allocForClosure =
            rewriter.create<LLVM::CallOp>(mallocFun, ValueRange{sizeOfClosure});

        // then we create an instance and initialize it
        auto closureInstance = rewriter.create<LLVM::UndefOp>(fullClosureTy);
        auto wrapperAddress = rewriter.create<LLVM::AddressOfOp>(wrapperFun);
        auto dropAddress = rewriter.create<LLVM::AddressOfOp>(dropFun);
        // initially the closure is allocated with a refcount of 0
        auto initialRefCount =
            rewriter.create<LLVM::ConstantOp>(refCountType, 0);
        Value closureBeingBuilt = rewriter.create<LLVM::InsertValueOp>(
            closureInstance,
            wrapperAddress,
            ArrayRef<int64_t>{0});
        closureBeingBuilt = rewriter.create<LLVM::InsertValueOp>(
            closureBeingBuilt,
            initialRefCount,
            ArrayRef<int64_t>{1});
        closureBeingBuilt = rewriter.create<LLVM::InsertValueOp>(
            closureBeingBuilt,
            dropAddress,
            ArrayRef<int64_t>{2});
        // initialize all captured fields
        int64_t i = 0;
        for (auto captArg : adaptor.getCaptureArgs()) {
            auto captured = capture(
                captArg,
                originalCaptureArgTypes[i],
                moduleOp,
                rewriter);
            closureBeingBuilt = rewriter.create<LLVM::InsertValueOp>(
                closureBeingBuilt,
                captured,
                ArrayRef<int64_t>{3, i});
            i++;
        }

        // that is the type needed to store
        auto castPtrToStrongType = rewriter.create<LLVM::BitcastOp>(
            ptrType(fullClosureTy),
            allocForClosure.getResults());

        // finally store it into the alloc
        rewriter.create<LLVM::StoreOp>(
            closureBeingBuilt,
            castPtrToStrongType.getResult());

        // that is the LLVM type that the CallOp expects
        auto castPtrToErasedTy = rewriter.create<LLVM::BitcastOp>(
            getTypeConverter()->convertType(op.getClosureType()),
            allocForClosure.getResults());

        rewriter0.replaceOp(op, {castPtrToErasedTy.getResult()});

        return success();
    }
};

struct ConvertClosureReturnToLLVM
        : public ConvertOpToLLVMPattern<closure::ReturnOp> {
    using ConvertOpToLLVMPattern<closure::ReturnOp>::ConvertOpToLLVMPattern;

    LogicalResult matchAndRewrite(
        closure::ReturnOp op,
        OpAdaptor adaptor,
        ConversionPatternRewriter &rewriter) const override
    {
        return LLVM::detail::oneToOneRewrite(
            op,
            LLVM::ReturnOp::getOperationName(),
            adaptor.getOperands(),
            op->getAttrs(),
            *getTypeConverter(),
            rewriter);
    }
};
struct ConvertClosureDropToLLVM
        : public ConvertOpToLLVMPattern<closure::DropOp> {
    using ConvertOpToLLVMPattern<closure::DropOp>::ConvertOpToLLVMPattern;

    LogicalResult matchAndRewrite(
        closure::DropOp op,
        OpAdaptor adaptor,
        ConversionPatternRewriter &rewriter0) const override
    {
        auto moduleOp = op->getParentOfType<ModuleOp>();

        LLVM::LLVMFuncOp popFunc;
        auto dropFunc = getCheckDropFunc(moduleOp);

        auto cast = rewriter0.create<LLVM::BitcastOp>(
            op.getLoc(),
            dropFunc.getArgumentTypes()[0],
            adaptor.getCallee());

        rewriter0.replaceOpWithNewOp<LLVM::CallOp>(
            op,
            dropFunc,
            cast.getResult());

        return success();
    }
};

struct ConvertClosureCallToLLVM
        : public ConvertOpToLLVMPattern<closure::CallOp> {
    using ConvertOpToLLVMPattern<closure::CallOp>::ConvertOpToLLVMPattern;

    LogicalResult matchAndRewrite(
        closure::CallOp op,
        OpAdaptor adaptor,
        ConversionPatternRewriter &rewriter0) const override
    {
        auto loc = op.getLoc();
        ImplicitLocOpBuilder rewriter(loc, rewriter0);

        auto llvmFunPtrTy =
            getTypeConverter()->convertType(op.getClosureType());

        // Given a strongly typed ptr to the closure type,
        auto callee = adaptor.getCallee();

        if (callee.getType() != llvmFunPtrTy)
            return rewriter0.notifyMatchFailure(
                callee.getLoc(),
                [&](Diagnostic &diag) {
                    diag << "Expected callee " << callee
                         << " to have pointer type " << llvmFunPtrTy
                         << " but got " << callee.getType() << ".";
                });

        // Load the actual wrapper function pointer.
        // clang-format off
        // %funPtr = llvm.load %castCallee: !llvm.ptr<llvmFuncTy>
        // clang-format on
        auto funPtr =
            rewriter.create<LLVM::LoadOp>(untypedPtrType(getContext()), callee);

        // the worker expects an untyped !llvm.ptr that points to itself.
        auto erasedCalleePtr = rewriter.create<LLVM::BitcastOp>(
            untypedPtrType(getContext()),
            callee);

        // prepare call arguments
        SmallVector<Value> callArgs;
        callArgs.reserve(adaptor.getCalleeOperands().size() + 2);
        callArgs.push_back(funPtr.getResult()); // first the callee fun
        callArgs.push_back(erasedCalleePtr);    // this is the closure ptr
        // then all other func args
        for (auto arg : adaptor.getCalleeOperands()) callArgs.push_back(arg);

        SmallVector<Type> llvmResultTypes;
        if (failed(getTypeConverter()->convertTypes(
                op.getResults().getTypes(),
                llvmResultTypes)))
            return failure();

        // clang-format off
        // %result = llvm.call %funPtr(%callee, %calleeArgs): (!llvm.ptr, argTypes...) -> resType
        // clang-format on
        rewriter0.replaceOpWithNewOp<LLVM::CallOp>(
            op.getOperation(),
            llvmResultTypes,
            ValueRange(callArgs));

        return success();
    }
};

struct ConvertClosureToLLVMPass
        : public impl::ConvertClosureToLLVMBase<ConvertClosureToLLVMPass> {
    void runOnOperation() final;
};

} // namespace

void mlir::closure::populateClosureToLLVMFinalTypeConversions(
    LLVMTypeConverter &typeConverter)
{
    // Convert BoxedClosure type to the underlying implementation type.
    typeConverter.addConversion(
        [&](closure::BoxedClosureType type) -> std::optional<Type> {
            // turns (a1, .., an) -> b
            // to !llvm.ptr<ptr<func<b (ptr, a1, ..., an)>>>
            TypeConverter::SignatureConversion conversion(
                type.getFunctionType().getInputs().size());
            auto funcTy = type.getFunctionType();
            auto llvmTy = typeConverter.convertFunctionSignature(
                funcTy,
                false,
                true,
                conversion);
                
            if (llvmTy)
                // means the function type is convertible to LLVM
                return ptrType(ptrType(insertClosureParameter(
                    mlir::cast<LLVM::LLVMFunctionType>(llvmTy))));

            return std::nullopt;
        });
}

void mlir::closure::populateClosureToLLVMConversionPatterns(
    LLVMTypeConverter &typeConverter,
    RewritePatternSet &patterns)
{
    patterns.add<
        ConvertClosureCallToLLVM,
        ConvertClosureBoxToLLVM,
        ConvertClosureDropToLLVM,
        ConvertClosureReturnToLLVM>(typeConverter);
}

/***
 * Conversion Target
 ***/
void ConvertClosureToLLVMPass::runOnOperation()
{
    LLVMTypeConverter converter(&getContext());

    mlir::closure::populateClosureToLLVMFinalTypeConversions(converter);
    const auto addUnrealizedCast =
        [](OpBuilder &builder, Type type, ValueRange inputs, Location loc) {
            return builder.create<UnrealizedConversionCastOp>(loc, type, inputs)
                .getResult(0);
        };
    converter.addSourceMaterialization(addUnrealizedCast);
    converter.addTargetMaterialization(addUnrealizedCast);

    ConversionTarget target(getContext());
    RewritePatternSet patterns(&getContext());

    // convert closure dialect operations.
    populateClosureToLLVMConversionPatterns(converter, patterns);

    // Remove unrealized casts wherever possible.
    // populateReconcileUnrealizedCastsPatterns(patterns);

    target.addIllegalDialect<closure::ClosureDialect>();
    target.markUnknownOpDynamicallyLegal([](Operation*) { return true; });

    if (failed(applyPartialConversion(
            getOperation(),
            target,
            std::move(patterns))))
        signalPassFailure();
}

std::unique_ptr<Pass> mlir::closure::createConvertClosureToLLVMPass()
{
    return std::make_unique<ConvertClosureToLLVMPass>();
}
