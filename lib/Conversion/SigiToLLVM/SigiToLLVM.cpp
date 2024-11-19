#include "sigi-mlir/Conversion/SigiToLLVM/SigiToLLVM.h"
#include "sigi-mlir/Conversion/ClosureToLLVM/ClosureToLLVM.h"
#include <llvm/IR/Type.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/LogicalResult.h>
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include <llvm/Support/raw_ostream.h>
#include <mlir/Conversion/LLVMCommon/Pattern.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/LLVMIR/FunctionCallUtils.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/Diagnostics.h>
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/IR/ValueRange.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Transforms/DialectConversion.h>
#include <ostream>
#include <stdbool.h>
#include "../PassDetails.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureBase.h"
#include "sigi-mlir/Dialect/Closure/IR/ClosureTypes.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiBase.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiOps.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiTypes.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiAttributes.h"

using namespace mlir;
using namespace mlir::sigi;

#define DEBUG_TYPE "SigiToLLVM"

namespace {


    static LLVM::LLVMPointerType ptrType(Type ty)
    {
        return LLVM::LLVMPointerType::get(ty.getContext());
    }

    static LLVM::LLVMPointerType untypedPtrType(MLIRContext* ctx)
    {
        return LLVM::LLVMPointerType::get(ctx);
    }

    static LLVM::LLVMPointerType llvmStackStructType(MLIRContext* ctx) 
    {
        // Opaque Struct instead of simply identified -> Body should not be changed.
        return ptrType(LLVM::LLVMStructType::getOpaque("sigi_stack_impl",ctx));        
    }
    
    // static bool isClosure(Type type) {
    //     bool isClosure = false;

    //     auto stackTy = sigi::StackType::get(type.getContext());
    //     if(auto closure = llvm::dyn_cast<closure::BoxedClosureType>(type)) {
    //         auto funTy = closure.getFunctionType();
    //         isClosure = funTy.getInputs().equals(stackTy)
    //            && funTy.getResults().equals(stackTy);
    //     } else if (auto ptype = llvm::dyn_cast<LLVM::LLVMPointerType>(type)) {
    //         isClosure == true;
    //     }

    //     return isClosure;
    // }

    static LLVM::LLVMFuncOp getSigiInitStack(ModuleOp moduleOp) {
        ::mlir::MLIRContext *context = moduleOp->getContext();
        return LLVM::lookupOrCreateFn(
            moduleOp,
        "sigi_init_stack",
        llvmStackStructType(context),
        LLVM::LLVMVoidType::get(context)
        );
    }

    static LLVM::LLVMFuncOp getSigiFreeStack(ModuleOp moduleOp) {
        ::mlir::MLIRContext *context = moduleOp->getContext();
        return LLVM::lookupOrCreateFn(
            moduleOp,
        "sigi_free_stack",
        llvmStackStructType(context),
        LLVM::LLVMVoidType::get(context)
        );
    }

    static LLVM::LLVMFuncOp getSigiPopBool(ModuleOp moduleOp) {
        ::mlir::MLIRContext *context = moduleOp->getContext();
        return LLVM::lookupOrCreateFn(
            moduleOp,
        "sigi_pop_bool",
        llvmStackStructType(context),
        IntegerType::get(context, 1)
        );
    }

    static LLVM::LLVMFuncOp getSigiPopI32(ModuleOp moduleOp) {
        ::mlir::MLIRContext *context = moduleOp->getContext();
        return LLVM::lookupOrCreateFn(
            moduleOp,
        "sigi_pop_i32",
        llvmStackStructType(context),
        IntegerType::get(context, 32)
        );
    }

// TODO CHECK WHETHER llvmStackStructType(context) should return a pointer?
    static LLVM::LLVMFuncOp getSigiPopClosure(ModuleOp moduleOp, Type resultType) {
        ::mlir::MLIRContext *context = moduleOp->getContext();


        // does not match closurebox correctly and I do not know why
        return LLVM::lookupOrCreateFn(
            moduleOp,
        "sigi_pop_closure",
        llvmStackStructType(context),
        ptrType(resultType)
        );
    }

    static LLVM::LLVMFuncOp getSigiPushBool(ModuleOp moduleOp) {
        ::mlir::MLIRContext *context = moduleOp->getContext();
        return LLVM::lookupOrCreateFn(
            moduleOp,
        "sigi_push_bool",
        {llvmStackStructType(context), IntegerType::get(context, 1)},
        LLVM::LLVMVoidType::get(context)   
        );
    }

    static LLVM::LLVMFuncOp getSigiPushI32(ModuleOp moduleOp) {
        ::mlir::MLIRContext *context = moduleOp->getContext();
        return LLVM::lookupOrCreateFn(
            moduleOp,
        "sigi_push_i32",
        {llvmStackStructType(context), IntegerType::get(context, 32)},
        LLVM::LLVMVoidType::get(context)   
        );
    }

    static LLVM::LLVMFuncOp getSigiPushClosure(ModuleOp moduleOp, Type resultType) {
        ::mlir::MLIRContext *context = moduleOp->getContext();
        return LLVM::lookupOrCreateFn(
            moduleOp,
        "sigi_push_closure",
        {llvmStackStructType(context), ptrType(resultType)},
        LLVM::LLVMVoidType::get(context)
        );
    }

    struct ConvertSigiPopOpToLLVM : public ConvertOpToLLVMPattern<sigi::PopOp> {
        using ConvertOpToLLVMPattern<sigi::PopOp>::ConvertOpToLLVMPattern;

        LogicalResult matchAndRewrite(
            sigi::PopOp op,
            PopOpAdaptor adaptor,
            ConversionPatternRewriter &rewriter) const override {
            
            ModuleOp moduleOp = op->getParentOfType<ModuleOp>();

            // Error might lie here? op.getElement is not yet converted -> for push i can use adaptor, but here not
            auto valueType = op.getElement().getType();
            LLVM::LLVMFuncOp pop;
            if(valueType.isInteger(1)) {
                pop = getSigiPopBool(moduleOp);
            } else if (valueType.isInteger(32)) {
                pop = getSigiPopI32(moduleOp);
            } else if (llvm::isa<closure::BoxedClosureType>(valueType) || llvm::isa<LLVM::LLVMPointerType>(valueType)) {

                // if verifier for popOp is correct, then any invalid argument should get marked down during parsing? check for pointer enough?
                pop = getSigiPopClosure(moduleOp, valueType);
            } else {
                return rewriter.notifyMatchFailure(op->getLoc(), [&](Diagnostic &diag) {
                        diag << "Encountered element with invalid type " << valueType 
                        << "during PopOp conversion.";
                }); 
            }

            LLVM::CallOp pop_call = rewriter.create<LLVM::CallOp>(op->getLoc(), pop, adaptor.getInStack());

            rewriter.replaceOp(op, {adaptor.getInStack(), pop_call.getResult()});

            // Does not work... maybe different order necessary? I think this does not replace the op itself, which causes eraseOp to fail?
            // rewriter.replaceAllUsesWith(op.getElement(),c_pop.getResult());
            // rewriter.replaceAllUsesWith(op.getOutStack(),adaptor.getInStack());
            // rewriter.eraseOp(op);
            return success();
        };
    };

    struct ConvertSigiPushOpToLLVM : public ConvertOpToLLVMPattern<sigi::PushOp> {
        using ConvertOpToLLVMPattern<sigi::PushOp>::ConvertOpToLLVMPattern;

        LogicalResult matchAndRewrite(
            sigi::PushOp op,
            PushOpAdaptor adaptor,
            ConversionPatternRewriter &rewriter) const override { 
                
            ModuleOp moduleOp = op->getParentOfType<ModuleOp>();
            auto valueType = adaptor.getElement().getType();
            LLVM::LLVMFuncOp push;
            if(valueType.isInteger(1)) {
                push = getSigiPushBool(moduleOp);
            } else if (valueType.isInteger(32)) {
                push = getSigiPushI32(moduleOp);
            } else if (llvm::isa<closure::BoxedClosureType>(valueType) || llvm::isa<LLVM::LLVMPointerType>(valueType)) {
                
                // no match -> maybe error in verifier?
                push = getSigiPushClosure(moduleOp, valueType);

                // match but wrong parameter(type) for c_func call?
                // push = getSigiPushClosure(moduleOp, getTypeConverter()->convertType(valueType));
            } else {
                return rewriter.notifyMatchFailure(op->getLoc(), [&](Diagnostic &diag) {
                        diag << "Encountered element with invalid type " << valueType << "during PopOp conversion.";
                }); 
            }

            rewriter.create<LLVM::CallOp>(op->getLoc(), push, adaptor.getOperands());

            // Does not work. Maybe because op.getOutStack() (StackType) and adaptor.getInstack(Value T) cause type mismatch?
            // rewriter.replaceAllUsesWith(op.getOutStack(), adaptor.getInStack());

            // Does also not work. I thought i might have to replace the op as well but also did not work
            // rewriter.replaceAllUsesWith(op, op.getInStack());
            // rewriter.replaceAllUsesWith(op.getOutStack(), adaptor.getInStack());
            // rewriter.eraseOp(op);
            
            // does work and might be preferred way based on name?
            rewriter.replaceOp(op, adaptor.getInStack());
            return success();
            };
    };
    
    struct createSigiMainWrapper : public ConvertOpToLLVMPattern<func::FuncOp> {
        using ConvertOpToLLVMPattern<func::FuncOp>::ConvertOpToLLVMPattern;

         LogicalResult matchAndRewrite(
            func::FuncOp op,
            func::FuncOpAdaptor adaptor,
            ConversionPatternRewriter &rewriter) const override { 
                if(op->hasAttr("sigi.main")) {
                    ModuleOp moduleOp = op->getParentOfType<ModuleOp>();
                    MLIRContext *context = op->getContext();
                    auto initStack = getSigiInitStack(moduleOp);
                    auto freeStack = getSigiFreeStack(moduleOp);
                    auto mallocSignature = LLVM::lookupOrCreateMallocFn(moduleOp, rewriter.getI64Type());
                    rewriter.setInsertionPointToEnd(moduleOp.getBody());
                    auto newMainFunc = rewriter.create<LLVM::LLVMFuncOp>(op->getLoc(), "main",
                        LLVM::LLVMFunctionType::get(LLVM::LLVMVoidType::get(context), {}));
                    rewriter.createBlock(&newMainFunc.getBody());
                    rewriter.setInsertionPointToStart(&newMainFunc->getRegion(0).front());
                    auto size = rewriter.create<LLVM::ConstantOp>(op.getLoc(), rewriter.getI64Type(), 128);
                    auto c_malloc = rewriter.create<LLVM::CallOp>(op.getLoc(), mallocSignature, size.getResult());
                    auto c_init_stack = rewriter.create<LLVM::CallOp>(op.getLoc(),initStack, c_malloc.getResult());
                    auto castToLLVMStruct = rewriter.create<LLVM::BitcastOp>(op.getLoc(), llvmStackStructType(context), c_malloc.getResult());
                    auto oldMain = rewriter.create<LLVM::CallOp>(op.getLoc(), op, castToLLVMStruct.getResult());
                    auto c_freeStack = rewriter.create<LLVM::CallOp>(op.getLoc(), freeStack, c_malloc.getResult());
                    rewriter.create<LLVM::ReturnOp>(op.getLoc(), ValueRange{});
                    rewriter.modifyOpInPlace(op, [&]() {op->removeAttr("sigi.main");});
                } else {
                    LLVM_DEBUG(llvm::errs() << "Matched op is not main: "<< op);
                }
                    return llvm::success();
            }
    }
};

struct ConvertSigiToLLVMPass : public impl::ConvertSigiToLLVMBase<ConvertSigiToLLVMPass> {
        void runOnOperation() final;
};



void sigiAttributeWalk(auto op, MLIRContext *context){
    op->walk([&](LLVM::LLVMFuncOp op){
        ModuleOp moduleOp = op->getParentOfType<ModuleOp>();
        PatternRewriter rewriter(context);
       if(op->hasAttr("sigi.main")) {
            // LLVM_DEBUG(llvm::errs() <<"Found Main Operation: " << op << "\n");
            auto initStack = getSigiInitStack(moduleOp);
            auto freeStack = getSigiFreeStack(moduleOp);
            auto mallocSignature = LLVM::lookupOrCreateMallocFn(moduleOp, rewriter.getI64Type());
            rewriter.setInsertionPointToEnd(moduleOp.getBody());
            auto newMainFunc = rewriter.create<LLVM::LLVMFuncOp>(op->getLoc(), "main",
                LLVM::LLVMFunctionType::get(LLVM::LLVMVoidType::get(context), {}));
            rewriter.createBlock(&newMainFunc.getBody());
            rewriter.setInsertionPointToStart(&newMainFunc->getRegion(0).front());
            auto size = rewriter.create<LLVM::ConstantOp>(op.getLoc(), rewriter.getI64Type(), 128);
            auto c_malloc = rewriter.create<LLVM::CallOp>(op.getLoc(), mallocSignature, size.getResult());
            auto c_init_stack = rewriter.create<LLVM::CallOp>(op.getLoc(),initStack, c_malloc.getResult());
            auto castToLLVMStruct = rewriter.create<LLVM::BitcastOp>(op.getLoc(), llvmStackStructType(context), c_malloc.getResult());
            auto oldMain = rewriter.create<LLVM::CallOp>(op.getLoc(), op, castToLLVMStruct.getResult());
            auto c_freeStack = rewriter.create<LLVM::CallOp>(op.getLoc(), freeStack, c_malloc.getResult());
            rewriter.create<LLVM::ReturnOp>(op.getLoc(), ValueRange{});
            rewriter.modifyOpInPlace(op, [&]() {op->removeAttr("sigi.main");});
       } else if(op->hasAttr("sigi.builtinfunc") && op.getName() == "sigi::pp") {
            LLVM_DEBUG(llvm::errs() <<"Found sigi::pp: " << op << "\n");
            LLVM_DEBUG(llvm::errs() <<"Argument Types: " << op.getArgumentTypes()[0] << "\n");

            LLVM::LLVMFuncOp builtin_signature = LLVM::lookupOrCreateFn(
                moduleOp,
                "sigi_builtin__pp",
                LLVM::LLVMPointerType::get(context),
                LLVM::LLVMPointerType::get(context));
                
            LLVM_DEBUG(llvm::errs() <<"Build c signature: " << builtin_signature << "\n");
            rewriter.startOpModification(op);
            if(llvm::failed(op.replaceAllSymbolUses(
                rewriter.getStringAttr("sigi_builtin__pp"), moduleOp))) {

                LLVM_DEBUG(llvm::errs() << "Failure.");
                rewriter.cancelOpModification(op);
            } else {
                op.setSymName("sigi_builtin_pp");
                op->removeAttr("sigi.builtinfunc");
                rewriter.finalizeOpModification(op);
                LLVM_DEBUG(llvm::errs() << "Success.");
            }
            LLVM_DEBUG(llvm::errs() << "Result: " << moduleOp->getBlock());

            // LLVM_DEBUG(llvm::errs() <<"Has body block: " << op.getRegion().hasOneBlock() << "\n");
            // rewriter.createBlock(&op.getBody());
            // LLVM_DEBUG(llvm::errs() <<"Has body block: " << op.getRegion().hasOneBlock() << "\n");
            // rewriter.setInsertionPointToStart(&op->getRegion(0).front());

            // LLVM_DEBUG(llvm::errs() << "try to write c_print. \n");
            // auto builtin_call = rewriter.create<LLVM::CallOp>(op->getLoc(), builtin_signature, op.getArguments());
            
            // LLVM_DEBUG(llvm::errs() << "try to write return \n");
            // rewriter.create<LLVM::ReturnOp>(op->getLoc(), builtin_call.getResult());
            // LLVM_DEBUG(llvm::errs() << "Result: " << op);
       }
    }); 
}

void ConvertSigiToLLVMPass::runOnOperation() {
    LLVMTypeConverter converter(&getContext());

    mlir::ConversionTarget target(getContext());
    target.addIllegalDialect<sigi::SigiDialect, func::FuncDialect, closure::ClosureDialect>();
    target.markUnknownOpDynamicallyLegal([](Operation*) { return true; });
    RewritePatternSet patterns(&getContext());

     const auto addUnrealizedCast =
        [](OpBuilder &builder, Type type, ValueRange inputs, Location loc) {
            return builder.create<UnrealizedConversionCastOp>(loc, type, inputs)
                .getResult(0);
        };

    converter.addSourceMaterialization(addUnrealizedCast);
    converter.addTargetMaterialization(addUnrealizedCast);

    populateSigiToLLVMConversionPatterns(converter, patterns);
    populateSigiToLLVMFinalTypeConversions(converter);

    closure::populateClosureToLLVMFinalTypeConversions(converter);
    closure::populateClosureToLLVMConversionPatterns(converter, patterns);

    mlir::populateFuncToLLVMConversionPatterns(converter, patterns);
    
    auto op = getOperation();
    sigiAttributeWalk(op, &getContext());

    if (failed(applyFullConversion(
            getOperation(), // Walk IR 
            target,
            std::move(patterns))))
        signalPassFailure(); 

}

void mlir::sigi::populateSigiToLLVMConversionPatterns(LLVMTypeConverter &typeConverter, RewritePatternSet &patterns) {
    patterns.add<ConvertSigiPopOpToLLVM,ConvertSigiPushOpToLLVM>(typeConverter);
}

void mlir::sigi::populateSigiToLLVMFinalTypeConversions(LLVMTypeConverter &typeConverter )
{
    typeConverter.addConversion(
        [&] (sigi::StackType stackType) 
            {return llvmStackStructType(stackType.getContext());}
    );
}




std::unique_ptr<Pass> mlir::sigi::createConvertSigiToLLVMPass()
{
    return std::make_unique<ConvertSigiToLLVMPass>();
}