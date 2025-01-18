module {
  llvm.func @sigi_free_stack(!llvm.ptr)
  llvm.func @sigi_init_stack(!llvm.ptr)
  llvm.func @sigi_pop_bool(!llvm.ptr) -> i1
  llvm.func private @closure_worker_1(%arg0: i32, %arg1: i32, %arg2: !llvm.ptr) -> !llvm.ptr {
    llvm.call @sigi_push_i32(%arg2, %arg1) : (!llvm.ptr, i32) -> ()
    %0 = llvm.call @show(%arg2) : (!llvm.ptr) -> !llvm.ptr
    llvm.call @sigi_push_i32(%0, %arg0) : (!llvm.ptr, i32) -> ()
    %1 = llvm.mlir.constant(1 : i32) : i32
    llvm.call @sigi_push_i32(%0, %1) : (!llvm.ptr, i32) -> ()
    %2 = llvm.call @sigi_pop_i32(%0) : (!llvm.ptr) -> i32
    %3 = llvm.call @sigi_pop_i32(%0) : (!llvm.ptr) -> i32
    %4 = llvm.sub %3, %2 : i32
    llvm.call @sigi_push_i32(%0, %4) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%0, %arg1) : (!llvm.ptr, i32) -> ()
    %5 = llvm.mlir.constant(10 : i32) : i32
    llvm.call @sigi_push_i32(%0, %5) : (!llvm.ptr, i32) -> ()
    %6 = llvm.call @sigi_pop_i32(%0) : (!llvm.ptr) -> i32
    %7 = llvm.call @sigi_pop_i32(%0) : (!llvm.ptr) -> i32
    %8 = llvm.mul %7, %6 : i32
    llvm.call @sigi_push_i32(%0, %8) : (!llvm.ptr, i32) -> ()
    %9 = llvm.call @fibloop(%0) : (!llvm.ptr) -> !llvm.ptr
    llvm.return %9 : !llvm.ptr
  }
  llvm.func private @closure_wrapper_1(%arg0: !llvm.ptr, %arg1: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.getelementptr %arg0[0, 3] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)>
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.struct<(i32, i32)>
    %2 = llvm.extractvalue %1[0] : !llvm.struct<(i32, i32)> 
    %3 = llvm.extractvalue %1[1] : !llvm.struct<(i32, i32)> 
    %4 = llvm.call @closure_worker_1(%2, %3, %arg1) : (i32, i32, !llvm.ptr) -> !llvm.ptr
    llvm.return %4 : !llvm.ptr
  }
  llvm.func @sigi_push_closure(!llvm.ptr, !llvm.ptr)
  llvm.func @malloc(i64) -> !llvm.ptr
  llvm.func private @closure_drop_nothing(%arg0: !llvm.ptr) {
    llvm.return
  }
  llvm.func private @closure_worker_0(%arg0: !llvm.ptr) -> !llvm.ptr {
    llvm.return %arg0 : !llvm.ptr
  }
  llvm.func private @closure_wrapper_0(%arg0: !llvm.ptr, %arg1: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.getelementptr %arg0[0, 3] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %1 = llvm.call @closure_worker_0(%arg1) : (!llvm.ptr) -> !llvm.ptr
    llvm.return %1 : !llvm.ptr
  }
  llvm.func @closure_decr_then_drop(!llvm.ptr)
  llvm.func @sigi_push_bool(!llvm.ptr, i1)
  llvm.func @sigi_push_i32(!llvm.ptr, i32)
  llvm.func @sigi_pop_i32(!llvm.ptr) -> i32
  llvm.func @sigi_pop_closure(!llvm.ptr) -> !llvm.ptr
  llvm.func @sigi_builtin__pp(!llvm.ptr) -> !llvm.ptr
  llvm.func @sigi_builtin_pp(!llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"}
  llvm.func @apply(%arg0: !llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"} {
    %0 = llvm.call @sigi_pop_closure(%arg0) : (!llvm.ptr) -> !llvm.ptr
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    %2 = llvm.bitcast %0 : !llvm.ptr to !llvm.ptr
    %3 = llvm.call %1(%2, %arg0) : !llvm.ptr, (!llvm.ptr, !llvm.ptr) -> !llvm.ptr
    llvm.return %3 : !llvm.ptr
  }
  llvm.func @show(%arg0: !llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"} {
    %0 = llvm.call @sigi_builtin__pp(%arg0) : (!llvm.ptr) -> !llvm.ptr
    %1 = llvm.call @sigi_pop_i32(%0) : (!llvm.ptr) -> i32
    llvm.return %0 : !llvm.ptr
  }
  llvm.func @fibloop(%arg0: !llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"} {
    %0 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %1 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    llvm.call @sigi_push_i32(%arg0, %1) : (!llvm.ptr, i32) -> ()
    %2 = llvm.mlir.constant(0 : i32) : i32
    llvm.call @sigi_push_i32(%arg0, %2) : (!llvm.ptr, i32) -> ()
    %3 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %4 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %5 = llvm.icmp "eq" %4, %3 : i32
    llvm.call @sigi_push_bool(%arg0, %5) : (!llvm.ptr, i1) -> ()
    %6 = llvm.mlir.undef : !llvm.ptr
    %7 = llvm.getelementptr %6[1] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %8 = llvm.ptrtoint %7 : !llvm.ptr to i64
    %9 = llvm.call @malloc(%8) : (i64) -> !llvm.ptr
    %10 = llvm.mlir.undef : !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %11 = llvm.mlir.addressof @closure_wrapper_0 : !llvm.ptr
    %12 = llvm.mlir.addressof @closure_drop_nothing : !llvm.ptr
    %13 = llvm.insertvalue %11, %10[0] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %14 = llvm.insertvalue %2, %13[1] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %15 = llvm.insertvalue %12, %14[2] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %16 = llvm.bitcast %9 : !llvm.ptr to !llvm.ptr
    llvm.store %15, %16 : !llvm.struct<(ptr, i32, ptr, struct<()>)>, !llvm.ptr
    llvm.call @sigi_push_closure(%arg0, %16) : (!llvm.ptr, !llvm.ptr) -> ()
    %17 = llvm.getelementptr %6[1] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)>
    %18 = llvm.ptrtoint %17 : !llvm.ptr to i64
    %19 = llvm.call @malloc(%18) : (i64) -> !llvm.ptr
    %20 = llvm.mlir.undef : !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)>
    %21 = llvm.mlir.addressof @closure_wrapper_1 : !llvm.ptr
    %22 = llvm.insertvalue %21, %20[0] : !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)> 
    %23 = llvm.insertvalue %2, %22[1] : !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)> 
    %24 = llvm.insertvalue %12, %23[2] : !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)> 
    %25 = llvm.insertvalue %1, %24[3, 0] : !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)> 
    %26 = llvm.insertvalue %0, %25[3, 1] : !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)> 
    %27 = llvm.bitcast %19 : !llvm.ptr to !llvm.ptr
    llvm.store %26, %27 : !llvm.struct<(ptr, i32, ptr, struct<(i32, i32)>)>, !llvm.ptr
    llvm.call @sigi_push_closure(%arg0, %27) : (!llvm.ptr, !llvm.ptr) -> ()
    %28 = llvm.call @sigi_pop_closure(%arg0) : (!llvm.ptr) -> !llvm.ptr
    %29 = llvm.call @sigi_pop_closure(%arg0) : (!llvm.ptr) -> !llvm.ptr
    %30 = llvm.call @sigi_pop_bool(%arg0) : (!llvm.ptr) -> i1
    llvm.cond_br %30, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    llvm.br ^bb3(%29 : !llvm.ptr)
  ^bb2:  // pred: ^bb0
    llvm.br ^bb3(%28 : !llvm.ptr)
  ^bb3(%31: !llvm.ptr):  // 2 preds: ^bb1, ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    llvm.call @sigi_push_closure(%arg0, %31) : (!llvm.ptr, !llvm.ptr) -> ()
    %32 = llvm.call @apply(%arg0) : (!llvm.ptr) -> !llvm.ptr
    llvm.return %32 : !llvm.ptr
  }
  llvm.func @__main__(%arg0: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.mlir.constant(10 : i32) : i32
    llvm.call @sigi_push_i32(%arg0, %0) : (!llvm.ptr, i32) -> ()
    %1 = llvm.mlir.constant(1 : i32) : i32
    llvm.call @sigi_push_i32(%arg0, %1) : (!llvm.ptr, i32) -> ()
    %2 = llvm.call @fibloop(%arg0) : (!llvm.ptr) -> !llvm.ptr
    llvm.return %2 : !llvm.ptr
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

