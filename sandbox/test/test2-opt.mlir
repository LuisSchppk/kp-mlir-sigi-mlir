module {
  func.func private @"sigi::pp_i1"(i1) attributes {sigi.builtinfunc}
  func.func private @"sigi::pp_i32"(i32) attributes {sigi.builtinfunc}
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @pow_loop(%arg0: i32, %arg1: i32, %arg2: i32) -> (i32, i1) attributes {sigi.stackType = (i32, i32, i32) -> (i32, i1)} {
    %0 = "sigi.load.global"() : () -> !sigi.stack
    %1 = sigi.push %0, %arg0 : i32
    %2 = sigi.push %1, %arg1 : i32
    %3 = sigi.push %2, %arg2 : i32
    %c1_i32 = arith.constant 1 : i32
    %true = arith.constant true
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %3 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %out_stack_2, %value_3 = sigi.pop %out_stack_0 : i32
    %4 = arith.cmpi sle, %value_1, %c0_i32 : i32
    %5:3 = scf.if %4 -> (i32, i1, !sigi.stack) {
      scf.yield %value, %true, %out_stack_2 : i32, i1, !sigi.stack
    } else {
      func.call @"sigi::pp_i32"(%value) : (i32) -> ()
      %8 = sigi.push %out_stack_2, %value_3 : i32
      %9 = arith.subi %value_1, %c1_i32 : i32
      %10 = sigi.push %8, %9 : i32
      %11 = arith.muli %value_3, %value : i32
      %12 = sigi.push %10, %11 : i32
      %out_stack_8, %value_9 = sigi.pop %12 : i32
      %out_stack_10, %value_11 = sigi.pop %out_stack_8 : i32
      %out_stack_12, %value_13 = sigi.pop %out_stack_10 : i32
      "sigi.store.global"(%out_stack_12) : (!sigi.stack) -> ()
      %13:2 = func.call @pow_loop(%value_13, %value_11, %value_9) : (i32, i32, i32) -> (i32, i1)
      %14 = "sigi.load.global"() : () -> !sigi.stack
      scf.yield %13#0, %13#1, %14 : i32, i1, !sigi.stack
    }
    %6 = sigi.push %5#2, %5#0 : i32
    %7 = sigi.push %6, %5#1 : i1
    %out_stack_4, %value_5 = sigi.pop %7 : i1
    %out_stack_6, %value_7 = sigi.pop %out_stack_4 : i32
    "sigi.store.global"(%out_stack_6) : (!sigi.stack) -> ()
    return %value_7, %value_5 : i32, i1
  }
  func.func @__main__() attributes {sigi.main, sigi.stackType = () -> ()} {
    %0 = "sigi.load.global"() : () -> !sigi.stack
    %c1_i32 = arith.constant 1 : i32
    %c6_i32 = arith.constant 6 : i32
    %c2_i32 = arith.constant 2 : i32
    %1 = sigi.push %0, %c2_i32 : i32
    %2 = sigi.push %1, %c6_i32 : i32
    %3 = sigi.push %2, %c1_i32 : i32
    %out_stack, %value = sigi.pop %3 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %out_stack_2, %value_3 = sigi.pop %out_stack_0 : i32
    "sigi.store.global"(%out_stack_2) : (!sigi.stack) -> ()
    %4:2 = call @pow_loop(%value_3, %value_1, %value) : (i32, i32, i32) -> (i32, i1)
    %5 = "sigi.load.global"() : () -> !sigi.stack
    call @"sigi::pp_i1"(%4#1) : (i1) -> ()
    call @"sigi::pp_i32"(%4#0) : (i32) -> ()
    "sigi.store.global"(%5) : (!sigi.stack) -> ()
    return
  }
}

