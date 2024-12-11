/// Implements the Sigi dialect base.
///
/// @file

#include "sigi-mlir/Dialect/Sigi/IR/SigiBase.h"

#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Transforms/InliningUtils.h"
#include "sigi-mlir/Dialect/Sigi/IR/SigiDialect.h"
#include "mlir/Transforms/InliningUtils.h"

#define DEBUG_TYPE "sigi-base"

using namespace mlir;
using namespace mlir::sigi;

//===- Generated implementation -------------------------------------------===//

#include "sigi-mlir/Dialect/Sigi/IR/SigiBase.cpp.inc"
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// SigiDialect
//===----------------------------------------------------------------------===//

class SigiInliner : public DialectInlinerInterface {
public:
    explicit SigiInliner(Dialect *dialect) : DialectInlinerInterface(dialect) {};

    bool  isLegalToInline(Operation *call, Operation *callable, bool wouldBeCloned) const override{
     return true;
  }

  bool isLegalToInline(Region *dest, Region *src, bool wouldBeCloned,
                               IRMapping &valueMapping) const override{
    return true;
  }

  virtual bool isLegalToInline(Operation *op, Region *dest, bool wouldBeCloned,
                               IRMapping &valueMapping) const override{
    return true;
  }
};

void SigiDialect::initialize()
{
    registerOps();
    registerTypes();
    addInterfaces<SigiInliner>();
}
