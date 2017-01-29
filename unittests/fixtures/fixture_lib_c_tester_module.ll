declare i32 @sum(i32, i32)
define i32 @test_main() {
entry:
  %result = alloca i32, align 4
  %result_matches = alloca i32, align 4
  %call = call i32 @sum(i32 3, i32 5)
  store i32 %call, i32* %result, align 4
  %0 = load i32, i32* %result, align 4
  %cmp = icmp eq i32 %0, 8
  %conv = zext i1 %cmp to i32
  store i32 %conv, i32* %result_matches, align 4
  %1 = load i32, i32* %result_matches, align 4
  ret i32 %1
}