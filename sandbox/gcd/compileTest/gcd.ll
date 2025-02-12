; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

declare void @sigi_free_stack(ptr)

declare void @sigi_init_stack(ptr)

declare ptr @malloc(i64)

declare ptr @sigi_builtin__pp(ptr)

declare void @sigi_builtin__pp_i32(i32)

declare ptr @external(ptr)

define i32 @gcd(i32 %0, i32 %1) {
  %3 = icmp eq i32 %0, 0
  br i1 %3, label %4, label %5

4:                                                ; preds = %2
  br label %8

5:                                                ; preds = %2
  %6 = urem i32 %1, %0
  %7 = call i32 @gcd(i32 %6, i32 %0)
  br label %8

8:                                                ; preds = %4, %5
  %9 = phi i32 [ %7, %5 ], [ %1, %4 ]
  br label %10

10:                                               ; preds = %8
  ret i32 %9
}

define void @__main__() {
  %1 = call i32 @gcd(i32 8, i32 24)
  call void @sigi_builtin__pp_i32(i32 %1)
  ret void
}

define void @main() {
  call void @__main__()
  ret void
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
