module {
  func.func private @"sigi::pp_i32"(i32) attributes {sigi.builtinfunc}
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @external(!sigi.stack) -> !sigi.stack
  func.func private @gcd(%arg0: i32, %arg1: i32) -> i32 attributes {sigi.stackType = (i32, i32) -> i32} {
    %c0_i32 = arith.constant 0 : i32
    %0 = arith.cmpi eq, %arg0, %c0_i32 : i32
    %1 = scf.if %0 -> (i32) {
      scf.yield %arg1 : i32
    } else {
      %2 = arith.remui %arg1, %arg0 : i32
      %3 = func.call @gcd(%2, %arg0) : (i32, i32) -> i32
      scf.yield %3 : i32
    }
    return %1 : i32
  }
  func.func @__main__() attributes {sigi.main, sigi.stackType = () -> ()} {
    %c24_i32 = arith.constant 24 : i32
    %c8_i32 = arith.constant 8 : i32
    %0 = call @gcd(%c8_i32, %c24_i32) : (i32, i32) -> i32
    call @"sigi::pp_i32"(%0) : (i32) -> ()
    return
  }
}

