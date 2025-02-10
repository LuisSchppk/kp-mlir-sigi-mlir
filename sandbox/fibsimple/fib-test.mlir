module {
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @fibloop(%arg0: i32, %arg1: i32) attributes {sigi.stackType = (i32, i32) -> ()} {
    %0 = "sigi.global"() : () -> !sigi.stack
    %1 = sigi.push %0, %arg1 : i32
    %2 = sigi.push %1, %arg0 : i32
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %2 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %3 = arith.cmpi eq, %value_1, %c0_i32 : i32
    %4 = scf.if %3 -> (!sigi.stack) {
      scf.yield %out_stack_0 : !sigi.stack
    } else {
      %c10_i32 = arith.constant 10 : i32
      %c1_i32 = arith.constant 1 : i32
      %5 = sigi.push %out_stack_0, %value : i32
      %6 = func.call @"sigi::pp"(%5) {sigi.stackType = (i32) -> i32} : (!sigi.stack) -> !sigi.stack
      %out_stack_2, %value_3 = sigi.pop %6 : i32
      %7 = arith.subi %value_1, %c1_i32 : i32
      %8 = sigi.push %out_stack_2, %7 : i32
      %9 = arith.muli %value, %c10_i32 : i32
      %10 = sigi.push %8, %9 : i32
      %out_stack_4, %value_5 = sigi.pop %10 : i32
      %out_stack_6, %value_7 = sigi.pop %out_stack_4 : i32
      func.call @fibloop(%value_5, %value_7) : (i32, i32) -> ()
      scf.yield %out_stack_6 : !sigi.stack
    }
    return
  }
  func.func @__main__() attributes {sigi.main, sigi.stackType = () -> ()} {
    %0 = "sigi.global"() : () -> !sigi.stack
    %c1_i32 = arith.constant 1 : i32
    %c10_i32 = arith.constant 10 : i32
    %1 = sigi.push %0, %c10_i32 : i32
    %2 = sigi.push %1, %c1_i32 : i32
    %out_stack, %value = sigi.pop %2 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    call @fibloop(%value, %value_1) : (i32, i32) -> ()
    return
  }
}

