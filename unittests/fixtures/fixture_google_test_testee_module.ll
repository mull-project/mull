; ModuleID = './Testee.cpp'
source_filename = "./Testee.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

%class.Testee = type { i8 }

; Function Attrs: nounwind ssp uwtable
define i32 @_ZN6Testee3sumEii(%class.Testee*, i32, i32) #0 align 2 !dbg !7 {
  %4 = alloca %class.Testee*, align 8
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store %class.Testee* %0, %class.Testee** %4, align 8
  call void @llvm.dbg.declare(metadata %class.Testee** %4, metadata !17, metadata !19), !dbg !20
  store i32 %1, i32* %5, align 4
  call void @llvm.dbg.declare(metadata i32* %5, metadata !21, metadata !19), !dbg !22
  store i32 %2, i32* %6, align 4
  call void @llvm.dbg.declare(metadata i32* %6, metadata !23, metadata !19), !dbg !24
  %8 = load %class.Testee*, %class.Testee** %4, align 8
  call void @llvm.dbg.declare(metadata i32* %7, metadata !25, metadata !19), !dbg !26
  %9 = load i32, i32* %5, align 4, !dbg !27
  %10 = load i32, i32* %6, align 4, !dbg !28
  %11 = add nsw i32 %9, %10, !dbg !29
  store i32 %11, i32* %7, align 4, !dbg !26
  %12 = load i32, i32* %7, align 4, !dbg !30
  ret i32 %12, !dbg !31
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 3.9.1 (tags/RELEASE_391/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "Testee.cpp", directory: "/Users/Stanislaw/Projects/mull/lab/google_test")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 2}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 3.9.1 (tags/RELEASE_391/final)"}
!7 = distinct !DISubprogram(name: "sum", linkageName: "_ZN6Testee3sumEii", scope: !9, file: !8, line: 3, type: !13, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !12, variables: !2)
!8 = !DIFile(filename: "./Testee.cpp", directory: "/Users/Stanislaw/Projects/mull/lab/google_test")
!9 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Testee", file: !10, line: 1, size: 8, align: 8, elements: !11, identifier: "_ZTS6Testee")
!10 = !DIFile(filename: "./Testee.h", directory: "/Users/Stanislaw/Projects/mull/lab/google_test")
!11 = !{!12}
!12 = !DISubprogram(name: "sum", linkageName: "_ZN6Testee3sumEii", scope: !9, file: !10, line: 4, type: !13, isLocal: false, isDefinition: false, scopeLine: 4, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!13 = !DISubroutineType(types: !14)
!14 = !{!15, !16, !15, !15}
!15 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!16 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64, align: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!17 = !DILocalVariable(name: "this", arg: 1, scope: !7, type: !18, flags: DIFlagArtificial | DIFlagObjectPointer)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64, align: 64)
!19 = !DIExpression()
!20 = !DILocation(line: 0, scope: !7)
!21 = !DILocalVariable(name: "a", arg: 2, scope: !7, file: !8, line: 3, type: !15)
!22 = !DILocation(line: 3, column: 21, scope: !7)
!23 = !DILocalVariable(name: "b", arg: 3, scope: !7, file: !8, line: 3, type: !15)
!24 = !DILocation(line: 3, column: 28, scope: !7)
!25 = !DILocalVariable(name: "c", scope: !7, file: !8, line: 4, type: !15)
!26 = !DILocation(line: 4, column: 7, scope: !7)
!27 = !DILocation(line: 4, column: 11, scope: !7)
!28 = !DILocation(line: 4, column: 15, scope: !7)
!29 = !DILocation(line: 4, column: 13, scope: !7)
!30 = !DILocation(line: 6, column: 10, scope: !7)
!31 = !DILocation(line: 6, column: 3, scope: !7)
