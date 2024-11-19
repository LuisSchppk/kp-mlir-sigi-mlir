add_mlir_conversion_library(MLIRSigiToLLVM
  SigiToLLVM.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/sigi-mlir/Conversion/SigiToLLVM

  DEPENDS
  SigiConversionPassIncGen

  LINK_COMPONENTS
  Core

  LINK_LIBS PUBLIC
  SigiIR
#   SigiTransforms
  MLIRDialectUtils
  MLIRFuncDialect
  MLIRTransformUtils
  MLIRReconcileUnrealizedCasts
  )