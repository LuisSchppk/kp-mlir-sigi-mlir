add_mlir_dialect_library(AdditionalPasses
            TailRecursionPass.cpp
            ClosureInline.cpp
            DestackPass.cpp


    LINK_LIBS PUBLIC
        ClosureIR
        SigiIR
        MLIRIR
        MLIRParser
        MLIRSideEffectInterfaces
)
