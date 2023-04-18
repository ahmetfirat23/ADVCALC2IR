; ModuleID = 'advcalc2ir'
declare i32 @printf(i8*, ...)
@print.str = constant [4 x i8] c"%d\0A\00"

define i32 @main() {
	%siu = alloca i32
	store i32 11, i32* %siu
	%siuuu = alloca i32
	store i32 7, i32* %siuuu
	%reg1 = load i32, i32* %siu
	%reg2 = load i32, i32* %siuuu
	%reg3 = sdiv i32 %reg1, %reg2
	call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %reg3)
	%reg4 = load i32, i32* %siu
	%reg5 = load i32, i32* %siuuu
	%reg6 = mul i32 %reg4, %reg5
	%siu = alloca i32
	store i32 77, i32* %siu
	%reg7 = load i32, i32* %siu
	%reg8 = load i32, i32* %siu
	%reg9 = load i32, i32* %siu
	%reg10 = load i32, i32* %siu
	%reg11 = load i32, i32* %siu
	%reg12 = mul i32 %reg9, %reg10
	%reg13 = sdiv i32 %reg12, %reg11
	%reg14 = sub i32 %reg7, %reg8
	%reg15 = add i32 %reg14, %reg13
	call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %reg15)

	ret i32 0
}