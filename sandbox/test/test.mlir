module {
    func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes { sigi.builtinfunc }
    // apply: (-> int) -> int
    func.func private @apply(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> i32} {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (-> int)
        %s2 = closure.call %v1_f (%s1) { sigi.stackType = () -> i32 } : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: -> int
        return %s2: !sigi.stack
    }
    // show: int ->
    func.func private @show(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32) -> ()} {
        %s1 = func.call @"sigi::pp"(%s0) { sigi.stackType = (i32) -> i32 } : (!sigi.stack) -> !sigi.stack // int -> int
        %s2, %v1 = sigi.pop %s1: i32 // pop intrinsic
        return %s2: !sigi.stack
    }
    // pow: int, int -> int
    func.func private @pow(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32) -> i32} {
        %v1 = arith.constant 1: i32
        %s1 = sigi.push %s0, %v1: i32
        %s2 = func.call @pow_loop(%s1) { sigi.stackType = (i32, i32, i32) -> i32 } : (!sigi.stack) -> !sigi.stack // int, int, int -> int
        return %s2: !sigi.stack
    }
    // pow_loop: int, int, int -> int
    func.func private @pow_loop(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32, i32) -> i32} {
        // -> base, exp, result;
        %s1, %v1_result = sigi.pop %s0: i32 // result: int
        %s2, %v2_exp = sigi.pop %s1: i32 // exp: int
        %s3, %v3_base = sigi.pop %s2: i32 // base: int
        %s4 = sigi.push %s3, %v2_exp: i32 // push exp
        %v4 = arith.constant 1: i32
        %s5 = sigi.push %s4, %v4: i32
        // =
        %s6, %v5 = sigi.pop %s5: i32
        %s7, %v6 = sigi.pop %s6: i32
        %v7 = arith.cmpi "eq", %v6, %v5: i32
        %s8 = sigi.push %s7, %v7: i1
        %v9 = closure.box [%v8_result = %v1_result : i32] (%s9 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> i32 } { // -> int
            %s10 = sigi.push %s9, %v8_result: i32 // push result
            closure.return %s10: !sigi.stack
        }
        %s11 = sigi.push %s8, %v9: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v13 = closure.box [%v10_base = %v3_base : i32, %v11_exp = %v2_exp : i32, %v12_result = %v1_result : i32] (%s12 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> i32 } { // -> int
            %s13 = sigi.push %s12, %v12_result: i32 // push result
            %s14 = func.call @show(%s13) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
            %s15 = sigi.push %s14, %v10_base: i32 // push base
            %s16 = sigi.push %s15, %v11_exp: i32 // push exp
            %v14 = arith.constant 1: i32
            %s17 = sigi.push %s16, %v14: i32
            // -
            %s18, %v15 = sigi.pop %s17: i32
            %s19, %v16 = sigi.pop %s18: i32
            %v17 = arith.subi %v16, %v15: i32
            %s20 = sigi.push %s19, %v17: i32
            %s21 = sigi.push %s20, %v10_base: i32 // push base
            %s22 = sigi.push %s21, %v12_result: i32 // push result
            // *
            %s23, %v18 = sigi.pop %s22: i32
            %s24, %v19 = sigi.pop %s23: i32
            %v20 = arith.muli %v19, %v18: i32
            %s25 = sigi.push %s24, %v20: i32
            %s26 = func.call @pow_loop(%s25) { sigi.stackType = (i32, i32, i32) -> i32 } : (!sigi.stack) -> !sigi.stack // int, int, int -> int
            closure.return %s26: !sigi.stack
        }
        %s27 = sigi.push %s11, %v13: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s28, %v21 = sigi.pop %s27: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s29, %v22 = sigi.pop %s28: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s30, %v23 = sigi.pop %s29: i1
        
        %v24 = scf.if %v23 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v22: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v21: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s31 = sigi.push %s30, %v24: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s32 = func.call @apply(%s31) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> i32 } : (!sigi.stack) -> !sigi.stack // (-> int) -> int
        return %s32: !sigi.stack
    }
    // __main__: ->
    func.func @__main__(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> (), sigi.main} {
        %v1 = arith.constant 2: i32
        %s1 = sigi.push %s0, %v1: i32
        %v2 = arith.constant 6: i32
        %s2 = sigi.push %s1, %v2: i32
        %s3 = func.call @pow(%s2) { sigi.stackType = (i32, i32) -> i32 } : (!sigi.stack) -> !sigi.stack // int, int -> int
        %s4 = func.call @show(%s3) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
        return %s4: !sigi.stack
    }
}
