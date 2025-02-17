module {
  llvm.func @sigi_free_stack(!llvm.ptr)
  llvm.func @sigi_init_stack(!llvm.ptr)
  llvm.func private @closure_worker_1(%arg0: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %1 = llvm.call @sigi_pop_bool(%arg0) : (!llvm.ptr) -> i1
    llvm.call @sigi_builtin__pp_i1(%1) : (i1) -> ()
    llvm.call @sigi_push_i32(%arg0, %0) : (!llvm.ptr, i32) -> ()
    llvm.return %arg0 : !llvm.ptr
  }
  llvm.func private @closure_wrapper_1(%arg0: !llvm.ptr, %arg1: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.getelementptr %arg0[0, 3] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.struct<()>
    %2 = llvm.call @closure_worker_1(%arg1) : (!llvm.ptr) -> !llvm.ptr
    llvm.return %2 : !llvm.ptr
  }
  llvm.func @malloc(i64) -> !llvm.ptr
  llvm.func private @closure_drop_nothing(%arg0: !llvm.ptr) {
    llvm.return
  }
  llvm.func private @closure_worker_0(%arg0: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.mlir.constant(0 : i32) : i32
    %1 = llvm.mlir.constant(1 : i32) : i32
    %2 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %3 = llvm.sub %2, %1 : i32
    llvm.call @sigi_push_i32(%arg0, %3) : (!llvm.ptr, i32) -> ()
    %4 = llvm.icmp "sgt" %2, %0 : i32
    llvm.call @sigi_push_bool(%arg0, %4) : (!llvm.ptr, i1) -> ()
    llvm.return %arg0 : !llvm.ptr
  }
  llvm.func private @closure_wrapper_0(%arg0: !llvm.ptr, %arg1: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.getelementptr %arg0[0, 3] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.struct<()>
    %2 = llvm.call @closure_worker_0(%arg1) : (!llvm.ptr) -> !llvm.ptr
    llvm.return %2 : !llvm.ptr
  }
  llvm.func @closure_decr_then_drop(!llvm.ptr)
  llvm.func @sigi_pop_i32(!llvm.ptr) -> i32
  llvm.func @sigi_pop_bool(!llvm.ptr) -> i1
  llvm.func @sigi_push_i32(!llvm.ptr, i32)
  llvm.func @sigi_push_bool(!llvm.ptr, i1)
  llvm.mlir.global linkonce @sigi.global.stack() {addr_space = 0 : i32} : !llvm.ptr
  llvm.func @sigi_builtin__pp_i32(i32) attributes {sym_visibility = "private"}
  llvm.func @sigi_builtin__pp_i1(i1) attributes {sym_visibility = "private"}
  llvm.func @sigi_builtin__pp(!llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"}
  llvm.func @while(%arg0: i1, %arg1: i32, %arg2: !llvm.ptr, %arg3: !llvm.ptr) -> i32 attributes {sym_visibility = "private"} {
    %0 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.call @sigi_push_bool(%1, %arg0) : (!llvm.ptr, i1) -> ()
    llvm.call @sigi_push_i32(%1, %arg1) : (!llvm.ptr, i32) -> ()
    %2 = llvm.load %arg2 : !llvm.ptr -> !llvm.ptr
    %3 = llvm.bitcast %arg2 : !llvm.ptr to !llvm.ptr
    %4 = llvm.call %2(%3, %1) : !llvm.ptr, (!llvm.ptr, !llvm.ptr) -> !llvm.ptr
    %5 = llvm.call @sigi_pop_bool(%4) : (!llvm.ptr) -> i1
    %6 = llvm.load %arg3 : !llvm.ptr -> !llvm.ptr
    %7 = llvm.bitcast %arg3 : !llvm.ptr to !llvm.ptr
    %8 = llvm.call %6(%7, %4) : !llvm.ptr, (!llvm.ptr, !llvm.ptr) -> !llvm.ptr
    llvm.cond_br %5, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    %9 = llvm.call @sigi_pop_i32(%8) : (!llvm.ptr) -> i32
    %10 = llvm.call @sigi_pop_bool(%8) : (!llvm.ptr) -> i1
    %11 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    llvm.store %8, %11 : !llvm.ptr, !llvm.ptr
    %12 = llvm.call @while(%10, %9, %arg2, %arg3) : (i1, i32, !llvm.ptr, !llvm.ptr) -> i32
    %13 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    %14 = llvm.load %13 : !llvm.ptr -> !llvm.ptr
    llvm.call @sigi_push_i32(%14, %12) : (!llvm.ptr, i32) -> ()
    %15 = builtin.unrealized_conversion_cast %14 : !llvm.ptr to !sigi.stack
    llvm.br ^bb3(%14 : !llvm.ptr)
  ^bb2:  // pred: ^bb0
    llvm.br ^bb3(%8 : !llvm.ptr)
  ^bb3(%16: !llvm.ptr):  // 2 preds: ^bb1, ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    %17 = llvm.call @sigi_pop_i32(%16) : (!llvm.ptr) -> i32
    %18 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    llvm.store %16, %18 : !llvm.ptr, !llvm.ptr
    llvm.return %17 : i32
  }
  llvm.func @print_stack_and_pop(%arg0: i1, %arg1: i32) -> i32 attributes {sigi.stackType = (i1, i32) -> i32, sym_visibility = "private"} {
    %0 = llvm.mlir.undef : !llvm.ptr
    %1 = llvm.getelementptr %0[1] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %2 = llvm.ptrtoint %1 : !llvm.ptr to i64
    %3 = llvm.call @malloc(%2) : (i64) -> !llvm.ptr
    %4 = llvm.mlir.undef : !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %5 = llvm.mlir.addressof @closure_wrapper_0 : !llvm.ptr
    %6 = llvm.mlir.addressof @closure_drop_nothing : !llvm.ptr
    %7 = llvm.mlir.constant(0 : i32) : i32
    %8 = llvm.insertvalue %5, %4[0] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %9 = llvm.insertvalue %7, %8[1] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %10 = llvm.insertvalue %6, %9[2] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %11 = llvm.bitcast %3 : !llvm.ptr to !llvm.ptr
    llvm.store %10, %11 : !llvm.struct<(ptr, i32, ptr, struct<()>)>, !llvm.ptr
    %12 = llvm.bitcast %3 : !llvm.ptr to !llvm.ptr
    %13 = llvm.mlir.undef : !llvm.ptr
    %14 = llvm.getelementptr %13[1] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %15 = llvm.ptrtoint %14 : !llvm.ptr to i64
    %16 = llvm.call @malloc(%15) : (i64) -> !llvm.ptr
    %17 = llvm.mlir.undef : !llvm.struct<(ptr, i32, ptr, struct<()>)>
    %18 = llvm.mlir.addressof @closure_wrapper_1 : !llvm.ptr
    %19 = llvm.mlir.addressof @closure_drop_nothing : !llvm.ptr
    %20 = llvm.mlir.constant(0 : i32) : i32
    %21 = llvm.insertvalue %18, %17[0] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %22 = llvm.insertvalue %20, %21[1] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %23 = llvm.insertvalue %19, %22[2] : !llvm.struct<(ptr, i32, ptr, struct<()>)> 
    %24 = llvm.bitcast %16 : !llvm.ptr to !llvm.ptr
    llvm.store %23, %24 : !llvm.struct<(ptr, i32, ptr, struct<()>)>, !llvm.ptr
    %25 = llvm.bitcast %16 : !llvm.ptr to !llvm.ptr
    %26 = llvm.call @while(%arg0, %arg1, %12, %25) : (i1, i32, !llvm.ptr, !llvm.ptr) -> i32
    llvm.return %26 : i32
  }
  llvm.func @pow_loop(%arg0: i32, %arg1: i32, %arg2: i32) -> !llvm.struct<(i32, i1)> attributes {sigi.stackType = (i32, i32, i32) -> (i32, i1), sym_visibility = "private"} {
    %0 = llvm.mlir.constant(0 : i32) : i32
    %1 = llvm.mlir.constant(true) : i1
    %2 = llvm.mlir.constant(1 : i32) : i32
    %3 = llvm.icmp "sle" %arg1, %0 : i32
    llvm.cond_br %3, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    llvm.br ^bb3(%arg2, %1 : i32, i1)
  ^bb2:  // pred: ^bb0
    llvm.call @sigi_builtin__pp_i32(%arg2) : (i32) -> ()
    %4 = llvm.sub %arg1, %2 : i32
    %5 = llvm.mul %arg0, %arg2 : i32
    %6 = llvm.call @pow_loop(%arg0, %4, %5) : (i32, i32, i32) -> !llvm.struct<(i32, i1)>
    %7 = llvm.extractvalue %6[0] : !llvm.struct<(i32, i1)> 
    %8 = llvm.extractvalue %6[1] : !llvm.struct<(i32, i1)> 
    llvm.br ^bb3(%7, %8 : i32, i1)
  ^bb3(%9: i32, %10: i1):  // 2 preds: ^bb1, ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    %11 = llvm.mlir.undef : !llvm.struct<(i32, i1)>
    %12 = llvm.insertvalue %9, %11[0] : !llvm.struct<(i32, i1)> 
    %13 = llvm.insertvalue %10, %12[1] : !llvm.struct<(i32, i1)> 
    llvm.return %13 : !llvm.struct<(i32, i1)>
  }
  llvm.func @__main__() -> !llvm.struct<(i32, i32)> attributes {sigi.stackType = () -> (i32, i32)} {
    %0 = llvm.mlir.constant(2 : i32) : i32
    %1 = llvm.mlir.constant(6 : i32) : i32
    %2 = llvm.mlir.constant(1 : i32) : i32
    %3 = llvm.call @pow_loop(%0, %1, %2) : (i32, i32, i32) -> !llvm.struct<(i32, i1)>
    %4 = llvm.extractvalue %3[0] : !llvm.struct<(i32, i1)> 
    %5 = llvm.extractvalue %3[1] : !llvm.struct<(i32, i1)> 
    %6 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    %7 = llvm.load %6 : !llvm.ptr -> !llvm.ptr
    llvm.call @sigi_push_i32(%7, %4) : (!llvm.ptr, i32) -> ()
    %8 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    llvm.store %7, %8 : !llvm.ptr, !llvm.ptr
    %9 = llvm.call @print_stack_and_pop(%5, %2) : (i1, i32) -> i32
    %10 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    %11 = llvm.load %10 : !llvm.ptr -> !llvm.ptr
    %12 = llvm.call @sigi_pop_i32(%11) : (!llvm.ptr) -> i32
    %13 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    llvm.store %11, %13 : !llvm.ptr, !llvm.ptr
    %14 = llvm.mlir.undef : !llvm.struct<(i32, i32)>
    %15 = llvm.insertvalue %12, %14[0] : !llvm.struct<(i32, i32)> 
    %16 = llvm.insertvalue %9, %15[1] : !llvm.struct<(i32, i32)> 
    llvm.return %16 : !llvm.struct<(i32, i32)>
  }
  llvm.func @main() -> i1 {
    %0 = llvm.mlir.constant(128 : i64) : i64
    %1 = llvm.call @malloc(%0) : (i64) -> !llvm.ptr
    llvm.call @sigi_init_stack(%1) : (!llvm.ptr) -> ()
    %2 = llvm.bitcast %1 : !llvm.ptr to !llvm.ptr
    %3 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    llvm.store %2, %3 : !llvm.ptr, !llvm.ptr
    %4 = llvm.call @__main__() : () -> !llvm.struct<(i32, i32)>
    llvm.call @sigi_free_stack(%1) : (!llvm.ptr) -> ()
    %5 = llvm.mlir.constant(false) : i1
    llvm.return %5 : i1
  }
}

