module {
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @fizzbuzz(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32) -> ()} {
    %out_stack, %value = sigi.pop %arg0 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %0 = arith.cmpi sle, %value_1, %value : i32
    %1 = closure.box [%arg1 = %value_1 : i32, %arg2 = %value : i32](%arg3: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> ()} {
      %c0_i32 = arith.constant 0 : i32
      %c2_i32 = arith.constant 2 : i32
      %5 = arith.remui %arg1, %c2_i32 : i32
      %6 = arith.cmpi eq, %5, %c0_i32 : i32
      %7 = closure.box [%arg4 = %arg1 : i32](%arg5: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> ()} {
        %c0_i32_2 = arith.constant 0 : i32
        %c5_i32 = arith.constant 5 : i32
        %11 = arith.remui %arg4, %c5_i32 : i32
        %12 = arith.cmpi eq, %11, %c0_i32_2 : i32
        %13 = closure.box [](%arg6: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> ()} {
          %c-3_i32 = arith.constant -3 : i32
          %17 = sigi.push %arg6, %c-3_i32 : i32
          %18 = func.call @"sigi::pp"(%17) {sigi.stackType = (i32) -> i32} : (!sigi.stack) -> !sigi.stack
          %out_stack_3, %value_4 = sigi.pop %18 : i32
          closure.return %out_stack_3 : !sigi.stack
        }
        %14 = closure.box [](%arg6: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> ()} {
          %c-2_i32 = arith.constant -2 : i32
          %17 = sigi.push %arg6, %c-2_i32 : i32
          %18 = func.call @"sigi::pp"(%17) {sigi.stackType = (i32) -> i32} : (!sigi.stack) -> !sigi.stack
          %out_stack_3, %value_4 = sigi.pop %18 : i32
          closure.return %out_stack_3 : !sigi.stack
        }
        %15 = arith.select %12, %13, %14 : !closure.box<(!sigi.stack) -> !sigi.stack>
        %16 = closure.call %15(%arg5) {sigi.stackType = () -> ()} : <(!sigi.stack) -> !sigi.stack>
        closure.return %16 : !sigi.stack
      }
      %8 = closure.box [%arg4 = %arg1 : i32, %arg5 = %arg2 : i32](%arg6: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> ()} {
        %c1_i32 = arith.constant 1 : i32
        %11 = sigi.push %arg6, %arg4 : i32
        %12 = func.call @"sigi::pp"(%11) {sigi.stackType = (i32) -> i32} : (!sigi.stack) -> !sigi.stack
        %out_stack_2, %value_3 = sigi.pop %12 : i32
        %13 = arith.addi %arg4, %c1_i32 : i32
        %14 = sigi.push %out_stack_2, %13 : i32
        %15 = sigi.push %14, %arg5 : i32
        %16 = func.call @fizzbuzz(%15) {sigi.stackType = (i32, i32) -> ()} : (!sigi.stack) -> !sigi.stack
        closure.return %16 : !sigi.stack
      }
      %9 = arith.select %6, %7, %8 : !closure.box<(!sigi.stack) -> !sigi.stack>
      %10 = closure.call %9(%arg3) {sigi.stackType = () -> ()} : <(!sigi.stack) -> !sigi.stack>
      closure.return %10 : !sigi.stack
    }
    %2 = closure.box [](%arg1: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> ()} {
      closure.return %arg1 : !sigi.stack
    }
    %3 = arith.select %0, %1, %2 : !closure.box<(!sigi.stack) -> !sigi.stack>
    %4 = closure.call %3(%out_stack_0) {sigi.stackType = () -> ()} : <(!sigi.stack) -> !sigi.stack>
    return %4 : !sigi.stack
  }
  func.func @__main__(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.main, sigi.stackType = () -> ()} {
    %c100_i32 = arith.constant 100 : i32
    %c0_i32 = arith.constant 0 : i32
    %0 = sigi.push %arg0, %c0_i32 : i32
    %1 = sigi.push %0, %c100_i32 : i32
    %2 = call @fizzbuzz(%1) {sigi.stackType = (i32, i32) -> ()} : (!sigi.stack) -> !sigi.stack
    return %2 : !sigi.stack
  }
}

