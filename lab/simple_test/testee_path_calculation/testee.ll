; ModuleID = 'testee.c'
source_filename = "testee.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.11.0"

; Function Attrs: nounwind ssp uwtable
define i32 @testee4() #0 {
  ret i32 1
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee3() #0 {
  %1 = call i32 @testee4()
  ret i32 %1
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee2() #0 {
  %1 = call i32 @testee3()
  ret i32 %1
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee1() #0 {
  %1 = call i32 @testee2()
  ret i32 %1
}

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 8.0.0 (clang-800.0.42.1)"}
