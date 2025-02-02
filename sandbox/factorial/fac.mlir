module {
    func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes { sigi.builtinfunc }
    // apply: (-> int) -> int
    func.func private @apply(%s0: !sigi.stack) -> !sigi.stack {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (-> int)
        %s2 = closure.call %v1_f (%s1) : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: -> int
        return %s2: !sigi.stack
    }
    // show: int ->
    func.func private @show(%s0: !sigi.stack) -> !sigi.stack {
        %s1 = func.call @"sigi::pp"(%s0) : (!sigi.stack) -> !sigi.stack // int -> int
        %s2, %v1 = sigi.pop %s1: i32 // pop intrinsic
        return %s2: !sigi.stack
    }
    // factorial: int -> int
    func.func private @factorial(%s0: !sigi.stack) -> !sigi.stack {
        // -> n;
        %s1, %v1_n = sigi.pop %s0: i32 // n: int
        %s2 = sigi.push %s1, %v1_n: i32 // push n
        %v2 = arith.constant 0: i32
        %s3 = sigi.push %s2, %v2: i32
        // =
        %s4, %v3 = sigi.pop %s3: i32
        %s5, %v4 = sigi.pop %s4: i32
        %v5 = arith.cmpi "eq", %v4, %v3: i32
        %s6 = sigi.push %s5, %v5: i1
        %v6 = closure.box [] (%s7 : !sigi.stack) -> !sigi.stack { // -> int
            %v7 = arith.constant 1: i32
            %s8 = sigi.push %s7, %v7: i32
            closure.return %s8: !sigi.stack
        }
        %s9 = sigi.push %s6, %v6: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v9 = closure.box [%v8_n = %v1_n : i32] (%s10 : !sigi.stack) -> !sigi.stack { // -> int
            %s11 = sigi.push %s10, %v8_n: i32 // push n
            %s12 = sigi.push %s11, %v8_n: i32 // push n
            %v10 = arith.constant 1: i32
            %s13 = sigi.push %s12, %v10: i32
            // -
            %s14, %v11 = sigi.pop %s13: i32
            %s15, %v12 = sigi.pop %s14: i32
            %v13 = arith.subi %v12, %v11: i32
            %s16 = sigi.push %s15, %v13: i32
            %s17 = func.call @factorial(%s16) : (!sigi.stack) -> !sigi.stack // int -> int
            // *
            %s18, %v14 = sigi.pop %s17: i32
            %s19, %v15 = sigi.pop %s18: i32
            %v16 = arith.muli %v15, %v14: i32
            %s20 = sigi.push %s19, %v16: i32
            closure.return %s20: !sigi.stack
        }
        %s21 = sigi.push %s9, %v9: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s22, %v17 = sigi.pop %s21: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s23, %v18 = sigi.pop %s22: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s24, %v19 = sigi.pop %s23: i1
        
        %v20 = scf.if %v19 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v18: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v17: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s25 = sigi.push %s24, %v20: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s26 = func.call @apply(%s25) : (!sigi.stack) -> !sigi.stack // (-> int) -> int
        return %s26: !sigi.stack
    }
    // __main__: ->
    func.func @__main__(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.main} {
        %v1 = arith.constant 5: i32
        %s1 = sigi.push %s0, %v1: i32
        %s2 = func.call @factorial(%s1) : (!sigi.stack) -> !sigi.stack // int -> int
        %s3 = func.call @show(%s2) : (!sigi.stack) -> !sigi.stack // int ->
        return %s3: !sigi.stack
    }
}
