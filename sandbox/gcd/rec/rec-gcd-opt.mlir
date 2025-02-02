module {
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func @gcd(%arg0: !sigi.stack) -> !sigi.stack attributes {transformed_to_iterative} {
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %arg0 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %0 = arith.cmpi eq, %value, %c0_i32 : i32
    %1:2 = scf.while (%arg1 = %value, %arg2 = %value_1, %arg3 = %0) : (i32, i32, i1) -> (i32, i32) {
      scf.condition(%arg3) %arg1, %arg2 : i32, i32
    } do {
    ^bb0(%arg1: i32, %arg2: i32):
      %3 = arith.remsi %arg2, %arg1 : i32
      %4 = arith.cmpi eq, %3, %c0_i32 : i32
      scf.yield %3, %arg1, %4 : i32, i32, i1
    }
    %2 = sigi.push %out_stack_0, %1#1 : i32
    return %2 : !sigi.stack
  }
  func.func @__main__(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.main} {
    %c8_i32 = arith.constant 8 : i32
    %c24_i32 = arith.constant 24 : i32
    %0 = sigi.push %arg0, %c24_i32 : i32
    %1 = sigi.push %0, %c8_i32 : i32
    %2 = call @gcd(%1) : (!sigi.stack) -> !sigi.stack
    %3 = call @"sigi::pp"(%2) : (!sigi.stack) -> !sigi.stack
    %out_stack, %value = sigi.pop %3 : i32
    return %out_stack : !sigi.stack
  }
}

