module {
  func.func private @"sigi::pp_i32"(i32) attributes {sigi.builtinfunc}
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @fizzbuzz(%arg0: i32, %arg1: i32) attributes {sigi.stackType = (i32, i32) -> ()} {
    %0 = "sigi.load.global"() : () -> !sigi.stack
    %1 = sigi.push %0, %arg1 : i32
    %2 = sigi.push %1, %arg0 : i32
    %c2_i32 = arith.constant 2 : i32
    %c0_i32 = arith.constant 0 : i32
    %c-1_i32 = arith.constant -1 : i32
    %c-3_i32 = arith.constant -3 : i32
    %c1_i32 = arith.constant 1 : i32
    %c5_i32 = arith.constant 5 : i32
    %c-2_i32 = arith.constant -2 : i32
    %out_stack, %value = sigi.pop %2 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %3 = arith.cmpi sle, %value_1, %value : i32
    %4 = scf.if %3 -> (!sigi.stack) {
      %5 = arith.remui %value_1, %c2_i32 : i32
      %6 = arith.cmpi eq, %5, %c0_i32 : i32
      %7 = scf.if %6 -> (!sigi.stack) {
        %8 = arith.remui %value_1, %c5_i32 : i32
        %9 = arith.cmpi eq, %8, %c0_i32 : i32
        %10 = scf.if %9 -> (!sigi.stack) {
          func.call @"sigi::pp_i32"(%c-3_i32) : (i32) -> ()
          %11 = arith.addi %value_1, %c1_i32 : i32
          %12 = sigi.push %out_stack_0, %11 : i32
          %13 = sigi.push %12, %value : i32
          %out_stack_2, %value_3 = sigi.pop %13 : i32
          %out_stack_4, %value_5 = sigi.pop %out_stack_2 : i32
          "sigi.store.global"(%out_stack_4) : (!sigi.stack) -> ()
          func.call @fizzbuzz(%value_3, %value_5) : (i32, i32) -> ()
          %14 = "sigi.load.global"() : () -> !sigi.stack
          scf.yield %14 : !sigi.stack
        } else {
          func.call @"sigi::pp_i32"(%c-1_i32) : (i32) -> ()
          %11 = arith.addi %value_1, %c1_i32 : i32
          %12 = sigi.push %out_stack_0, %11 : i32
          %13 = sigi.push %12, %value : i32
          %out_stack_2, %value_3 = sigi.pop %13 : i32
          %out_stack_4, %value_5 = sigi.pop %out_stack_2 : i32
          "sigi.store.global"(%out_stack_4) : (!sigi.stack) -> ()
          func.call @fizzbuzz(%value_3, %value_5) : (i32, i32) -> ()
          %14 = "sigi.load.global"() : () -> !sigi.stack
          scf.yield %14 : !sigi.stack
        }
        scf.yield %10 : !sigi.stack
      } else {
        %8 = arith.remui %value_1, %c5_i32 : i32
        %9 = arith.cmpi eq, %8, %c0_i32 : i32
        %10 = scf.if %9 -> (!sigi.stack) {
          func.call @"sigi::pp_i32"(%c-2_i32) : (i32) -> ()
          %11 = arith.addi %value_1, %c1_i32 : i32
          %12 = sigi.push %out_stack_0, %11 : i32
          %13 = sigi.push %12, %value : i32
          %out_stack_2, %value_3 = sigi.pop %13 : i32
          %out_stack_4, %value_5 = sigi.pop %out_stack_2 : i32
          "sigi.store.global"(%out_stack_4) : (!sigi.stack) -> ()
          func.call @fizzbuzz(%value_3, %value_5) : (i32, i32) -> ()
          %14 = "sigi.load.global"() : () -> !sigi.stack
          scf.yield %14 : !sigi.stack
        } else {
          func.call @"sigi::pp_i32"(%value_1) : (i32) -> ()
          %11 = arith.addi %value_1, %c1_i32 : i32
          %12 = sigi.push %out_stack_0, %11 : i32
          %13 = sigi.push %12, %value : i32
          %out_stack_2, %value_3 = sigi.pop %13 : i32
          %out_stack_4, %value_5 = sigi.pop %out_stack_2 : i32
          "sigi.store.global"(%out_stack_4) : (!sigi.stack) -> ()
          func.call @fizzbuzz(%value_3, %value_5) : (i32, i32) -> ()
          %14 = "sigi.load.global"() : () -> !sigi.stack
          scf.yield %14 : !sigi.stack
        }
        scf.yield %10 : !sigi.stack
      }
      scf.yield %7 : !sigi.stack
    } else {
      scf.yield %out_stack_0 : !sigi.stack
    }
    "sigi.store.global"(%4) : (!sigi.stack) -> ()
    return
  }
  func.func @__main__() attributes {sigi.main, sigi.stackType = () -> ()} {
    %0 = "sigi.load.global"() : () -> !sigi.stack
    %c100_i32 = arith.constant 100 : i32
    %c0_i32 = arith.constant 0 : i32
    %1 = sigi.push %0, %c0_i32 : i32
    %2 = sigi.push %1, %c100_i32 : i32
    %out_stack, %value = sigi.pop %2 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    "sigi.store.global"(%out_stack_0) : (!sigi.stack) -> ()
    call @fizzbuzz(%value, %value_1) : (i32, i32) -> ()
    %3 = "sigi.load.global"() : () -> !sigi.stack
    "sigi.store.global"(%3) : (!sigi.stack) -> ()
    return
  }
}

