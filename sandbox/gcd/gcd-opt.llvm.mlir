module {
  llvm.func @malloc(i64) -> !llvm.ptr
  llvm.func @sigi_free_stack(!llvm.ptr)
  llvm.func @sigi_init_stack(!llvm.ptr)
  llvm.func @sigi_push_i32(!llvm.ptr, i32)
  llvm.func @sigi_pop_i32(!llvm.ptr) -> i32
  llvm.func @sigi_builtin__pp(!llvm.ptr) -> !llvm.ptr
  llvm.func @sigi_builtin_pp(!llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"}
  llvm.func @gcd(%arg0: !llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"} {
    %0 = llvm.mlir.constant(0 : i32) : i32
    %1 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %2 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %3 = llvm.icmp "eq" %1, %0 : i32
    llvm.cond_br %3, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    llvm.call @sigi_push_i32(%arg0, %2) : (!llvm.ptr, i32) -> ()
    llvm.br ^bb3(%arg0 : !llvm.ptr)
  ^bb2:  // pred: ^bb0
    llvm.call @sigi_push_i32(%arg0, %1) : (!llvm.ptr, i32) -> ()
    %4 = llvm.srem %2, %1  : i32
    llvm.call @sigi_push_i32(%arg0, %4) : (!llvm.ptr, i32) -> ()
    %5 = llvm.call @gcd(%arg0) : (!llvm.ptr) -> !llvm.ptr
    llvm.br ^bb3(%5 : !llvm.ptr)
  ^bb3(%6: !llvm.ptr):  // 2 preds: ^bb1, ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    llvm.return %6 : !llvm.ptr
  }
  llvm.func @__main__(%arg0: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.mlir.constant(8 : i32) : i32
    %1 = llvm.mlir.constant(24 : i32) : i32
    llvm.call @sigi_push_i32(%arg0, %1) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%arg0, %0) : (!llvm.ptr, i32) -> ()
    %2 = llvm.call @gcd(%arg0) : (!llvm.ptr) -> !llvm.ptr
    %3 = llvm.call @sigi_builtin__pp(%2) : (!llvm.ptr) -> !llvm.ptr
    %4 = llvm.call @sigi_pop_i32(%3) : (!llvm.ptr) -> i32
    llvm.return %3 : !llvm.ptr
  }
  llvm.func @main() {
    %0 = llvm.mlir.constant(128 : i64) : i64
    %1 = llvm.call @malloc(%0) : (i64) -> !llvm.ptr
    llvm.call @sigi_init_stack(%1) : (!llvm.ptr) -> ()
    %2 = llvm.bitcast %1 : !llvm.ptr to !llvm.ptr
    %3 = llvm.call @__main__(%2) : (!llvm.ptr) -> !llvm.ptr
    llvm.call @sigi_free_stack(%1) : (!llvm.ptr) -> ()
    llvm.return
  }
}

