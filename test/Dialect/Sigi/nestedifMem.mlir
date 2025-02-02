module {
  func.func @nested_if(%arg0: i32, %arg1: i32, %arg2: i32) -> i32 attributes {transformed_to_iterative} {
    %c0_i32 = arith.constant 0 : i32
    %c1_i32 = arith.constant 1 : i32
    %c2_i32 = arith.constant 2 : i32
    %c3_i32 = arith.constant 3 : i32
    %0 = arith.remsi %arg0, %c2_i32 : i32
    %1 = arith.cmpi eq, %arg0, %c0_i32 : i32
    %2 = arith.cmpi eq, %0, %c0_i32 : i32
    %3 = arith.cmpi ne, %arg0, %c3_i32 : i32
    %false = arith.constant false
    %alloc = memref.alloc() : memref<1xi32>
    %4:7 = scf.while (%arg3 = %arg0, %arg4 = %arg1, %arg5 = %arg2, %arg6 = %2, %arg7 = %3, %arg8 = %1, %arg9 = %false) : (i32, i32, i32, i1, i1, i1, i1) -> (i32, i32, i32, i1, i1, i1, i1) {
      scf.condition(%arg8) %arg3, %arg4, %arg5, %arg6, %arg7, %arg8, %arg9 : i32, i32, i32, i1, i1, i1, i1
    } do {
    ^bb0(%arg3: i32, %arg4: i32, %arg5: i32, %arg6: i1, %arg7: i1, %arg8: i1, %arg9: i1):
      %6 = arith.subi %arg3, %c1_i32 : i32
      %7:4 = scf.if %arg7 -> (i32, i32, i32, i1) {
        %13:4 = scf.if %arg6 -> (i32, i32, i32, i1) {
          %14 = arith.addi %arg4, %arg3 : i32
          scf.yield {CallYield} %6, %14, %arg5, %arg9 : i32, i32, i32, i1
        } else {
          %c0 = arith.constant 0 : index
          memref.store %6, %alloc[%c0] : memref<1xi32>
          %true = arith.constant true
          scf.yield {terminationYield} %arg3, %arg3, %arg3, %true : i32, i32, i32, i1
        }
        scf.yield {CallYield} %13#0, %13#1, %13#2, %13#3 : i32, i32, i32, i1
      } else {
        %13 = arith.addi %arg5, %arg3 : i32
        %c0 = arith.constant 0 : index
        memref.store %13, %alloc[%c0] : memref<1xi32>
        %true = arith.constant true
        scf.yield {terminationYield} %arg3, %arg3, %arg3, %true : i32, i32, i32, i1
      }
      %8 = arith.remsi %6, %c2_i32 : i32
      %9 = arith.cmpi eq, %6, %c0_i32 : i32
      %10 = arith.cmpi eq, %8, %c0_i32 : i32
      %11 = arith.cmpi ne, %6, %c3_i32 : i32
      %12 = arith.cmpi sgt, %9, %7#3 : i1
      scf.yield {CallYield} %7#0, %7#1, %7#2, %10, %11, %12, %7#3 : i32, i32, i32, i1, i1, i1, i1
    }
    %5 = scf.if %4#6 -> (i32) {
      %c0 = arith.constant 0 : index
      %6 = memref.load %alloc[%c0] : memref<1xi32>
      scf.yield %6 : i32
    } else {
      %6 = arith.addi %4#1, %4#2 : i32
      scf.yield %6 : i32
    }
    memref.dealloc %alloc : memref<1xi32>
    return %5 : i32
  }
}

