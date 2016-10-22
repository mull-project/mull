@.str = private unnamed_addr constant [6 x i8] c"ababa\00", align 1
define i32 @test_count_letters() {
	entry:
	%call = call i32 @count_letters(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i8 signext 97)
	%cmp = icmp eq i32 %call, 3
	%conv = zext i1 %cmp to i32
	ret i32 %conv
}
declare i32 @count_letters(i8*, i8 signext)

