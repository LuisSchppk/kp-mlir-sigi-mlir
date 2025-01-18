module {
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @fibloop(%arg0: !sigi.stack) -> !sigi.stack {
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %arg0 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %0 = arith.cmpi eq, %value_1, %c0_i32 : i32
    %1 = scf.if %0 -> (!sigi.stack) {
      scf.yield %out_stack_0 : !sigi.stack
    } else {
      %c10_i32 = arith.constant 10 : i32
      %c1_i32 = arith.constant 1 : i32
      %2 = sigi.push %out_stack_0, %value : i32
      %3 = func.call @"sigi::pp"(%2) : (!sigi.stack) -> !sigi.stack
      %out_stack_2, %value_3 = sigi.pop %3 : i32
      %4 = arith.subi %value_1, %c1_i32 : i32
      %5 = sigi.push %out_stack_2, %4 : i32
      %6 = arith.muli %value, %c10_i32 : i32
      %7 = sigi.push %5, %6 : i32
      %8 = func.call @fibloop(%7) : (!sigi.stack) -> !sigi.stack
      scf.yield %8 : !sigi.stack
    }
    return %1 : !sigi.stack
  }
  
  func.func @__main__(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.main} {
    %c1_i32 = arith.constant 1 : i32
    %c10_i32 = arith.constant 10 : i32
    %0 = sigi.push %arg0, %c10_i32 : i32
    %1 = sigi.push %0, %c1_i32 : i32
    %2 = call @fibloop(%1) : (!sigi.stack) -> !sigi.stack
    return %2 : !sigi.stack
  }
}