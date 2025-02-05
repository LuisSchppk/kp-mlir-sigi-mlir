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
    // gcd: int, int -> int
    func.func private @gcd(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32) -> i32} {
        // -> a, b;
        %s1, %v1_b = sigi.pop %s0: i32 // b: int
        %s2, %v2_a = sigi.pop %s1: i32 // a: int
        %s3 = sigi.push %s2, %v1_b: i32 // push b
        %v3 = arith.constant 0: i32
        %s4 = sigi.push %s3, %v3: i32
        // =
        %s5, %v4 = sigi.pop %s4: i32
        %s6, %v5 = sigi.pop %s5: i32
        %v6 = arith.cmpi "eq", %v5, %v4: i32
        %s7 = sigi.push %s6, %v6: i1
        %v8 = closure.box [%v7_a = %v2_a : i32] (%s8 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> i32 } { // -> int
            %s9 = sigi.push %s8, %v7_a: i32 // push a
            closure.return %s9: !sigi.stack
        }
        %s10 = sigi.push %s7, %v8: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v11 = closure.box [%v9_a = %v2_a : i32, %v10_b = %v1_b : i32] (%s11 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> i32 } { // -> int
            %s12 = sigi.push %s11, %v10_b: i32 // push b
            %s13 = sigi.push %s12, %v9_a: i32 // push a
            %s14 = sigi.push %s13, %v10_b: i32 // push b
            // %
            %s15, %v12 = sigi.pop %s14: i32
            %s16, %v13 = sigi.pop %s15: i32
            %v14 = arith.remui %v13, %v12: i32
            %s17 = sigi.push %s16, %v14: i32
            %s18 = func.call @gcd(%s17) { sigi.stackType = (i32, i32) -> i32 } : (!sigi.stack) -> !sigi.stack // int, int -> int
            closure.return %s18: !sigi.stack
        }
        %s19 = sigi.push %s10, %v11: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s20, %v15 = sigi.pop %s19: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s21, %v16 = sigi.pop %s20: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s22, %v17 = sigi.pop %s21: i1
        
        %v18 = scf.if %v17 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v16: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v15: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s23 = sigi.push %s22, %v18: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s24 = func.call @apply(%s23) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> i32 } : (!sigi.stack) -> !sigi.stack // (-> int) -> int
        return %s24: !sigi.stack
    }
    // __main__: ->
    func.func @__main__(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> (), sigi.main} {
        %v1 = arith.constant 24: i32
        %s1 = sigi.push %s0, %v1: i32
        %v2 = arith.constant 8: i32
        %s2 = sigi.push %s1, %v2: i32
        %s3 = func.call @gcd(%s2) { sigi.stackType = (i32, i32) -> i32 } : (!sigi.stack) -> !sigi.stack // int, int -> int
        %s4 = func.call @show(%s3) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
        return %s4: !sigi.stack
    }
}
