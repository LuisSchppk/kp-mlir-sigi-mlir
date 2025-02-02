; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

declare ptr @malloc(i64)

declare void @sigi_free_stack(ptr)

declare void @sigi_init_stack(ptr)

declare void @sigi_push_i32(ptr, i32)

declare i32 @sigi_pop_i32(ptr)

declare ptr @sigi_builtin__pp(ptr)

declare ptr @sigi_builtin_pp(ptr)

define ptr @gcd(ptr %0) {
  %2 = call i32 @sigi_pop_i32(ptr %0)
  %3 = call i32 @sigi_pop_i32(ptr %0)
  %4 = icmp eq i32 %2, 0
  br label %5

5:                                                ; preds = %9, %1
  %6 = phi i32 [ %12, %9 ], [ %2, %1 ]
  %7 = phi i32 [ %10, %9 ], [ %3, %1 ]
  %8 = phi i1 [ %13, %9 ], [ %4, %1 ]
  br i1 %8, label %9, label %14

9:                                                ; preds = %5
  %10 = phi i32 [ %6, %5 ]
  %11 = phi i32 [ %7, %5 ]
  %12 = srem i32 %11, %10
  %13 = icmp eq i32 %12, 0
  br label %5

14:                                               ; preds = %5
  call void @sigi_push_i32(ptr %0, i32 %7)
  ret ptr %0
}

define ptr @__main__(ptr %0) {
  call void @sigi_push_i32(ptr %0, i32 24)
  call void @sigi_push_i32(ptr %0, i32 8)
  %2 = call ptr @gcd(ptr %0)
  %3 = call ptr @sigi_builtin__pp(ptr %2)
  %4 = call i32 @sigi_pop_i32(ptr %3)
  ret ptr %3
}

define void @main() {
  %1 = call ptr @malloc(i64 128)
  call void @sigi_init_stack(ptr %1)
  %2 = call ptr @__main__(ptr %1)
  call void @sigi_free_stack(ptr %1)
  ret void
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
