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
  br i1 %4, label %5, label %6

5:                                                ; preds = %1
  call void @sigi_push_i32(ptr %0, i32 %3)
  br label %9

6:                                                ; preds = %1
  call void @sigi_push_i32(ptr %0, i32 %2)
  %7 = srem i32 %3, %2
  call void @sigi_push_i32(ptr %0, i32 %7)
  %8 = call ptr @gcd(ptr %0)
  br label %9

9:                                                ; preds = %5, %6
  %10 = phi ptr [ %8, %6 ], [ %0, %5 ]
  br label %11

11:                                               ; preds = %9
  ret ptr %10
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
