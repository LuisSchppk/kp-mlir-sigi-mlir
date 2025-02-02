module {
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @factorial(%arg0: !sigi.stack) -> !sigi.stack {
    %c1_i32 = arith.constant 1 : i32
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %arg0 : i32
    %0 = arith.cmpi eq, %value, %c0_i32 : i32
    %1 = scf.if %0 -> (!sigi.stack) {
      %2 = sigi.push %out_stack, %c1_i32 : i32
      scf.yield %2 : !sigi.stack
    } else {
      %2 = sigi.push %out_stack, %value : i32
      %3 = arith.subi %value, %c1_i32 : i32
      %4 = sigi.push %2, %3 : i32
      %5 = func.call @factorial(%4) : (!sigi.stack) -> !sigi.stack
      %out_stack_0, %value_1 = sigi.pop %5 : i32
      %out_stack_2, %value_3 = sigi.pop %out_stack_0 : i32
      %6 = arith.muli %value_3, %value_1 : i32
      %7 = sigi.push %out_stack_2, %6 : i32
      scf.yield %7 : !sigi.stack
    }
    return %1 : !sigi.stack
  }
  func.func @__main__(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.main} {
    %c5_i32 = arith.constant 5 : i32
    %0 = sigi.push %arg0, %c5_i32 : i32
    %1 = call @factorial(%0) : (!sigi.stack) -> !sigi.stack
    %2 = call @"sigi::pp"(%1) : (!sigi.stack) -> !sigi.stack
    %out_stack, %value = sigi.pop %2 : i32
    return %out_stack : !sigi.stack
  }
}

