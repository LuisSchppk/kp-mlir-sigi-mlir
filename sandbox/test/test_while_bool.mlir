module {
    func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes { sigi.builtinfunc }
    // apply: bool, int, (bool, int -> bool, int, bool) -> bool, int, bool
    func.func private @apply(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>) -> (i1, i32, i1)} {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (bool, int -> bool, int, bool)
        %s2 = closure.call %v1_f (%s1) { sigi.stackType = (i1, i32) -> (i1, i32, i1) } : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: bool, int -> bool, int, bool
        return %s2: !sigi.stack
    }
    // apply: int, (int -> int) -> int
    func.func private @"apply$2"(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32} {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (int -> int)
        %s2 = closure.call %v1_f (%s1) { sigi.stackType = (i32) -> i32 } : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: int -> int
        return %s2: !sigi.stack
    }
    // apply: bool, int, (bool, int -> int) -> int
    func.func private @"apply$3"(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32} {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (bool, int -> int)
        %s2 = closure.call %v1_f (%s1) { sigi.stackType = (i1, i32) -> i32 } : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: bool, int -> int
        return %s2: !sigi.stack
    }
    // apply: (-> int, bool) -> int, bool
    func.func private @"apply$4"(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> (i32, i1)} {
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
    // while: bool, int, (bool, int -> bool, int, bool), (bool, int -> int) -> int
    func.func private @while(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32} {
        // -> evalCondition, body;
        %s1, %v1_body = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // body: (bool, int -> int)
        %s2, %v2_evalCondition = sigi.pop %s1: !closure.box<(!sigi.stack) -> !sigi.stack> // evalCondition: (bool, int -> bool, int, bool)
        %s3 = sigi.push %s2, %v2_evalCondition: !closure.box<(!sigi.stack) -> !sigi.stack> // push evalCondition
        %s4 = func.call @apply(%s3) { sigi.stackType = (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>) -> (i1, i32, i1) } : (!sigi.stack) -> !sigi.stack // bool, int, (bool, int -> bool, int, bool) -> bool, int, bool
        // -> conditionTrue;
        %s5, %v3_conditionTrue = sigi.pop %s4: i1 // conditionTrue: bool
        %s6 = sigi.push %s5, %v1_body: !closure.box<(!sigi.stack) -> !sigi.stack> // push body
        %s7 = func.call @"apply$3"(%s6) { sigi.stackType = (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32 } : (!sigi.stack) -> !sigi.stack // bool, int, (bool, int -> int) -> int
        %s8 = sigi.push %s7, %v3_conditionTrue: i1 // push conditionTrue
        %v6 = closure.box [%v4_evalCondition = %v2_evalCondition : !closure.box<(!sigi.stack) -> !sigi.stack>, %v5_body = %v1_body : !closure.box<(!sigi.stack) -> !sigi.stack>] (%s9 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = (i32) -> i32 } { // int -> int
            %s10 = sigi.push %s9, %v4_evalCondition: !closure.box<(!sigi.stack) -> !sigi.stack> // push evalCondition
            %s11 = sigi.push %s10, %v5_body: !closure.box<(!sigi.stack) -> !sigi.stack> // push body
            %s12 = func.call @while(%s11) { sigi.stackType = (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32 } : (!sigi.stack) -> !sigi.stack // bool, int, (bool, int -> bool, int, bool), (bool, int -> int) -> int
            closure.return %s12: !sigi.stack
        }
        %s13 = sigi.push %s8, %v6: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v7 = closure.box [] (%s14 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
            closure.return %s14: !sigi.stack
        }
        %s15 = sigi.push %s13, %v7: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s16, %v8 = sigi.pop %s15: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s17, %v9 = sigi.pop %s16: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s18, %v10 = sigi.pop %s17: i1
        
        %v11 = scf.if %v10 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v9: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v8: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s19 = sigi.push %s18, %v11: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s20 = func.call @"apply$2"(%s19) { sigi.stackType = (i32, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32 } : (!sigi.stack) -> !sigi.stack // int, (int -> int) -> int
        return %s20: !sigi.stack
    }
    // show_second: bool, int -> int
    func.func private @show_second(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i1, i32) -> i32} {
        // -> top, count;
        %s1, %v1_count = sigi.pop %s0: i32 // count: int
        %s2, %v2_top = sigi.pop %s1: i1 // top: bool
        %s3 = sigi.push %s2, %v2_top: i1 // push top
        %s4 = func.call @"show$2"(%s3) { sigi.stackType = (i1) -> () } : (!sigi.stack) -> !sigi.stack // bool ->
        %s5 = sigi.push %s4, %v1_count: i32 // push count
        return %s5: !sigi.stack
    }
    // decr_and_check: int -> int, bool
    func.func private @decr_and_check(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32) -> (i32, i1)} {
        // -> size;
        %s1, %v1_size = sigi.pop %s0: i32 // size: int
        %s2 = sigi.push %s1, %v1_size: i32 // push size
        %v2 = arith.constant 1: i32
        %s3 = sigi.push %s2, %v2: i32
        // -
        %s4, %v3 = sigi.pop %s3: i32
        %s5, %v4 = sigi.pop %s4: i32
        %v5 = arith.subi %v4, %v3: i32
        %s6 = sigi.push %s5, %v5: i32
        %s7 = sigi.push %s6, %v1_size: i32 // push size
        %v6 = arith.constant 0: i32
        %s8 = sigi.push %s7, %v6: i32
        // >
        %s9, %v7 = sigi.pop %s8: i32
        %s10, %v8 = sigi.pop %s9: i32
        %v9 = arith.cmpi "sgt", %v8, %v7: i32
        %s11 = sigi.push %s10, %v9: i1
        return %s11: !sigi.stack
    }
    // print_stack_and_pop: bool, int -> int
    func.func private @print_stack_and_pop(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i1, i32) -> i32} {
        %v1 = closure.box [] (%s1 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = (i32) -> (i32, i1) } { // int -> int, bool
            %s2 = func.call @decr_and_check(%s1) { sigi.stackType = (i32) -> (i32, i1) } : (!sigi.stack) -> !sigi.stack // int -> int, bool
            closure.return %s2: !sigi.stack
        }
        %s3 = sigi.push %s0, %v1: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v2 = closure.box [] (%s4 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = (i1, i32) -> i32 } { // bool, int -> int
            %s5 = func.call @show_second(%s4) { sigi.stackType = (i1, i32) -> i32 } : (!sigi.stack) -> !sigi.stack // bool, int -> int
            closure.return %s5: !sigi.stack
        }
        %s6 = sigi.push %s3, %v2: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s7 = func.call @while(%s6) { sigi.stackType = (i1, i32, !closure.box<(!sigi.stack) -> !sigi.stack>, !closure.box<(!sigi.stack) -> !sigi.stack>) -> i32 } : (!sigi.stack) -> !sigi.stack // bool, int, (bool, int -> bool, int, bool), (bool, int -> int) -> int
        return %s7: !sigi.stack
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
        %s33 = func.call @"apply$4"(%s32) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> (i32, i1) } : (!sigi.stack) -> !sigi.stack // (-> int, bool) -> int, bool
        return %s33: !sigi.stack
    }
    // __main__: -> int, int
    func.func @__main__(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> (i32, i32), sigi.main} {
        %v1 = arith.constant 2: i32
        %s1 = sigi.push %s0, %v1: i32
        %v2 = arith.constant 6: i32
        %s2 = sigi.push %s1, %v2: i32
        %s3 = func.call @pow(%s2) { sigi.stackType = (i32, i32) -> (i32, i1) } : (!sigi.stack) -> !sigi.stack // int, int -> int, bool
        %v3 = arith.constant 2: i32
        %s4 = sigi.push %s3, %v3: i32
        %s5 = func.call @print_stack_and_pop(%s4) { sigi.stackType = (i1, i32) -> i32 } : (!sigi.stack) -> !sigi.stack // bool, int -> int
        return %s5: !sigi.stack
    }
}
