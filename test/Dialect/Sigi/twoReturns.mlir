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
  //     scf.yield %result : i32
  //   }
  //   return %result : i32
  // }

  //   func.func @sum_n(%n: i32, %acc: i32) -> i32 {
  //   %zero = arith.constant 0 : i32
  //   %is_zero = arith.cmpi "eq", %n, %zero : i32
  //   %result = scf.if %is_zero -> i32 {
  //     scf.yield %acc : i32
  //   } else {
  //     %next_n = arith.subi %n, %zero : i32
  //     %next_acc = arith.addi %acc, %n : i32
  //     %result = func.call @sum_n(%next_n, %next_acc) : (i32, i32) -> i32
  //     scf.yield %result : i32
  //   }
  //   return %result : i32
  // }

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

  // func.func private @"sigi::pp"(!sigi.stack) -> !sigi.stack attributes {sigi.builtinfunc}
  // func.func private @fibloop(%arg0: !sigi.stack) -> !sigi.stack {
  //   %c0_i32 = arith.constant 0 : i32
  //   %out_stack, %value = sigi.pop %arg0 : i32 // %arg0 > %out_stack, %value
  //   %out_stack_0, %value_1 = sigi.pop %out_stack : i32 //  %out_stack > %out_stack_0, %value_1
  //   %0 = arith.cmpi eq, %value_1, %c0_i32 : i32
  //   %1 = scf.if %0 -> (!sigi.stack) {
  //     scf.yield %out_stack_0 : !sigi.stack
  //   } else {
  //     %c10_i32 = arith.constant 10 : i32
  //     %c1_i32 = arith.constant 1 : i32
  //     %2 = sigi.push %out_stack_0, %value : i32
  //     %3 = func.call @"sigi::pp"(%2) : (!sigi.stack) -> !sigi.stack
  //     %out_stack_2, %value_3 = sigi.pop %3 : i32
  //     %4 = arith.subi %value_1, %c1_i32 : i32
  //     %5 = sigi.push %out_stack_2, %4 : i32
  //     %6 = arith.muli %value, %c10_i32 : i32
  //     %7 = sigi.push %5, %6 : i32
  //     %8 = func.call @fibloop(%7) : (!sigi.stack) -> !sigi.stack
  //     scf.yield %8 : !sigi.stack
  //   }
  //   return %1 : !sigi.stack
  // }
  
  // func.func @__main__(%arg0: !sigi.stack) -> !sigi.stack attributes {sigi.main} {
  //   %c1_i32 = arith.constant 1 : i32
  //   %c10_i32 = arith.constant 10 : i32
  //   %0 = sigi.push %arg0, %c10_i32 : i32
  //   %1 = sigi.push %0, %c1_i32 : i32
  //   %2 = call @fibloop(%1) : (!sigi.stack) -> !sigi.stack
  //   return %2 : !sigi.stack
  // }  

//   func.func @double_tail_recursive(%n: i32, %acc1: i32, %acc2: i32) -> i32 {
//   %zero = arith.constant 0 : i32
//   %one = arith.constant 1 : i32
//   %two = arith.constant 2 : i32
//   %remainder = arith.remsi %n, %two : i32
//   %is_zero = arith.cmpi "eq", %n, %zero : i32
//   %is_even = arith.cmpi "eq", %remainder, %zero : i32

//   %result = scf.if %is_zero -> i32 {     // Base case
//     %sum = arith.addi %acc1, %acc2 : i32
//     scf.yield %sum : i32
//   } else {
//     %next_n = arith.subi %n, %one : i32

//     %result = scf.if %is_even -> i32 {   // Tail-recursive case 1
//       %next_acc1 = arith.addi %acc1, %n : i32
//       %tail_call = func.call @double_tail_recursive(%next_n, %next_acc1, %acc2) : (i32, i32, i32) -> i32
//       scf.yield %tail_call : i32
//     } else {                             // Tail-recursive case 2
//       %next_acc2 = arith.addi %acc2, %n : i32
//       %tail_call = func.call @double_tail_recursive(%next_n, %acc1, %next_acc2) : (i32, i32, i32) -> i32
//       scf.yield %tail_call : i32
//     }
//     scf.yield %result : i32
//   }
//   return %result : i32
// }

//   func.func @worse_double_tail_recursive(%n: i32, %acc1: i32, %acc2: i32) -> i32 {
//   %zero = arith.constant 0 : i32
//   %one = arith.constant 1 : i32
//   %two = arith.constant 2 : i32
//   %three = arith.constant 3 : i32
//   %remainder = arith.remsi %n, %two : i32
//   %is_zero = arith.cmpi "eq", %n, %zero : i32
//   %is_even = arith.cmpi "eq", %remainder, %zero : i32
//   %not_three = arith.cmpi "ne", %n, %three : i32

//   %result = scf.if %is_zero -> i32 {     // Base case
//     %sum = arith.addi %acc1, %acc2 : i32
//     scf.yield %sum : i32
//   } else {
//     %next_n = arith.subi %n, %one : i32

//     %test = scf.if %not_three -> i32 {
//       %result = scf.if %is_even -> i32 {   // Tail-recursive case 1 if 1. not zero 2. not three 3. even
//         %next_acc1 = arith.addi %acc1, %n : i32
//         %tail_call = func.call @worse_double_tail_recursive(%next_n, %next_acc1, %acc2) : (i32, i32, i32) -> i32
//         scf.yield %tail_call : i32
//         } else {                 
//           scf.yield %next_n : i32            // Tail-recursive case 2 if 1. not zero 2. three          scf.yield %next_n : i32
//         }
//       scf.yield %result : i32
//     } else {
//       %next_acc2 = arith.addi %acc2, %n : i32
//       %tail_call = func.call @worse_double_tail_recursive(%next_n, %acc1, %next_acc2) : (i32, i32, i32) -> i32
//       scf.yield %tail_call : i32
//     }
//     scf.yield %test :i32
//   }
//   return %result : i32
// }

  func.func @nested_if(%n: i32, %acc1: i32, %acc2: i32) -> i32 {
  %zero = arith.constant 0 : i32
  %one = arith.constant 1 : i32
  %two = arith.constant 2 : i32
  %three = arith.constant 3 : i32
  %remainder = arith.remsi %n, %two : i32
  %is_zero = arith.cmpi "eq", %n, %zero : i32
  %is_even = arith.cmpi "eq", %remainder, %zero : i32
  %not_three = arith.cmpi "ne", %n, %three : i32

  %result = scf.if %is_zero -> i32 {     // Base case
    %sum = arith.addi %acc1, %acc2 : i32
    scf.yield %sum : i32
  } else {
    %next_n = arith.subi %n, %one : i32
    %test = scf.if %not_three -> i32 {
      %result = scf.if %is_even -> i32 {   // Tail-recursive case 1 if 1. not zero 2. not three 3. even
        %next_acc1 = arith.addi %acc1, %n : i32
        %tail_call = func.call @nested_if(%next_n, %next_acc1, %acc2) : (i32, i32, i32) -> i32
        scf.yield %tail_call : i32
        } else {                 
          scf.yield %next_n : i32            // Tail-recursive case 2 if 1. not zero 2. three          scf.yield %next_n : i32
        }
      scf.yield %result : i32
    } else {
      %next_acc2 = arith.addi %acc2, %n : i32
      // %tail_call = func.call @nested_if(%next_n, %acc1, %next_acc2) : (i32, i32, i32) -> i32
      scf.yield %next_acc2 : i32
    }
    scf.yield %test : i32
  }
  return %result : i32
}
}