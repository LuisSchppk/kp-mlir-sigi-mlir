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
            %v19 = closure.box [%v17_n = %v6_n : i32, %v18_upperbound = %v7_upperbound : i32] (%s18 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                %s19 = sigi.push %s18, %v17_n: i32 // push n
                %v20 = arith.constant 5: i32
                %s20 = sigi.push %s19, %v20: i32
                // %
                %s21, %v21 = sigi.pop %s20: i32
                %s22, %v22 = sigi.pop %s21: i32
                %v23 = arith.remui %v22, %v21: i32
                %s23 = sigi.push %s22, %v23: i32
                %v24 = arith.constant 0: i32
                %s24 = sigi.push %s23, %v24: i32
                // =
                %s25, %v25 = sigi.pop %s24: i32
                %s26, %v26 = sigi.pop %s25: i32
                %v27 = arith.cmpi "eq", %v26, %v25: i32
                %s27 = sigi.push %s26, %v27: i1
                %v30 = closure.box [%v28_n = %v17_n : i32, %v29_upperbound = %v18_upperbound : i32] (%s28 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                    %v31 = arith.constant 3: i32
                    %s29 = sigi.push %s28, %v31: i32
                    // unary_-
                    %v32 = arith.constant 0: i32
                    %s30, %v33 = sigi.pop %s29: i32
                    %v34 = arith.subi %v32, %v33: i32
                    %s31 = sigi.push %s30, %v34: i32
                    %s32 = func.call @show(%s31) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
                    %s33 = sigi.push %s32, %v28_n: i32 // push n
                    %v35 = arith.constant 1: i32
                    %s34 = sigi.push %s33, %v35: i32
                    // +
                    %s35, %v36 = sigi.pop %s34: i32
                    %s36, %v37 = sigi.pop %s35: i32
                    %v38 = arith.addi %v37, %v36: i32
                    %s37 = sigi.push %s36, %v38: i32
                    %s38 = sigi.push %s37, %v29_upperbound: i32 // push upperbound
                    %s39 = func.call @fizzbuzz(%s38) { sigi.stackType = (i32, i32) -> () } : (!sigi.stack) -> !sigi.stack // int, int ->
                    closure.return %s39: !sigi.stack
                }
                %s40 = sigi.push %s27, %v30: !closure.box<(!sigi.stack) -> !sigi.stack>
                %v41 = closure.box [%v39_n = %v17_n : i32, %v40_upperbound = %v18_upperbound : i32] (%s41 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                    %v42 = arith.constant 1: i32
                    %s42 = sigi.push %s41, %v42: i32
                    // unary_-
                    %v43 = arith.constant 0: i32
                    %s43, %v44 = sigi.pop %s42: i32
                    %v45 = arith.subi %v43, %v44: i32
                    %s44 = sigi.push %s43, %v45: i32
                    %s45 = func.call @show(%s44) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
                    %s46 = sigi.push %s45, %v39_n: i32 // push n
                    %v46 = arith.constant 1: i32
                    %s47 = sigi.push %s46, %v46: i32
                    // +
                    %s48, %v47 = sigi.pop %s47: i32
                    %s49, %v48 = sigi.pop %s48: i32
                    %v49 = arith.addi %v48, %v47: i32
                    %s50 = sigi.push %s49, %v49: i32
                    %s51 = sigi.push %s50, %v40_upperbound: i32 // push upperbound
                    %s52 = func.call @fizzbuzz(%s51) { sigi.stackType = (i32, i32) -> () } : (!sigi.stack) -> !sigi.stack // int, int ->
                    closure.return %s52: !sigi.stack
                }
                %s53 = sigi.push %s40, %v41: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s54, %v50 = sigi.pop %s53: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s55, %v51 = sigi.pop %s54: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s56, %v52 = sigi.pop %s55: i1
                
                %v53 = scf.if %v52 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
                  scf.yield %v51: !closure.box<(!sigi.stack) -> !sigi.stack>
                } else {
                  scf.yield %v50: !closure.box<(!sigi.stack) -> !sigi.stack>
                }
                %s57 = sigi.push %s56, %v53: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s58 = func.call @apply(%s57) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> () } : (!sigi.stack) -> !sigi.stack // (->) ->
                closure.return %s58: !sigi.stack
            }
            %s59 = sigi.push %s17, %v19: !closure.box<(!sigi.stack) -> !sigi.stack>
            %v56 = closure.box [%v54_n = %v6_n : i32, %v55_upperbound = %v7_upperbound : i32] (%s60 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                %s61 = sigi.push %s60, %v54_n: i32 // push n
                %v57 = arith.constant 5: i32
                %s62 = sigi.push %s61, %v57: i32
                // %
                %s63, %v58 = sigi.pop %s62: i32
                %s64, %v59 = sigi.pop %s63: i32
                %v60 = arith.remui %v59, %v58: i32
                %s65 = sigi.push %s64, %v60: i32
                %v61 = arith.constant 0: i32
                %s66 = sigi.push %s65, %v61: i32
                // =
                %s67, %v62 = sigi.pop %s66: i32
                %s68, %v63 = sigi.pop %s67: i32
                %v64 = arith.cmpi "eq", %v63, %v62: i32
                %s69 = sigi.push %s68, %v64: i1
                %v67 = closure.box [%v65_n = %v54_n : i32, %v66_upperbound = %v55_upperbound : i32] (%s70 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                    %v68 = arith.constant 2: i32
                    %s71 = sigi.push %s70, %v68: i32
                    // unary_-
                    %v69 = arith.constant 0: i32
                    %s72, %v70 = sigi.pop %s71: i32
                    %v71 = arith.subi %v69, %v70: i32
                    %s73 = sigi.push %s72, %v71: i32
                    %s74 = func.call @show(%s73) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
                    %s75 = sigi.push %s74, %v65_n: i32 // push n
                    %v72 = arith.constant 1: i32
                    %s76 = sigi.push %s75, %v72: i32
                    // +
                    %s77, %v73 = sigi.pop %s76: i32
                    %s78, %v74 = sigi.pop %s77: i32
                    %v75 = arith.addi %v74, %v73: i32
                    %s79 = sigi.push %s78, %v75: i32
                    %s80 = sigi.push %s79, %v66_upperbound: i32 // push upperbound
                    %s81 = func.call @fizzbuzz(%s80) { sigi.stackType = (i32, i32) -> () } : (!sigi.stack) -> !sigi.stack // int, int ->
                    closure.return %s81: !sigi.stack
                }
                %s82 = sigi.push %s69, %v67: !closure.box<(!sigi.stack) -> !sigi.stack>
                %v78 = closure.box [%v76_n = %v54_n : i32, %v77_upperbound = %v55_upperbound : i32] (%s83 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
                    %s84 = sigi.push %s83, %v76_n: i32 // push n
                    %s85 = func.call @show(%s84) { sigi.stackType = (i32) -> () } : (!sigi.stack) -> !sigi.stack // int ->
                    %s86 = sigi.push %s85, %v76_n: i32 // push n
                    %v79 = arith.constant 1: i32
                    %s87 = sigi.push %s86, %v79: i32
                    // +
                    %s88, %v80 = sigi.pop %s87: i32
                    %s89, %v81 = sigi.pop %s88: i32
                    %v82 = arith.addi %v81, %v80: i32
                    %s90 = sigi.push %s89, %v82: i32
                    %s91 = sigi.push %s90, %v77_upperbound: i32 // push upperbound
                    %s92 = func.call @fizzbuzz(%s91) { sigi.stackType = (i32, i32) -> () } : (!sigi.stack) -> !sigi.stack // int, int ->
                    closure.return %s92: !sigi.stack
                }
                %s93 = sigi.push %s82, %v78: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s94, %v83 = sigi.pop %s93: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s95, %v84 = sigi.pop %s94: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s96, %v85 = sigi.pop %s95: i1
                
                %v86 = scf.if %v85 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
                  scf.yield %v84: !closure.box<(!sigi.stack) -> !sigi.stack>
                } else {
                  scf.yield %v83: !closure.box<(!sigi.stack) -> !sigi.stack>
                }
                %s97 = sigi.push %s96, %v86: !closure.box<(!sigi.stack) -> !sigi.stack>
                %s98 = func.call @apply(%s97) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> () } : (!sigi.stack) -> !sigi.stack // (->) ->
                closure.return %s98: !sigi.stack
            }
            %s99 = sigi.push %s59, %v56: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s100, %v87 = sigi.pop %s99: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s101, %v88 = sigi.pop %s100: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s102, %v89 = sigi.pop %s101: i1
            
            %v90 = scf.if %v89 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
              scf.yield %v88: !closure.box<(!sigi.stack) -> !sigi.stack>
            } else {
              scf.yield %v87: !closure.box<(!sigi.stack) -> !sigi.stack>
            }
            %s103 = sigi.push %s102, %v90: !closure.box<(!sigi.stack) -> !sigi.stack>
            %s104 = func.call @apply(%s103) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> () } : (!sigi.stack) -> !sigi.stack // (->) ->
            closure.return %s104: !sigi.stack
        }
        %s105 = sigi.push %s7, %v8: !closure.box<(!sigi.stack) -> !sigi.stack>
        %v91 = closure.box [] (%s106 : !sigi.stack) -> !sigi.stack attributes { sigi.stackType = () -> () } { // ->
            closure.return %s106: !sigi.stack
        }
        %s107 = sigi.push %s105, %v91: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s108, %v92 = sigi.pop %s107: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s109, %v93 = sigi.pop %s108: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s110, %v94 = sigi.pop %s109: i1
        
        %v95 = scf.if %v94 -> !closure.box<(!sigi.stack) -> !sigi.stack> {
          scf.yield %v93: !closure.box<(!sigi.stack) -> !sigi.stack>
        } else {
          scf.yield %v92: !closure.box<(!sigi.stack) -> !sigi.stack>
        }
        %s111 = sigi.push %s110, %v95: !closure.box<(!sigi.stack) -> !sigi.stack>
        %s112 = func.call @apply(%s111) { sigi.stackType = (!closure.box<(!sigi.stack) -> !sigi.stack>) -> () } : (!sigi.stack) -> !sigi.stack // (->) ->
        return %s112: !sigi.stack
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
