; ModuleID = 'tester.c'
source_filename = "tester.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.11.0"

@.str = private unnamed_addr constant [49 x i8] c"test_testee_path_calculation returns result: %d\0A\00", align 1

; Function Attrs: nounwind ssp uwtable
define i32 @test_testee_path_calculation() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (...) @testee1()
  store i32 %2, i32* %1, align 4
  %3 = load i32, i32* %1, align 4
  %4 = icmp sgt i32 %3, 1
  %5 = zext i1 %4 to i32
  %6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str, i32 0, i32 0), i32 %5)
  %7 = load i32, i32* %1, align 4
  %8 = icmp sgt i32 %7, 1
  %9 = zext i1 %8 to i32
  ret i32 %9
}

declare i32 @testee1(...) #1

declare i32 @printf(i8*, ...) #1

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 8.0.0 (clang-800.0.42.1)"}
