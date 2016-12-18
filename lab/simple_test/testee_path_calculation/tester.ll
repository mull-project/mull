; ModuleID = 'tester.c'
source_filename = "tester.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.11.0"

@.str = private unnamed_addr constant [49 x i8] c"test_testee_path_calculation returns result: %d\0A\00", align 1

; Function Attrs: nounwind ssp uwtable
define i32 @test_testee_path_calculation() #0 !dbg !7 {
  %1 = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %1, metadata !11, metadata !12), !dbg !13
  %2 = call i32 (...) @testee1(), !dbg !14
  store i32 %2, i32* %1, align 4, !dbg !13
  %3 = load i32, i32* %1, align 4, !dbg !15
  %4 = icmp sgt i32 %3, 1, !dbg !16
  %5 = zext i1 %4 to i32, !dbg !16
  %6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str, i32 0, i32 0), i32 %5), !dbg !17
  %7 = load i32, i32* %1, align 4, !dbg !18
  %8 = icmp sgt i32 %7, 1, !dbg !19
  %9 = zext i1 %8 to i32, !dbg !19
  ret i32 %9, !dbg !20
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i32 @testee1(...) #2

declare i32 @printf(i8*, ...) #2

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.9.0 (tags/RELEASE_390/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "tester.c", directory: "/Users/Stanislaw/Projects/LLVM/llvm/projects/mutang-project/lab/simple_test/testee_path_calculation")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 2}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 3.9.0 (tags/RELEASE_390/final)"}
!7 = distinct !DISubprogram(name: "test_testee_path_calculation", scope: !1, file: !1, line: 5, type: !8, isLocal: false, isDefinition: true, scopeLine: 5, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "result", scope: !7, file: !1, line: 6, type: !10)
!12 = !DIExpression()
!13 = !DILocation(line: 6, column: 7, scope: !7)
!14 = !DILocation(line: 6, column: 16, scope: !7)
!15 = !DILocation(line: 8, column: 63, scope: !7)
!16 = !DILocation(line: 8, column: 70, scope: !7)
!17 = !DILocation(line: 8, column: 3, scope: !7)
!18 = !DILocation(line: 10, column: 10, scope: !7)
!19 = !DILocation(line: 10, column: 17, scope: !7)
!20 = !DILocation(line: 10, column: 3, scope: !7)
