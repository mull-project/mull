%"union.llvm::APFloat::Significand" = type { i64 }
%"struct.llvm::fltSemantics.134" = type { i16, i16, i32, i32 }
%"class.llvm::APFloat.136" = type <{ %"struct.llvm::fltSemantics.134"*, %"union.llvm::APFloat::Significand", i16, i8, [5 x i8] }>

declare i1 @_ZNK4llvm7APFloat5isNaNEv(%"class.llvm::APFloat.136"*)
declare i64* @_ZNK4llvm7APFloat16significandPartsEv(%"class.llvm::APFloat.136"*)
declare i32 @_ZN4llvm5APInt12tcExtractBitEPKyj(i64*, i32)

; Function Attrs: nounwind ssp uwtable
define zeroext i1 @_ZNK4llvm7APFloat11isSignalingEv(%"class.llvm::APFloat.136"* %this) #0 align 2 {
entry:
  %retval = alloca i1, align 1
  %this.addr = alloca %"class.llvm::APFloat.136"*, align 8
  store %"class.llvm::APFloat.136"* %this, %"class.llvm::APFloat.136"** %this.addr, align 8
  %this1 = load %"class.llvm::APFloat.136"*, %"class.llvm::APFloat.136"** %this.addr, align 8
  %call = call zeroext i1 @_ZNK4llvm7APFloat5isNaNEv(%"class.llvm::APFloat.136"* %this1)
  br i1 %call, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  store i1 false, i1* %retval, align 1
  br label %return

if.end:                                           ; preds = %entry
  %call2 = call i64* @_ZNK4llvm7APFloat16significandPartsEv(%"class.llvm::APFloat.136"* %this1)
  %semantics = getelementptr inbounds %"class.llvm::APFloat.136", %"class.llvm::APFloat.136"* %this1, i32 0, i32 0
  %0 = load %"struct.llvm::fltSemantics.134"*, %"struct.llvm::fltSemantics.134"** %semantics, align 8
  %precision = getelementptr inbounds %"struct.llvm::fltSemantics.134", %"struct.llvm::fltSemantics.134"* %0, i32 0, i32 2
  %1 = load i32, i32* %precision, align 4
  %sub = sub i32 %1, 2
  %call3 = call i32 @_ZN4llvm5APInt12tcExtractBitEPKyj(i64* %call2, i32 %sub)
  %tobool = icmp ne i32 %call3, 0
  %lnot = xor i1 %tobool, true
  store i1 %lnot, i1* %retval, align 1
  br label %return

return:                                           ; preds = %if.end, %if.then
  %2 = load i1, i1* %retval, align 1
  ret i1 %2
}
