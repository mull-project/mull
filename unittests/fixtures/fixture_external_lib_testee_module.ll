%struct.sqlite3 = type opaque

@.str = private unnamed_addr constant [9 x i8] c":memory:\00", align 1

define i32 @in_memory_db() #0 {
entry:
  %db = alloca %struct.sqlite3*, align 8
  %call = call i32 @sqlite3_open(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), %struct.sqlite3** %db)
  ret i32 %call
}

declare i32 @sqlite3_open(i8*, %struct.sqlite3**) #1