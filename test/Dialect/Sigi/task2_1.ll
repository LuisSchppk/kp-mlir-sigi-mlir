; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

declare void @sigi_push_i32(ptr, i32)

declare i32 @sigi_pop_i32(ptr)

define ptr @simpleSigi(ptr %0) {
  %2 = call i32 @sigi_pop_i32(ptr %0)
  %3 = call i32 @sigi_pop_i32(ptr %0)
  call void @sigi_push_i32(ptr %0, i32 %3)
  call void @sigi_push_i32(ptr %0, i32 %2)
  %4 = call i32 @sigi_pop_i32(ptr %0)
  %5 = call i32 @sigi_pop_i32(ptr %0)
  %6 = mul i32 %5, %4
  call void @sigi_push_i32(ptr %0, i32 %6)
  call void @sigi_push_i32(ptr %0, i32 1)
  %7 = call i32 @sigi_pop_i32(ptr %0)
  %8 = call i32 @sigi_pop_i32(ptr %0)
  %9 = add i32 %8, %7
  call void @sigi_push_i32(ptr %0, i32 %9)
  ret ptr %0
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
