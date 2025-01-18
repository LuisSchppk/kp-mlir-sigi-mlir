module {
    func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes { sigi.builtinfunc }
    // apply: (->) ->
    func.func private @apply(%s0: !sigi.stack) -> !sigi.stack {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (->)
        %s2 = closure.call %v1_f (%s1) : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: ->
        return %s2: !sigi.stack
    }
    // show: int ->
    func.func private @show(%s0: !sigi.stack) -> !sigi.stack {
        %s1 = func.call @"sigi::pp"(%s0) : (!sigi.stack) -> !sigi.stack // int -> int
        %s2, %v1 = sigi.pop %s1: i32 // pop intrinsic
        return %s2: !sigi.stack
    }
    // fibloop: int, int ->
    func.func private @fibloop(%s0: !sigi.stack) -> !sigi.stack {
        // -> i, running;
        %s1, %v1_running = sigi.pop %s0: i32 // running: int
        %s2, %v2_i = sigi.pop %s1: i32 // i: int
        %s3 = sigi.push %s2, %v2_i: i32 // push i
        %v3 = arith.constant 0: i32
        %s4 = sigi.push %s3, %v3: i32
        // =
        %s5, %v4 = sigi.pop %s4: i32
        %s6, %v5 = sigi.pop %s5: i32
        %v6 = arith.cmpi "eq", %v5, %v4: i32
        %s7 = sigi.push %s6, %v6: i1
        %v7 = closure.box [] (%s8 : !sigi.stack) -> !sigi.stack { // ->
            closure.return %s8: !sigi.stack
        }
        %s9 = sigi.push %s7, %v7: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v10 = closure.box [%v8_i = %v2_i : i32, %v9_running = %v1_running : i32] (%s10 : !sigi.stack) -> !sigi.stack { // ->
            %s11 = sigi.push %s10, %v9_running: i32 // push running
            %s12 = func.call @show(%s11) : (!sigi.stack) -> !sigi.stack // int ->
            %s13 = sigi.push %s12, %v8_i: i32 // push i
            %v11 = arith.constant 1: i32
            %s14 = sigi.push %s13, %v11: i32
            // -
            %s15, %v12 = sigi.pop %s14: i32
            %s16, %v13 = sigi.pop %s15: i32
            %v14 = arith.subi %v13, %v12: i32
            %s17 = sigi.push %s16, %v14: i32
            %s18 = sigi.push %s17, %v9_running: i32 // push running
            %v15 = arith.constant 10: i32
            %s19 = sigi.push %s18, %v15: i32
            // *
            %s20, %v16 = sigi.pop %s19: i32
            %s21, %v17 = sigi.pop %s20: i32
            %v18 = arith.muli %v17, %v16: i32
            %s22 = sigi.push %s21, %v18: i32
            %s23 = func.call @fibloop(%s22) : (!sigi.stack) -> !sigi.stack // int, int ->
            closure.return %s23: !sigi.stack
        }
        %s24 = sigi.push %s9, %v10: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s25, %v19 = sigi.pop %s24: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s26, %v20 = sigi.pop %s25: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s27, %v21 = sigi.pop %s26: i1
        
        %v22 = scf.if %v21 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v20: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v19: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s28 = sigi.push %s27, %v22: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s29 = func.call @apply(%s28) : (!sigi.stack) -> !sigi.stack // (->) ->
        return %s29: !sigi.stack
    }
    
    // __main__: ->
    func.func @__main__(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.main} {
        %v1 = arith.constant 10: i32
        %s1 = sigi.push %s0, %v1: i32
        %v2 = arith.constant 1: i32
        %s2 = sigi.push %s1, %v2: i32
        %s3 = func.call @fibloop(%s2) : (!sigi.stack) -> !sigi.stack // int, int ->
        return %s3: !sigi.stack
    }
}
