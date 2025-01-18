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

    bool  isLegalToInline(Operation *, Operation *, bool ) const final override{
     return true;
  }

  bool isLegalToInline(Region *, Region *, bool ,
                               IRMapping &) const final  override{
    return true;
  }

  bool isLegalToInline(Operation *, Region *, bool ,
                               IRMapping &) const final override{
    return true;
  }

  void handleTerminator(Operation*, Block*) const final override{}

  void handleTerminator(Operation*, ValueRange) const final override{}
};

void SigiDialect::initialize()
{
    registerOps();
    registerTypes();
    addInterface<SigiInliner>();
}
