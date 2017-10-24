; ModuleID = 'distance.c'
source_filename = "distance.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; Function Attrs: noinline nounwind ssp uwtable
define i32 @min(i32, i32) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, i32* %4, align 4
  store i32 %1, i32* %5, align 4
  %6 = load i32, i32* %4, align 4
  %7 = load i32, i32* %5, align 4
  %8 = icmp slt i32 %6, %7
  br i1 %8, label %9, label %11

; <label>:9:                                      ; preds = %2
  %10 = load i32, i32* %4, align 4
  store i32 %10, i32* %3, align 4
  br label %13

; <label>:11:                                     ; preds = %2
  %12 = load i32, i32* %5, align 4
  store i32 %12, i32* %3, align 4
  br label %13

; <label>:13:                                     ; preds = %11, %9
  %14 = load i32, i32* %3, align 4
  ret i32 %14
}

; Function Attrs: noinline nounwind ssp uwtable
define i32 @minimum(i32, i32, i32) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, i32* %4, align 4
  store i32 %1, i32* %5, align 4
  store i32 %2, i32* %6, align 4
  %8 = load i32, i32* %4, align 4
  %9 = load i32, i32* %5, align 4
  %10 = call i32 @min(i32 %8, i32 %9)
  store i32 %10, i32* %7, align 4
  %11 = load i32, i32* %7, align 4
  %12 = load i32, i32* %6, align 4
  %13 = call i32 @min(i32 %11, i32 %12)
  ret i32 %13
}

; Function Attrs: noinline nounwind ssp uwtable
define i32 @LevenshteinDistance(i8*, i32, i8*, i32) #0 {
  %5 = alloca i32, align 4
  %6 = alloca i8*, align 8
  %7 = alloca i32, align 4
  %8 = alloca i8*, align 8
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  store i8* %0, i8** %6, align 8
  store i32 %1, i32* %7, align 4
  store i8* %2, i8** %8, align 8
  store i32 %3, i32* %9, align 4
  %11 = load i32, i32* %7, align 4
  %12 = icmp eq i32 %11, 0
  br i1 %12, label %13, label %15

; <label>:13:                                     ; preds = %4
  %14 = load i32, i32* %9, align 4
  store i32 %14, i32* %5, align 4
  br label %63

; <label>:15:                                     ; preds = %4
  %16 = load i32, i32* %9, align 4
  %17 = icmp eq i32 %16, 0
  br i1 %17, label %18, label %20

; <label>:18:                                     ; preds = %15
  %19 = load i32, i32* %7, align 4
  store i32 %19, i32* %5, align 4
  br label %63

; <label>:20:                                     ; preds = %15
  %21 = load i8*, i8** %6, align 8
  %22 = load i32, i32* %7, align 4
  %23 = sub nsw i32 %22, 1
  %24 = sext i32 %23 to i64
  %25 = getelementptr inbounds i8, i8* %21, i64 %24
  %26 = load i8, i8* %25, align 1
  %27 = sext i8 %26 to i32
  %28 = load i8*, i8** %8, align 8
  %29 = load i32, i32* %9, align 4
  %30 = sub nsw i32 %29, 1
  %31 = sext i32 %30 to i64
  %32 = getelementptr inbounds i8, i8* %28, i64 %31
  %33 = load i8, i8* %32, align 1
  %34 = sext i8 %33 to i32
  %35 = icmp eq i32 %27, %34
  br i1 %35, label %36, label %37

; <label>:36:                                     ; preds = %20
  store i32 0, i32* %10, align 4
  br label %38

; <label>:37:                                     ; preds = %20
  store i32 1, i32* %10, align 4
  br label %38

; <label>:38:                                     ; preds = %37, %36
  %39 = load i8*, i8** %6, align 8
  %40 = load i32, i32* %7, align 4
  %41 = sub nsw i32 %40, 1
  %42 = load i8*, i8** %8, align 8
  %43 = load i32, i32* %9, align 4
  %44 = call i32 @LevenshteinDistance(i8* %39, i32 %41, i8* %42, i32 %43)
  %45 = add nsw i32 %44, 1
  %46 = load i8*, i8** %6, align 8
  %47 = load i32, i32* %7, align 4
  %48 = load i8*, i8** %8, align 8
  %49 = load i32, i32* %9, align 4
  %50 = sub nsw i32 %49, 1
  %51 = call i32 @LevenshteinDistance(i8* %46, i32 %47, i8* %48, i32 %50)
  %52 = add nsw i32 %51, 1
  %53 = load i8*, i8** %6, align 8
  %54 = load i32, i32* %7, align 4
  %55 = sub nsw i32 %54, 1
  %56 = load i8*, i8** %8, align 8
  %57 = load i32, i32* %9, align 4
  %58 = sub nsw i32 %57, 1
  %59 = call i32 @LevenshteinDistance(i8* %53, i32 %55, i8* %56, i32 %58)
  %60 = load i32, i32* %10, align 4
  %61 = add nsw i32 %59, %60
  %62 = call i32 @minimum(i32 %45, i32 %52, i32 %61)
  store i32 %62, i32* %5, align 4
  br label %63

; <label>:63:                                     ; preds = %38, %18, %13
  %64 = load i32, i32* %5, align 4
  ret i32 %64
}

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 9.0.0 (clang-900.0.37)"}
