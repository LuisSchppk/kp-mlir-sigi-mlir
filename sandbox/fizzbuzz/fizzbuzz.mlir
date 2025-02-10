module {
    func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes { sigi.builtinfunc }
    // apply: (->) ->
    func.func private @apply(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> ()} {
        // -> \f;
        %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (->)
        %s2 = closure.call %v1_f (%s1) { sigi.stackType = () -> () } : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: ->
        return %s2: !sigi.stack
    }
    // show: int ->
    func.func private @show(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32) -> ()} {
        %s1 = func.call @"sigi::pp"(%s0) { sigi.stackType = (i32) -> i32 } : (!sigi.stack) -> !sigi.stack // int -> int
        %s2, %v1 = sigi.pop %s1: i32 // pop intrinsic
        return %s2: !sigi.stack
    }
    // fizzbuzz: int, int ->
    func.func private @fizzbuzz(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = (i32, i32) -> ()} {
        // -> n, upperbound;
        %s1, %v1_upperbound = sigi.pop %s0: i32 // upperbound: int
        %s2, %v2_n = sigi.pop %s1: i32 // n: int
        %s3 = sigi.push %s2, %v2_n: i32 // push n
        %s4 = sigi.push %s3, %v1_upperbound: i32 // push upperbound
        // <=
        %s5, %v3 = sigi.pop %s4: i32
        %s6, %v4 = sigi.pop %s5: i32
        %v5 = arith.cmpi "sle", %v4, %v3: i32
        %s7 = sigi.push %s6, %v5: i1
        %v8 = closure.box [%v6_n = %v2_n : i32, %v7_upperbound = %v1_upperbound : i32] (%s8 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
            %s9 = sigi.push %s8, %v6_n: i32 // push n
            %v9 = arith.constant 2: i32
            %s10 = sigi.push %s9, %v9: i32
            // %
            %s11, %v10 = sigi.pop %s10: i32
            %s12, %v11 = sigi.pop %s11: i32
            %v12 = arith.remui %v11, %v10: i32
            %s13 = sigi.push %s12, %v12: i32
            %v13 = arith.constant 0: i32
            %s14 = sigi.push %s13, %v13: i32
            // =
            %s15, %v14 = sigi.pop %s14: i32
            %s16, %v15 = sigi.pop %s15: i32
            %v16 = arith.cmpi "eq", %v15, %v14: i32
            %s17 = sigi.push %s16, %v16: i1
            %v18 = closure.box [%v17_n = %v6_n : i32] (%s18 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                %s19 = sigi.push %s18, %v17_n: i32 // push n
                %v19 = arith.constant 5: i32
                %s20 = sigi.push %s19, %v19: i32
                // %
                %s21, %v20 = sigi.pop %s20: i32
                %s22, %v21 = sigi.pop %s21: i32
                %v22 = arith.remui %v21, %v20: i32
                %s23 = sigi.push %s22, %v22: i32
                %v23 = arith.constant 0: i32
                %s24 = sigi.push %s23, %v23: i32
                // =
                %s25, %v24 = sigi.pop %s24: i32
                %s26, %v25 = sigi.pop %s25: i32
                %v26 = arith.cmpi "eq", %v25, %v24: i32
                %s27 = sigi.push %s26, %v26: i1
                %v27 = closure.box [] (%s28 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                    %v28 = arith.constant 3: i32
                    %s29 = sigi.push %s28, %v28: i32
                    // unary_-
                    %v29 = arith.constant 0: i32
                    %s30, %v30 = sigi.pop %s29: i32
                    %v31 = arith.subi %v29, %v30: i32
                    %s31 = sigi.push %s30, %v31: i32
                    %s32 = func.call @show(%s31) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
                    closure.return %s32: !sigi.stack
                }
                %s33 = sigi.push %s27, %v27: !closure.box<(!sigi.stack) -> !sigi.stack>
                %v32 = closure.box [] (%s34 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                    %v33 = arith.constant 2: i32
                    %s35 = sigi.push %s34, %v33: i32
                    // unary_-
                    %v34 = arith.constant 0: i32
                    %s36, %v35 = sigi.pop %s35: i32
                    %v36 = arith.subi %v34, %v35: i32
                    %s37 = sigi.push %s36, %v36: i32
                    %s38 = func.call @show(%s37) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
                    closure.return %s38: !sigi.stack
                }
                %s39 = sigi.push %s33, %v32: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s40, %v37 = sigi.pop %s39: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s41, %v38 = sigi.pop %s40: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s42, %v39 = sigi.pop %s41: i1
                
                %v40 = scf.if %v39 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
                  scf.yield %v38: !closure.box<(!sigi.stack) -> !sigi.stack>
                } else {
                  scf.yield %v37: !closure.box<(!sigi.stack) -> !sigi.stack>
                }
                %s43 = sigi.push %s42, %v40: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s44 = func.call @apply(%s43) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> () } : (!sigi.stack) -> !sigi.stack // (->) ->
                closure.return %s44: !sigi.stack
            }
            %s45 = sigi.push %s17, %v18: !closure.box<(!sigi.stack) -> !sigi.stack>
            %v43 = closure.box [%v41_n = %v6_n : i32, %v42_upperbound = %v7_upperbound : i32] (%s46 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                %s47 = sigi.push %s46, %v41_n: i32 // push n
                %s48 = func.call @show(%s47) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
                %s49 = sigi.push %s48, %v41_n: i32 // push n
                %v44 = arith.constant 1: i32
                %s50 = sigi.push %s49, %v44: i32
                // +
                %s51, %v45 = sigi.pop %s50: i32
                %s52, %v46 = sigi.pop %s51: i32
                %v47 = arith.addi %v46, %v45: i32
                %s53 = sigi.push %s52, %v47: i32
                %s54 = sigi.push %s53, %v42_upperbound: i32 // push upperbound
                %s55 = func.call @fizzbuzz(%s54) { sigi.stackType = (i32, i32) -> () } : (!sigi.stack) -> !sigi.stack // int, int ->
                closure.return %s55: !sigi.stack
            }
            %s56 = sigi.push %s45, %v43: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s57, %v48 = sigi.pop %s56: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s58, %v49 = sigi.pop %s57: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s59, %v50 = sigi.pop %s58: i1
            
            %v51 = scf.if %v50 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
              scf.yield %v49: !closure.box<(!sigi.stack) -> !sigi.stack>
            } else {
              scf.yield %v48: !closure.box<(!sigi.stack) -> !sigi.stack>
            }
            %s60 = sigi.push %s59, %v51: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s61 = func.call @apply(%s60) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> () } : (!sigi.stack) -> !sigi.stack // (->) ->
            closure.return %s61: !sigi.stack
        }
        %s62 = sigi.push %s7, %v8: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v52 = closure.box [] (%s63 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
            closure.return %s63: !sigi.stack
        }
        %s64 = sigi.push %s62, %v52: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s65, %v53 = sigi.pop %s64: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s66, %v54 = sigi.pop %s65: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s67, %v55 = sigi.pop %s66: i1
        
        %v56 = scf.if %v55 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v54: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v53: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s68 = sigi.push %s67, %v56: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s69 = func.call @apply(%s68) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> () } : (!sigi.stack) -> !sigi.stack // (->) ->
        return %s69: !sigi.stack
    }
    // __main__: ->
    func.func @__main__(%s0: !sigi.stack) -> !sigi.stack attributes {sigi.stackType = () -> (), sigi.main} {
        %v1 = arith.constant 0: i32
        %s1 = sigi.push %s0, %v1: i32
        %v2 = arith.constant 100: i32
        %s2 = sigi.push %s1, %v2: i32
        %s3 = func.call @fizzbuzz(%s2) { sigi.stackType = (i32, i32) -> () } : (!sigi.stack) -> !sigi.stack // int, int ->
        return %s3: !sigi.stack
    }
}
