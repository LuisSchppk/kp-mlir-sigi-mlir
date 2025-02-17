module {
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @while(%arg0: i1, %arg1: i32, %arg2: !closure.box<(!sigi.stack) -> !sigi.stack>, %arg3: !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32 attributes {sigi.stackType = (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32} {
    %0 = "sigi.load.global"() : () -> !sigi.stack
    %1 = sigi.push %0, %arg0 : i1
    %2 = sigi.push %1, %arg1 : i32
    %3 = closure.call %arg2(%2) {sigi.stackType = (i1, i32) -> (i1, i32, i1)} : <(!sigi.stack) -> !sigi.stack>
    %out_stack, %value = sigi.pop %3 : i1
    %4 = closure.call %arg3(%out_stack) {sigi.stackType = (i1, i32) -> i32} : <(!sigi.stack) -> !sigi.stack>
    %5 = scf.if %value -> (!sigi.stack) {
      %out_stack_2, %value_3 = sigi.pop %4 : i32
      %out_stack_4, %value_5 = sigi.pop %out_stack_2 : i1
      "sigi.store.global"(%out_stack_4) : (!sigi.stack) -> ()
      %6 = func.call @while(%value_5, %value_3, %arg2, %arg3) : (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32
      %7 = "sigi.load.global"() : () -> !sigi.stack
      %8 = sigi.push %7, %6 : i32
      scf.yield %8 : !sigi.stack
    } else {
      scf.yield %4 : !sigi.stack
    }
    %out_stack_0, %value_1 = sigi.pop %5 : i32
    "sigi.store.global"(%out_stack_0) : (!sigi.stack) -> ()
    return %value_1 : i32
  }
  func.func private @print_stack_and_pop(%arg0: i1, %arg1: i32) -> i32 attributes {sigi.stackType = (i1, i32) -> i32} {
    %0 = closure.box [](%arg2: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32) -> (i32, i1)} {
      %c0_i32 = arith.constant 0 : i32
      %c1_i32 = arith.constant 1 : i32
      %out_stack, %value = sigi.pop %arg2 : i32
      %3 = arith.subi %value, %c1_i32 : i32
      %4 = sigi.push %out_stack, %3 : i32
      %5 = arith.cmpi sgt, %value, %c0_i32 : i32
      %6 = sigi.push %4, %5 : i1
      closure.return %6 : !sigi.stack
    }
    %1 = closure.box [](%arg2: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i1, i32) -> i32} {
      %out_stack, %value = sigi.pop %arg2 : i32
      %out_stack_0, %value_1 = sigi.pop %out_stack : i1
      %3 = sigi.push %out_stack_0, %value_1 : i1
      %4 = func.call @"sigi::pp"(%3) {sigi.stackType = (i1) -> i1} : (!sigi.stack) -> !sigi.stack
      %out_stack_2, %value_3 = sigi.pop %4 : i1
      %5 = sigi.push %out_stack_2, %value : i32
      closure.return %5 : !sigi.stack
    }
    %2 = call @while(%arg0, %arg1, %0, %1) : (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32
    return %2 : i32
  }
  func.func private @pow_loop(%arg0: i32, %arg1: i32, %arg2: i32) -> (i32, i1) attributes {sigi.stackType = (i32, i32, i32) -> (i32, i1)} {
    %c0_i32 = arith.constant 0 : i32
    %true = arith.constant true
    %c1_i32 = arith.constant 1 : i32
    %0 = "sigi.load.global"() : () -> !sigi.stack
    %1 = arith.cmpi sle, %arg1, %c0_i32 : i32
    %2:2 = scf.if %1 -> (i1, !sigi.stack) {
      %4 = sigi.push %0, %arg2 : i32
      scf.yield %true, %4 : i1, !sigi.stack
    } else {
      %4 = sigi.push %0, %arg2 : i32
      %5 = func.call @"sigi::pp"(%4) {sigi.stackType = (i32) -> i32} : (!sigi.stack) -> !sigi.stack
      %out_stack_2, %value_3 = sigi.pop %5 : i32
      %6 = arith.subi %arg1, %c1_i32 : i32
      %7 = arith.muli %arg0, %arg2 : i32
      "sigi.store.global"(%out_stack_2) : (!sigi.stack) -> ()
      %8:2 = func.call @pow_loop(%arg0, %6, %7) : (i32, i32, i32) -> (i32, i1)
      %9 = "sigi.load.global"() : () -> !sigi.stack
      %10 = sigi.push %9, %8#0 : i32
      scf.yield %8#1, %10 : i1, !sigi.stack
    }
    %3 = sigi.push %2#1, %2#0 : i1
    %out_stack, %value = sigi.pop %3 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i1
    "sigi.store.global"(%out_stack_0) : (!sigi.stack) -> ()
    return %value, %value_1 : i32, i1
  }
  func.func @__main__() -> (i32, i32) attributes {sigi.main, sigi.stackType = () -> (i32, i32)} {
    %c2_i32 = arith.constant 2 : i32
    %c6_i32 = arith.constant 6 : i32
    %c1_i32 = arith.constant 1 : i32
    %0:2 = call @pow_loop(%c2_i32, %c6_i32, %c1_i32) : (i32, i32, i32) -> (i32, i1)
    %1 = "sigi.load.global"() : () -> !sigi.stack
    %2 = sigi.push %1, %0#0 : i32
    "sigi.store.global"(%2) : (!sigi.stack) -> ()
    %3 = call @print_stack_and_pop(%0#1, %c2_i32) : (i1, i32) -> i32
    %4 = "sigi.load.global"() : () -> !sigi.stack
    %out_stack, %value = sigi.pop %4 : i32
    "sigi.store.global"(%out_stack) : (!sigi.stack) -> ()
    return %3, %value : i32, i32
  }
}

