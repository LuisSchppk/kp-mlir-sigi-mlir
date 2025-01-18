; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

declare void @sigi_free_stack(ptr)

declare void @sigi_init_stack(ptr)

declare i1 @sigi_pop_bool(ptr)

define private ptr @closure_worker_1(i32 %0, i32 %1, ptr %2) {
  call void @sigi_push_i32(ptr %2, i32 %1)
  %4 = call ptr @show(ptr %2)
  call void @sigi_push_i32(ptr %4, i32 %0)
  call void @sigi_push_i32(ptr %4, i32 1)
  %5 = call i32 @sigi_pop_i32(ptr %4)
  %6 = call i32 @sigi_pop_i32(ptr %4)
  %7 = sub i32 %6, %5
  call void @sigi_push_i32(ptr %4, i32 %7)
  call void @sigi_push_i32(ptr %4, i32 %1)
  call void @sigi_push_i32(ptr %4, i32 10)
  %8 = call i32 @sigi_pop_i32(ptr %4)
  %9 = call i32 @sigi_pop_i32(ptr %4)
  %10 = mul i32 %9, %8
  call void @sigi_push_i32(ptr %4, i32 %10)
  %11 = call ptr @fibloop(ptr %4)
  ret ptr %11
}

define private ptr @closure_wrapper_1(ptr %0, ptr %1) {
  %3 = getelementptr { ptr, i32, ptr, { i32, i32 } }, ptr %0, i32 0, i32 3
  %4 = load { i32, i32 }, ptr %3, align 4
  %5 = extractvalue { i32, i32 } %4, 0
  %6 = extractvalue { i32, i32 } %4, 1
  %7 = call ptr @closure_worker_1(i32 %5, i32 %6, ptr %1)
  ret ptr %7
}

declare void @sigi_push_closure(ptr, ptr)

declare ptr @malloc(i64)

define private void @closure_drop_nothing(ptr %0) {
  ret void
}

define private ptr @closure_worker_0(ptr %0) {
  ret ptr %0
}

define private ptr @closure_wrapper_0(ptr %0, ptr %1) {
  %3 = getelementptr { ptr, i32, ptr, {} }, ptr %0, i32 0, i32 3
  %4 = call ptr @closure_worker_0(ptr %1)
  ret ptr %4
}

declare void @closure_decr_then_drop(ptr)

declare void @sigi_push_bool(ptr, i1)

declare void @sigi_push_i32(ptr, i32)

declare i32 @sigi_pop_i32(ptr)

declare ptr @sigi_pop_closure(ptr)

declare ptr @sigi_builtin__pp(ptr)

declare ptr @sigi_builtin_pp(ptr)

define ptr @apply(ptr %0) {
  %2 = call ptr @sigi_pop_closure(ptr %0)
  %3 = load ptr, ptr %2, align 8
  %4 = call ptr %3(ptr %2, ptr %0)
  ret ptr %4
}

define ptr @show(ptr %0) {
  %2 = call ptr @sigi_builtin__pp(ptr %0)
  %3 = call i32 @sigi_pop_i32(ptr %2)
  ret ptr %2
}

define ptr @fibloop(ptr %0) {
  %2 = call i32 @sigi_pop_i32(ptr %0)
  %3 = call i32 @sigi_pop_i32(ptr %0)
  call void @sigi_push_i32(ptr %0, i32 %3)
  call void @sigi_push_i32(ptr %0, i32 0)
  %4 = call i32 @sigi_pop_i32(ptr %0)
  %5 = call i32 @sigi_pop_i32(ptr %0)
  %6 = icmp eq i32 %5, %4
  call void @sigi_push_bool(ptr %0, i1 %6)
  %7 = call ptr @malloc(i64 undef)
  store { ptr, i32, ptr, {} } { ptr @closure_wrapper_0, i32 0, ptr @closure_drop_nothing, {} undef }, ptr %7, align 8
  call void @sigi_push_closure(ptr %0, ptr %7)
  %8 = call ptr @malloc(i64 undef)
  %9 = insertvalue { ptr, i32, ptr, { i32, i32 } } { ptr @closure_wrapper_1, i32 0, ptr @closure_drop_nothing, { i32, i32 } undef }, i32 %3, 3, 0
  %10 = insertvalue { ptr, i32, ptr, { i32, i32 } } %9, i32 %2, 3, 1
  store { ptr, i32, ptr, { i32, i32 } } %10, ptr %8, align 8
  call void @sigi_push_closure(ptr %0, ptr %8)
  %11 = call ptr @sigi_pop_closure(ptr %0)
  %12 = call ptr @sigi_pop_closure(ptr %0)
  %13 = call i1 @sigi_pop_bool(ptr %0)
  br i1 %13, label %14, label %15

14:                                               ; preds = %1
  br label %16

15:                                               ; preds = %1
  br label %16

16:                                               ; preds = %14, %15
  %17 = phi ptr [ %11, %15 ], [ %12, %14 ]
  br label %18

18:                                               ; preds = %16
  call void @sigi_push_closure(ptr %0, ptr %17)
  %19 = call ptr @apply(ptr %0)
  ret ptr %19
}

define ptr @__main__(ptr %0) {
  call void @sigi_push_i32(ptr %0, i32 10)
  call void @sigi_push_i32(ptr %0, i32 1)
  %2 = call ptr @fibloop(ptr %0)
  ret ptr %2
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
