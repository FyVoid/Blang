declare i32 @printf(i8*, ...)
declare i32 @scanf(i8*, ...)

@.str1 = private constant [3 x i8] c"%d\00"
@.str2 = private constant [3 x i8] c"%c\00"

define i32 @getchar() {
getchar_entry:
    %x = alloca i8, align 1
    %0 = getelementptr [3 x i8], [3 x i8]* @.str2, i32 0, i32 0
    %1 = call i32 (i8*, ...) @scanf(i8* %0, i8* %x)
    %2 = load i8, i8* %x
    %3 = sext i8 %2 to i32
    ret i32 %3
}

define i32 @getint() {
getint_entry:
    %x = alloca i32, align 4
    %0 = getelementptr [3 x i8], [3 x i8]* @.str1, i32 0, i32 0
    %1 = call i32 (i8*, ...) @scanf(i8* %0, i32* %x)
    br label %clear_entry
clear_entry:
    %2 = call i32 @getchar()
    %3 = icmp ne i32 %2, 10
    br i1 %3, label %clear_entry, label %clear_out
clear_out:
    %4 = load i32, i32* %x
    ret i32 %4
}

define void @putint(i32 %out) {
putint_entry:
    %0 = getelementptr [3 x i8], [3 x i8]* @.str1, i32 0, i32 0
    %1 = call i32 (i8*, ...) @printf(i8* %0, i32 %out)
    ret void
}

define void @putchar(i32 %ch) {
putchar_entry:
    %0 = getelementptr [3 x i8], [3 x i8]* @.str2, i32 0, i32 0
    %1 = call i32 (i8*, ...) @printf(i8* %0, i32 %ch)
    ret void
}

define void @putstr(i8* %str) {
putstr_entry:
    %0 = call i32 (i8*, ...) @printf(i8* %str)
    ret void
}