module {
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func @gcd(%arg0: !sigi.stack) -> !sigi.stack attributes {transformed_to_iterative} {
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %arg0 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %0 = arith.cmpi eq, %value, %c0_i32 : i32
    %1:4 = scf.while (%arg1 = %out_stack_0, %arg2 = %value, %arg3 = %value_1, %arg4 = %0) : (!sigi.stack, i32, i32, i1) -> (!sigi.stack, i32, i32, i1) {
      scf.condition(%arg4) %arg1, %arg2, %arg3, %arg4 : !sigi.stack, i32, i32, i1
    } do {
    ^bb0(%arg1: !sigi.stack, %arg2: i32, %arg3: i32, %arg4: i1):
      %3 = sigi.push %arg1, %arg2 : i32
      %4 = arith.remsi %arg3, %arg2 : i32
      %5 = sigi.push %3, %4 : i32
      %out_stack_2, %value_3 = sigi.pop %5 : i32
      %out_stack_4, %value_5 = sigi.pop %out_stack_2 : i32
      %6 = arith.cmpi eq, %value_3, %c0_i32 : i32
      scf.yield {CallYield} %out_stack_4, %value_3, %value_5, %6 : !sigi.stack, i32, i32, i1
    }
    %2 = sigi.push %1#0, %1#2 : i32
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

