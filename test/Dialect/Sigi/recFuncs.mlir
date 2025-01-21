module {

  // 1. Non-recursive: Add two numbers
  func.func @add_two(%a: i32, %b: i32) -> i32 {
    %sum = arith.addi %a, %b : i32
    return %sum : i32
  }

  // 2. Recursive: Factorial
  func.func @factorial(%n: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %one = arith.constant 1 : i32
    %is_zero = arith.cmpi "eq", %n, %zero : i32
    %result = scf.if %is_zero -> i32 {
      scf.yield %one : i32
    } else {
      %n_minus_1 = arith.subi %n, %one : i32
      %fact = func.call @factorial(%n_minus_1) : (i32) -> i32
      %product = arith.muli %n, %fact : i32
      scf.yield %product : i32
    }
    return %result : i32
  }

  // 3. Tail-recursive: GCD
  func.func @gcd(%a: i32, %b: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %is_zero = arith.cmpi "eq", %b, %zero : i32
    %result = scf.if %is_zero -> i32 {
      scf.yield %a : i32
    } else {
      %remainder = arith.remsi %a, %b : i32
      %result = func.call @gcd(%b, %remainder) : (i32, i32) -> i32
      scf.yield %result : i32
    }
    return %result : i32
  }


    // 3.1. Tail-recursive: GCD with unrelated ops
  func.func @gcd_2(%a: i32, %b: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %is_zero = arith.cmpi "eq", %b, %zero : i32
    %result = scf.if %is_zero -> i32 {
      scf.yield %a : i32
    } else {
      %remainder = arith.remsi %a, %b : i32
      %result = func.call @gcd_2(%b, %remainder) : (i32, i32) -> i32
      scf.yield %result : i32
    }
    %const = arith.constant 12: i32 
    %ignore = arith.muli %a, %const : i32
    return %result : i32
  }

  // 4. Non-recursive: Square a number
  func.func @square(%x: i32) -> i32 {
    %result = arith.muli %x, %x : i32
    return %result : i32
  }

  // 5. Recursive: Power (multiple recursive calls)
  func.func @power(%base: i32, %exp: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %one = arith.constant 1 : i32
    %is_zero = arith.cmpi "eq", %exp, %zero : i32
    %result = scf.if %is_zero -> i32 {
      scf.yield %one : i32
    } else {
      %half_exp = arith.divui %exp, %one : i32
      %half_power = func.call @power(%base, %half_exp) : (i32, i32) -> i32
      %product = arith.muli %half_power, %half_power : i32
      %mod = arith.remsi %exp, %one : i32
      %is_odd = arith.cmpi "ne", %mod, %zero : i32
      %final_result = scf.if %is_odd -> i32 {
        %temp_result = arith.muli %product, %base : i32
        scf.yield %temp_result : i32
      } else {
        scf.yield %product : i32
      }
      scf.yield %final_result : i32
    }
    return %result : i32
  }

  // // // 6. Tail-recursive: Sum of N numbers
  // func.func @sum_n(%n: i32, %acc: i32) -> i32 {
  //   %zero = arith.constant 0 : i32
  //   %is_zero = arith.cmpi "eq", %n, %zero : i32
  //   %is_non_negative = arith.cmpi "sge", %n, %zero : i32
  //   %result = scf.if %is_zero -> i32 {
  //       %result = scf.if %is_zero -> i32 { 
  //       %next_n = arith.subi %n, %zero : i32
  //       %next_acc = arith.addi %acc, %n : i32
  //       %result = func.call @sum_n(%next_n, %next_acc) : (i32, i32) -> i32
  //       scf.yield %result : i32
  //     } else {
  //       scf.yield %acc : i32
  //     }
  //     scf.yield %acc : i32
  //   } else {
  //     %next_n = arith.subi %n, %zero : i32
  //     %next_acc = arith.addi %acc, %n : i32
  //     // %result = func.call @sum_n(%next_n, %next_acc) : (i32, i32) -> i32
  //     scf.yield %next_acc : i32
  //   }
  //   return %result : i32
  // }

  // 7. Non-recursive: Absolute value
  func.func @absolute(%x: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %is_non_negative = arith.cmpi "sge", %x, %zero : i32
    %result = scf.if %is_non_negative -> i32 {
      scf.yield %x : i32
    } else {
      %neg = arith.subi %zero, %x : i32
      scf.yield %neg : i32
    }
    return %result : i32
  }

  // 8. Recursive: Fibonacci (multiple recursive calls)
  func.func @fibonacci(%n: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %one = arith.constant 1 : i32
    %is_zero = arith.cmpi "eq", %n, %zero : i32
    %is_one = arith.cmpi "eq", %n, %one : i32
    %result = scf.if %is_zero -> i32 {
      scf.yield %zero : i32
    } else {
      %result = scf.if %is_one -> i32 {
        scf.yield %one : i32
      } else {
        %n_minus_1 = arith.subi %n, %one : i32
        %n_minus_2 = arith.subi %n, %one : i32
        %fib1 = func.call @fibonacci(%n_minus_1) : (i32) -> i32
        %fib2 = func.call @fibonacci(%n_minus_2) : (i32) -> i32
        %sum = arith.addi %fib1, %fib2 : i32
        scf.yield %sum : i32
      }
      scf.yield %result : i32
    }
    return %result : i32
  }

  // 9. Tail-recursive: Reverse an array (mocked with indices)
  func.func @reverse(%arr: memref<?xi32>, %i: index, %j: index) -> memref<?xi32> {
    %one = arith.constant 1: index
    %cond = arith.cmpi "slt", %i, %j : index
    %result = scf.if %cond -> memref<?xi32> {
      %temp_i = memref.load %arr[%i] : memref<?xi32>
      %temp_j = memref.load %arr[%j] : memref<?xi32>
      memref.store %temp_i, %arr[%j] : memref<?xi32>
      memref.store %temp_j, %arr[%i] : memref<?xi32>
      %next_i = arith.addi %i, %one : index
      %next_j = arith.subi %j, %one : index
      %reversed = func.call @reverse(%arr, %next_i, %next_j) : (memref<?xi32>, index, index) -> memref<?xi32>
      scf.yield %reversed : memref<?xi32>
    } else {
      scf.yield %arr : memref<?xi32>
    }
    return %result : memref<?xi32>
  }

  // 10. Non-recursive: Multiply two numbers
  func.func @multiply(%a: i32, %b: i32) -> i32 {
    %product = arith.muli %a, %b : i32
    return %product : i32
  }

    func.func @double_tail_recursive(%a: i32, %b: i32, %acc1: i32, %acc2: i32) -> (i32, i32) {
     %zero = arith.constant 0 : i32
     %one = arith.constant 1: i32
     %is_a_zero = arith.cmpi "eq", %a, %zero : i32
     %is_b_zero = arith.cmpi "eq", %b, %zero : i32
     %result2 = arith.constant 0 : i32
     %result1 = scf.if %is_a_zero -> (i32) {
       scf.yield %acc1 : i32
      } else {
       %new_a = arith.subi %a, %one : i32
      %new_acc1 = arith.addi %acc1, %a : i32
       %res1, %ignore1 = func.call @double_tail_recursive(%new_a, %b, %new_acc1, %acc2) : (i32, i32, i32, i32) -> (i32, i32)
      scf.yield %res1 : i32
     }

     %result3 = scf.if %is_b_zero -> (i32) {
       scf.yield %acc2 : i32
     } else {
      %new_b = arith.subi %b, %one : i32
     %new_acc2 = arith.muli %acc2, %b : i32
        %res2, %ignore2 = func.call @double_tail_recursive(%a, %new_b, %acc1, %new_acc2) : (i32, i32, i32, i32) -> (i32, i32)
        scf.yield %res2 : i32
    } 

    return %result1, %result2 : i32, i32
    }   
}
