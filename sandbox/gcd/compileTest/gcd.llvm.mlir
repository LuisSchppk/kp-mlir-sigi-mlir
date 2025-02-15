module {
  llvm.func @sigi_free_stack(!llvm.ptr)
  llvm.func @sigi_init_stack(!llvm.ptr)
  llvm.func @malloc(i64) -> !llvm.ptr
  llvm.func @sigi_builtin__pp_i32(i32) attributes {sym_visibility = "private"}
  llvm.func @sigi_builtin__pp(!llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"}
  llvm.func @external(!llvm.ptr) -> !llvm.ptr attributes {sym_visibility = "private"}
  llvm.func @gcd(%arg0: i32, %arg1: i32) -> i32 attributes {sigi.stackType = (i32, i32) -> i32, sym_visibility = "private"} {
    %0 = llvm.mlir.constant(0 : i32) : i32
    %1 = llvm.icmp "eq" %arg0, %0 : i32
    llvm.cond_br %1, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    llvm.br ^bb3(%arg1 : i32)
  ^bb2:  // pred: ^bb0
    %2 = llvm.urem %arg1, %arg0  : i32
    %3 = llvm.call @gcd(%2, %arg0) : (i32, i32) -> i32
    llvm.br ^bb3(%3 : i32)
  ^bb3(%4: i32):  // 2 preds: ^bb1, ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    llvm.return %4 : i32
  }
  llvm.func @__main__() attributes {sigi.stackType = () -> ()} {
    %0 = llvm.mlir.constant(24 : i32) : i32
    %1 = llvm.mlir.constant(8 : i32) : i32
    %2 = llvm.call @gcd(%1, %0) : (i32, i32) -> i32
    llvm.call @sigi_builtin__pp_i32(%2) : (i32) -> ()
    llvm.return
  }
  llvm.func @main() -> i1 {
    llvm.call @__main__() : () -> ()
    %0 = llvm.mlir.constant(false) : i1
    llvm.return %0 : i1
  }
}

