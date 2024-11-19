declare i32 @printf(i8*, ...)
declare i32 @scanf(i8*, ...)

@.str1 = private constant [3 x i8] c"%d\00"
@.str2 = private constant [3 x i8] c"%c\00"

define i32 @getchar() {
getchar_entry:
    %x = alloca i8, align 1
    %1 = getelementptr [3 x i8], ptr @.str2, i32 0, i32 0
    %2 = call i32 @scanf(ptr %1, ptr %x)
    %3 = load i8, ptr %x
    %4 = sext i8 %3 to i32
    ret i32 %4
}

define i32 @getint() {
getint_entry:
    %xx = alloca i32, align 4
    %4 = getelementptr [3 x i8], ptr @.str1, i32 0, i32 0
    %5 = call i32 @scanf(ptr %4, ptr %xx)
    br label %clear_entry
clear_entry:
    %6 = call i32 @getchar()
    %7 = icmp eq i32 %6, 10
    br i1 %7, label %clear_entry, label %clear_out
clear_out:
    ret i32 %5
}

define void @putint(i32 %out) {
putint_entry:
    %8 = getelementptr [3 x i8], ptr @.str1, i32 0, i32 0
    %9 = call i32 @printf(ptr %8, i32 %out)
    ret void
}

define void @putchar(i32 %ch) {
putchar_entry:
    %10 = getelementptr [3 x i8], ptr @.str2, i32 0, i32 0
    %11 = call i32 @printf(ptr %10, i32 %ch)
    ret void
}

define void @putstr(i8* %str) {
putstr_entry:
    %12 = call i32 @printf(ptr %str)
    ret void
}