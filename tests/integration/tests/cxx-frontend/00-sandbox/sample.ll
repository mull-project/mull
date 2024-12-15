; ModuleID = '/workspaces/mull/tests-lit/tests/cxx-frontend/00-sandbox/sample.cpp'
source_filename = "/workspaces/mull/tests-lit/tests/cxx-frontend/00-sandbox/sample.cpp"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-gnu"

@.str = private unnamed_addr constant [88 x i8] c"cxx_add_to_sub:/workspaces/mull/tests-lit/tests/cxx-frontend/00-sandbox/sample.cpp:6:12\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"NORMAL\0A\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"MUTATED\0A\00", align 1
@mull_mutation_5_6_12 = dso_local constant [93 x i8] c"cxx_add_to_sub:/workspaces/mull/tests-lit/tests/cxx-frontend/00-sandbox/sample.cpp:6:12:6:13\00", section ".mull_mutants", align 1

; Function Attrs: noinline optnone uwtable mustprogress
define dso_local i32 @_Z3sumii(i32 %0, i32 %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = call i8* @getenv(i8* getelementptr inbounds ([88 x i8], [88 x i8]* @.str, i64 0, i64 0))
  %6 = icmp ne i8* %5, null
  br i1 %6, label %7, label %11

7:                                                ; preds = %2
  %8 = load i32, i32* %3, align 4
  %9 = load i32, i32* %4, align 4
  %10 = sub nsw i32 %8, %9
  br label %15

11:                                               ; preds = %2
  %12 = load i32, i32* %3, align 4
  %13 = load i32, i32* %4, align 4
  %14 = add nsw i32 %12, %13
  br label %15

15:                                               ; preds = %11, %7
  %16 = phi i32 [ %10, %7 ], [ %14, %11 ]
  ret i32 %16
}

declare dso_local i8* @getenv(i8*) #1

; Function Attrs: noinline norecurse optnone uwtable mustprogress
define dso_local i32 @main() #2 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %2 = call i32 @_Z3sumii(i32 -2, i32 2)
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

4:                                                ; preds = %0
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0))
  store i32 0, i32* %1, align 4
  br label %8

6:                                                ; preds = %0
  %7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0))
  store i32 1, i32* %1, align 4
  br label %8

8:                                                ; preds = %6, %4
  %9 = load i32, i32* %1, align 4
  ret i32 %9
}

declare dso_local i32 @printf(i8*, ...) #1

attributes #0 = { noinline optnone uwtable mustprogress "disable-tail-calls"="false" "frame-pointer"="non-leaf" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "frame-pointer"="non-leaf" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline norecurse optnone uwtable mustprogress "disable-tail-calls"="false" "frame-pointer"="non-leaf" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"branch-target-enforcement", i32 0}
!2 = !{i32 1, !"sign-return-address", i32 0}
!3 = !{i32 1, !"sign-return-address-all", i32 0}
!4 = !{i32 1, !"sign-return-address-with-bkey", i32 0}
!5 = !{!"Ubuntu clang version 12.0.0-3ubuntu1~20.04.5"}
