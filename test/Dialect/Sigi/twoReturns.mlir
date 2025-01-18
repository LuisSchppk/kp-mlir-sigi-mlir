module {
  //   // 3.1. Tail-recursive: GCD with unrelated ops
  //   func.func @gcd_2(%a: i32, %b: i32) -> i32 {
  //   %zero = arith.constant 0 : i32
  //   %is_zero = arith.cmpi "eq", %b, %zero : i32
  //   %result = scf.if %is_zero -> i32 {
  //     scf.yield %a : i32
  //   } else {
  //     %remainder = arith.remsi %a, %b : i32
  //     %result = func.call @gcd_2(%b, %remainder) : (i32, i32) -> i32
  //     scf.yield %remainder : i32
  //   }
  //   return %result : i32
  // }

    func.func @sum_n(%n: i32, %acc: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %is_zero = arith.cmpi "eq", %n, %zero : i32
    %result = scf.if %is_zero -> i32 {
      scf.yield %acc : i32
    } else {
      %next_n = arith.subi %n, %zero : i32
      %next_acc = arith.addi %acc, %n : i32
      %result = func.call @sum_n(%next_n, %next_acc) : (i32, i32) -> i32
      scf.yield %result : i32
    }
    return %result : i32
  }

  //   func.func @reverse(%arr: memref<?xi32>, %i: index, %j: index) -> memref<?xi32> {
  //   %one = arith.constant 1: index
  //   %cond = arith.cmpi "slt", %i, %j : index
  //   %result = scf.if %cond -> memref<?xi32> {
  //     %temp_i = memref.load %arr[%i] : memref<?xi32>
  //     %temp_j = memref.load %arr[%j] : memref<?xi32>
  //     memref.store %temp_i, %arr[%j] : memref<?xi32>
  //     memref.store %temp_j, %arr[%i] : memref<?xi32>
  //     %next_i = arith.addi %i, %one : index
  //     %next_j = arith.subi %j, %one : index
  //     %reversed = func.call @reverse(%arr, %next_i, %next_j) : (memref<?xi32>, index, index) -> memref<?xi32>
  //     scf.yield %reversed : memref<?xi32>
  //   } else {
  //     scf.yield %arr : memref<?xi32>
  //   }
  //   return %result : memref<?xi32>
  // }

  //     func.func @double_tail_recursive(%a: i32, %b: i32, %acc1: i32, %acc2: i32) -> (i32, i32) {
  //    %zero = arith.constant 0 : i32
  //    %one = arith.constant 1: i32
  //    %is_a_zero = arith.cmpi "eq", %a, %zero : i32
  //    %is_b_zero = arith.cmpi "eq", %b, %zero : i32
  //    %result2 = arith.constant 0 : i32
  //    %result1 = scf.if %is_a_zero -> (i32) {
  //      scf.yield %acc1 : i32
  //     } else {
  //      %new_a = arith.subi %a, %one : i32
  //     %new_acc1 = arith.addi %acc1, %a : i32
  //      %res1, %ignore1 = func.call @double_tail_recursive(%new_a, %b, %new_acc1, %acc2) : (i32, i32, i32, i32) -> (i32, i32)
  //     scf.yield %res1 : i32
  //    }

    //  %result2 = scf.if %is_b_zero -> (i32) {
    //    scf.yield %acc2 : i32
    //  } else {
    //   %new_b = arith.subi %b, %one : i32
    //  %new_acc2 = arith.muli %acc2, %b : i32
    //     %res2, %ignore2 = func.call @double_tail_recursive(%a, %new_b, %acc1, %new_acc2) : (i32, i32, i32, i32) -> (i32, i32)
    //     scf.yield %res2 : i32
    // } 

    // return %result1, %result2 : i32, i32
    // }   

  func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  func.func private @fibloop(%arg0: !sigi.stack) -> !sigi.stack {
    %c0_i32 = arith.constant 0 : i32
    %out_stack, %value = sigi.pop %arg0 : i32 // %arg0 > %out_stack, %value
    %out_stack_0, %value_1 = sigi.pop %out_stack : i32 //  %out_stack > %out_stack_0, %value_1
    %0 = arith.cmpi eq, %value_1, %c0_i32 : i32
    %1 = scf.if %0 -> (!sigi.stack) {
      scf.yield %out_stack_0 : !sigi.stack
    } else {
      %c10_i32 = arith.constant 10 : i32
      %c1_i32 = arith.constant 1 : i32
      %2 = sigi.push %out_stack_0, %value : i32
      %3 = func.call @"sigi::pp"(%2) : (!sigi.stack) -> !sigi.stack
      %out_stack_2, %value_3 = sigi.pop %3 : i32
      %4 = arith.subi %value_1, %c1_i32 : i32
      %5 = sigi.push %out_stack_2, %4 : i32
      %6 = arith.muli %value, %c10_i32 : i32
      %7 = sigi.push %5, %6 : i32
      %8 = func.call @fibloop(%7) : (!sigi.stack) -> !sigi.stack
      scf.yield %8 : !sigi.stack
    }
    return %1 : !sigi.stack
  }
  
  func.func @__main__(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.main} {
    %c1_i32 = arith.constant 1 : i32
    %c10_i32 = arith.constant 10 : i32
    %0 = sigi.push %arg0, %c10_i32 : i32
    %1 = sigi.push %0, %c1_i32 : i32
    %2 = call @fibloop(%1) : (!sigi.stack) -> !sigi.stack
    return %2 : !sigi.stack
  }  

  //   func.func @sum_n(%n: i32, %acc: i32) -> i32 {
  //   %zero = arith.constant 0 : i32
  //   %one = arith.constant 1 : i32
  //   %is_zero = arith.cmpi "eq", %n, %zero : i32
  //   %is_non_negative = arith.cmpi "sge", %n, %one : i32
  //   %result = scf.if %is_zero -> i32 {   // n acc is_non_negative
  //     %result2 = scf.if %is_non_negative -> i32 {
  //         %next_n = arith.subi %n, %zero : i32
  //         %next_acc = arith.addi %acc, %n : i32
  //         %result3 = func.call @sum_n(%next_n, %next_acc) : (i32, i32) -> i32
  //         scf.yield %result3 : i32
  //     } else {
  //       scf.yield %acc : i32
  //     }
  //     scf.yield %result2: i32 // 
  //   } else {
  //     %next_n = arith.subi %n, %zero : i32
  //     %next_acc = arith.addi %acc, %n : i32
  //     // %result = func.call @sum_n(%next_n, %next_acc) : (i32, i32) -> i32
  //     scf.yield %next_acc : i32
  //   }
  //   return %result : i32
  // }
}