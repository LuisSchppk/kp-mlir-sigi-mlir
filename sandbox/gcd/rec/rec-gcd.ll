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

5:                                                ; preds = %10, %1
  %6 = phi ptr [ %11, %10 ], [ %0, %1 ]
  %7 = phi i32 [ %16, %10 ], [ %2, %1 ]
  %8 = phi i32 [ %17, %10 ], [ %3, %1 ]
  %9 = phi i1 [ %18, %10 ], [ %4, %1 ]
  br i1 %9, label %10, label %19

10:                                               ; preds = %5
  %11 = phi ptr [ %6, %5 ]
  %12 = phi i32 [ %7, %5 ]
  %13 = phi i32 [ %8, %5 ]
  %14 = phi i1 [ %9, %5 ]
  call void @sigi_push_i32(ptr %11, i32 %12)
  %15 = srem i32 %13, %12
  call void @sigi_push_i32(ptr %11, i32 %15)
  %16 = call i32 @sigi_pop_i32(ptr %11)
  %17 = call i32 @sigi_pop_i32(ptr %11)
  %18 = icmp eq i32 %16, 0
  br label %5

19:                                               ; preds = %5
  call void @sigi_push_i32(ptr %6, i32 %8)
  ret ptr %6
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
