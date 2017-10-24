; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@fail = constant i32 1, align 4
@success = constant i32 2, align 4
@.str = private unnamed_addr constant [9 x i8] c"industry\00", align 1
@.str.1 = private unnamed_addr constant [9 x i8] c"interest\00", align 1
@.str.2 = private unnamed_addr constant [34 x i8] c"test failed: expected %d, got %d\0A\00", align 1
@.str.3 = private unnamed_addr constant [6 x i8] c"horse\00", align 1
@.str.4 = private unnamed_addr constant [4 x i8] c"ros\00", align 1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @passing_test() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %2, align 4
  store i32 0, i32* %3, align 4
  %4 = call i32 @LevenshteinDistance(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 9, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 9)
  store i32 %4, i32* %2, align 4
  store i32 6, i32* %3, align 4
  %5 = load i32, i32* %2, align 4
  %6 = load i32, i32* %3, align 4
  %7 = icmp ne i32 %5, %6
  br i1 %7, label %8, label %12

; <label>:8:                                      ; preds = %0
  %9 = load i32, i32* %3, align 4
  %10 = load i32, i32* %2, align 4
  %11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.2, i32 0, i32 0), i32 %9, i32 %10)
  store i32 1, i32* %1, align 4
  br label %22

; <label>:12:                                     ; preds = %0
  %13 = call i32 @LevenshteinDistance(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.3, i32 0, i32 0), i32 6, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.4, i32 0, i32 0), i32 4)
  store i32 %13, i32* %2, align 4
  store i32 3, i32* %3, align 4
  %14 = load i32, i32* %2, align 4
  %15 = load i32, i32* %3, align 4
  %16 = icmp ne i32 %14, %15
  br i1 %16, label %17, label %21

; <label>:17:                                     ; preds = %12
  %18 = load i32, i32* %3, align 4
  %19 = load i32, i32* %2, align 4
  %20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.2, i32 0, i32 0), i32 %18, i32 %19)
  store i32 1, i32* %1, align 4
  br label %22

; <label>:21:                                     ; preds = %12
  store i32 2, i32* %1, align 4
  br label %22

; <label>:22:                                     ; preds = %21, %17, %8
  %23 = load i32, i32* %1, align 4
  ret i32 %23
}

declare i32 @LevenshteinDistance(i8*, i32, i8*, i32) #1

declare i32 @printf(i8*, ...) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @failing_test() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %2, align 4
  store i32 0, i32* %3, align 4
  %4 = call i32 @LevenshteinDistance(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 9, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 9)
  store i32 %4, i32* %2, align 4
  store i32 7, i32* %3, align 4
  %5 = load i32, i32* %2, align 4
  %6 = load i32, i32* %3, align 4
  %7 = icmp ne i32 %5, %6
  br i1 %7, label %8, label %12

; <label>:8:                                      ; preds = %0
  %9 = load i32, i32* %3, align 4
  %10 = load i32, i32* %2, align 4
  %11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.2, i32 0, i32 0), i32 %9, i32 %10)
  store i32 1, i32* %1, align 4
  br label %13

; <label>:12:                                     ; preds = %0
  store i32 2, i32* %1, align 4
  br label %13

; <label>:13:                                     ; preds = %12, %8
  %14 = load i32, i32* %1, align 4
  ret i32 %14
}

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 9.0.0 (clang-900.0.37)"}
