add_mlir_dialect_library(AdditionalPasses
            TailRecursionPass.cpp
            ClosureInline.cpp
            StackElimination.cpp
            GlobalStackPass.cpp
            TypedBuiltInsPass.cpp
            Utils.h
            Utils.cpp

    LINK_LIBS PUBLIC
        ClosureIR
        SigiIR
        MLIRIR
        MLIRParser
        MLIRSideEffectInterfaces
)
