; ModuleID = 'scalar_value.c'
source_filename = "scalar_value.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@IntScalar = internal global i32 10, align 4

; Function Attrs: noinline nounwind ssp uwtable
define i32 @scalar_value(i32) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  store i32 5, i32* %3, align 4
  store i32 5, i32* %4, align 4
  %5 = load i32, i32* %2, align 4
  %6 = load i32, i32* @IntScalar, align 4
  %7 = mul nsw i32 %5, %6
  %8 = mul nsw i32 %7, 10
  %9 = load i32, i32* %3, align 4
  %10 = mul nsw i32 %8, %9
  %11 = mul nsw i32 %10, 5
  ret i32 %11
}

; Function Attrs: noinline nounwind ssp uwtable
define i32 @test_scalar_value() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 @scalar_value(i32 4)
  %3 = icmp eq i32 %2, 10000
  %4 = zext i1 %3 to i32
  store i32 %4, i32* %1, align 4
  %5 = load i32, i32* %1, align 4
  ret i32 %5
}

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 9.0.0 (clang-900.0.37)"}
