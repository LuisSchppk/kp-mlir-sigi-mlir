module {
  llvm.func @sigi_free_stack(!llvm.ptr)
  llvm.func @sigi_init_stack(!llvm.ptr)
  llvm.func @malloc(i64) -> !llvm.ptr
  llvm.func @sigi_pop_i32(!llvm.ptr) -> i32
  llvm.func @sigi_push_i32(!llvm.ptr, i32)
  llvm.mlir.global linkonce @sigi.global.stack() {addr_space = 0 : i32} : !llvm.ptr
  llvm.func @sigi_builtin__pp_i32(i32) attributes {sym_visibility = "private"}
  llvm.func @sigi_builtin__pp(!llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"}
  llvm.func @fizzbuzz(%arg0: i32, %arg1: i32) attributes {sigi.stackType = (i32, i32) -> (), sym_visibility = "private"} {
    %0 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.call @sigi_push_i32(%1, %arg1) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %arg0) : (!llvm.ptr, i32) -> ()
    %2 = llvm.mlir.constant(2 : i32) : i32
    %3 = llvm.mlir.constant(0 : i32) : i32
    %4 = llvm.mlir.constant(-1 : i32) : i32
    %5 = llvm.mlir.constant(-3 : i32) : i32
    %6 = llvm.mlir.constant(1 : i32) : i32
    %7 = llvm.mlir.constant(5 : i32) : i32
    %8 = llvm.mlir.constant(-2 : i32) : i32
    %9 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %10 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %11 = llvm.icmp "sle" %10, %9 : i32
    llvm.cond_br %11, ^bb1, ^bb14
  ^bb1:  // pred: ^bb0
    %12 = llvm.urem %10, %2  : i32
    %13 = llvm.icmp "eq" %12, %3 : i32
    llvm.cond_br %13, ^bb2, ^bb7
  ^bb2:  // pred: ^bb1
    %14 = llvm.urem %10, %7  : i32
    %15 = llvm.icmp "eq" %14, %3 : i32
    llvm.cond_br %15, ^bb3, ^bb4
  ^bb3:  // pred: ^bb2
    llvm.call @sigi_builtin__pp_i32(%5) : (i32) -> ()
    %16 = llvm.add %10, %6 : i32
    llvm.call @sigi_push_i32(%1, %16) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %9) : (!llvm.ptr, i32) -> ()
    %17 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %18 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    llvm.store %1, %0 : !llvm.ptr, !llvm.ptr
    llvm.call @fizzbuzz(%17, %18) : (i32, i32) -> ()
    %19 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.br ^bb5(%19 : !llvm.ptr)
  ^bb4:  // pred: ^bb2
    llvm.call @sigi_builtin__pp_i32(%4) : (i32) -> ()
    %20 = llvm.add %10, %6 : i32
    llvm.call @sigi_push_i32(%1, %20) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %9) : (!llvm.ptr, i32) -> ()
    %21 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %22 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    llvm.store %1, %0 : !llvm.ptr, !llvm.ptr
    llvm.call @fizzbuzz(%21, %22) : (i32, i32) -> ()
    %23 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.br ^bb5(%23 : !llvm.ptr)
  ^bb5(%24: !llvm.ptr):  // 2 preds: ^bb3, ^bb4
    llvm.br ^bb6
  ^bb6:  // pred: ^bb5
    llvm.br ^bb12(%24 : !llvm.ptr)
  ^bb7:  // pred: ^bb1
    %25 = llvm.urem %10, %7  : i32
    %26 = llvm.icmp "eq" %25, %3 : i32
    llvm.cond_br %26, ^bb8, ^bb9
  ^bb8:  // pred: ^bb7
    llvm.call @sigi_builtin__pp_i32(%8) : (i32) -> ()
    %27 = llvm.add %10, %6 : i32
    llvm.call @sigi_push_i32(%1, %27) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %9) : (!llvm.ptr, i32) -> ()
    %28 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %29 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    llvm.store %1, %0 : !llvm.ptr, !llvm.ptr
    llvm.call @fizzbuzz(%28, %29) : (i32, i32) -> ()
    %30 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.br ^bb10(%30 : !llvm.ptr)
  ^bb9:  // pred: ^bb7
    llvm.call @sigi_builtin__pp_i32(%10) : (i32) -> ()
    %31 = llvm.add %10, %6 : i32
    llvm.call @sigi_push_i32(%1, %31) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %9) : (!llvm.ptr, i32) -> ()
    %32 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %33 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    llvm.store %1, %0 : !llvm.ptr, !llvm.ptr
    llvm.call @fizzbuzz(%32, %33) : (i32, i32) -> ()
    %34 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.br ^bb10(%34 : !llvm.ptr)
  ^bb10(%35: !llvm.ptr):  // 2 preds: ^bb8, ^bb9
    llvm.br ^bb11
  ^bb11:  // pred: ^bb10
    llvm.br ^bb12(%35 : !llvm.ptr)
  ^bb12(%36: !llvm.ptr):  // 2 preds: ^bb6, ^bb11
    llvm.br ^bb13
  ^bb13:  // pred: ^bb12
    llvm.br ^bb15(%36 : !llvm.ptr)
  ^bb14:  // pred: ^bb0
    llvm.br ^bb15(%1 : !llvm.ptr)
  ^bb15(%37: !llvm.ptr):  // 2 preds: ^bb13, ^bb14
    llvm.br ^bb16
  ^bb16:  // pred: ^bb15
    llvm.store %37, %0 : !llvm.ptr, !llvm.ptr
    llvm.return
  }
  llvm.func @__main__() attributes {sigi.stackType = () -> ()} {
    %0 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    %2 = llvm.mlir.constant(100 : i32) : i32
    %3 = llvm.mlir.constant(0 : i32) : i32
    llvm.call @sigi_push_i32(%1, %3) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %2) : (!llvm.ptr, i32) -> ()
    %4 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %5 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    llvm.store %1, %0 : !llvm.ptr, !llvm.ptr
    llvm.call @fizzbuzz(%4, %5) : (i32, i32) -> ()
    %6 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.store %6, %0 : !llvm.ptr, !llvm.ptr
    llvm.return
  }
  llvm.func @main() -> i1 {
    %0 = llvm.mlir.constant(128 : i64) : i64
    %1 = llvm.call @malloc(%0) : (i64) -> !llvm.ptr
    llvm.call @sigi_init_stack(%1) : (!llvm.ptr) -> ()
    %2 = llvm.bitcast %1 : !llvm.ptr to !llvm.ptr
    %3 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    llvm.store %2, %3 : !llvm.ptr, !llvm.ptr
    llvm.call @__main__() : () -> ()
    llvm.call @sigi_free_stack(%1) : (!llvm.ptr) -> ()
    %4 = llvm.mlir.constant(false) : i1
    llvm.return %4 : i1
  }
}

