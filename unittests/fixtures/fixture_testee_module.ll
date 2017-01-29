define i32 @count_letters(i8* %s, i8 signext %c) {
entry:
  %s.addr = alloca i8*, align 8
  %c.addr = alloca i8, align 1
  %count = alloca i32, align 4
  %found = alloca i8*, align 8
  store i8* %s, i8** %s.addr, align 8
  store i8 %c, i8* %c.addr, align 1
  store i32 0, i32* %count, align 4
  %0 = load i8*, i8** %s.addr, align 8
  store i8* %0, i8** %found, align 8
  br label %while.cond

while.cond: ; preds = %while.body, %entry
  %1 = load i8*, i8** %found, align 8
  %2 = load i8, i8* %c.addr, align 1
  %conv = sext i8 %2 to i32
  %call = call i8* @strchr(i8* %1, i32 %conv)
  store i8* %call, i8** %found, align 8
  %tobool = icmp ne i8* %call, null
  br i1 %tobool, label %while.body, label %while.end

while.body: ; preds = %while.cond
  %3 = load i32, i32* %count, align 4
  %add = add nsw i32 %3, 1
  store i32 %add, i32* %count, align 4
  %4 = load i8*, i8** %found, align 8
  %add.ptr = getelementptr inbounds i8, i8* %4, i64 1
  store i8* %add.ptr, i8** %found, align 8
  br label %while.cond

while.end:  ; preds = %while.cond
  %5 = load i32, i32* %count, align 4
  ret i32 %5
}

declare i8* @strchr(i8*, i32)