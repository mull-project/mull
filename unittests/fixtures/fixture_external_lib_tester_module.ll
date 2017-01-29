define i32 @test_in_memory_db() #0 {
entry:
  %call = call i32 (...) @in_memory_db()
  %cmp = icmp eq i32 %call, 0
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

declare i32 @in_memory_db(...)