; ModuleID = 'llvm-link'
source_filename = "llvm-link"

@ONE = constant i32 1
@ZERO = constant i32 0
@var2 = global i32 2
@var3 = global i32 3
@.str1 = private constant [3 x i8] c"%d\00"
@.str2 = private constant [3 x i8] c"%c\00"

define void @fun() {
entry_fun:
  %0 = alloca i32, align 4
  %1 = add i32 1, 0
  store i32 %1, i32* %0, align 4
  %2 = alloca i32, align 4
  %3 = add i32 1, 0
  store i32 %3, i32* %2, align 4
  br label %for_entry0

for_entry0:                                       ; preds = %entry_fun
  br label %for_in0

for_in0:                                          ; preds = %for_out0, %for_entry0
  %4 = load i32, i32* %2, align 4
  %5 = add i32 1000, 0
  %6 = icmp slt i32 %4, %5
  br i1 %6, label %for_body0, label %for_end0

for_body0:                                        ; preds = %for_in0
  %7 = load i32, i32* %2, align 4
  %8 = add i32 2, 0
  %9 = mul i32 %7, %8
  store i32 %9, i32* %2, align 4
  br label %for_out0

for_out0:                                         ; preds = %for_body0
  br label %for_in0

for_end0:                                         ; preds = %for_in0
  %10 = alloca [2 x i8], align 1
  store [2 x i8] c"\0A\00", [2 x i8]* %10, align 1
  %11 = getelementptr [2 x i8], [2 x i8]* %10, i32 0, i32 0
  call void @putstr(i8* %11)
  %12 = load i32, i32* %2, align 4
  call void @putint(i32 %12)
  ret void
}

define i32 @main() {
entry_main:
  %0 = alloca [10 x i8], align 1
  store [10 x i8] c"21373457\0A\00", [10 x i8]* %0, align 1
  %1 = getelementptr [10 x i8], [10 x i8]* %0, i32 0, i32 0
  call void @putstr(i8* %1)
  br label %if_entry1

if_entry1:                                        ; preds = %entry_main
  br label %and_entry2

and_entry2:                                       ; preds = %if_entry1
  %2 = alloca i1, align 1
  br label %and_left2

and_left2:                                        ; preds = %and_entry2
  %3 = load i32, i32* @ZERO, align 4
  %4 = load i32, i32* @var2, align 4
  %5 = add i32 %3, %4
  %6 = load i32, i32* @var3, align 4
  %7 = load i32, i32* @ONE, align 4
  %8 = sub i32 %6, %7
  %9 = icmp eq i32 %5, %8
  br i1 %9, label %and_right2, label %and_false2

and_right2:                                       ; preds = %and_left2
  %10 = load i32, i32* @ONE, align 4
  %11 = icmp ne i32 %10, 0
  br i1 %11, label %and_true2, label %and_false2

and_true2:                                        ; preds = %and_right2
  %12 = add i1 true, false
  store i1 %12, i1* %2, align 1
  br label %and_end2

and_false2:                                       ; preds = %and_right2, %and_left2
  %13 = add i1 false, false
  store i1 %13, i1* %2, align 1
  br label %and_end2

and_end2:                                         ; preds = %and_false2, %and_true2
  %14 = load i1, i1* %2, align 1
  br i1 %14, label %if_body1, label %if_end1

if_body1:                                         ; preds = %and_end2
  br label %if_entry3

if_entry3:                                        ; preds = %if_body1
  br label %or_entry4

or_entry4:                                        ; preds = %if_entry3
  %15 = alloca i1, align 1
  br label %or_left4

or_left4:                                         ; preds = %or_entry4
  %16 = load i32, i32* @ZERO, align 4
  %17 = icmp ne i32 %16, 0
  br i1 %17, label %or_true4, label %or_right4

or_right4:                                        ; preds = %or_left4
  br label %and_entry5

and_entry5:                                       ; preds = %or_right4
  %18 = alloca i1, align 1
  br label %and_left5

and_left5:                                        ; preds = %and_entry5
  %19 = load i32, i32* @ZERO, align 4
  %20 = icmp ne i32 %19, 0
  %21 = icmp eq i1 %20, false
  br i1 %21, label %and_right5, label %and_false5

and_right5:                                       ; preds = %and_left5
  %22 = load i32, i32* @ONE, align 4
  %23 = add i32 1, 0
  %24 = add i32 %22, %23
  %25 = load i32, i32* @var2, align 4
  %26 = add i32 %24, %25
  %27 = add i32 0, 0
  %28 = icmp slt i32 %26, %27
  br i1 %28, label %and_true5, label %and_false5

and_true5:                                        ; preds = %and_right5
  %29 = add i1 true, false
  store i1 %29, i1* %18, align 1
  br label %and_end5

and_false5:                                       ; preds = %and_right5, %and_left5
  %30 = add i1 false, false
  store i1 %30, i1* %18, align 1
  br label %and_end5

and_end5:                                         ; preds = %and_false5, %and_true5
  %31 = load i1, i1* %18, align 1
  br i1 %31, label %or_true4, label %or_false4

or_true4:                                         ; preds = %and_end5, %or_left4
  %32 = add i1 true, false
  store i1 %32, i1* %15, align 1
  br label %or_end4

or_false4:                                        ; preds = %and_end5
  %33 = add i1 false, false
  store i1 %33, i1* %15, align 1
  br label %or_end4

or_end4:                                          ; preds = %or_false4, %or_true4
  %34 = load i1, i1* %15, align 1
  br i1 %34, label %if_body3, label %else_body3

if_body3:                                         ; preds = %or_end4
  %35 = alloca [8 x i8], align 1
  store [8 x i8] c"ERROR!\0A\00", [8 x i8]* %35, align 1
  %36 = getelementptr [8 x i8], [8 x i8]* %35, i32 0, i32 0
  call void @putstr(i8* %36)
  br label %if_end3

else_body3:                                       ; preds = %or_end4
  %37 = alloca [14 x i8], align 1
  store [14 x i8] c"And success!\0A\00", [14 x i8]* %37, align 1
  %38 = getelementptr [14 x i8], [14 x i8]* %37, i32 0, i32 0
  call void @putstr(i8* %38)
  br label %if_end3

if_end3:                                          ; preds = %else_body3, %if_body3
  br label %if_end1

if_end1:                                          ; preds = %if_end3, %and_end2
  br label %if_entry6

if_entry6:                                        ; preds = %if_end1
  br label %or_entry7

or_entry7:                                        ; preds = %if_entry6
  %39 = alloca i1, align 1
  br label %or_left7

or_left7:                                         ; preds = %or_entry7
  %40 = load i32, i32* @var3, align 4
  %41 = add i32 3, 0
  %42 = icmp ne i32 %40, %41
  br i1 %42, label %or_true7, label %or_right7

or_right7:                                        ; preds = %or_left7
  %43 = load i32, i32* @var2, align 4
  %44 = add i32 22, 0
  %45 = sub i32 %43, %44
  %46 = add i32 20, 0
  %47 = sub i32 0, %46
  %48 = icmp eq i32 %45, %47
  br i1 %48, label %or_true7, label %or_false7

or_true7:                                         ; preds = %or_right7, %or_left7
  %49 = add i1 true, false
  store i1 %49, i1* %39, align 1
  br label %or_end7

or_false7:                                        ; preds = %or_right7
  %50 = add i1 false, false
  store i1 %50, i1* %39, align 1
  br label %or_end7

or_end7:                                          ; preds = %or_false7, %or_true7
  %51 = load i1, i1* %39, align 1
  br i1 %51, label %if_body6, label %if_end6

if_body6:                                         ; preds = %or_end7
  br label %if_entry8

if_entry8:                                        ; preds = %if_body6
  br label %or_entry9

or_entry9:                                        ; preds = %if_entry8
  %52 = alloca i1, align 1
  br label %or_left9

or_left9:                                         ; preds = %or_entry9
  %53 = load i32, i32* @ONE, align 4
  %54 = add i32 2, 0
  %55 = srem i32 %53, %54
  %56 = add i32 3, 0
  %57 = add i32 %55, %56
  %58 = add i32 4, 0
  %59 = add i32 2, 0
  %60 = mul i32 %58, %59
  %61 = sub i32 %57, %60
  %62 = load i32, i32* @var3, align 4
  %63 = add i32 %61, %62
  %64 = load i32, i32* @var2, align 4
  %65 = add i32 %63, %64
  %66 = add i32 100, 0
  %67 = icmp sle i32 %65, %66
  br i1 %67, label %or_true9, label %or_right9

or_right9:                                        ; preds = %or_left9
  %68 = load i32, i32* @ONE, align 4
  %69 = icmp ne i32 %68, 0
  br i1 %69, label %or_true9, label %or_false9

or_true9:                                         ; preds = %or_right9, %or_left9
  %70 = add i1 true, false
  store i1 %70, i1* %52, align 1
  br label %or_end9

or_false9:                                        ; preds = %or_right9
  %71 = add i1 false, false
  store i1 %71, i1* %52, align 1
  br label %or_end9

or_end9:                                          ; preds = %or_false9, %or_true9
  %72 = load i1, i1* %52, align 1
  br i1 %72, label %if_body8, label %if_end8

if_body8:                                         ; preds = %or_end9
  %73 = alloca [10 x i8], align 1
  store [10 x i8] c"Or pass!\0A\00", [10 x i8]* %73, align 1
  %74 = getelementptr [10 x i8], [10 x i8]* %73, i32 0, i32 0
  call void @putstr(i8* %74)
  br label %if_end8

if_end8:                                          ; preds = %if_body8, %or_end9
  br label %if_end6

if_end6:                                          ; preds = %if_end8, %or_end7
  %75 = alloca [15 x i8], align 1
  store [15 x i8] c"Test1 Success!\00", [15 x i8]* %75, align 1
  %76 = getelementptr [15 x i8], [15 x i8]* %75, i32 0, i32 0
  call void @putstr(i8* %76)
  call void @fun()
  call void @fun()
  call void @fun()
  call void @fun()
  call void @fun()
  call void @fun()
  %77 = add i32 0, 0
  ret i32 %77
}

define i32 @getchar() {
getchar_entry:
  %x = alloca i8, align 1
  %0 = getelementptr [3 x i8], [3 x i8]* @.str2, i32 0, i32 0
  %1 = call i32 (i8*, ...) @scanf(i8* %0, i8* %x)
  %2 = load i8, i8* %x, align 1
  %3 = sext i8 %2 to i32
  ret i32 %3
}

declare i32 @scanf(i8*, ...)

define i32 @getint() {
getint_entry:
  %x = alloca i32, align 4
  %0 = getelementptr [3 x i8], [3 x i8]* @.str1, i32 0, i32 0
  %1 = call i32 (i8*, ...) @scanf(i8* %0, i32* %x)
  br label %clear_entry

clear_entry:                                      ; preds = %clear_entry, %getint_entry
  %2 = call i32 @getchar()
  %3 = icmp ne i32 %2, 10
  br i1 %3, label %clear_entry, label %clear_out

clear_out:                                        ; preds = %clear_entry
  %4 = load i32, i32* %x, align 4
  ret i32 %4
}

define void @putint(i32 %out) {
putint_entry:
  %0 = getelementptr [3 x i8], [3 x i8]* @.str1, i32 0, i32 0
  %1 = call i32 (i8*, ...) @printf(i8* %0, i32 %out)
  ret void
}

declare i32 @printf(i8*, ...)

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
