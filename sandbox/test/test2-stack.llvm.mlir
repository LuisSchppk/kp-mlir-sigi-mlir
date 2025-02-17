module {
  llvm.func @sigi_free_stack(!llvm.ptr)
  llvm.func @sigi_init_stack(!llvm.ptr)
  llvm.func @malloc(i64) -> !llvm.ptr
  llvm.func @sigi_pop_bool(!llvm.ptr) -> i1
  llvm.func @sigi_push_bool(!llvm.ptr, i1)
  llvm.func @sigi_pop_i32(!llvm.ptr) -> i32
  llvm.func @sigi_push_i32(!llvm.ptr, i32)
  llvm.mlir.global linkonce @sigi.global.stack() {addr_space = 0 : i32} : !llvm.ptr
  llvm.func @sigi_builtin__pp_i1(i1) attributes {sym_visibility = "private"}
  llvm.func @sigi_builtin__pp_i32(i32) attributes {sym_visibility = "private"}
  llvm.func @sigi_builtin__pp(!llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"}
  llvm.func @pow_loop(%arg0: i32, %arg1: i32, %arg2: i32) -> !llvm.struct<(i32, i1)> attributes {sigi.stackType = (i32, i32, i32) -> (i32, i1), sym_visibility = "private"} {
    %0 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.call @sigi_push_i32(%1, %arg0) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %arg1) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %arg2) : (!llvm.ptr, i32) -> ()
    %2 = llvm.mlir.constant(1 : i32) : i32
    %3 = llvm.mlir.constant(true) : i1
    %4 = llvm.mlir.constant(0 : i32) : i32
    %5 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %6 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %7 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %8 = llvm.icmp "sle" %6, %4 : i32
    llvm.cond_br %8, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    llvm.br ^bb3(%5, %3, %1 : i32, i1, !llvm.ptr)
  ^bb2:  // pred: ^bb0
    llvm.call @sigi_builtin__pp_i32(%5) : (i32) -> ()
    llvm.call @sigi_push_i32(%1, %7) : (!llvm.ptr, i32) -> ()
    %9 = llvm.sub %6, %2 : i32
    llvm.call @sigi_push_i32(%1, %9) : (!llvm.ptr, i32) -> ()
    %10 = llvm.mul %7, %5 : i32
    llvm.call @sigi_push_i32(%1, %10) : (!llvm.ptr, i32) -> ()
    %11 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %12 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %13 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    llvm.store %1, %0 : !llvm.ptr, !llvm.ptr
    %14 = llvm.call @pow_loop(%13, %12, %11) : (i32, i32, i32) -> !llvm.struct<(i32, i1)>
    %15 = llvm.extractvalue %14[0] : !llvm.struct<(i32, i1)> 
    %16 = llvm.extractvalue %14[1] : !llvm.struct<(i32, i1)> 
    %17 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.br ^bb3(%15, %16, %17 : i32, i1, !llvm.ptr)
  ^bb3(%18: i32, %19: i1, %20: !llvm.ptr):  // 2 preds: ^bb1, ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    llvm.call @sigi_push_i32(%20, %18) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_bool(%20, %19) : (!llvm.ptr, i1) -> ()
    %21 = llvm.call @sigi_pop_bool(%20) : (!llvm.ptr) -> i1
    %22 = llvm.call @sigi_pop_i32(%20) : (!llvm.ptr) -> i32
    llvm.store %20, %0 : !llvm.ptr, !llvm.ptr
    %23 = llvm.mlir.undef : !llvm.struct<(i32, i1)>
    %24 = llvm.insertvalue %22, %23[0] : !llvm.struct<(i32, i1)> 
    %25 = llvm.insertvalue %21, %24[1] : !llvm.struct<(i32, i1)> 
    llvm.return %25 : !llvm.struct<(i32, i1)>
  }
  llvm.func @__main__() attributes {sigi.stackType = () -> ()} {
    %0 = llvm.mlir.addressof @sigi.global.stack : !llvm.ptr
    %1 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    %2 = llvm.mlir.constant(1 : i32) : i32
    %3 = llvm.mlir.constant(6 : i32) : i32
    %4 = llvm.mlir.constant(2 : i32) : i32
    llvm.call @sigi_push_i32(%1, %4) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %3) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%1, %2) : (!llvm.ptr, i32) -> ()
    %5 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %6 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    %7 = llvm.call @sigi_pop_i32(%1) : (!llvm.ptr) -> i32
    llvm.store %1, %0 : !llvm.ptr, !llvm.ptr
    %8 = llvm.call @pow_loop(%7, %6, %5) : (i32, i32, i32) -> !llvm.struct<(i32, i1)>
    %9 = llvm.extractvalue %8[0] : !llvm.struct<(i32, i1)> 
    %10 = llvm.extractvalue %8[1] : !llvm.struct<(i32, i1)> 
    %11 = llvm.load %0 : !llvm.ptr -> !llvm.ptr
    llvm.call @sigi_builtin__pp_i1(%10) : (i1) -> ()
    llvm.call @sigi_builtin__pp_i32(%9) : (i32) -> ()
    llvm.store %11, %0 : !llvm.ptr, !llvm.ptr
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

