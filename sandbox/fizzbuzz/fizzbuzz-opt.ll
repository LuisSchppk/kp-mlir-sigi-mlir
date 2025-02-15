; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

@sigi.global.stack = linkonce global ptr undef

declare void @sigi_free_stack(ptr)

declare void @sigi_init_stack(ptr)

declare ptr @malloc(i64)

declare i32 @sigi_pop_i32(ptr)

declare void @sigi_push_i32(ptr, i32)

declare void @sigi_builtin__pp_i32(i32)

declare ptr @sigi_builtin__pp(ptr)

define void @fizzbuzz(i32 %0, i32 %1) {
  %3 = load ptr, ptr @sigi.global.stack, align 8
  call void @sigi_push_i32(ptr %3, i32 %1)
  call void @sigi_push_i32(ptr %3, i32 %0)
  %4 = call i32 @sigi_pop_i32(ptr %3)
  %5 = call i32 @sigi_pop_i32(ptr %3)
  %6 = icmp sle i32 %5, %4
  br i1 %6, label %7, label %45

7:                                                ; preds = %2
  %8 = urem i32 %5, 2
  %9 = icmp eq i32 %8, 0
  br i1 %9, label %10, label %26

10:                                               ; preds = %7
  %11 = urem i32 %5, 5
  %12 = icmp eq i32 %11, 0
  br i1 %12, label %13, label %18

13:                                               ; preds = %10
  call void @sigi_builtin__pp_i32(i32 -3)
  %14 = add i32 %5, 1
  call void @sigi_push_i32(ptr %3, i32 %14)
  call void @sigi_push_i32(ptr %3, i32 %4)
  %15 = call i32 @sigi_pop_i32(ptr %3)
  %16 = call i32 @sigi_pop_i32(ptr %3)
  store ptr %3, ptr @sigi.global.stack, align 8
  call void @fizzbuzz(i32 %15, i32 %16)
  %17 = load ptr, ptr @sigi.global.stack, align 8
  br label %23

18:                                               ; preds = %10
  call void @sigi_builtin__pp_i32(i32 -1)
  %19 = add i32 %5, 1
  call void @sigi_push_i32(ptr %3, i32 %19)
  call void @sigi_push_i32(ptr %3, i32 %4)
  %20 = call i32 @sigi_pop_i32(ptr %3)
  %21 = call i32 @sigi_pop_i32(ptr %3)
  store ptr %3, ptr @sigi.global.stack, align 8
  call void @fizzbuzz(i32 %20, i32 %21)
  %22 = load ptr, ptr @sigi.global.stack, align 8
  br label %23

23:                                               ; preds = %13, %18
  %24 = phi ptr [ %22, %18 ], [ %17, %13 ]
  br label %25

25:                                               ; preds = %23
  br label %42

26:                                               ; preds = %7
  %27 = urem i32 %5, 5
  %28 = icmp eq i32 %27, 0
  br i1 %28, label %29, label %34

29:                                               ; preds = %26
  call void @sigi_builtin__pp_i32(i32 -2)
  %30 = add i32 %5, 1
  call void @sigi_push_i32(ptr %3, i32 %30)
  call void @sigi_push_i32(ptr %3, i32 %4)
  %31 = call i32 @sigi_pop_i32(ptr %3)
  %32 = call i32 @sigi_pop_i32(ptr %3)
  store ptr %3, ptr @sigi.global.stack, align 8
  call void @fizzbuzz(i32 %31, i32 %32)
  %33 = load ptr, ptr @sigi.global.stack, align 8
  br label %39

34:                                               ; preds = %26
  call void @sigi_builtin__pp_i32(i32 %5)
  %35 = add i32 %5, 1
  call void @sigi_push_i32(ptr %3, i32 %35)
  call void @sigi_push_i32(ptr %3, i32 %4)
  %36 = call i32 @sigi_pop_i32(ptr %3)
  %37 = call i32 @sigi_pop_i32(ptr %3)
  store ptr %3, ptr @sigi.global.stack, align 8
  call void @fizzbuzz(i32 %36, i32 %37)
  %38 = load ptr, ptr @sigi.global.stack, align 8
  br label %39

39:                                               ; preds = %29, %34
  %40 = phi ptr [ %38, %34 ], [ %33, %29 ]
  br label %41

41:                                               ; preds = %39
  br label %42

42:                                               ; preds = %25, %41
  %43 = phi ptr [ %40, %41 ], [ %24, %25 ]
  br label %44

44:                                               ; preds = %42
  br label %46

45:                                               ; preds = %2
  br label %46

46:                                               ; preds = %44, %45
  %47 = phi ptr [ %3, %45 ], [ %43, %44 ]
  br label %48

48:                                               ; preds = %46
  store ptr %47, ptr @sigi.global.stack, align 8
  ret void
}

define void @__main__() {
  %1 = load ptr, ptr @sigi.global.stack, align 8
  call void @sigi_push_i32(ptr %1, i32 0)
  call void @sigi_push_i32(ptr %1, i32 100)
  %2 = call i32 @sigi_pop_i32(ptr %1)
  %3 = call i32 @sigi_pop_i32(ptr %1)
  store ptr %1, ptr @sigi.global.stack, align 8
  call void @fizzbuzz(i32 %2, i32 %3)
  %4 = load ptr, ptr @sigi.global.stack, align 8
  store ptr %4, ptr @sigi.global.stack, align 8
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
