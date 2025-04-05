; Function Attrs: nounwind ssp uwtable
define i32 @_ZN4llvm5APInt12tcExtractBitEPKyj(i64* %parts, i32 %bit) #0 align 2 {
entry:
  %parts.addr = alloca i64*, align 8
  %bit.addr = alloca i32, align 4
  store i64* %parts, i64** %parts.addr, align 8
  store i32 %bit, i32* %bit.addr, align 4
  %0 = load i32, i32* %bit.addr, align 4
  %div = udiv i32 %0, 64
  %idxprom = zext i32 %div to i64
  %1 = load i64*, i64** %parts.addr, align 8
  %arrayidx = getelementptr inbounds i64, i64* %1, i64 %idxprom
  %2 = load i64, i64* %arrayidx, align 8
  %3 = load i32, i32* %bit.addr, align 4
  %rem = urem i32 %3, 64
  %sh_prom = zext i32 %rem to i64
  %shl = shl i64 1, %sh_prom
  %and = and i64 %2, %shl
  %cmp = icmp ne i64 %and, 0
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}
