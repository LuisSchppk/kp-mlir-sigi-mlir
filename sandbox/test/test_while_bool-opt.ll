; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

@sigi.global.stack = linkonce global ptr undef

declare void @sigi_free_stack(ptr)

declare void @sigi_init_stack(ptr)

define private ptr @closure_worker_1(ptr %0) {
  %2 = call i32 @sigi_pop_i32(ptr %0)
  %3 = call i1 @sigi_pop_bool(ptr %0)
  call void @sigi_builtin__pp_i1(i1 %3)
  call void @sigi_push_i32(ptr %0, i32 %2)
  ret ptr %0
}

define private ptr @closure_wrapper_1(ptr %0, ptr %1) {
  %3 = getelementptr { ptr, i32, ptr, {} }, ptr %0, i32 0, i32 3
  %4 = call ptr @closure_worker_1(ptr %1)
  ret ptr %4
}

declare ptr @malloc(i64)

define private void @closure_drop_nothing(ptr %0) {
  ret void
}

define private ptr @closure_worker_0(ptr %0) {
  %2 = call i32 @sigi_pop_i32(ptr %0)
  %3 = sub i32 %2, 1
  call void @sigi_push_i32(ptr %0, i32 %3)
  %4 = icmp sgt i32 %2, 0
  call void @sigi_push_bool(ptr %0, i1 %4)
  ret ptr %0
}

define private ptr @closure_wrapper_0(ptr %0, ptr %1) {
  %3 = getelementptr { ptr, i32, ptr, {} }, ptr %0, i32 0, i32 3
  %4 = call ptr @closure_worker_0(ptr %1)
  ret ptr %4
}

declare void @closure_decr_then_drop(ptr)

declare i32 @sigi_pop_i32(ptr)

declare i1 @sigi_pop_bool(ptr)

declare void @sigi_push_i32(ptr, i32)

declare void @sigi_push_bool(ptr, i1)

declare void @sigi_builtin__pp_i32(i32)

declare void @sigi_builtin__pp_i1(i1)

declare ptr @sigi_builtin__pp(ptr)

define i32 @while(i1 %0, i32 %1, ptr %2, ptr %3) {
  %5 = load ptr, ptr @sigi.global.stack, align 8
  call void @sigi_push_bool(ptr %5, i1 %0)
  call void @sigi_push_i32(ptr %5, i32 %1)
  %6 = load ptr, ptr %2, align 8
  %7 = call ptr %6(ptr %2, ptr %5)
  %8 = call i1 @sigi_pop_bool(ptr %7)
  %9 = load ptr, ptr %3, align 8
  %10 = call ptr %9(ptr %3, ptr %7)
  br i1 %8, label %11, label %16

11:                                               ; preds = %4
  %12 = call i32 @sigi_pop_i32(ptr %10)
  %13 = call i1 @sigi_pop_bool(ptr %10)
  store ptr %10, ptr @sigi.global.stack, align 8
  %14 = call i32 @while(i1 %13, i32 %12, ptr %2, ptr %3)
  %15 = load ptr, ptr @sigi.global.stack, align 8
  call void @sigi_push_i32(ptr %15, i32 %14)
  br label %17

16:                                               ; preds = %4
  br label %17

17:                                               ; preds = %11, %16
  %18 = phi ptr [ %10, %16 ], [ %15, %11 ]
  br label %19

19:                                               ; preds = %17
  %20 = call i32 @sigi_pop_i32(ptr %18)
  store ptr %18, ptr @sigi.global.stack, align 8
  ret i32 %20
}

define i32 @print_stack_and_pop(i1 %0, i32 %1) {
  %3 = call ptr @malloc(i64 undef)
  store { ptr, i32, ptr, {} } { ptr @closure_wrapper_0, i32 0, ptr @closure_drop_nothing, {} undef }, ptr %3, align 8
  %4 = call ptr @malloc(i64 undef)
  store { ptr, i32, ptr, {} } { ptr @closure_wrapper_1, i32 0, ptr @closure_drop_nothing, {} undef }, ptr %4, align 8
  %5 = call i32 @while(i1 %0, i32 %1, ptr %3, ptr %4)
  ret i32 %5
}

define { i32, i1 } @pow_loop(i32 %0, i32 %1, i32 %2) {
  %4 = icmp sle i32 %1, 0
  br i1 %4, label %5, label %6

5:                                                ; preds = %3
  br label %12

6:                                                ; preds = %3
  call void @sigi_builtin__pp_i32(i32 %2)
  %7 = sub i32 %1, 1
  %8 = mul i32 %0, %2
  %9 = call { i32, i1 } @pow_loop(i32 %0, i32 %7, i32 %8)
  %10 = extractvalue { i32, i1 } %9, 0
  %11 = extractvalue { i32, i1 } %9, 1
  br label %12

12:                                               ; preds = %5, %6
  %13 = phi i32 [ %10, %6 ], [ %2, %5 ]
  %14 = phi i1 [ %11, %6 ], [ true, %5 ]
  br label %15

15:                                               ; preds = %12
  %16 = insertvalue { i32, i1 } undef, i32 %13, 0
  %17 = insertvalue { i32, i1 } %16, i1 %14, 1
  ret { i32, i1 } %17
}

define { i32, i32 } @__main__() {
  %1 = call { i32, i1 } @pow_loop(i32 2, i32 6, i32 1)
  %2 = extractvalue { i32, i1 } %1, 0
  %3 = extractvalue { i32, i1 } %1, 1
  %4 = load ptr, ptr @sigi.global.stack, align 8
  call void @sigi_push_i32(ptr %4, i32 %2)
  store ptr %4, ptr @sigi.global.stack, align 8
  %5 = call i32 @print_stack_and_pop(i1 %3, i32 2)
  %6 = load ptr, ptr @sigi.global.stack, align 8
  %7 = call i32 @sigi_pop_i32(ptr %6)
  store ptr %6, ptr @sigi.global.stack, align 8
  %8 = insertvalue { i32, i32 } undef, i32 %7, 0
  %9 = insertvalue { i32, i32 } %8, i32 %5, 1
  ret { i32, i32 } %9
}

define i1 @main() {
  %1 = call ptr @malloc(i64 128)
  call void @sigi_init_stack(ptr %1)
  store ptr %1, ptr @sigi.global.stack, align 8
  %2 = call { i32, i32 } @__main__()
  call void @sigi_free_stack(ptr %1)
  ret i1 false
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
