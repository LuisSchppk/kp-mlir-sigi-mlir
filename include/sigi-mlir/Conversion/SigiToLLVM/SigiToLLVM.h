#pragma once

#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir::sigi {

void populateSigiToLLVMFinalTypeConversions(
    LLVMTypeConverter &typeConverter);

void populateSigiToLLVMConversionPatterns(
    LLVMTypeConverter &typeConverter,
    RewritePatternSet &patterns);

std::unique_ptr<Pass> createConvertSigiToLLVMPass();

}