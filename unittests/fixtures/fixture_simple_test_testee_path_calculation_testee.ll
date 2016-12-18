; ModuleID = 'testee.c'
source_filename = "testee.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.11.0"

@.str = private unnamed_addr constant [28 x i8] c"testee4 returns result: %d\0A\00", align 1

; Function Attrs: nounwind ssp uwtable
define i32 @testee4(i32) #0 !dbg !7 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !11, metadata !12), !dbg !13
  call void @llvm.dbg.declare(metadata i32* %3, metadata !14, metadata !12), !dbg !15
  %4 = load i32, i32* %2, align 4, !dbg !16
  %5 = add nsw i32 20, %4, !dbg !17
  store i32 %5, i32* %3, align 4, !dbg !15
  %6 = load i32, i32* %3, align 4, !dbg !18
  %7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str, i32 0, i32 0), i32 %6), !dbg !19
  %8 = load i32, i32* %3, align 4, !dbg !20
  ret i32 %8, !dbg !21
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i32 @printf(i8*, ...) #2

; Function Attrs: nounwind ssp uwtable
define i32 @testee3() #0 !dbg !22 {
  %1 = call i32 @testee4(i32 1), !dbg !25
  ret i32 %1, !dbg !26
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee2() #0 !dbg !27 {
  %1 = call i32 @testee3(), !dbg !28
  ret i32 %1, !dbg !29
}

; Function Attrs: nounwind ssp uwtable
define i32 @testee1() #0 !dbg !30 {
  %1 = call i32 @testee2(), !dbg !31
  ret i32 %1, !dbg !32
}

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.9.0 (tags/RELEASE_390/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "testee.c", directory: "/Users/Stanislaw/Projects/LLVM/llvm/projects/mutang-project/lab/simple_test/testee_path_calculation")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 2}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 3.9.0 (tags/RELEASE_390/final)"}
!7 = distinct !DISubprogram(name: "testee4", scope: !1, file: !1, line: 5, type: !8, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10}
!10 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "x", arg: 1, scope: !7, file: !1, line: 5, type: !10)
!12 = !DIExpression()
!13 = !DILocation(line: 5, column: 17, scope: !7)
!14 = !DILocalVariable(name: "result", scope: !7, file: !1, line: 8, type: !10)
!15 = !DILocation(line: 8, column: 9, scope: !7)
!16 = !DILocation(line: 8, column: 23, scope: !7)
!17 = !DILocation(line: 8, column: 21, scope: !7)
!18 = !DILocation(line: 10, column: 44, scope: !7)
!19 = !DILocation(line: 10, column: 5, scope: !7)
!20 = !DILocation(line: 11, column: 12, scope: !7)
!21 = !DILocation(line: 11, column: 5, scope: !7)
!22 = distinct !DISubprogram(name: "testee3", scope: !1, file: !1, line: 14, type: !23, isLocal: false, isDefinition: true, scopeLine: 14, isOptimized: false, unit: !0, variables: !2)
!23 = !DISubroutineType(types: !24)
!24 = !{!10}
!25 = !DILocation(line: 15, column: 12, scope: !22)
!26 = !DILocation(line: 15, column: 5, scope: !22)
!27 = distinct !DISubprogram(name: "testee2", scope: !1, file: !1, line: 18, type: !23, isLocal: false, isDefinition: true, scopeLine: 18, isOptimized: false, unit: !0, variables: !2)
!28 = !DILocation(line: 19, column: 12, scope: !27)
!29 = !DILocation(line: 19, column: 5, scope: !27)
!30 = distinct !DISubprogram(name: "testee1", scope: !1, file: !1, line: 22, type: !23, isLocal: false, isDefinition: true, scopeLine: 22, isOptimized: false, unit: !0, variables: !2)
!31 = !DILocation(line: 23, column: 12, scope: !30)
!32 = !DILocation(line: 23, column: 5, scope: !30)
