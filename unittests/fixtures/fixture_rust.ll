; ModuleID = 'example.cgu-0.rs'
source_filename = "example.cgu-0.rs"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-darwin"

%str_slice = type { i8*, i64 }
%"2.std::fmt::ArgumentV1" = type { %"2.core::fmt::Void"*, i8 (%"2.core::fmt::Void"*, %"2.std::fmt::Formatter"*)* }
%"2.core::fmt::Void" = type {}
%"2.std::fmt::Formatter" = type { i32, i32, i8, %"2.std::option::Option<usize>", %"2.std::option::Option<usize>", { i8*, void (i8*)** }, %"2.std::slice::Iter<std::fmt::ArgumentV1>", { %"2.std::fmt::ArgumentV1"*, i64 } }
%"2.std::option::Option<usize>" = type { i64, [0 x i64], [1 x i64] }
%"2.std::slice::Iter<std::fmt::ArgumentV1>" = type { %"2.std::fmt::ArgumentV1"*, %"2.std::fmt::ArgumentV1"*, %"2.std::marker::PhantomData<&std::fmt::ArgumentV1>" }
%"2.std::marker::PhantomData<&std::fmt::ArgumentV1>" = type {}
%"2.std::any::TypeId" = type { i64 }
%"2.std::fmt::Arguments" = type { { %str_slice*, i64 }, %"2.std::option::Option<&[std::fmt::rt::v1::Argument]>", { %"2.std::fmt::ArgumentV1"*, i64 } }
%"2.std::option::Option<&[std::fmt::rt::v1::Argument]>" = type { { %"2.std::fmt::rt::v1::Argument"*, i64 } }
%"2.std::fmt::rt::v1::Argument" = type { %"2.std::fmt::rt::v1::Position", %"2.std::fmt::rt::v1::FormatSpec" }
%"2.std::fmt::rt::v1::Position" = type { i64, [0 x i64], [1 x i64] }
%"2.std::fmt::rt::v1::FormatSpec" = type { i32, i8, i32, %"2.std::fmt::rt::v1::Count", %"2.std::fmt::rt::v1::Count" }
%"2.std::fmt::rt::v1::Count" = type { i64, [0 x i64], [1 x i64] }
%"12.__test::test::TestDescAndFn" = type { %"12.__test::test::TestDesc", %"12.__test::test::TestFn" }
%"12.__test::test::TestDesc" = type { %"12.__test::test::TestName", i8, %"12.__test::test::ShouldPanic" }
%"12.__test::test::TestName" = type { i64, [0 x i64], [3 x i64] }
%"12.__test::test::ShouldPanic" = type { i64, [0 x i64], [2 x i64] }
%"12.__test::test::TestFn" = type { i64, [0 x i64], [2 x i64] }
%"8.unwind::libunwind::_Unwind_Exception" = type { i64, void (i32, %"8.unwind::libunwind::_Unwind_Exception"*)*, [6 x i64] }
%"8.unwind::libunwind::_Unwind_Context" = type {}

@_ZN7example10just_print15__STATIC_FMTSTR17hd8f4a8b1a7bd93feE = internal constant { %str_slice*, i64 } { %str_slice* getelementptr inbounds ([1 x %str_slice], [1 x %str_slice]* @ref.9, i32 0, i32 0), i64 1 }, align 8
@vtable.0 = internal unnamed_addr constant { void (i8*)*, i64, i64, i64 (%str_slice*)* } { void (i8*)* @_ZN4drop17h7115f936263dd8e8E, i64 16, i64 8, i64 (%str_slice*)* @"_ZN36_$LT$T$u20$as$u20$core..any..Any$GT$11get_type_id17h4bed9676a70da661E" }, align 8
@str.1 = internal constant [10 x i8] c"example.rs"
@str.2 = internal constant [28 x i8] c"attempt to add with overflow"
@panic_loc.3 = internal unnamed_addr constant { %str_slice, %str_slice, i32 } { %str_slice { i8* getelementptr inbounds ([28 x i8], [28 x i8]* @str.2, i32 0, i32 0), i64 28 }, %str_slice { i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str.1, i32 0, i32 0), i64 10 }, i32 3 }, align 8
@ref.4 = internal unnamed_addr constant [0 x %"2.std::fmt::ArgumentV1"] zeroinitializer, align 8
@str.5 = internal constant [32 x i8] c"assertion failed: sum(3, 4) == 7"
@str.6 = internal constant [32 x i8] c"assertion failed: sum(4, 5) == 9"
@str.7 = internal constant [33 x i8] c"assertion failed: sum(5, 6) == 11"
@str.8 = internal constant [46 x i8] c"I am just_print() function. I just say hello!\0A"
@ref.9 = internal unnamed_addr constant [1 x %str_slice] [%str_slice { i8* getelementptr inbounds ([46 x i8], [46 x i8]* @str.8, i32 0, i32 0), i64 46 }], align 8
@_ZN7example17rusttest_foo_sum110_FILE_LINE17hc8287d48fb620a31E = internal constant { %str_slice, i32, [4 x i8] } { %str_slice { i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str.1, i32 0, i32 0), i64 10 }, i32 12, [4 x i8] undef }, align 8
@_ZN7example17rusttest_foo_sum210_FILE_LINE17h4d705b6f32c12b79E = internal constant { %str_slice, i32, [4 x i8] } { %str_slice { i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str.1, i32 0, i32 0), i64 10 }, i32 17, [4 x i8] undef }, align 8
@_ZN7example17rusttest_foo_sum310_FILE_LINE17h8fbd632de25157e2E = internal constant { %str_slice, i32, [4 x i8] } { %str_slice { i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str.1, i32 0, i32 0), i64 10 }, i32 22, [4 x i8] undef }, align 8
@_ZN7example17rusttest_foo_sum410_FILE_LINE17he7849d355e168406E = internal constant { %str_slice, i32, [4 x i8] } { %str_slice { i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str.1, i32 0, i32 0), i64 10 }, i32 27, [4 x i8] undef }, align 8
@str.10 = internal constant [17 x i8] c"rusttest_foo_sum1"
@str.11 = internal constant [17 x i8] c"rusttest_foo_sum2"
@str.12 = internal constant [17 x i8] c"rusttest_foo_sum3"
@str.13 = internal constant [17 x i8] c"rusttest_foo_sum4"
@ref.14 = internal unnamed_addr constant { { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } } } { { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } } { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } } { { i64, %str_slice, [8 x i8] } { i64 0, %str_slice { i8* getelementptr inbounds ([17 x i8], [17 x i8]* @str.10, i32 0, i32 0), i64 17 }, [8 x i8] undef }, i1 false, [7 x i8] undef, { i64, [16 x i8] } { i64 0, [16 x i8] undef } }, { i64, void ()*, [8 x i8] } { i64 0, void ()* @_ZN7example17rusttest_foo_sum117h7aa5d6b570662192E, [8 x i8] undef } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } } { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } } { { i64, %str_slice, [8 x i8] } { i64 0, %str_slice { i8* getelementptr inbounds ([17 x i8], [17 x i8]* @str.11, i32 0, i32 0), i64 17 }, [8 x i8] undef }, i1 false, [7 x i8] undef, { i64, [16 x i8] } { i64 0, [16 x i8] undef } }, { i64, void ()*, [8 x i8] } { i64 0, void ()* @_ZN7example17rusttest_foo_sum217h389da0550735b641E, [8 x i8] undef } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } } { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } } { { i64, %str_slice, [8 x i8] } { i64 0, %str_slice { i8* getelementptr inbounds ([17 x i8], [17 x i8]* @str.12, i32 0, i32 0), i64 17 }, [8 x i8] undef }, i1 false, [7 x i8] undef, { i64, [16 x i8] } { i64 0, [16 x i8] undef } }, { i64, void ()*, [8 x i8] } { i64 0, void ()* @_ZN7example17rusttest_foo_sum317h792193082ea99887E, [8 x i8] undef } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } } { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } } { { i64, %str_slice, [8 x i8] } { i64 0, %str_slice { i8* getelementptr inbounds ([17 x i8], [17 x i8]* @str.13, i32 0, i32 0), i64 17 }, [8 x i8] undef }, i1 false, [7 x i8] undef, { i64, [16 x i8] } { i64 0, [16 x i8] undef } }, { i64, void ()*, [8 x i8] } { i64 0, void ()* @_ZN7example17rusttest_foo_sum417h38f8b273364164edE, [8 x i8] undef } } }, align 8

; Function Attrs: uwtable
define internal i64 @"_ZN36_$LT$T$u20$as$u20$core..any..Any$GT$11get_type_id17h4bed9676a70da661E"(%str_slice* noalias readonly dereferenceable(16)) unnamed_addr #0 !dbg !69 {
entry-block:
  %arg0 = alloca %str_slice*
  %_0 = alloca %"2.std::any::TypeId"
  %self = alloca %str_slice*
  %abi_cast = alloca i64
  store %str_slice* %0, %str_slice** %arg0
  call void @llvm.dbg.declare(metadata %str_slice** %arg0, metadata !82, metadata !83), !dbg !84
  call void @llvm.dbg.declare(metadata %str_slice** %self, metadata !85, metadata !83), !dbg !87
  br label %start

start:                                            ; preds = %entry-block
  %1 = load %str_slice*, %str_slice** %arg0, !dbg !88, !nonnull !81
  store %str_slice* %1, %str_slice** %self, !dbg !88
  %2 = call i64 @_ZN4core3any6TypeId2of17hbcd2920c2d5de549E(), !dbg !87
  store i64 %2, i64* %abi_cast, !dbg !87
  %3 = bitcast %"2.std::any::TypeId"* %_0 to i8*, !dbg !87
  %4 = bitcast i64* %abi_cast to i8*, !dbg !87
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %3, i8* %4, i64 8, i32 8, i1 false), !dbg !87
  br label %bb1, !dbg !87

bb1:                                              ; preds = %start
  %5 = bitcast %"2.std::any::TypeId"* %_0 to i64*, !dbg !87
  %6 = load i64, i64* %5, align 8, !dbg !87
  ret i64 %6, !dbg !87
}

; Function Attrs: cold noinline noreturn uwtable
define internal void @_ZN3std9panicking11begin_panic17h634e2b37a96f78d4E(i8* noalias nonnull readonly, i64, { %str_slice, i32 }* noalias readonly dereferenceable(24)) unnamed_addr #1 personality i32 (i32, i32, i64, %"8.unwind::libunwind::_Unwind_Exception"*, %"8.unwind::libunwind::_Unwind_Context"*)* @rust_eh_personality !dbg !89 {
entry-block:
  %arg0.i = alloca %str_slice
  call void @llvm.dbg.declare(metadata %str_slice* %arg0.i, metadata !99, metadata !83), !dbg !108
  %x.i = alloca %str_slice
  call void @llvm.dbg.declare(metadata %str_slice* %x.i, metadata !111, metadata !83), !dbg !113
  %arg0 = alloca %str_slice
  %arg1 = alloca { %str_slice, i32 }*
  %msg = alloca %str_slice
  %file_line = alloca { %str_slice, i32 }*
  %personalityslot = alloca { i8*, i32 }
  %3 = getelementptr inbounds %str_slice, %str_slice* %arg0, i32 0, i32 0
  store i8* %0, i8** %3
  %4 = getelementptr inbounds %str_slice, %str_slice* %arg0, i32 0, i32 1
  store i64 %1, i64* %4
  call void @llvm.dbg.declare(metadata %str_slice* %arg0, metadata !114, metadata !83), !dbg !115
  store { %str_slice, i32 }* %2, { %str_slice, i32 }** %arg1
  call void @llvm.dbg.declare(metadata { %str_slice, i32 }** %arg1, metadata !116, metadata !83), !dbg !115
  call void @llvm.dbg.declare(metadata %str_slice* %msg, metadata !117, metadata !83), !dbg !118
  call void @llvm.dbg.declare(metadata { %str_slice, i32 }** %file_line, metadata !119, metadata !83), !dbg !118
  br label %start

start:                                            ; preds = %entry-block
  %5 = getelementptr inbounds %str_slice, %str_slice* %arg0, i32 0, i32 0, !dbg !120
  %6 = load i8*, i8** %5, !dbg !120, !nonnull !81
  %7 = getelementptr inbounds %str_slice, %str_slice* %arg0, i32 0, i32 1, !dbg !120
  %8 = load i64, i64* %7, !dbg !120
  %9 = getelementptr inbounds %str_slice, %str_slice* %msg, i32 0, i32 0, !dbg !120
  store i8* %6, i8** %9, !dbg !120
  %10 = getelementptr inbounds %str_slice, %str_slice* %msg, i32 0, i32 1, !dbg !120
  store i64 %8, i64* %10, !dbg !120
  %11 = load { %str_slice, i32 }*, { %str_slice, i32 }** %arg1, !dbg !120, !nonnull !81
  store { %str_slice, i32 }* %11, { %str_slice, i32 }** %file_line, !dbg !120
  %12 = getelementptr inbounds %str_slice, %str_slice* %msg, i32 0, i32 0, !dbg !121
  %13 = load i8*, i8** %12, !dbg !121, !nonnull !81
  %14 = getelementptr inbounds %str_slice, %str_slice* %msg, i32 0, i32 1, !dbg !121
  %15 = load i64, i64* %14, !dbg !121
  %16 = getelementptr inbounds %str_slice, %str_slice* %arg0.i, i32 0, i32 0, !dbg !121
  store i8* %13, i8** %16, !dbg !121, !noalias !122
  %17 = getelementptr inbounds %str_slice, %str_slice* %arg0.i, i32 0, i32 1, !dbg !121
  store i64 %15, i64* %17, !dbg !121, !noalias !122
  %18 = getelementptr inbounds %str_slice, %str_slice* %arg0.i, i32 0, i32 0, !dbg !125
  %19 = load i8*, i8** %18, !dbg !125, !noalias !122, !nonnull !81
  %20 = getelementptr inbounds %str_slice, %str_slice* %arg0.i, i32 0, i32 1, !dbg !125
  %21 = load i64, i64* %20, !dbg !125, !noalias !122
  %22 = getelementptr inbounds %str_slice, %str_slice* %x.i, i32 0, i32 0, !dbg !125
  store i8* %19, i8** %22, !dbg !125, !noalias !122
  %23 = getelementptr inbounds %str_slice, %str_slice* %x.i, i32 0, i32 1, !dbg !125
  store i64 %21, i64* %23, !dbg !125, !noalias !122
  %24 = invoke i8* @_ZN5alloc4heap15exchange_malloc17h48324f7e7f415602E(i64 16, i64 8)
          to label %"_ZN5alloc5boxed30_$LT$impl$u20$Box$LT$T$GT$$GT$3new17h206de9ebd9c06f29E.exit" unwind label %cleanup, !dbg !126

"_ZN5alloc5boxed30_$LT$impl$u20$Box$LT$T$GT$$GT$3new17h206de9ebd9c06f29E.exit": ; preds = %start
  %25 = bitcast i8* %24 to %str_slice*, !dbg !126
  %26 = getelementptr inbounds %str_slice, %str_slice* %x.i, i32 0, i32 0, !dbg !126
  %27 = load i8*, i8** %26, !dbg !126, !noalias !122, !nonnull !81
  %28 = getelementptr inbounds %str_slice, %str_slice* %x.i, i32 0, i32 1, !dbg !126
  %29 = load i64, i64* %28, !dbg !126, !noalias !122
  %30 = getelementptr inbounds %str_slice, %str_slice* %25, i32 0, i32 0, !dbg !126
  store i8* %27, i8** %30, !dbg !126
  %31 = getelementptr inbounds %str_slice, %str_slice* %25, i32 0, i32 1, !dbg !126
  store i64 %29, i64* %31, !dbg !126
  br label %bb2, !dbg !127

bb1:                                              ; preds = %cleanup
  %32 = load { i8*, i32 }, { i8*, i32 }* %personalityslot, !dbg !120
  resume { i8*, i32 } %32, !dbg !120

bb2:                                              ; preds = %"_ZN5alloc5boxed30_$LT$impl$u20$Box$LT$T$GT$$GT$3new17h206de9ebd9c06f29E.exit"
  %33 = bitcast %str_slice* %25 to i8*, !dbg !121
  %34 = load { %str_slice, i32 }*, { %str_slice, i32 }** %file_line, !dbg !121, !nonnull !81
  invoke void @_ZN3std9panicking20rust_panic_with_hook17h109e116a3a861224E(i8* noalias nonnull %33, void (i8*)** nonnull getelementptr inbounds ({ void (i8*)*, i64, i64, i64 (%str_slice*)* }, { void (i8*)*, i64, i64, i64 (%str_slice*)* }* @vtable.0, i32 0, i32 0), { %str_slice, i32 }* noalias readonly dereferenceable(24) %34)
          to label %unreachable unwind label %cleanup, !dbg !121

cleanup:                                          ; preds = %start, %bb2
  %35 = landingpad { i8*, i32 }
          cleanup
  store { i8*, i32 } %35, { i8*, i32 }* %personalityslot
  br label %bb1

unreachable:                                      ; preds = %bb2
  unreachable
}

; Function Attrs: uwtable
define internal i64 @_ZN4core3any6TypeId2of17hbcd2920c2d5de549E() unnamed_addr #0 !dbg !128 {
entry-block:
  %_0 = alloca %"2.std::any::TypeId"
  %tmp_ret = alloca i64
  br label %start

start:                                            ; preds = %entry-block
  store i64 3020856465566936866, i64* %tmp_ret
  %0 = load i64, i64* %tmp_ret, !dbg !131
  br label %bb1, !dbg !131

bb1:                                              ; preds = %start
  %1 = getelementptr inbounds %"2.std::any::TypeId", %"2.std::any::TypeId"* %_0, i32 0, i32 0, !dbg !132
  store i64 %0, i64* %1, !dbg !132
  %2 = bitcast %"2.std::any::TypeId"* %_0 to i64*, !dbg !133
  %3 = load i64, i64* %2, align 8, !dbg !133
  ret i64 %3, !dbg !133
}

; Function Attrs: inlinehint uwtable
define internal void @_ZN4core3fmt9Arguments6new_v117hf82ebeeaebdcd474E(%"2.std::fmt::Arguments"* noalias nocapture sret dereferenceable(48), %str_slice* noalias nonnull readonly, i64, %"2.std::fmt::ArgumentV1"* noalias nonnull readonly, i64) unnamed_addr #2 !dbg !134 {
entry-block:
  %arg0 = alloca { %str_slice*, i64 }
  %arg1 = alloca { %"2.std::fmt::ArgumentV1"*, i64 }
  %pieces = alloca { %str_slice*, i64 }
  %args = alloca { %"2.std::fmt::ArgumentV1"*, i64 }
  %_6 = alloca %"2.std::option::Option<&[std::fmt::rt::v1::Argument]>"
  %5 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %arg0, i32 0, i32 0
  store %str_slice* %1, %str_slice** %5
  %6 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %arg0, i32 0, i32 1
  store i64 %2, i64* %6
  call void @llvm.dbg.declare(metadata { %str_slice*, i64 }* %arg0, metadata !256, metadata !83), !dbg !257
  %7 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %arg1, i32 0, i32 0
  store %"2.std::fmt::ArgumentV1"* %3, %"2.std::fmt::ArgumentV1"** %7
  %8 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %arg1, i32 0, i32 1
  store i64 %4, i64* %8
  call void @llvm.dbg.declare(metadata { %"2.std::fmt::ArgumentV1"*, i64 }* %arg1, metadata !258, metadata !83), !dbg !257
  call void @llvm.dbg.declare(metadata { %str_slice*, i64 }* %pieces, metadata !259, metadata !83), !dbg !261
  call void @llvm.dbg.declare(metadata { %"2.std::fmt::ArgumentV1"*, i64 }* %args, metadata !262, metadata !83), !dbg !263
  br label %start

start:                                            ; preds = %entry-block
  %9 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %arg0, i32 0, i32 0, !dbg !264
  %10 = load %str_slice*, %str_slice** %9, !dbg !264, !nonnull !81
  %11 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %arg0, i32 0, i32 1, !dbg !264
  %12 = load i64, i64* %11, !dbg !264
  %13 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %pieces, i32 0, i32 0, !dbg !264
  store %str_slice* %10, %str_slice** %13, !dbg !264
  %14 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %pieces, i32 0, i32 1, !dbg !264
  store i64 %12, i64* %14, !dbg !264
  %15 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %arg1, i32 0, i32 0, !dbg !265
  %16 = load %"2.std::fmt::ArgumentV1"*, %"2.std::fmt::ArgumentV1"** %15, !dbg !265, !nonnull !81
  %17 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %arg1, i32 0, i32 1, !dbg !265
  %18 = load i64, i64* %17, !dbg !265
  %19 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %args, i32 0, i32 0, !dbg !265
  store %"2.std::fmt::ArgumentV1"* %16, %"2.std::fmt::ArgumentV1"** %19, !dbg !265
  %20 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %args, i32 0, i32 1, !dbg !265
  store i64 %18, i64* %20, !dbg !265
  %21 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %pieces, i32 0, i32 0, !dbg !266
  %22 = load %str_slice*, %str_slice** %21, !dbg !266, !nonnull !81
  %23 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %pieces, i32 0, i32 1, !dbg !266
  %24 = load i64, i64* %23, !dbg !266
  %25 = getelementptr inbounds %"2.std::option::Option<&[std::fmt::rt::v1::Argument]>", %"2.std::option::Option<&[std::fmt::rt::v1::Argument]>"* %_6, i32 0, i32 0, i32 0, !dbg !267
  store %"2.std::fmt::rt::v1::Argument"* null, %"2.std::fmt::rt::v1::Argument"** %25, !dbg !267
  %26 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %args, i32 0, i32 0, !dbg !268
  %27 = load %"2.std::fmt::ArgumentV1"*, %"2.std::fmt::ArgumentV1"** %26, !dbg !268, !nonnull !81
  %28 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %args, i32 0, i32 1, !dbg !268
  %29 = load i64, i64* %28, !dbg !268
  %30 = getelementptr inbounds %"2.std::fmt::Arguments", %"2.std::fmt::Arguments"* %0, i32 0, i32 0, !dbg !269
  %31 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %30, i32 0, i32 0, !dbg !269
  store %str_slice* %22, %str_slice** %31, !dbg !269
  %32 = getelementptr inbounds { %str_slice*, i64 }, { %str_slice*, i64 }* %30, i32 0, i32 1, !dbg !269
  store i64 %24, i64* %32, !dbg !269
  %33 = getelementptr inbounds %"2.std::fmt::Arguments", %"2.std::fmt::Arguments"* %0, i32 0, i32 1, !dbg !269
  %34 = bitcast %"2.std::option::Option<&[std::fmt::rt::v1::Argument]>"* %_6 to i8*, !dbg !269
  %35 = bitcast %"2.std::option::Option<&[std::fmt::rt::v1::Argument]>"* %33 to i8*, !dbg !269
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %35, i8* %34, i64 16, i32 8, i1 false), !dbg !269
  %36 = getelementptr inbounds %"2.std::fmt::Arguments", %"2.std::fmt::Arguments"* %0, i32 0, i32 2, !dbg !269
  %37 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %36, i32 0, i32 0, !dbg !269
  store %"2.std::fmt::ArgumentV1"* %27, %"2.std::fmt::ArgumentV1"** %37, !dbg !269
  %38 = getelementptr inbounds { %"2.std::fmt::ArgumentV1"*, i64 }, { %"2.std::fmt::ArgumentV1"*, i64 }* %36, i32 0, i32 1, !dbg !269
  store i64 %29, i64* %38, !dbg !269
  ret void, !dbg !270
}

; Function Attrs: inlinehint uwtable
define internal zeroext i1 @"_ZN4core3ptr31_$LT$impl$u20$$BP$mut$u20$T$GT$7is_null17h0e69dab9481e4a3cE"(i8*) unnamed_addr #2 !dbg !271 {
entry-block:
  %arg0 = alloca i8*
  %self = alloca i8*
  store i8* %0, i8** %arg0
  call void @llvm.dbg.declare(metadata i8** %arg0, metadata !281, metadata !83), !dbg !282
  call void @llvm.dbg.declare(metadata i8** %self, metadata !283, metadata !83), !dbg !285
  br label %start

start:                                            ; preds = %entry-block
  %1 = load i8*, i8** %arg0, !dbg !286
  store i8* %1, i8** %self, !dbg !286
  %2 = load i8*, i8** %self, !dbg !287
  %3 = call i8* @_ZN4core3ptr8null_mut17h6b7dc6e8748e0386E(), !dbg !287
  br label %bb1, !dbg !287

bb1:                                              ; preds = %start
  %4 = icmp eq i8* %2, %3, !dbg !287
  ret i1 %4, !dbg !288
}

; Function Attrs: inlinehint uwtable
define internal i8* @_ZN4core3ptr8null_mut17h6b7dc6e8748e0386E() unnamed_addr #2 !dbg !289 {
entry-block:
  br label %start

start:                                            ; preds = %entry-block
  ret i8* null, !dbg !292
}

define internal void @_ZN4drop17h7115f936263dd8e8E(i8*) unnamed_addr #3 {
entry-block:
  ret void
}

; Function Attrs: inlinehint uwtable
define internal i8* @_ZN5alloc4heap15exchange_malloc17h48324f7e7f415602E(i64, i64) unnamed_addr #2 !dbg !293 {
entry-block:
  %arg0 = alloca i64
  %arg1 = alloca i64
  %_0 = alloca i8*
  %size = alloca i64
  %align = alloca i64
  %ptr = alloca i8*
  %_10 = alloca {}
  store i64 %0, i64* %arg0
  call void @llvm.dbg.declare(metadata i64* %arg0, metadata !298, metadata !83), !dbg !299
  store i64 %1, i64* %arg1
  call void @llvm.dbg.declare(metadata i64* %arg1, metadata !300, metadata !83), !dbg !299
  call void @llvm.dbg.declare(metadata i64* %size, metadata !301, metadata !83), !dbg !303
  call void @llvm.dbg.declare(metadata i64* %align, metadata !304, metadata !83), !dbg !303
  call void @llvm.dbg.declare(metadata i8** %ptr, metadata !305, metadata !83), !dbg !307
  br label %start

start:                                            ; preds = %entry-block
  %2 = load i64, i64* %arg0, !dbg !308
  store i64 %2, i64* %size, !dbg !308
  %3 = load i64, i64* %arg1, !dbg !308
  store i64 %3, i64* %align, !dbg !308
  %4 = load i64, i64* %size, !dbg !309
  %5 = icmp eq i64 %4, 0, !dbg !309
  br i1 %5, label %bb1, label %bb2, !dbg !309

bb1:                                              ; preds = %start
  store i8* inttoptr (i64 1 to i8*), i8** %_0, !dbg !310
  br label %bb7, !dbg !309

bb2:                                              ; preds = %start
  %6 = load i64, i64* %size, !dbg !311
  %7 = load i64, i64* %align, !dbg !311
  %8 = call i8* @_ZN5alloc4heap8allocate17h36ab8478f42466f5E(i64 %6, i64 %7), !dbg !311
  store i8* %8, i8** %ptr, !dbg !311
  br label %bb3, !dbg !311

bb3:                                              ; preds = %bb2
  %9 = load i8*, i8** %ptr, !dbg !312
  %10 = call zeroext i1 @"_ZN4core3ptr31_$LT$impl$u20$$BP$mut$u20$T$GT$7is_null17h0e69dab9481e4a3cE"(i8* %9), !dbg !312
  br label %bb4, !dbg !312

bb4:                                              ; preds = %bb3
  br i1 %10, label %bb5, label %bb6, !dbg !312

bb5:                                              ; preds = %bb4
  call void @_ZN5alloc3oom3oom17hcac7549e1d9cf2ccE(), !dbg !313
  unreachable, !dbg !313

bb6:                                              ; preds = %bb4
  %11 = load i8*, i8** %ptr, !dbg !314
  store i8* %11, i8** %_0, !dbg !314
  br label %bb7, !dbg !309

bb7:                                              ; preds = %bb1, %bb6
  %12 = load i8*, i8** %_0, !dbg !315
  ret i8* %12, !dbg !315
}

; Function Attrs: inlinehint uwtable
define internal i8* @_ZN5alloc4heap8allocate17h36ab8478f42466f5E(i64, i64) unnamed_addr #2 !dbg !316 {
entry-block:
  %arg0.i = alloca i64
  call void @llvm.dbg.declare(metadata i64* %arg0.i, metadata !317, metadata !83), !dbg !321
  %arg1.i = alloca i64
  call void @llvm.dbg.declare(metadata i64* %arg1.i, metadata !324, metadata !83), !dbg !321
  %_0.i = alloca {}
  %size.i = alloca i64
  call void @llvm.dbg.declare(metadata i64* %size.i, metadata !325, metadata !83), !dbg !327
  %align.i = alloca i64
  call void @llvm.dbg.declare(metadata i64* %align.i, metadata !328, metadata !83), !dbg !327
  %_5.i = alloca {}
  %_6.i = alloca {}
  %_7.i = alloca {}
  %arg0 = alloca i64
  %arg1 = alloca i64
  %size = alloca i64
  %align = alloca i64
  store i64 %0, i64* %arg0
  call void @llvm.dbg.declare(metadata i64* %arg0, metadata !329, metadata !83), !dbg !330
  store i64 %1, i64* %arg1
  call void @llvm.dbg.declare(metadata i64* %arg1, metadata !331, metadata !83), !dbg !330
  call void @llvm.dbg.declare(metadata i64* %size, metadata !332, metadata !83), !dbg !333
  call void @llvm.dbg.declare(metadata i64* %align, metadata !334, metadata !83), !dbg !333
  br label %start

start:                                            ; preds = %entry-block
  %2 = load i64, i64* %arg0, !dbg !335
  store i64 %2, i64* %size, !dbg !335
  %3 = load i64, i64* %arg1, !dbg !335
  store i64 %3, i64* %align, !dbg !335
  %4 = load i64, i64* %size, !dbg !336
  %5 = load i64, i64* %align, !dbg !336
  store i64 %4, i64* %arg0.i, !dbg !336
  store i64 %5, i64* %arg1.i, !dbg !336
  %6 = load i64, i64* %arg0.i, !dbg !337
  store i64 %6, i64* %size.i, !dbg !337
  %7 = load i64, i64* %arg1.i, !dbg !337
  store i64 %7, i64* %align.i, !dbg !337
  br label %bb1, !dbg !336

bb1:                                              ; preds = %start
  %8 = load i64, i64* %size, !dbg !338
  %9 = load i64, i64* %align, !dbg !338
  %10 = call i8* @__rust_allocate(i64 %8, i64 %9), !dbg !338
  br label %bb2, !dbg !338

bb2:                                              ; preds = %bb1
  ret i8* %10, !dbg !339
}

; Function Attrs: uwtable
define internal i32 @_ZN7example3sum17hf4e2be639b15369cE(i32, i32) unnamed_addr #0 !dbg !340 {
entry-block:
  %arg0 = alloca i32
  %arg1 = alloca i32
  %a = alloca i32
  %b = alloca i32
  store i32 %0, i32* %arg0
  call void @llvm.dbg.declare(metadata i32* %arg0, metadata !344, metadata !83), !dbg !345
  store i32 %1, i32* %arg1
  call void @llvm.dbg.declare(metadata i32* %arg1, metadata !346, metadata !83), !dbg !345
  call void @llvm.dbg.declare(metadata i32* %a, metadata !347, metadata !83), !dbg !349
  call void @llvm.dbg.declare(metadata i32* %b, metadata !350, metadata !83), !dbg !349
  br label %start

start:                                            ; preds = %entry-block
  %2 = load i32, i32* %arg0, !dbg !351
  store i32 %2, i32* %a, !dbg !351
  %3 = load i32, i32* %arg1, !dbg !351
  store i32 %3, i32* %b, !dbg !351
  %4 = load i32, i32* %a, !dbg !352
  %5 = load i32, i32* %b, !dbg !352
  %6 = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 %4, i32 %5), !dbg !352
  %7 = extractvalue { i32, i1 } %6, 0, !dbg !352
  %8 = extractvalue { i32, i1 } %6, 1, !dbg !352
  %9 = call i1 @llvm.expect.i1(i1 %8, i1 false), !dbg !352
  br i1 %9, label %panic, label %bb1, !dbg !352

bb1:                                              ; preds = %start
  ret i32 %7, !dbg !353

panic:                                            ; preds = %start
  call void @_ZN4core9panicking5panic17hd383cb12a44b01ffE({ %str_slice, %str_slice, i32 }* @panic_loc.3), !dbg !352
  unreachable, !dbg !352
}

; Function Attrs: uwtable
define internal void @_ZN7example10just_print17hb8312419c6b33f99E() unnamed_addr #0 !dbg !354 {
entry-block:
  %_0 = alloca {}
  %_2 = alloca %"2.std::fmt::Arguments"
  %_7 = alloca {}
  br label %start

start:                                            ; preds = %entry-block
  %0 = load %str_slice*, %str_slice** getelementptr inbounds ({ %str_slice*, i64 }, { %str_slice*, i64 }* @_ZN7example10just_print15__STATIC_FMTSTR17hd8f4a8b1a7bd93feE, i32 0, i32 0), !dbg !357, !nonnull !81
  %1 = load i64, i64* getelementptr inbounds ({ %str_slice*, i64 }, { %str_slice*, i64 }* @_ZN7example10just_print15__STATIC_FMTSTR17hd8f4a8b1a7bd93feE, i32 0, i32 1), !dbg !357
  call void @_ZN4core3fmt9Arguments6new_v117hf82ebeeaebdcd474E(%"2.std::fmt::Arguments"* noalias nocapture sret dereferenceable(48) %_2, %str_slice* noalias nonnull readonly %0, i64 %1, %"2.std::fmt::ArgumentV1"* noalias nonnull readonly getelementptr inbounds ([0 x %"2.std::fmt::ArgumentV1"], [0 x %"2.std::fmt::ArgumentV1"]* @ref.4, i32 0, i32 0), i64 0), !dbg !357
  br label %bb1, !dbg !357

bb1:                                              ; preds = %start
  call void @_ZN3std2io5stdio6_print17he48522be5b0a80d9E(%"2.std::fmt::Arguments"* noalias nocapture dereferenceable(48) %_2), !dbg !357
  br label %bb2, !dbg !357

bb2:                                              ; preds = %bb1
  ret void, !dbg !358
}

; Function Attrs: uwtable
define internal void @_ZN7example17rusttest_foo_sum117h7aa5d6b570662192E() unnamed_addr #0 !dbg !359 {
entry-block:
  %_0 = alloca {}
  %_1 = alloca {}
  br label %start

start:                                            ; preds = %entry-block
  %0 = call i32 @_ZN7example3sum17hf4e2be639b15369cE(i32 3, i32 4), !dbg !360
  br label %bb1, !dbg !360

bb1:                                              ; preds = %start
  %1 = icmp eq i32 %0, 7, !dbg !360
  %2 = xor i1 %1, true, !dbg !360
  br i1 %2, label %bb2, label %bb3, !dbg !360

bb2:                                              ; preds = %bb1
  call void @_ZN3std9panicking11begin_panic17h634e2b37a96f78d4E(i8* noalias nonnull readonly getelementptr inbounds ([32 x i8], [32 x i8]* @str.5, i32 0, i32 0), i64 32, { %str_slice, i32 }* noalias readonly dereferenceable(24) bitcast ({ %str_slice, i32, [4 x i8] }* @_ZN7example17rusttest_foo_sum110_FILE_LINE17hc8287d48fb620a31E to { %str_slice, i32 }*)), !dbg !360
  unreachable, !dbg !360

bb3:                                              ; preds = %bb1
  ret void, !dbg !361
}

; Function Attrs: uwtable
define internal void @_ZN7example17rusttest_foo_sum217h389da0550735b641E() unnamed_addr #0 !dbg !362 {
entry-block:
  %_0 = alloca {}
  %_1 = alloca {}
  br label %start

start:                                            ; preds = %entry-block
  %0 = call i32 @_ZN7example3sum17hf4e2be639b15369cE(i32 4, i32 5), !dbg !363
  br label %bb1, !dbg !363

bb1:                                              ; preds = %start
  %1 = icmp eq i32 %0, 9, !dbg !363
  %2 = xor i1 %1, true, !dbg !363
  br i1 %2, label %bb2, label %bb3, !dbg !363

bb2:                                              ; preds = %bb1
  call void @_ZN3std9panicking11begin_panic17h634e2b37a96f78d4E(i8* noalias nonnull readonly getelementptr inbounds ([32 x i8], [32 x i8]* @str.6, i32 0, i32 0), i64 32, { %str_slice, i32 }* noalias readonly dereferenceable(24) bitcast ({ %str_slice, i32, [4 x i8] }* @_ZN7example17rusttest_foo_sum210_FILE_LINE17h4d705b6f32c12b79E to { %str_slice, i32 }*)), !dbg !363
  unreachable, !dbg !363

bb3:                                              ; preds = %bb1
  ret void, !dbg !364
}

; Function Attrs: uwtable
define internal void @_ZN7example17rusttest_foo_sum317h792193082ea99887E() unnamed_addr #0 !dbg !365 {
entry-block:
  %_0 = alloca {}
  %_1 = alloca {}
  br label %start

start:                                            ; preds = %entry-block
  %0 = call i32 @_ZN7example3sum17hf4e2be639b15369cE(i32 5, i32 6), !dbg !366
  br label %bb1, !dbg !366

bb1:                                              ; preds = %start
  %1 = icmp eq i32 %0, 11, !dbg !366
  %2 = xor i1 %1, true, !dbg !366
  br i1 %2, label %bb2, label %bb3, !dbg !366

bb2:                                              ; preds = %bb1
  call void @_ZN3std9panicking11begin_panic17h634e2b37a96f78d4E(i8* noalias nonnull readonly getelementptr inbounds ([33 x i8], [33 x i8]* @str.7, i32 0, i32 0), i64 33, { %str_slice, i32 }* noalias readonly dereferenceable(24) bitcast ({ %str_slice, i32, [4 x i8] }* @_ZN7example17rusttest_foo_sum310_FILE_LINE17h8fbd632de25157e2E to { %str_slice, i32 }*)), !dbg !366
  unreachable, !dbg !366

bb3:                                              ; preds = %bb1
  ret void, !dbg !367
}

; Function Attrs: uwtable
define internal void @_ZN7example17rusttest_foo_sum417h38f8b273364164edE() unnamed_addr #0 !dbg !368 {
entry-block:
  %_0 = alloca {}
  %_1 = alloca {}
  br label %start

start:                                            ; preds = %entry-block
  %0 = call i32 @_ZN7example3sum17hf4e2be639b15369cE(i32 5, i32 6), !dbg !369
  br label %bb1, !dbg !369

bb1:                                              ; preds = %start
  %1 = icmp eq i32 %0, 11, !dbg !369
  %2 = xor i1 %1, true, !dbg !369
  br i1 %2, label %bb2, label %bb3, !dbg !369

bb2:                                              ; preds = %bb1
  call void @_ZN3std9panicking11begin_panic17h634e2b37a96f78d4E(i8* noalias nonnull readonly getelementptr inbounds ([33 x i8], [33 x i8]* @str.7, i32 0, i32 0), i64 33, { %str_slice, i32 }* noalias readonly dereferenceable(24) bitcast ({ %str_slice, i32, [4 x i8] }* @_ZN7example17rusttest_foo_sum410_FILE_LINE17he7849d355e168406E to { %str_slice, i32 }*)), !dbg !369
  unreachable, !dbg !369

bb3:                                              ; preds = %bb1
  ret void, !dbg !370
}

; Function Attrs: uwtable
define internal void @_ZN7example6__test4main17h2a7ca5b1ce24a7fdE() unnamed_addr #0 !dbg !371 {
entry-block:
  br label %start

start:                                            ; preds = %entry-block
  call void @_ZN4test16test_main_static17h9317ea9d7ac82cffE(%"12.__test::test::TestDescAndFn"* noalias nonnull readonly bitcast ({ { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } }, { { { i64, %str_slice, [8 x i8] }, i1, [7 x i8], { i64, [16 x i8] } }, { i64, void ()*, [8 x i8] } } }* @ref.14 to %"12.__test::test::TestDescAndFn"*), i64 4), !dbg !373
  br label %bb1, !dbg !373

bb1:                                              ; preds = %start
  ret void, !dbg !373
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #4

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #5

; Function Attrs: nounwind
declare i32 @rust_eh_personality(i32, i32, i64, %"8.unwind::libunwind::_Unwind_Exception"*, %"8.unwind::libunwind::_Unwind_Context"*) unnamed_addr #6

; Function Attrs: cold noinline noreturn
declare void @_ZN3std9panicking20rust_panic_with_hook17h109e116a3a861224E(i8* noalias nonnull, void (i8*)** nonnull, { %str_slice, i32 }* noalias readonly dereferenceable(24)) unnamed_addr #7

; Function Attrs: cold noinline noreturn
declare void @_ZN5alloc3oom3oom17hcac7549e1d9cf2ccE() unnamed_addr #7

; Function Attrs: nounwind
declare noalias i8* @__rust_allocate(i64, i64) unnamed_addr #6

; Function Attrs: nounwind readnone
declare { i32, i1 } @llvm.sadd.with.overflow.i32(i32, i32) #4

; Function Attrs: nounwind readnone
declare i1 @llvm.expect.i1(i1, i1) #4

; Function Attrs: cold noinline noreturn
declare void @_ZN4core9panicking5panic17hd383cb12a44b01ffE({ %str_slice, %str_slice, i32 }* noalias readonly dereferenceable(40)) unnamed_addr #7

declare void @_ZN3std2io5stdio6_print17he48522be5b0a80d9E(%"2.std::fmt::Arguments"* noalias nocapture dereferenceable(48)) unnamed_addr #3

declare void @_ZN4test16test_main_static17h9317ea9d7ac82cffE(%"12.__test::test::TestDescAndFn"* noalias nonnull readonly, i64) unnamed_addr #3

define i64 @main(i64, i8**) unnamed_addr #3 {
top:
  %2 = call i64 @_ZN3std2rt10lang_start17hd661476ce2fc2931E(i8* bitcast (void ()* @_ZN7example6__test4main17h2a7ca5b1ce24a7fdE to i8*), i64 %0, i8** %1)
  ret i64 %2
}

declare i64 @_ZN3std2rt10lang_start17hd661476ce2fc2931E(i8*, i64, i8**) unnamed_addr #3

attributes #0 = { uwtable "no-frame-pointer-elim"="true" }
attributes #1 = { cold noinline noreturn uwtable "no-frame-pointer-elim"="true" }
attributes #2 = { inlinehint uwtable "no-frame-pointer-elim"="true" }
attributes #3 = { "no-frame-pointer-elim"="true" }
attributes #4 = { nounwind readnone }
attributes #5 = { argmemonly nounwind }
attributes #6 = { nounwind "no-frame-pointer-elim"="true" }
attributes #7 = { cold noinline noreturn "no-frame-pointer-elim"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.dbg.cu = !{!3}

!0 = !{i32 1, !"PIE Level", i32 2}
!1 = !{i32 2, !"Dwarf Version", i32 2}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DICompileUnit(language: DW_LANG_Rust, file: !4, producer: "rustc version 1.14.0 (e8a012324 2016-12-16)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !5, globals: !39)
!4 = !DIFile(filename: "./example.rs", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!5 = !{!6, !16, !23, !29, !34}
!6 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "Position", scope: !8, file: !7, baseType: !12, size: 64, align: 64, elements: !13)
!7 = !DIFile(filename: "<unknown>", directory: "")
!8 = !DINamespace(name: "v1", scope: !9, file: !7)
!9 = !DINamespace(name: "rt", scope: !10, file: !7)
!10 = !DINamespace(name: "fmt", scope: !11, file: !7)
!11 = !DINamespace(name: "core", scope: null, file: !7)
!12 = !DIBasicType(name: "u64", size: 64, align: 64, encoding: DW_ATE_unsigned)
!13 = !{!14, !15}
!14 = !DIEnumerator(name: "Next", value: 0)
!15 = !DIEnumerator(name: "At", value: 1)
!16 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "Alignment", scope: !8, file: !7, baseType: !17, size: 8, align: 8, elements: !18)
!17 = !DIBasicType(name: "u8", size: 8, align: 8, encoding: DW_ATE_unsigned)
!18 = !{!19, !20, !21, !22}
!19 = !DIEnumerator(name: "Left", value: 0)
!20 = !DIEnumerator(name: "Right", value: 1)
!21 = !DIEnumerator(name: "Center", value: 2)
!22 = !DIEnumerator(name: "Unknown", value: 3)
!23 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "Count", scope: !8, file: !7, baseType: !12, size: 64, align: 64, elements: !24)
!24 = !{!25, !26, !27, !28}
!25 = !DIEnumerator(name: "Is", value: 0)
!26 = !DIEnumerator(name: "Param", value: 1)
!27 = !DIEnumerator(name: "NextParam", value: 2)
!28 = !DIEnumerator(name: "Implied", value: 3)
!29 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "Result", scope: !30, file: !7, baseType: !17, size: 8, align: 8, elements: !31)
!30 = !DINamespace(name: "result", scope: !11, file: !7)
!31 = !{!32, !33}
!32 = !DIEnumerator(name: "Ok", value: 0)
!33 = !DIEnumerator(name: "Err", value: 1)
!34 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "Option", scope: !35, file: !7, baseType: !12, size: 64, align: 64, elements: !36)
!35 = !DINamespace(name: "option", scope: !11, file: !7)
!36 = !{!37, !38}
!37 = !DIEnumerator(name: "None", value: 0)
!38 = !DIEnumerator(name: "Some", value: 1)
!39 = !{!40, !56, !63, !65, !67}
!40 = distinct !DIGlobalVariable(name: "__STATIC_FMTSTR", linkageName: "_ZN7example10just_print15__STATIC_FMTSTRE", scope: !41, file: !44, line: 2, type: !45, isLocal: true, isDefinition: true, variable: { %str_slice*, i64 }* @_ZN7example10just_print15__STATIC_FMTSTR17hd8f4a8b1a7bd93feE)
!41 = !DINamespace(name: "just_print", scope: !43, file: !42, line: 6)
!42 = !DIFile(filename: "/Users/stanislaw/Projects/mull/lab/rust/example.rs", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!43 = !DINamespace(name: "example", scope: null, file: !7)
!44 = !DIFile(filename: "<std macros>", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!45 = !DICompositeType(tag: DW_TAG_structure_type, name: "&[&str]", file: !7, size: 128, align: 64, elements: !46, identifier: "feaae0c54c7c7da1c9df23d36eb11d9c3735aa")
!46 = !{!47, !55}
!47 = !DIDerivedType(tag: DW_TAG_member, name: "data_ptr", scope: !45, file: !7, baseType: !48, size: 64, align: 64)
!48 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "*const &str", baseType: !49, size: 64, align: 64)
!49 = !DICompositeType(tag: DW_TAG_structure_type, name: "&str", file: !7, size: 128, align: 64, elements: !50, identifier: "ba943fec7ffec6f5b5383b3687bffbb545ec77f")
!50 = !{!51, !53}
!51 = !DIDerivedType(tag: DW_TAG_member, name: "data_ptr", scope: !49, file: !7, baseType: !52, size: 64, align: 64)
!52 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "*const u8", baseType: !17, size: 64, align: 64)
!53 = !DIDerivedType(tag: DW_TAG_member, name: "length", scope: !49, file: !7, baseType: !54, size: 64, align: 64, offset: 64)
!54 = !DIBasicType(name: "usize", size: 64, align: 64, encoding: DW_ATE_unsigned)
!55 = !DIDerivedType(tag: DW_TAG_member, name: "length", scope: !45, file: !7, baseType: !54, size: 64, align: 64, offset: 64)
!56 = distinct !DIGlobalVariable(name: "_FILE_LINE", linkageName: "_ZN7example17rusttest_foo_sum110_FILE_LINEE", scope: !57, file: !44, line: 5, type: !58, isLocal: true, isDefinition: true, variable: { %str_slice, i32, [4 x i8] }* @_ZN7example17rusttest_foo_sum110_FILE_LINE17hc8287d48fb620a31E)
!57 = !DINamespace(name: "rusttest_foo_sum1", scope: !43, file: !42, line: 11)
!58 = !DICompositeType(tag: DW_TAG_structure_type, name: "(&str, u32)", file: !7, size: 192, align: 64, elements: !59, identifier: "c5cde3a439cadccb7a27333adc9bf57da4510cb")
!59 = !{!60, !61}
!60 = !DIDerivedType(tag: DW_TAG_member, name: "__0", scope: !58, file: !7, baseType: !49, size: 128, align: 64)
!61 = !DIDerivedType(tag: DW_TAG_member, name: "__1", scope: !58, file: !7, baseType: !62, size: 32, align: 32, offset: 128)
!62 = !DIBasicType(name: "u32", size: 32, align: 32, encoding: DW_ATE_unsigned)
!63 = distinct !DIGlobalVariable(name: "_FILE_LINE", linkageName: "_ZN7example17rusttest_foo_sum210_FILE_LINEE", scope: !64, file: !44, line: 5, type: !58, isLocal: true, isDefinition: true, variable: { %str_slice, i32, [4 x i8] }* @_ZN7example17rusttest_foo_sum210_FILE_LINE17h4d705b6f32c12b79E)
!64 = !DINamespace(name: "rusttest_foo_sum2", scope: !43, file: !42, line: 16)
!65 = distinct !DIGlobalVariable(name: "_FILE_LINE", linkageName: "_ZN7example17rusttest_foo_sum310_FILE_LINEE", scope: !66, file: !44, line: 5, type: !58, isLocal: true, isDefinition: true, variable: { %str_slice, i32, [4 x i8] }* @_ZN7example17rusttest_foo_sum310_FILE_LINE17h8fbd632de25157e2E)
!66 = !DINamespace(name: "rusttest_foo_sum3", scope: !43, file: !42, line: 21)
!67 = distinct !DIGlobalVariable(name: "_FILE_LINE", linkageName: "_ZN7example17rusttest_foo_sum410_FILE_LINEE", scope: !68, file: !44, line: 5, type: !58, isLocal: true, isDefinition: true, variable: { %str_slice, i32, [4 x i8] }* @_ZN7example17rusttest_foo_sum410_FILE_LINE17he7849d355e168406E)
!68 = !DINamespace(name: "rusttest_foo_sum4", scope: !43, file: !42, line: 26)
!69 = distinct !DISubprogram(name: "get_type_id<&str>", linkageName: "_ZN4core3any8{{impl}}17get_type_id<&str>E", scope: !71, file: !70, line: 115, type: !73, isLocal: false, isDefinition: true, scopeLine: 115, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !79, variables: !81)
!70 = !DIFile(filename: "/Users/rustbuild/src/rust-buildbot/slave/stable-dist-rustc-mac/build/obj/../src/libcore/any.rs", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!71 = !DINamespace(name: "{{impl}}", scope: !72, file: !7)
!72 = !DINamespace(name: "any", scope: !11, file: !7)
!73 = !DISubroutineType(types: !74)
!74 = !{!75, !78}
!75 = !DICompositeType(tag: DW_TAG_structure_type, name: "TypeId", scope: !72, file: !7, size: 64, align: 64, elements: !76, identifier: "bf7d48731e03b6e1cedb66ee729d6c6f41787c")
!76 = !{!77}
!77 = !DIDerivedType(tag: DW_TAG_member, name: "t", scope: !75, file: !7, baseType: !12, size: 64, align: 64)
!78 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "&&str", baseType: !49, size: 64, align: 64)
!79 = !{!80}
!80 = !DITemplateTypeParameter(name: "T", type: !49)
!81 = !{}
!82 = !DILocalVariable(name: "self", arg: 1, scope: !69, file: !42, line: 1, type: !78)
!83 = !DIExpression()
!84 = !DILocation(line: 1, scope: !69)
!85 = !DILocalVariable(name: "self", scope: !86, file: !70, line: 115, type: !78)
!86 = distinct !DILexicalBlock(scope: !69, file: !70, line: 115, column: 36)
!87 = !DILocation(line: 115, scope: !86)
!88 = !DILocation(line: 115, scope: !69)
!89 = distinct !DISubprogram(name: "begin_panic<&str>", linkageName: "_ZN3std9panicking17begin_panic<&str>E", scope: !91, file: !90, line: 405, type: !93, isLocal: false, isDefinition: true, scopeLine: 405, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !97, variables: !81)
!90 = !DIFile(filename: "/Users/rustbuild/src/rust-buildbot/slave/stable-dist-rustc-mac/build/obj/../src/libstd/panicking.rs", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!91 = !DINamespace(name: "panicking", scope: !92, file: !7)
!92 = !DINamespace(name: "std", scope: null, file: !7)
!93 = !DISubroutineType(types: !94)
!94 = !{!95, !49, !96}
!95 = !DIBasicType(name: "!", align: 8, encoding: DW_ATE_unsigned)
!96 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "&(&str, u32)", baseType: !58, size: 64, align: 64)
!97 = !{!98}
!98 = !DITemplateTypeParameter(name: "M", type: !49)
!99 = !DILocalVariable(name: "x", arg: 1, scope: !100, file: !42, line: 1, type: !49)
!100 = distinct !DISubprogram(name: "new<&str>", linkageName: "_ZN5alloc5boxed8{{impl}}9new<&str>E", scope: !102, file: !101, line: 234, type: !105, isLocal: false, isDefinition: true, scopeLine: 234, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !79, variables: !81)
!101 = !DIFile(filename: "/Users/rustbuild/src/rust-buildbot/slave/stable-dist-rustc-mac/build/obj/../src/liballoc/boxed.rs", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!102 = !DINamespace(name: "{{impl}}", scope: !103, file: !7)
!103 = !DINamespace(name: "boxed", scope: !104, file: !7)
!104 = !DINamespace(name: "alloc", scope: null, file: !7)
!105 = !DISubroutineType(types: !106)
!106 = !{!107, !49}
!107 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "Box<&str>", baseType: !49, size: 64, align: 64)
!108 = !DILocation(line: 1, scope: !100, inlinedAt: !109)
!109 = distinct !DILocation(line: 413, scope: !110)
!110 = distinct !DILexicalBlock(scope: !89, file: !90, line: 405, column: 80)
!111 = !DILocalVariable(name: "x", scope: !112, file: !101, line: 234, type: !49)
!112 = distinct !DILexicalBlock(scope: !100, file: !101, line: 234, column: 31)
!113 = !DILocation(line: 234, scope: !112, inlinedAt: !109)
!114 = !DILocalVariable(name: "msg", arg: 1, scope: !89, file: !42, line: 1, type: !49)
!115 = !DILocation(line: 1, scope: !89)
!116 = !DILocalVariable(name: "file_line", arg: 2, scope: !89, file: !42, line: 1, type: !96)
!117 = !DILocalVariable(name: "msg", scope: !110, file: !90, line: 405, type: !49)
!118 = !DILocation(line: 405, scope: !110)
!119 = !DILocalVariable(name: "file_line", scope: !110, file: !90, line: 405, type: !96)
!120 = !DILocation(line: 405, scope: !89)
!121 = !DILocation(line: 413, scope: !110)
!122 = !{!123}
!123 = distinct !{!123, !124, !"_ZN5alloc5boxed30_$LT$impl$u20$Box$LT$T$GT$$GT$3new17h206de9ebd9c06f29E: argument 0"}
!124 = distinct !{!124, !"_ZN5alloc5boxed30_$LT$impl$u20$Box$LT$T$GT$$GT$3new17h206de9ebd9c06f29E"}
!125 = !DILocation(line: 234, scope: !100, inlinedAt: !109)
!126 = !DILocation(line: 235, scope: !112, inlinedAt: !109)
!127 = !DILocation(line: 236, scope: !112, inlinedAt: !109)
!128 = distinct !DISubprogram(name: "of<&str>", linkageName: "_ZN4core3any8{{impl}}8of<&str>E", scope: !75, file: !70, line: 366, type: !129, isLocal: false, isDefinition: true, scopeLine: 366, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !79, variables: !81)
!129 = !DISubroutineType(types: !130)
!130 = !{!75}
!131 = !DILocation(line: 368, scope: !128)
!132 = !DILocation(line: 367, scope: !128)
!133 = !DILocation(line: 370, scope: !128)
!134 = distinct !DISubprogram(name: "new_v1", linkageName: "_ZN4core3fmt8{{impl}}6new_v1E", scope: !136, file: !135, line: 239, type: !254, isLocal: false, isDefinition: true, scopeLine: 239, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!135 = !DIFile(filename: "/Users/rustbuild/src/rust-buildbot/slave/stable-dist-rustc-mac/build/obj/../src/libcore/fmt/mod.rs", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!136 = !DICompositeType(tag: DW_TAG_structure_type, name: "Arguments", scope: !10, file: !7, size: 384, align: 64, elements: !137, identifier: "83eb9ebabf3f6fd0ae2a6cdeba793e0a38addaf")
!137 = !{!138, !139, !194}
!138 = !DIDerivedType(tag: DW_TAG_member, name: "pieces", scope: !136, file: !7, baseType: !45, size: 128, align: 64)
!139 = !DIDerivedType(tag: DW_TAG_member, name: "fmt", scope: !136, file: !7, baseType: !140, size: 128, align: 64, offset: 128)
!140 = !DICompositeType(tag: DW_TAG_union_type, name: "Option<&[core::fmt::rt::v1::Argument]>", scope: !35, file: !7, size: 128, align: 64, elements: !141, identifier: "58737646dd18f2444d696745ac24cfb8fffb2b38")
!141 = !{!142}
!142 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENCODED$ENUM$0$0$None", scope: !140, file: !7, baseType: !143, size: 128, align: 64)
!143 = !DICompositeType(tag: DW_TAG_structure_type, name: "Some", scope: !35, file: !7, size: 128, align: 64, elements: !144, identifier: "58737646dd18f2444d696745ac24cfb8fffb2b38::Some")
!144 = !{!145}
!145 = !DIDerivedType(tag: DW_TAG_member, name: "__0", scope: !143, file: !7, baseType: !146, size: 128, align: 64)
!146 = !DICompositeType(tag: DW_TAG_structure_type, name: "&[core::fmt::rt::v1::Argument]", file: !7, size: 128, align: 64, elements: !147, identifier: "acb8b59b13b7737d210a6a244dc43df3c1186c1")
!147 = !{!148, !193}
!148 = !DIDerivedType(tag: DW_TAG_member, name: "data_ptr", scope: !146, file: !7, baseType: !149, size: 64, align: 64)
!149 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "*const core::fmt::rt::v1::Argument", baseType: !150, size: 64, align: 64)
!150 = !DICompositeType(tag: DW_TAG_structure_type, name: "Argument", scope: !8, file: !7, size: 512, align: 64, elements: !151, identifier: "87bca496c14e542a10853af2b39e8fc9e50ce67")
!151 = !{!152, !164}
!152 = !DIDerivedType(tag: DW_TAG_member, name: "position", scope: !150, file: !7, baseType: !153, size: 128, align: 64)
!153 = !DICompositeType(tag: DW_TAG_union_type, name: "Position", scope: !8, file: !7, size: 128, align: 64, elements: !154, identifier: "ad2e9748a4bf9a9a08f853bbce3267761a9fa")
!154 = !{!155, !159}
!155 = !DIDerivedType(tag: DW_TAG_member, scope: !153, file: !7, baseType: !156, size: 64, align: 64)
!156 = !DICompositeType(tag: DW_TAG_structure_type, name: "Next", scope: !8, file: !7, size: 64, align: 64, elements: !157, identifier: "ad2e9748a4bf9a9a08f853bbce3267761a9fa::Next")
!157 = !{!158}
!158 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !156, file: !7, baseType: !6, size: 64, align: 64)
!159 = !DIDerivedType(tag: DW_TAG_member, scope: !153, file: !7, baseType: !160, size: 128, align: 64)
!160 = !DICompositeType(tag: DW_TAG_structure_type, name: "At", scope: !8, file: !7, size: 128, align: 64, elements: !161, identifier: "ad2e9748a4bf9a9a08f853bbce3267761a9fa::At")
!161 = !{!162, !163}
!162 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !160, file: !7, baseType: !6, size: 64, align: 64)
!163 = !DIDerivedType(tag: DW_TAG_member, name: "__0", scope: !160, file: !7, baseType: !54, size: 64, align: 64, offset: 64)
!164 = !DIDerivedType(tag: DW_TAG_member, name: "format", scope: !150, file: !7, baseType: !165, size: 384, align: 64, offset: 128)
!165 = !DICompositeType(tag: DW_TAG_structure_type, name: "FormatSpec", scope: !8, file: !7, size: 384, align: 64, elements: !166, identifier: "6df852494a4ba5a76ea4b195c29c7e79b25eada")
!166 = !{!167, !169, !170, !171, !192}
!167 = !DIDerivedType(tag: DW_TAG_member, name: "fill", scope: !165, file: !7, baseType: !168, size: 32, align: 32)
!168 = !DIBasicType(name: "char", size: 32, align: 32, encoding: DW_ATE_unsigned_char)
!169 = !DIDerivedType(tag: DW_TAG_member, name: "align", scope: !165, file: !7, baseType: !16, size: 8, align: 8, offset: 32)
!170 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !165, file: !7, baseType: !62, size: 32, align: 32, offset: 64)
!171 = !DIDerivedType(tag: DW_TAG_member, name: "precision", scope: !165, file: !7, baseType: !172, size: 128, align: 64, offset: 128)
!172 = !DICompositeType(tag: DW_TAG_union_type, name: "Count", scope: !8, file: !7, size: 128, align: 64, elements: !173, identifier: "8da902bdb75d2893298ccedef93a1d21d8d3f")
!173 = !{!174, !179, !184, !188}
!174 = !DIDerivedType(tag: DW_TAG_member, scope: !172, file: !7, baseType: !175, size: 128, align: 64)
!175 = !DICompositeType(tag: DW_TAG_structure_type, name: "Is", scope: !8, file: !7, size: 128, align: 64, elements: !176, identifier: "8da902bdb75d2893298ccedef93a1d21d8d3f::Is")
!176 = !{!177, !178}
!177 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !175, file: !7, baseType: !23, size: 64, align: 64)
!178 = !DIDerivedType(tag: DW_TAG_member, name: "__0", scope: !175, file: !7, baseType: !54, size: 64, align: 64, offset: 64)
!179 = !DIDerivedType(tag: DW_TAG_member, scope: !172, file: !7, baseType: !180, size: 128, align: 64)
!180 = !DICompositeType(tag: DW_TAG_structure_type, name: "Param", scope: !8, file: !7, size: 128, align: 64, elements: !181, identifier: "8da902bdb75d2893298ccedef93a1d21d8d3f::Param")
!181 = !{!182, !183}
!182 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !180, file: !7, baseType: !23, size: 64, align: 64)
!183 = !DIDerivedType(tag: DW_TAG_member, name: "__0", scope: !180, file: !7, baseType: !54, size: 64, align: 64, offset: 64)
!184 = !DIDerivedType(tag: DW_TAG_member, scope: !172, file: !7, baseType: !185, size: 64, align: 64)
!185 = !DICompositeType(tag: DW_TAG_structure_type, name: "NextParam", scope: !8, file: !7, size: 64, align: 64, elements: !186, identifier: "8da902bdb75d2893298ccedef93a1d21d8d3f::NextParam")
!186 = !{!187}
!187 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !185, file: !7, baseType: !23, size: 64, align: 64)
!188 = !DIDerivedType(tag: DW_TAG_member, scope: !172, file: !7, baseType: !189, size: 64, align: 64)
!189 = !DICompositeType(tag: DW_TAG_structure_type, name: "Implied", scope: !8, file: !7, size: 64, align: 64, elements: !190, identifier: "8da902bdb75d2893298ccedef93a1d21d8d3f::Implied")
!190 = !{!191}
!191 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !189, file: !7, baseType: !23, size: 64, align: 64)
!192 = !DIDerivedType(tag: DW_TAG_member, name: "width", scope: !165, file: !7, baseType: !172, size: 128, align: 64, offset: 256)
!193 = !DIDerivedType(tag: DW_TAG_member, name: "length", scope: !146, file: !7, baseType: !54, size: 64, align: 64, offset: 64)
!194 = !DIDerivedType(tag: DW_TAG_member, name: "args", scope: !136, file: !7, baseType: !195, size: 128, align: 64, offset: 256)
!195 = !DICompositeType(tag: DW_TAG_structure_type, name: "&[core::fmt::ArgumentV1]", file: !7, size: 128, align: 64, elements: !196, identifier: "470f5eb0ba677a93b32e6f67e9e83e903326e2")
!196 = !{!197, !253}
!197 = !DIDerivedType(tag: DW_TAG_member, name: "data_ptr", scope: !195, file: !7, baseType: !198, size: 64, align: 64)
!198 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "*const core::fmt::ArgumentV1", baseType: !199, size: 64, align: 64)
!199 = !DICompositeType(tag: DW_TAG_structure_type, name: "ArgumentV1", scope: !10, file: !7, size: 128, align: 64, elements: !200, identifier: "4bf9de4540781d34a8cac17a87461583c1e857")
!200 = !{!201, !204}
!201 = !DIDerivedType(tag: DW_TAG_member, name: "value", scope: !199, file: !7, baseType: !202, size: 64, align: 64)
!202 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "&core::fmt::Void", baseType: !203, size: 64, align: 64)
!203 = !DICompositeType(tag: DW_TAG_union_type, name: "Void", scope: !10, file: !7, align: 8, elements: !81, identifier: "971a41309d713671d727b51cdfe21c17b550cf")
!204 = !DIDerivedType(tag: DW_TAG_member, name: "formatter", scope: !199, file: !7, baseType: !205, size: 64, align: 64, offset: 64)
!205 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "fn(&core::fmt::Void, &mut core::fmt::Formatter) -> core::result::Result<(), core::fmt::Error>", baseType: !206, size: 64, align: 64)
!206 = !DISubroutineType(types: !207)
!207 = !{!208, !202, !222}
!208 = !DICompositeType(tag: DW_TAG_union_type, name: "Result<(), core::fmt::Error>", scope: !30, file: !7, size: 8, align: 8, elements: !209, identifier: "844f18f8851d103a5327b8e997962573faa53d9e")
!209 = !{!210, !216}
!210 = !DIDerivedType(tag: DW_TAG_member, scope: !208, file: !7, baseType: !211, size: 8, align: 8)
!211 = !DICompositeType(tag: DW_TAG_structure_type, name: "Ok", scope: !30, file: !7, size: 8, align: 8, elements: !212, identifier: "844f18f8851d103a5327b8e997962573faa53d9e::Ok")
!212 = !{!213, !214}
!213 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !211, file: !7, baseType: !29, size: 8, align: 8)
!214 = !DIDerivedType(tag: DW_TAG_member, name: "__0", scope: !211, file: !7, baseType: !215, align: 8, offset: 8)
!215 = !DIBasicType(name: "()", align: 8, encoding: DW_ATE_unsigned)
!216 = !DIDerivedType(tag: DW_TAG_member, scope: !208, file: !7, baseType: !217, size: 8, align: 8)
!217 = !DICompositeType(tag: DW_TAG_structure_type, name: "Err", scope: !30, file: !7, size: 8, align: 8, elements: !218, identifier: "844f18f8851d103a5327b8e997962573faa53d9e::Err")
!218 = !{!219, !220}
!219 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !217, file: !7, baseType: !29, size: 8, align: 8)
!220 = !DIDerivedType(tag: DW_TAG_member, name: "__0", scope: !217, file: !7, baseType: !221, align: 8, offset: 8)
!221 = !DICompositeType(tag: DW_TAG_structure_type, name: "Error", scope: !10, file: !7, align: 8, elements: !81, identifier: "6ec6ef1d1538ee3cd7f516992793a08620bcc")
!222 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "&mut core::fmt::Formatter", baseType: !223, size: 64, align: 64)
!223 = !DICompositeType(tag: DW_TAG_structure_type, name: "Formatter", scope: !10, file: !7, size: 768, align: 64, elements: !224, identifier: "463d47a191b3ab0743ad18bc68d476df6fa56a")
!224 = !{!225, !226, !227, !228, !240, !241, !243, !252}
!225 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !223, file: !7, baseType: !62, size: 32, align: 32)
!226 = !DIDerivedType(tag: DW_TAG_member, name: "fill", scope: !223, file: !7, baseType: !168, size: 32, align: 32, offset: 32)
!227 = !DIDerivedType(tag: DW_TAG_member, name: "align", scope: !223, file: !7, baseType: !16, size: 8, align: 8, offset: 64)
!228 = !DIDerivedType(tag: DW_TAG_member, name: "width", scope: !223, file: !7, baseType: !229, size: 128, align: 64, offset: 128)
!229 = !DICompositeType(tag: DW_TAG_union_type, name: "Option<usize>", scope: !35, file: !7, size: 128, align: 64, elements: !230, identifier: "b345589ef2841a1b12bc86f407f9ce3ef8d1468")
!230 = !{!231, !235}
!231 = !DIDerivedType(tag: DW_TAG_member, scope: !229, file: !7, baseType: !232, size: 64, align: 64)
!232 = !DICompositeType(tag: DW_TAG_structure_type, name: "None", scope: !35, file: !7, size: 64, align: 64, elements: !233, identifier: "b345589ef2841a1b12bc86f407f9ce3ef8d1468::None")
!233 = !{!234}
!234 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !232, file: !7, baseType: !34, size: 64, align: 64)
!235 = !DIDerivedType(tag: DW_TAG_member, scope: !229, file: !7, baseType: !236, size: 128, align: 64)
!236 = !DICompositeType(tag: DW_TAG_structure_type, name: "Some", scope: !35, file: !7, size: 128, align: 64, elements: !237, identifier: "b345589ef2841a1b12bc86f407f9ce3ef8d1468::Some")
!237 = !{!238, !239}
!238 = !DIDerivedType(tag: DW_TAG_member, name: "RUST$ENUM$DISR", scope: !236, file: !7, baseType: !34, size: 64, align: 64)
!239 = !DIDerivedType(tag: DW_TAG_member, name: "__0", scope: !236, file: !7, baseType: !54, size: 64, align: 64, offset: 64)
!240 = !DIDerivedType(tag: DW_TAG_member, name: "precision", scope: !223, file: !7, baseType: !229, size: 128, align: 64, offset: 256)
!241 = !DIDerivedType(tag: DW_TAG_member, name: "buf", scope: !223, file: !7, baseType: !242, size: 128, align: 64, offset: 384)
!242 = !DICompositeType(tag: DW_TAG_structure_type, name: "&mut Write", scope: !10, file: !7, size: 128, align: 64, elements: !81, identifier: "d5fcd5e514931588789147f514e06e82c192bf")
!243 = !DIDerivedType(tag: DW_TAG_member, name: "curarg", scope: !223, file: !7, baseType: !244, size: 128, align: 64, offset: 512)
!244 = !DICompositeType(tag: DW_TAG_structure_type, name: "Iter<core::fmt::ArgumentV1>", scope: !245, file: !7, size: 128, align: 64, elements: !246, identifier: "ea47deb041936c5471b78333e5fe2719f70c084")
!245 = !DINamespace(name: "slice", scope: !11, file: !7)
!246 = !{!247, !248, !249}
!247 = !DIDerivedType(tag: DW_TAG_member, name: "ptr", scope: !244, file: !7, baseType: !198, size: 64, align: 64)
!248 = !DIDerivedType(tag: DW_TAG_member, name: "end", scope: !244, file: !7, baseType: !198, size: 64, align: 64, offset: 64)
!249 = !DIDerivedType(tag: DW_TAG_member, name: "_marker", scope: !244, file: !7, baseType: !250, align: 8, offset: 128)
!250 = !DICompositeType(tag: DW_TAG_structure_type, name: "PhantomData<&core::fmt::ArgumentV1>", scope: !251, file: !7, align: 8, elements: !81, identifier: "ba419515969f2a8a1c712f0bc8abc41dbebe8c8")
!251 = !DINamespace(name: "marker", scope: !11, file: !7)
!252 = !DIDerivedType(tag: DW_TAG_member, name: "args", scope: !223, file: !7, baseType: !195, size: 128, align: 64, offset: 640)
!253 = !DIDerivedType(tag: DW_TAG_member, name: "length", scope: !195, file: !7, baseType: !54, size: 64, align: 64, offset: 64)
!254 = !DISubroutineType(types: !255)
!255 = !{!136, !45, !195}
!256 = !DILocalVariable(name: "pieces", arg: 1, scope: !134, file: !42, line: 1, type: !45)
!257 = !DILocation(line: 1, scope: !134)
!258 = !DILocalVariable(name: "args", arg: 2, scope: !134, file: !42, line: 1, type: !195)
!259 = !DILocalVariable(name: "pieces", scope: !260, file: !135, line: 239, type: !45)
!260 = distinct !DILexicalBlock(scope: !134, file: !135, line: 240, column: 63)
!261 = !DILocation(line: 239, scope: !260)
!262 = !DILocalVariable(name: "args", scope: !260, file: !135, line: 240, type: !195)
!263 = !DILocation(line: 240, scope: !260)
!264 = !DILocation(line: 239, scope: !134)
!265 = !DILocation(line: 240, scope: !134)
!266 = !DILocation(line: 242, scope: !260)
!267 = !DILocation(line: 243, scope: !260)
!268 = !DILocation(line: 244, scope: !260)
!269 = !DILocation(line: 241, scope: !260)
!270 = !DILocation(line: 246, scope: !260)
!271 = distinct !DISubprogram(name: "is_null<u8>", linkageName: "_ZN4core3ptr8{{impl}}11is_null<u8>E", scope: !273, file: !272, line: 404, type: !275, isLocal: false, isDefinition: true, scopeLine: 404, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !279, variables: !81)
!272 = !DIFile(filename: "/Users/rustbuild/src/rust-buildbot/slave/stable-dist-rustc-mac/build/obj/../src/libcore/ptr.rs", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!273 = !DINamespace(name: "{{impl}}", scope: !274, file: !7)
!274 = !DINamespace(name: "ptr", scope: !11, file: !7)
!275 = !DISubroutineType(types: !276)
!276 = !{!277, !278}
!277 = !DIBasicType(name: "bool", size: 8, align: 8, encoding: DW_ATE_boolean)
!278 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "*mut u8", baseType: !17, size: 64, align: 64)
!279 = !{!280}
!280 = !DITemplateTypeParameter(name: "T", type: !17)
!281 = !DILocalVariable(name: "self", arg: 1, scope: !271, file: !42, line: 1, type: !278)
!282 = !DILocation(line: 1, scope: !271)
!283 = !DILocalVariable(name: "self", scope: !284, file: !272, line: 404, type: !278)
!284 = distinct !DILexicalBlock(scope: !271, file: !272, line: 404, column: 48)
!285 = !DILocation(line: 404, scope: !284)
!286 = !DILocation(line: 404, scope: !271)
!287 = !DILocation(line: 405, scope: !284)
!288 = !DILocation(line: 406, scope: !284)
!289 = distinct !DISubprogram(name: "null_mut<u8>", linkageName: "_ZN4core3ptr12null_mut<u8>E", scope: !274, file: !272, line: 69, type: !290, isLocal: false, isDefinition: true, scopeLine: 69, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !279, variables: !81)
!290 = !DISubroutineType(types: !291)
!291 = !{!278}
!292 = !DILocation(line: 69, scope: !289)
!293 = distinct !DISubprogram(name: "exchange_malloc", linkageName: "_ZN5alloc4heap15exchange_mallocE", scope: !295, file: !294, line: 133, type: !296, isLocal: false, isDefinition: true, scopeLine: 133, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!294 = !DIFile(filename: "/Users/rustbuild/src/rust-buildbot/slave/stable-dist-rustc-mac/build/obj/../src/liballoc/heap.rs", directory: "/Users/stanislaw/Projects/mull/lab/rust")
!295 = !DINamespace(name: "heap", scope: !104, file: !7)
!296 = !DISubroutineType(types: !297)
!297 = !{!278, !54, !54}
!298 = !DILocalVariable(name: "size", arg: 1, scope: !293, file: !42, line: 1, type: !54)
!299 = !DILocation(line: 1, scope: !293)
!300 = !DILocalVariable(name: "align", arg: 2, scope: !293, file: !42, line: 1, type: !54)
!301 = !DILocalVariable(name: "size", scope: !302, file: !294, line: 133, type: !54)
!302 = distinct !DILexicalBlock(scope: !293, file: !294, line: 133, column: 64)
!303 = !DILocation(line: 133, scope: !302)
!304 = !DILocalVariable(name: "align", scope: !302, file: !294, line: 133, type: !54)
!305 = !DILocalVariable(name: "ptr", scope: !306, file: !294, line: 137, type: !278)
!306 = distinct !DILexicalBlock(scope: !302, file: !294, line: 137, column: 40)
!307 = !DILocation(line: 137, scope: !306)
!308 = !DILocation(line: 133, scope: !293)
!309 = !DILocation(line: 134, scope: !302)
!310 = !DILocation(line: 135, scope: !302)
!311 = !DILocation(line: 137, scope: !302)
!312 = !DILocation(line: 138, scope: !306)
!313 = !DILocation(line: 139, scope: !306)
!314 = !DILocation(line: 141, scope: !306)
!315 = !DILocation(line: 143, scope: !302)
!316 = distinct !DISubprogram(name: "allocate", linkageName: "_ZN5alloc4heap8allocateE", scope: !295, file: !294, line: 57, type: !296, isLocal: false, isDefinition: true, scopeLine: 57, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!317 = !DILocalVariable(name: "size", arg: 1, scope: !318, file: !42, line: 1, type: !54)
!318 = distinct !DISubprogram(name: "check_size_and_alignment", linkageName: "_ZN5alloc4heap24check_size_and_alignmentE", scope: !295, file: !294, line: 37, type: !319, isLocal: false, isDefinition: true, scopeLine: 37, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!319 = !DISubroutineType(types: !320)
!320 = !{null, !54, !54}
!321 = !DILocation(line: 1, scope: !318, inlinedAt: !322)
!322 = distinct !DILocation(line: 58, scope: !323)
!323 = distinct !DILexicalBlock(scope: !316, file: !294, line: 57, column: 61)
!324 = !DILocalVariable(name: "align", arg: 2, scope: !318, file: !42, line: 1, type: !54)
!325 = !DILocalVariable(name: "size", scope: !326, file: !294, line: 37, type: !54)
!326 = distinct !DILexicalBlock(scope: !318, file: !294, line: 37, column: 55)
!327 = !DILocation(line: 37, scope: !326, inlinedAt: !322)
!328 = !DILocalVariable(name: "align", scope: !326, file: !294, line: 37, type: !54)
!329 = !DILocalVariable(name: "size", arg: 1, scope: !316, file: !42, line: 1, type: !54)
!330 = !DILocation(line: 1, scope: !316)
!331 = !DILocalVariable(name: "align", arg: 2, scope: !316, file: !42, line: 1, type: !54)
!332 = !DILocalVariable(name: "size", scope: !323, file: !294, line: 57, type: !54)
!333 = !DILocation(line: 57, scope: !323)
!334 = !DILocalVariable(name: "align", scope: !323, file: !294, line: 57, type: !54)
!335 = !DILocation(line: 57, scope: !316)
!336 = !DILocation(line: 58, scope: !323)
!337 = !DILocation(line: 37, scope: !318, inlinedAt: !322)
!338 = !DILocation(line: 59, scope: !323)
!339 = !DILocation(line: 60, scope: !323)
!340 = distinct !DISubprogram(name: "sum", linkageName: "_ZN7example3sumE", scope: !43, file: !42, line: 2, type: !341, isLocal: true, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!341 = !DISubroutineType(types: !342)
!342 = !{!343, !343, !343}
!343 = !DIBasicType(name: "i32", size: 32, align: 32, encoding: DW_ATE_signed)
!344 = !DILocalVariable(name: "a", arg: 1, scope: !340, file: !42, line: 1, type: !343)
!345 = !DILocation(line: 1, scope: !340)
!346 = !DILocalVariable(name: "b", arg: 2, scope: !340, file: !42, line: 1, type: !343)
!347 = !DILocalVariable(name: "a", scope: !348, file: !42, line: 2, type: !343)
!348 = distinct !DILexicalBlock(scope: !340, file: !42, line: 2, column: 34)
!349 = !DILocation(line: 2, scope: !348)
!350 = !DILocalVariable(name: "b", scope: !348, file: !42, line: 2, type: !343)
!351 = !DILocation(line: 2, scope: !340)
!352 = !DILocation(line: 3, scope: !348)
!353 = !DILocation(line: 4, scope: !348)
!354 = distinct !DISubprogram(name: "just_print", linkageName: "_ZN7example10just_printE", scope: !43, file: !42, line: 6, type: !355, isLocal: true, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!355 = !DISubroutineType(types: !356)
!356 = !{null}
!357 = !DILocation(line: 7, scope: !354)
!358 = !DILocation(line: 8, scope: !354)
!359 = distinct !DISubprogram(name: "rusttest_foo_sum1", linkageName: "_ZN7example17rusttest_foo_sum1E", scope: !43, file: !42, line: 11, type: !355, isLocal: true, isDefinition: true, scopeLine: 11, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!360 = !DILocation(line: 12, scope: !359)
!361 = !DILocation(line: 13, scope: !359)
!362 = distinct !DISubprogram(name: "rusttest_foo_sum2", linkageName: "_ZN7example17rusttest_foo_sum2E", scope: !43, file: !42, line: 16, type: !355, isLocal: true, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!363 = !DILocation(line: 17, scope: !362)
!364 = !DILocation(line: 18, scope: !362)
!365 = distinct !DISubprogram(name: "rusttest_foo_sum3", linkageName: "_ZN7example17rusttest_foo_sum3E", scope: !43, file: !42, line: 21, type: !355, isLocal: true, isDefinition: true, scopeLine: 21, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!366 = !DILocation(line: 22, scope: !365)
!367 = !DILocation(line: 23, scope: !365)
!368 = distinct !DISubprogram(name: "rusttest_foo_sum4", linkageName: "_ZN7example17rusttest_foo_sum4E", scope: !43, file: !42, line: 26, type: !355, isLocal: true, isDefinition: true, scopeLine: 26, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!369 = !DILocation(line: 27, scope: !368)
!370 = !DILocation(line: 28, scope: !368)
!371 = distinct !DISubprogram(name: "main", linkageName: "_ZN7example6__test4mainE", scope: !372, file: !42, line: 1, type: !355, isLocal: true, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !3, templateParams: !81, variables: !81)
!372 = !DINamespace(name: "__test", scope: !43, file: !7)
!373 = !DILocation(line: 1, scope: !371)
