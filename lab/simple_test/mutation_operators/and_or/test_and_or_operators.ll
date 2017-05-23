; ModuleID = 'test_and_or_operators.c'
source_filename = "test_and_or_operators.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@.str = private unnamed_addr constant [13 x i8] c"left branch\0A\00", align 1
@.str.1 = private unnamed_addr constant [14 x i8] c"right branch\0A\00", align 1

; Function Attrs: nounwind ssp uwtable
define i32 @testee_AND_operator_2branches(i32, i32, i32) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, i32* %5, align 4
  store i32 %1, i32* %6, align 4
  store i32 %2, i32* %7, align 4
  %8 = load i32, i32* %5, align 4
  %9 = load i32, i32* %6, align 4
  %10 = icmp slt i32 %8, %9
  br i1 %10, label %11, label %18

; <label>:11:                                     ; preds = %3
  %12 = load i32, i32* %6, align 4
  %13 = load i32, i32* %7, align 4
  %14 = icmp slt i32 %12, %13
  br i1 %14, label %15, label %18

; <label>:15:                                     ; preds = %11
  %16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0))
  %17 = load i32, i32* %5, align 4
  store i32 %17, i32* %4, align 4
  br label %21

; <label>:18:                                     ; preds = %11, %3
  %19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i32 0, i32 0))
  %20 = load i32, i32* %6, align 4
  store i32 %20, i32* %4, align 4
  br label %21

; <label>:21:                                     ; preds = %18, %15
  %22 = load i32, i32* %4, align 4
  ret i32 %22
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: nounwind ssp uwtable
define i32 @testee_AND_operator_1branch(i32, i32, i32) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, i32* %5, align 4
  store i32 %1, i32* %6, align 4
  store i32 %2, i32* %7, align 4
  %8 = load i32, i32* %5, align 4
  %9 = load i32, i32* %6, align 4
  %10 = icmp slt i32 %8, %9
  br i1 %10, label %11, label %18

; <label>:11:                                     ; preds = %3
  %12 = load i32, i32* %6, align 4
  %13 = load i32, i32* %7, align 4
  %14 = icmp slt i32 %12, %13
  br i1 %14, label %15, label %18

; <label>:15:                                     ; preds = %11
  %16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0))
  %17 = load i32, i32* %5, align 4
  store i32 %17, i32* %4, align 4
  br label %21

; <label>:18:                                     ; preds = %11, %3
  %19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i32 0, i32 0))
  %20 = load i32, i32* %6, align 4
  store i32 %20, i32* %4, align 4
  br label %21

; <label>:21:                                     ; preds = %18, %15
  %22 = load i32, i32* %4, align 4
  ret i32 %22
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee_OR_operator_2branches(i32, i32, i32) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, i32* %5, align 4
  store i32 %1, i32* %6, align 4
  store i32 %2, i32* %7, align 4
  %8 = load i32, i32* %5, align 4
  %9 = load i32, i32* %6, align 4
  %10 = icmp slt i32 %8, %9
  br i1 %10, label %15, label %11

; <label>:11:                                     ; preds = %3
  %12 = load i32, i32* %6, align 4
  %13 = load i32, i32* %7, align 4
  %14 = icmp slt i32 %12, %13
  br i1 %14, label %15, label %18

; <label>:15:                                     ; preds = %11, %3
  %16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0))
  %17 = load i32, i32* %5, align 4
  store i32 %17, i32* %4, align 4
  br label %21

; <label>:18:                                     ; preds = %11
  %19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i32 0, i32 0))
  %20 = load i32, i32* %6, align 4
  store i32 %20, i32* %4, align 4
  br label %21

; <label>:21:                                     ; preds = %18, %15
  %22 = load i32, i32* %4, align 4
  ret i32 %22
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee_OR_operator_1branch(i32, i32, i32) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, i32* %5, align 4
  store i32 %1, i32* %6, align 4
  store i32 %2, i32* %7, align 4
  %8 = load i32, i32* %5, align 4
  %9 = load i32, i32* %6, align 4
  %10 = icmp slt i32 %8, %9
  br i1 %10, label %15, label %11

; <label>:11:                                     ; preds = %3
  %12 = load i32, i32* %6, align 4
  %13 = load i32, i32* %7, align 4
  %14 = icmp slt i32 %12, %13
  br i1 %14, label %15, label %18

; <label>:15:                                     ; preds = %11, %3
  %16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0))
  %17 = load i32, i32* %5, align 4
  store i32 %17, i32* %4, align 4
  br label %21

; <label>:18:                                     ; preds = %11
  %19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i32 0, i32 0))
  %20 = load i32, i32* %6, align 4
  store i32 %20, i32* %4, align 4
  br label %21

; <label>:21:                                     ; preds = %18, %15
  %22 = load i32, i32* %4, align 4
  ret i32 %22
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee_compound_AND_then_OR_operator(i32, i32, i32) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, i32* %5, align 4
  store i32 %1, i32* %6, align 4
  store i32 %2, i32* %7, align 4
  %8 = load i32, i32* %5, align 4
  %9 = load i32, i32* %6, align 4
  %10 = icmp slt i32 %8, %9
  br i1 %10, label %11, label %22

; <label>:11:                                     ; preds = %3
  %12 = load i32, i32* %6, align 4
  %13 = load i32, i32* %7, align 4
  %14 = icmp slt i32 %12, %13
  br i1 %14, label %19, label %15

; <label>:15:                                     ; preds = %11
  %16 = load i32, i32* %5, align 4
  %17 = load i32, i32* %7, align 4
  %18 = icmp slt i32 %16, %17
  br i1 %18, label %19, label %22

; <label>:19:                                     ; preds = %15, %11
  %20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0))
  %21 = load i32, i32* %5, align 4
  store i32 %21, i32* %4, align 4
  br label %25

; <label>:22:                                     ; preds = %15, %3
  %23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i32 0, i32 0))
  %24 = load i32, i32* %6, align 4
  store i32 %24, i32* %4, align 4
  br label %25

; <label>:25:                                     ; preds = %22, %19
  %26 = load i32, i32* %4, align 4
  ret i32 %26
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee_compound_OR_then_AND_operator(i32, i32, i32) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, i32* %5, align 4
  store i32 %1, i32* %6, align 4
  store i32 %2, i32* %7, align 4
  %8 = load i32, i32* %5, align 4
  %9 = load i32, i32* %6, align 4
  %10 = icmp slt i32 %8, %9
  br i1 %10, label %19, label %11

; <label>:11:                                     ; preds = %3
  %12 = load i32, i32* %6, align 4
  %13 = load i32, i32* %7, align 4
  %14 = icmp slt i32 %12, %13
  br i1 %14, label %15, label %22

; <label>:15:                                     ; preds = %11
  %16 = load i32, i32* %5, align 4
  %17 = load i32, i32* %7, align 4
  %18 = icmp slt i32 %16, %17
  br i1 %18, label %19, label %22

; <label>:19:                                     ; preds = %15, %3
  %20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0))
  %21 = load i32, i32* %5, align 4
  store i32 %21, i32* %4, align 4
  br label %25

; <label>:22:                                     ; preds = %15, %11
  %23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i32 0, i32 0))
  %24 = load i32, i32* %6, align 4
  store i32 %24, i32* %4, align 4
  br label %25

; <label>:25:                                     ; preds = %22, %19
  %26 = load i32, i32* %4, align 4
  ret i32 %26
}

; Function Attrs: nounwind ssp uwtable
define i32 @test_AND_operator_2branches() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 @testee_AND_operator_2branches(i32 1, i32 3, i32 2)
  %3 = icmp eq i32 %2, 3
  %4 = zext i1 %3 to i32
  store i32 %4, i32* %1, align 4
  %5 = load i32, i32* %1, align 4
  ret i32 %5
}

; Function Attrs: nounwind ssp uwtable
define i32 @test_AND_operator_1branch() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 @testee_AND_operator_1branch(i32 1, i32 3, i32 2)
  %3 = icmp eq i32 %2, 3
  %4 = zext i1 %3 to i32
  store i32 %4, i32* %1, align 4
  %5 = load i32, i32* %1, align 4
  ret i32 %5
}

; Function Attrs: nounwind ssp uwtable
define i32 @test_OR_operator_2branches() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 @testee_OR_operator_2branches(i32 1, i32 3, i32 2)
  %3 = icmp eq i32 %2, 1
  %4 = zext i1 %3 to i32
  store i32 %4, i32* %1, align 4
  %5 = load i32, i32* %1, align 4
  ret i32 %5
}

; Function Attrs: nounwind ssp uwtable
define i32 @test_OR_operator_1branch() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 @testee_OR_operator_1branch(i32 1, i32 3, i32 2)
  %3 = icmp eq i32 %2, 1
  %4 = zext i1 %3 to i32
  store i32 %4, i32* %1, align 4
  %5 = load i32, i32* %1, align 4
  ret i32 %5
}

; Function Attrs: nounwind ssp uwtable
define i32 @test_compound_AND_then_OR_operator() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 @testee_compound_AND_then_OR_operator(i32 1, i32 3, i32 2)
  %3 = icmp eq i32 %2, 1
  %4 = zext i1 %3 to i32
  store i32 %4, i32* %1, align 4
  %5 = load i32, i32* %1, align 4
  ret i32 %5
}

; Function Attrs: nounwind ssp uwtable
define i32 @test_compound_OR_then_AND_operator() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 @testee_compound_OR_then_AND_operator(i32 1, i32 3, i32 2)
  %3 = icmp eq i32 %2, 1
  %4 = zext i1 %3 to i32
  store i32 %4, i32* %1, align 4
  %5 = load i32, i32* %1, align 4
  ret i32 %5
}

attributes #0 = { nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 8.1.0 (clang-802.0.42)"}
