; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

@sigi.global.stack = linkonce global ptr undef

declare void @sigi_free_stack(ptr)

declare void @sigi_init_stack(ptr)

declare ptr @malloc(i64)

declare i1 @sigi_pop_bool(ptr)

declare void @sigi_push_bool(ptr, i1)

declare i32 @sigi_pop_i32(ptr)

declare void @sigi_push_i32(ptr, i32)

declare void @sigi_builtin__pp_i1(i1)

declare void @sigi_builtin__pp_i32(i32)

declare ptr @sigi_builtin__pp(ptr)

define { i32, i1 } @pow_loop(i32 %0, i32 %1, i32 %2) {
  %4 = load ptr, ptr @sigi.global.stack, align 8
  call void @sigi_push_i32(ptr %4, i32 %0)
  call void @sigi_push_i32(ptr %4, i32 %1)
  call void @sigi_push_i32(ptr %4, i32 %2)
  %5 = call i32 @sigi_pop_i32(ptr %4)
  %6 = call i32 @sigi_pop_i32(ptr %4)
  %7 = call i32 @sigi_pop_i32(ptr %4)
  %8 = icmp sle i32 %6, 0
  br i1 %8, label %9, label %10

9:                                                ; preds = %3
  br label %20

10:                                               ; preds = %3
  call void @sigi_builtin__pp_i32(i32 %5)
  call void @sigi_push_i32(ptr %4, i32 %7)
  %11 = sub i32 %6, 1
  call void @sigi_push_i32(ptr %4, i32 %11)
  %12 = mul i32 %7, %5
  call void @sigi_push_i32(ptr %4, i32 %12)
  %13 = call i32 @sigi_pop_i32(ptr %4)
  %14 = call i32 @sigi_pop_i32(ptr %4)
  %15 = call i32 @sigi_pop_i32(ptr %4)
  store ptr %4, ptr @sigi.global.stack, align 8
  %16 = call { i32, i1 } @pow_loop(i32 %15, i32 %14, i32 %13)
  %17 = extractvalue { i32, i1 } %16, 0
  %18 = extractvalue { i32, i1 } %16, 1
  %19 = load ptr, ptr @sigi.global.stack, align 8
  br label %20

20:                                               ; preds = %9, %10
  %21 = phi i32 [ %17, %10 ], [ %5, %9 ]
  %22 = phi i1 [ %18, %10 ], [ true, %9 ]
  %23 = phi ptr [ %19, %10 ], [ %4, %9 ]
  br label %24

24:                                               ; preds = %20
  call void @sigi_push_i32(ptr %23, i32 %21)
  call void @sigi_push_bool(ptr %23, i1 %22)
  %25 = call i1 @sigi_pop_bool(ptr %23)
  %26 = call i32 @sigi_pop_i32(ptr %23)
  store ptr %23, ptr @sigi.global.stack, align 8
  %27 = insertvalue { i32, i1 } undef, i32 %26, 0
  %28 = insertvalue { i32, i1 } %27, i1 %25, 1
  ret { i32, i1 } %28
}

define void @__main__() {
  %1 = load ptr, ptr @sigi.global.stack, align 8
  call void @sigi_push_i32(ptr %1, i32 2)
  call void @sigi_push_i32(ptr %1, i32 6)
  call void @sigi_push_i32(ptr %1, i32 1)
  %2 = call i32 @sigi_pop_i32(ptr %1)
  %3 = call i32 @sigi_pop_i32(ptr %1)
  %4 = call i32 @sigi_pop_i32(ptr %1)
  store ptr %1, ptr @sigi.global.stack, align 8
  %5 = call { i32, i1 } @pow_loop(i32 %4, i32 %3, i32 %2)
  %6 = extractvalue { i32, i1 } %5, 0
  %7 = extractvalue { i32, i1 } %5, 1
  %8 = load ptr, ptr @sigi.global.stack, align 8
  call void @sigi_builtin__pp_i1(i1 %7)
  call void @sigi_builtin__pp_i32(i32 %6)
  store ptr %8, ptr @sigi.global.stack, align 8
  ret void
}

define i1 @main() {
  %1 = call ptr @malloc(i64 128)
  call void @sigi_init_stack(ptr %1)
  store ptr %1, ptr @sigi.global.stack, align 8
  call void @__main__()
  call void @sigi_free_stack(ptr %1)
  ret i1 false
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
