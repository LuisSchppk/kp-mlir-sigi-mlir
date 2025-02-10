module {
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @"external"(!sigi.stack) -> !sigi.stack
  func.func private @gcd(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32) -> i32} {
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %arg0 : i32
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32
    %0 = arith.cmpi eq, %value, %c0_i32 : i32
    %1 = scf.if %0 -> (!sigi.stack) {
      %2 = sigi.push %out_stack_0, %value_1 : i32
      scf.yield %2 : !sigi.stack
    } else {
      %2 = sigi.push %out_stack_0, %value : i32
      %3 = arith.remui %value_1, %value : i32
      %4 = sigi.push %2, %3 : i32
      %5 = func.call @gcd(%4) {sigi.stackType = (i32, i32) -> i32} : (!sigi.stack) -> !sigi.stack
      scf.yield %5 : !sigi.stack
    }
    return %1 : !sigi.stack
  }

  func.func @__main__(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.main, sigi.stackType = () -> ()} {
    %c8_i32 = arith.constant 8 : i32
    %c24_i32 = arith.constant 24 : i32
    %0 = sigi.push %arg0, %c24_i32 : i32
    %1 = sigi.push %0, %c8_i32 : i32
    %2 = call @gcd(%1) {sigi.stackType = (i32, i32) -> i32} : (!sigi.stack) -> !sigi.stack
    %3 = call @"sigi::pp"(%2) {sigi.stackType = (i32) -> i32} : (!sigi.stack) -> !sigi.stack
    %out_stack, %value = sigi.pop %3 : i32
    return %out_stack : !sigi.stack
  }

    func.func private @test(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32, i32) -> i32} {
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %arg0 : i32
    %0 = arith.cmpi eq, %value, %c0_i32 : i32
    %value_1 = scf.if %0 -> (i32) {
      %out_stack_0, %2 = sigi.pop %out_stack : i32
      scf.yield %2 : i32
    } else {
      %out_stack_0, %2 = sigi.pop %out_stack : i32
      %out_stack_2, %3 = sigi.pop %out_stack_0 : i32
      scf.yield %3 : i32
    }
    
    %1 = scf.if %0 -> (!sigi.stack) {
      %2 = sigi.push %out_stack, %value_1 : i32
      scf.yield %2 : !sigi.stack
    } else {
      %2 = sigi.push %out_stack, %value : i32
      scf.yield %2 : !sigi.stack
    }
    %3 = call @"external"(%1) {sigi.stackType = (i32, i32) -> i32} : (!sigi.stack) -> !sigi.stack
    return %1 : !sigi.stack
  }
}

