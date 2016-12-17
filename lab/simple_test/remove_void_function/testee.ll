; ModuleID = 'testee.c'
source_filename = "testee.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.11.0"

@.str = private unnamed_addr constant [25 x i8] c"Void function is called\0A\00", align 1
@global_state = internal global i32 0, align 4
@.str.1 = private unnamed_addr constant [32 x i8] c"Before void function is called\0A\00", align 1
@.str.2 = private unnamed_addr constant [31 x i8] c"After void function is called\0A\00", align 1

; Function Attrs: nounwind ssp uwtable
define void @void_function() #0 {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0))
  store i32 1, i32* @global_state, align 4
  ret void
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: nounwind ssp uwtable
define i32 @testee() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([32 x i8], [32 x i8]* @.str.1, i32 0, i32 0))
  call void @void_function()
  %3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.2, i32 0, i32 0))
  %4 = load i32, i32* @global_state, align 4
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %6, label %7

; <label>:6                                       ; preds = %0
  store i32 0, i32* %1, align 4
  br label %8

; <label>:7                                       ; preds = %0
  store i32 1, i32* %1, align 4
  br label %8

; <label>:8                                       ; preds = %7, %6
  %9 = load i32, i32* %1, align 4
  ret i32 %9
}

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 8.0.0 (clang-800.0.42.1)"}
