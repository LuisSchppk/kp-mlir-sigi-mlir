
func.func @simpleSigi(%s1: !sigi.stack) -> !sigi.stack {
    %s2, %v2_b = sigi.pop %s1: i32 // b: int
    %s3, %v3_a = sigi.pop %s2: i32 // a: int
    %s4 = sigi.push %s3, %v3_a: i32 // push a
    %s5 = sigi.push %s4, %v2_b: i32 // push b
    // *
    %s6, %v4 = sigi.pop %s5: i32
    %s7, %v5 = sigi.pop %s6: i32
    %v6 = arith.muli %v5, %v4: i32
    %s8 = sigi.push %s7, %v6: i32
    %v7 = arith.constant 1: i32
    %s9 = sigi.push %s8, %v7: i32
    // +
    %s10, %v8 = sigi.pop %s9: i32
    %s11, %v9 = sigi.pop %s10: i32
    %v10 = arith.addi %v9, %v8: i32
    %s12 = sigi.push %s11, %v10: i32
    return %s12: !sigi.stack
}