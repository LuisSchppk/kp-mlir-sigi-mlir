module {
    func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes { sigi.builtinfunc }
    // apply: (-> int, bool) -> int, bool
    func.func private @apply(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> (i32, i1)} {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (-> int, bool)
        %s2 = closure.call %v1_f (%s1) { sigi.stackType = () -> (i32, i1) } : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: -> int, bool
        return %s2: !sigi.stack
    }
    // show: int ->
    func.func private @show(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32) -> ()} {
        %s1 = func.call @"sigi::pp"(%s0) { sigi.stackType = (i32) -> i32 } : (!sigi.stack) -> !sigi.stack // int -> int
        %s2, %v1 = sigi.pop %s1: i32 // pop intrinsic
        return %s2: !sigi.stack
    }
    // show: bool ->
    func.func private @"show$2"(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i1) -> ()} {
        %s1 = func.call @"sigi::pp"(%s0) { sigi.stackType = (i1) -> i1 } : (!sigi.stack) -> !sigi.stack // bool -> bool
        %s2, %v1 = sigi.pop %s1: i1 // pop intrinsic
        return %s2: !sigi.stack
    }
    // pow: int, int -> int, bool
    func.func private @pow(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32) -> (i32, i1)} {
        %v1 = arith.constant 1: i32
        %s1 = sigi.push %s0, %v1: i32
        %s2 = func.call @pow_loop(%s1) { sigi.stackType = (i32, i32, i32) -> (i32, i1) } : (!sigi.stack) -> !sigi.stack // int, int, int -> int, bool
        return %s2: !sigi.stack
    }
    // pow_loop: int, int, int -> int, bool
    func.func private @pow_loop(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32, i32) -> (i32, i1)} {
        // -> base, exp, result;
        %s1, %v1_result = sigi.pop %s0: i32 // result: int
        %s2, %v2_exp = sigi.pop %s1: i32 // exp: int
        %s3, %v3_base = sigi.pop %s2: i32 // base: int
        %s4 = sigi.push %s3, %v2_exp: i32 // push exp
        %v4 = arith.constant 0: i32
        %s5 = sigi.push %s4, %v4: i32
        // <=
        %s6, %v5 = sigi.pop %s5: i32
        %s7, %v6 = sigi.pop %s6: i32
        %v7 = arith.cmpi "sle", %v6, %v5: i32
        %s8 = sigi.push %s7, %v7: i1
        %v9 = closure.box [%v8_result = %v1_result : i32] (%s9 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> (i32, i1) } { // -> int, bool
            %s10 = sigi.push %s9, %v8_result: i32 // push result
            %v10 = arith.constant 1: i1
            %s11 = sigi.push %s10, %v10: i1
            closure.return %s11: !sigi.stack
        }
        %s12 = sigi.push %s8, %v9: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v14 = closure.box [%v11_base = %v3_base : i32, %v12_exp = %v2_exp : i32, %v13_result = %v1_result : i32] (%s13 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> (i32, i1) } { // -> int, bool
            %s14 = sigi.push %s13, %v13_result: i32 // push result
            %s15 = func.call @show(%s14) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
            %s16 = sigi.push %s15, %v11_base: i32 // push base
            %s17 = sigi.push %s16, %v12_exp: i32 // push exp
            %v15 = arith.constant 1: i32
            %s18 = sigi.push %s17, %v15: i32
            // -
            %s19, %v16 = sigi.pop %s18: i32
            %s20, %v17 = sigi.pop %s19: i32
            %v18 = arith.subi %v17, %v16: i32
            %s21 = sigi.push %s20, %v18: i32
            %s22 = sigi.push %s21, %v11_base: i32 // push base
            %s23 = sigi.push %s22, %v13_result: i32 // push result
            // *
            %s24, %v19 = sigi.pop %s23: i32
            %s25, %v20 = sigi.pop %s24: i32
            %v21 = arith.muli %v20, %v19: i32
            %s26 = sigi.push %s25, %v21: i32
            %s27 = func.call @pow_loop(%s26) { sigi.stackType = (i32, i32, i32) -> (i32, i1) } : (!sigi.stack) -> !sigi.stack // int, int, int -> int, bool
            closure.return %s27: !sigi.stack
        }
        %s28 = sigi.push %s12, %v14: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s29, %v22 = sigi.pop %s28: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s30, %v23 = sigi.pop %s29: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s31, %v24 = sigi.pop %s30: i1
        
        %v25 = scf.if %v24 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v23: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v22: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s32 = sigi.push %s31, %v25: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s33 = func.call @apply(%s32) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> (i32, i1) } : (!sigi.stack) -> !sigi.stack // (-> int, bool) -> int, bool
        return %s33: !sigi.stack
    }
    // __main__: ->
    func.func @__main__(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> (), sigi.main} {
        %v1 = arith.constant 2: i32
        %s1 = sigi.push %s0, %v1: i32
        %v2 = arith.constant 6: i32
        %s2 = sigi.push %s1, %v2: i32
        %s3 = func.call @pow(%s2) { sigi.stackType = (i32, i32) -> (i32, i1) } : (!sigi.stack) -> !sigi.stack // int, int -> int, bool
        %s4 = func.call @"show$2"(%s3) { sigi.stackType = (i1) -> () } : (!sigi.stack) -> !sigi.stack // bool ->
        %s5 = func.call @show(%s4) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
        return %s5: !sigi.stack
    }
}
