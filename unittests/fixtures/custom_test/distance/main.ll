; ModuleID = 'main.c'
source_filename = "main.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@GlobalVariable = internal global i32 15, align 4
@.str = private unnamed_addr constant [19 x i8] c"no test specified\0A\00", align 1
@.str.1 = private unnamed_addr constant [38 x i8] c"wrong number of parameters specified\0A\00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"passing_test\00", align 1
@fail = external global i32, align 4
@.str.3 = private unnamed_addr constant [15 x i8] c"'%s' did fail\0A\00", align 1
@.str.4 = private unnamed_addr constant [13 x i8] c"failing_test\00", align 1
@.str.5 = private unnamed_addr constant [18 x i8] c"unknown test: %s\0A\00", align 1
@.str.6 = private unnamed_addr constant [14 x i8] c"did not fail\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @initGlobalVariable, i8* null }]

; Function Attrs: noinline nounwind ssp uwtable
define internal void @initGlobalVariable() #0 {
  store i32 42, i32* @GlobalVariable, align 4
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i8*, align 8
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  %9 = load i32, i32* %4, align 4
  %10 = icmp eq i32 %9, 1
  br i1 %10, label %11, label %13

; <label>:11:                                     ; preds = %2
  %12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i32 0, i32 0))
  store i32 48, i32* %3, align 4
  br label %56

; <label>:13:                                     ; preds = %2
  %14 = load i32, i32* %4, align 4
  %15 = icmp ne i32 %14, 2
  br i1 %15, label %16, label %18

; <label>:16:                                     ; preds = %13
  %17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str.1, i32 0, i32 0))
  store i32 93, i32* %3, align 4
  br label %56

; <label>:18:                                     ; preds = %13
  %19 = load i8**, i8*** %5, align 8
  %20 = getelementptr inbounds i8*, i8** %19, i64 1
  %21 = load i8*, i8** %20, align 8
  store i8* %21, i8** %6, align 8
  %22 = load i8*, i8** %6, align 8
  %23 = call i32 @strcmp(i8* %22, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.2, i32 0, i32 0))
  %24 = icmp eq i32 %23, 0
  br i1 %24, label %25, label %34

; <label>:25:                                     ; preds = %18
  %26 = call i32 (...) @passing_test()
  store i32 %26, i32* %7, align 4
  %27 = load i32, i32* %7, align 4
  %28 = load i32, i32* @fail, align 4
  %29 = icmp eq i32 %27, %28
  br i1 %29, label %30, label %33

; <label>:30:                                     ; preds = %25
  %31 = load i8*, i8** %6, align 8
  %32 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.3, i32 0, i32 0), i8* %31)
  store i32 112, i32* %3, align 4
  br label %56

; <label>:33:                                     ; preds = %25
  br label %54

; <label>:34:                                     ; preds = %18
  %35 = load i8*, i8** %6, align 8
  %36 = call i32 @strcmp(i8* %35, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.4, i32 0, i32 0))
  %37 = icmp eq i32 %36, 0
  br i1 %37, label %38, label %50

; <label>:38:                                     ; preds = %34
  %39 = load i32, i32* @GlobalVariable, align 4
  %40 = icmp eq i32 %39, 42
  br i1 %40, label %41, label %50

; <label>:41:                                     ; preds = %38
  %42 = call i32 (...) @failing_test()
  store i32 %42, i32* %8, align 4
  %43 = load i32, i32* %8, align 4
  %44 = load i32, i32* @fail, align 4
  %45 = icmp eq i32 %43, %44
  br i1 %45, label %46, label %49

; <label>:46:                                     ; preds = %41
  %47 = load i8*, i8** %6, align 8
  %48 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.3, i32 0, i32 0), i8* %47)
  store i32 112, i32* %3, align 4
  br label %56

; <label>:49:                                     ; preds = %41
  br label %53

; <label>:50:                                     ; preds = %38, %34
  %51 = load i8*, i8** %6, align 8
  %52 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.5, i32 0, i32 0), i8* %51)
  store i32 120, i32* %3, align 4
  br label %56

; <label>:53:                                     ; preds = %49
  br label %54

; <label>:54:                                     ; preds = %53, %33
  %55 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.6, i32 0, i32 0))
  store i32 0, i32* %3, align 4
  br label %56

; <label>:56:                                     ; preds = %54, %50, %46, %30, %16, %11
  %57 = load i32, i32* %3, align 4
  ret i32 %57
}

declare i32 @printf(i8*, ...) #1

declare i32 @strcmp(i8*, i8*) #1

declare i32 @passing_test(...) #1

declare i32 @failing_test(...) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 9.0.0 (clang-900.0.37)"}
