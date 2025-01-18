/// Main entry point for the sigi-mlir optimizer driver.
///
/// @file
/// @author      Karl F. A. Friebel (karl.friebel@tu-dresden.de)

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"

#include "sigi-mlir/Dialect/Closure/IR/ClosureDialect.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiDialect.h"

#include "mlir/IR/AsmState.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllExtensions.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/Pass.h"

#include "mlir/Pass/PassManager.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"

#include "sigi-mlir/Conversion/ClosurePasses.h"
#include "sigi-mlir/Conversion/SigiPasses.h"

using namespace mlir;

/// Add all the MLIR dialects to the provided registry.
inline void registerUsedDialects(DialectRegistry &registry)
{
    // clang-format off
  registry.insert<
    	        //   acc::OpenACCDialect,
                  affine::AffineDialect,
                //   amdgpu::AMDGPUDialect,
                //   amx::AMXDialect,
                  arith::ArithDialect,
                //   arm_neon::ArmNeonDialect,
                //   arm_sme::ArmSMEDialect,
                //   arm_sve::ArmSVEDialect,
                //   async::AsyncDialect,
                  bufferization::BufferizationDialect,
                  cf::ControlFlowDialect,
                //   complex::ComplexDialect,
                //   DLTIDialect,
                //   emitc::EmitCDialect,
                  func::FuncDialect,
                //   gpu::GPUDialect,
                //   index::IndexDialect,
                //   irdl::IRDLDialect,
                  linalg::LinalgDialect,
                  LLVM::LLVMDialect,
                  math::MathDialect,
                  memref::MemRefDialect,
                //   mesh::MeshDialect,
                //   ml_program::MLProgramDialect,
                //   mpi::MPIDialect,
                //   nvgpu::NVGPUDialect,
                //   NVVM::NVVMDialect,
                //   omp::OpenMPDialect,
                //   pdl::PDLDialect,
                //   pdl_interp::PDLInterpDialect,
                //   polynomial::PolynomialDialect,
                //   ptr::PtrDialect,
                //   quant::QuantizationDialect,
                //   ROCDL::ROCDLDialect,
//                   shape::ShapeDialect,
//                   sparse_tensor::SparseTensorDialect,
//                   spirv::SPIRVDialect,
//                   tensor::TensorDialect,
//                   tosa::TosaDialect,
//                   transform::TransformDialect,
//                   ub::UBDialect,
//                   vector::VectorDialect,
//                   x86vector::X86VectorDialect,
//                   xegpu::XeGPUDialect,
                  scf::SCFDialect>();
//   // clang-format on

  // Register all external models.
  affine::registerValueBoundsOpInterfaceExternalModels(registry);
  arith::registerBufferDeallocationOpInterfaceExternalModels(registry);
  arith::registerBufferizableOpInterfaceExternalModels(registry);
  arith::registerBufferViewFlowOpInterfaceExternalModels(registry);
  arith::registerValueBoundsOpInterfaceExternalModels(registry);
  bufferization::func_ext::registerBufferizableOpInterfaceExternalModels(
      registry);
  builtin::registerCastOpInterfaceExternalModels(registry);
  cf::registerBufferizableOpInterfaceExternalModels(registry);
  cf::registerBufferDeallocationOpInterfaceExternalModels(registry);
//   gpu::registerBufferDeallocationOpInterfaceExternalModels(registry);
  linalg::registerAllDialectInterfaceImplementations(registry);
  linalg::registerRuntimeVerifiableOpInterfaceExternalModels(registry);
  memref::registerAllocationOpInterfaceExternalModels(registry);
  memref::registerBufferViewFlowOpInterfaceExternalModels(registry);
  memref::registerRuntimeVerifiableOpInterfaceExternalModels(registry);
  memref::registerValueBoundsOpInterfaceExternalModels(registry);
  memref::registerMemorySlotExternalModels(registry);
//   ml_program::registerBufferizableOpInterfaceExternalModels(registry);
  scf::registerBufferDeallocationOpInterfaceExternalModels(registry);
  scf::registerBufferizableOpInterfaceExternalModels(registry);
  scf::registerValueBoundsOpInterfaceExternalModels(registry);
//   shape::registerBufferizableOpInterfaceExternalModels(registry);
//   sparse_tensor::registerBufferizableOpInterfaceExternalModels(registry);
//   tensor::registerBufferizableOpInterfaceExternalModels(registry);
//   tensor::registerFindPayloadReplacementOpInterfaceExternalModels(registry);
//   tensor::registerInferTypeOpInterfaceExternalModels(registry);
//   tensor::registerSubsetOpInterfaceExternalModels(registry);
//   tensor::registerTilingInterfaceExternalModels(registry);
//   tensor::registerValueBoundsOpInterfaceExternalModels(registry);
//   tosa::registerShardingInterfaceExternalModels(registry);
//   vector::registerBufferizableOpInterfaceExternalModels(registry);
//   vector::registerSubsetOpInterfaceExternalModels(registry);
//   vector::registerValueBoundsOpInterfaceExternalModels(registry);
//   NVVM::registerNVVMTargetInterfaceExternalModels(registry);
//   ROCDL::registerROCDLTargetInterfaceExternalModels(registry);
//   spirv::registerSPIRVTargetInterfaceExternalModels(registry);
}

int main(int argc, char* argv[])
{
    DialectRegistry registry;
    // registerAllDialects(registry);
    registerUsedDialects(registry);
    registerAllExtensions(registry);

    registry.insert<closure::ClosureDialect, sigi::SigiDialect>();

    registerAllPasses();
    closure::registerClosureConversionPasses();
    sigi::registerSigiConversionPasses();

    return asMainReturnCode(
        MlirOptMain(argc, argv, "sigi-mlir optimizer driver\n", registry));
}
