module {
  llvm.func @sigi_push_i32(!llvm.ptr, i32)
  llvm.func @sigi_pop_i32(!llvm.ptr) -> i32
  llvm.func @simpleSigi(%arg0: !llvm.ptr) -> !llvm.ptr {
    %0 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %1 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    llvm.call @sigi_push_i32(%arg0, %1) : (!llvm.ptr, i32) -> ()
    llvm.call @sigi_push_i32(%arg0, %0) : (!llvm.ptr, i32) -> ()
    %2 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %3 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %4 = llvm.mul %3, %2 : i32
    llvm.call @sigi_push_i32(%arg0, %4) : (!llvm.ptr, i32) -> ()
    %5 = llvm.mlir.constant(1 : i32) : i32
    llvm.call @sigi_push_i32(%arg0, %5) : (!llvm.ptr, i32) -> ()
    %6 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %7 = llvm.call @sigi_pop_i32(%arg0) : (!llvm.ptr) -> i32
    %8 = llvm.add %7, %6 : i32
    llvm.call @sigi_push_i32(%arg0, %8) : (!llvm.ptr, i32) -> ()
    llvm.return %arg0 : !llvm.ptr
  }
}

