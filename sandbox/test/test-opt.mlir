module {
  func.func private @"sigi::pp_i32"(i32) attributes {sigi.builtinfunc}
  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @pow_loop(%arg0: i32, %arg1: i32, %arg2: i32) -> i32 attributes {sigi.stackType = (i32, i32, i32) -> i32} {
    %c0_i32 = arith.constant 0 : i32
    %c1_i32 = arith.constant 1 : i32
    %0 = arith.cmpi sle, %arg1, %c0_i32 : i32
    %1 = scf.if %0 -> (i32) {
      scf.yield %arg0 : i32
    } else {
      func.call @"sigi::pp_i32"(%arg0) : (i32) -> ()
      %2 = arith.subi %arg1, %c1_i32 : i32
      %3 = arith.muli %arg2, %arg0 : i32
      %4 = func.call @pow_loop(%3, %2, %arg2) : (i32, i32, i32) -> i32
      scf.yield %4 : i32
    }
    return %1 : i32
  }
  func.func @__main__() -> i32 attributes {sigi.main, sigi.stackType = () -> i32} {
    %c2_i32 = arith.constant 2 : i32
    %c6_i32 = arith.constant 6 : i32
    %c1_i32 = arith.constant 1 : i32
    %0 = call @pow_loop(%c1_i32, %c6_i32, %c2_i32) : (i32, i32, i32) -> i32
    %1 = arith.muli %0, %0 : i32
    return %1 : i32
  }
}

