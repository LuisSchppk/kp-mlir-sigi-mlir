add_mlir_dialect_library(SigiIR
        SigiBase.cpp
        SigiOps.cpp
        SigiTypes.cpp

    DEPENDS
        SigiIncGen
        SigiConversionPassIncGen

    LINK_LIBS PUBLIC
        ClosureIR
        MLIRIR
        MLIRParser
        MLIRSideEffectInterfaces
)
