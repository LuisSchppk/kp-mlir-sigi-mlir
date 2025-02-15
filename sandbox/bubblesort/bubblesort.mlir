module {
    // apply: (-> int, int) -> int, int
    func.func private @apply(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> (i32, i32)} {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (-> int, int)
        %s2 = closure.call %v1_f (%s1) { sigi.stackType = () -> (i32, i32) } : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: -> int, int
        return %s2: !sigi.stack
    }
    // apply: int, int, (int, int -> int, int) -> int, int
    func.func private @"apply$2"(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32, !closure.box<(!sigi.stack) -> !sigi.stack>) -> (i32, i32)} {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (int, int -> int, int)
        %s2 = closure.call %v1_f (%s1) { sigi.stackType = (i32, i32) -> (i32, i32) } : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: int, int -> int, int
        return %s2: !sigi.stack
    }
    // bubble_once: int, int, int -> int, int
    func.func private @bubble_once(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32, i32) -> (i32, i32)} {
        // -> first, second, remaining;
        %s1, %v1_remaining = sigi.pop %s0: i32 // remaining: int
        %s2, %v2_second = sigi.pop %s1: i32 // second: int
        %s3, %v3_first = sigi.pop %s2: i32 // first: int
        %s4 = sigi.push %s3, %v3_first: i32 // push first
        %s5 = sigi.push %s4, %v2_second: i32 // push second
        // <=
        %s6, %v4 = sigi.pop %s5: i32
        %s7, %v5 = sigi.pop %s6: i32
        %v6 = arith.cmpi "sle", %v5, %v4: i32
        %s8 = sigi.push %s7, %v6: i1
        %v9 = closure.box [%v7_first = %v3_first : i32, %v8_second = %v2_second : i32] (%s9 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> (i32, i32) } { // -> int, int
            %s10 = sigi.push %s9, %v7_first: i32 // push first
            %s11 = sigi.push %s10, %v8_second: i32 // push second
            closure.return %s11: !sigi.stack
        }
        %s12 = sigi.push %s8, %v9: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v13 = closure.box [%v10_first = %v3_first : i32, %v11_second = %v2_second : i32, %v12_remaining = %v1_remaining : i32] (%s13 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> (i32, i32) } { // -> int, int
            %s14 = sigi.push %s13, %v11_second: i32 // push second
            %s15 = sigi.push %s14, %v10_first: i32 // push first
            %s16 = sigi.push %s15, %v12_remaining: i32 // push remaining
            %v14 = arith.constant 0: i32
            %s17 = sigi.push %s16, %v14: i32
            // >
            %s18, %v15 = sigi.pop %s17: i32
            %s19, %v16 = sigi.pop %s18: i32
            %v17 = arith.cmpi "sgt", %v16, %v15: i32
            %s20 = sigi.push %s19, %v17: i1
            %v19 = closure.box [%v18_remaining = %v12_remaining : i32] (%s21 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = (i32, i32) -> (i32, i32) } { // int, int -> int, int
                %s22 = sigi.push %s21, %v18_remaining: i32 // push remaining
                %v20 = arith.constant 1: i32
                %s23 = sigi.push %s22, %v20: i32
                // -
                %s24, %v21 = sigi.pop %s23: i32
                %s25, %v22 = sigi.pop %s24: i32
                %v23 = arith.subi %v22, %v21: i32
                %s26 = sigi.push %s25, %v23: i32
                %s27 = func.call @bubble_once(%s26) { sigi.stackType = (i32, i32, i32) -> (i32, i32) } : (!sigi.stack) -> !sigi.stack // int, int, int -> int, int
                closure.return %s27: !sigi.stack
            }
            %s28 = sigi.push %s20, %v19: !closure.box<(!sigi.stack) -> !sigi.stack>
            %v24 = closure.box [] (%s29 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                closure.return %s29: !sigi.stack
            }
            %s30 = sigi.push %s28, %v24: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s31, %v25 = sigi.pop %s30: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s32, %v26 = sigi.pop %s31: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s33, %v27 = sigi.pop %s32: i1
            
            %v28 = scf.if %v27 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
              scf.yield %v26: !closure.box<(!sigi.stack) -> !sigi.stack>
            } else {
              scf.yield %v25: !closure.box<(!sigi.stack) -> !sigi.stack>
            }
            %s34 = sigi.push %s33, %v28: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s35 = func.call @"apply$2"(%s34) { sigi.stackType = (i32, i32, !closure.box<(!sigi.stack) -> !sigi.stack>) -> (i32, i32) } : (!sigi.stack) -> !sigi.stack // int, int, (int, int -> int, int) -> int, int
            closure.return %s35: !sigi.stack
        }
        %s36 = sigi.push %s12, %v13: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s37, %v29 = sigi.pop %s36: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s38, %v30 = sigi.pop %s37: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s39, %v31 = sigi.pop %s38: i1
        
        %v32 = scf.if %v31 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v30: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v29: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s40 = sigi.push %s39, %v32: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s41 = func.call @apply(%s40) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> (i32, i32) } : (!sigi.stack) -> !sigi.stack // (-> int, int) -> int, int
        return %s41: !sigi.stack
    }
    // __main__: -> int, int, int, int, int
    func.func @__main__(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> (i32, i32, i32, i32, i32), sigi.main} {
        %v1 = arith.constant 42: i32
        %s1 = sigi.push %s0, %v1: i32
        %v2 = arith.constant 123: i32
        %s2 = sigi.push %s1, %v2: i32
        %v3 = arith.constant 123: i32
        %s3 = sigi.push %s2, %v3: i32
        %v4 = arith.constant 123: i32
        %s4 = sigi.push %s3, %v4: i32
        %v5 = arith.constant 12: i32
        %s5 = sigi.push %s4, %v5: i32
        %v6 = arith.constant 4: i32
        %s6 = sigi.push %s5, %v6: i32
        %s7 = func.call @bubble_once(%s6) { sigi.stackType = (i32, i32, i32) -> (i32, i32) } : (!sigi.stack) -> !sigi.stack // int, int, int -> int, int
        return %s7: !sigi.stack
    }
}
