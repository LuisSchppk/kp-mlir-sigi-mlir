func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes { sigi.builtinfunc }

// func.func private @apply(%s0: !sigi.stack) -> !sigi.stack {
//         // -> \f;
//         %s1, %v1_f = sigi.pop %s0: !closure.box<(!sigi.stack) -> !sigi.stack> // f: (->)
//         %s2 = closure.call %v1_f (%s1) : !closure.box<(!sigi.stack) -> !sigi.stack> // call f: ->
//         return %s2: !sigi.stack
// }

func.func private @show(%s0: !sigi.stack) -> !sigi.stack {
        %s1 = func.call @"sigi::pp"(%s0) : (!sigi.stack) -> !sigi.stack // int -> int
        %s2, %v1 = sigi.pop %s1: i32 // pop intrinsic
        return %s2: !sigi.stack
}

func.func @simpleSigi(%arg0: !sigi.stack) -> !sigi.stack {
%c2_i32 = arith.constant 2 : i32
%c1_i32 = arith.constant 1 : i32
%s0 = sigi.push %arg0, %c1_i32 : i32
%s1 = sigi.push %s0, %c2_i32 : i32
%s2 = closure.box [](%arg1: !sigi.stack) -> !sigi.stack {
  %c1_i32_0 = arith.constant 1 : i32
  %outStack, %value = sigi.pop %arg1 : i32
  %outStack_1, %value_2 = sigi.pop %outStack : i32
  %v4 = arith.muli %value_2, %value : i32
  %v5 = arith.addi %v4, %c1_i32_0 : i32
  %s6 = sigi.push %outStack_1, %v5 : i32
  %s7 = func.call @"sigi::pp"(%s6) : (!sigi.stack) -> !sigi.stack
  closure.return %s7 : !sigi.stack
}
%s3 = closure.call %s2(%s1) : <(!sigi.stack) -> !sigi.stack>
return %s3: !sigi.stack
}

func.func @fibloop(%arg0: !sigi.stack) -> !sigi.stack {
        %s1, %v1_running = sigi.pop %arg0: i32 // running: int
        %s2, %v2_i = sigi.pop %s1: i32 // i: int
        %s3, %b1 = sigi.pop %s2: i1
        %v10 = closure.box [%v8_i = %v2_i : i32, %v9_running = %v1_running : i32, %b2 = %b1 : i1] (%s10 : !sigi.stack) -> !sigi.stack { // ->
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

        %s29 = closure.call %v10 (%s2) : !closure.box<(!sigi.stack) -> !sigi.stack>
        return %s29: !sigi.stack
}