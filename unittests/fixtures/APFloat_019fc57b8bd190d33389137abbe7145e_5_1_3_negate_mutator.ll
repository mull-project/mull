%"union.llvm::APFloat::Significand.135" = type { i64 }
%"struct.llvm::fltSemantics.134" = type { i16, i16, i32, i32 }
%"class.llvm::APFloat.136" = type <{ %"struct.llvm::fltSemantics.134"*, %"union.llvm::APFloat::Significand.135", i16, i8, [5 x i8] }>

declare i1 @_ZNK4llvm7APFloat12needsCleanupEv(%"class.llvm::APFloat.136"*)
declare void @_ZdaPv(i8*)

; Function Attrs: nounwind ssp uwtable
define void @_ZN4llvm7APFloat15freeSignificandEv(%"class.llvm::APFloat.136"* %this) #0 align 2 {
entry:
  %this.addr = alloca %"class.llvm::APFloat.136"*, align 8
  store %"class.llvm::APFloat.136"* %this, %"class.llvm::APFloat.136"** %this.addr, align 8
  %this1 = load %"class.llvm::APFloat.136"*, %"class.llvm::APFloat.136"** %this.addr, align 8
  %call = call zeroext i1 @_ZNK4llvm7APFloat12needsCleanupEv(%"class.llvm::APFloat.136"* %this1)
  br i1 %call, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %significand = getelementptr inbounds %"class.llvm::APFloat.136", %"class.llvm::APFloat.136"* %this1, i32 0, i32 1
  %parts = bitcast %"union.llvm::APFloat::Significand.135"* %significand to i64**
  %0 = load i64*, i64** %parts, align 8
  %isnull = icmp eq i64* %0, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %if.then
  %1 = bitcast i64* %0 to i8*
  call void @_ZdaPv(i8* %1) #9
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %if.then
  br label %if.end

if.end:                                           ; preds = %delete.end, %entry
  ret void
}
