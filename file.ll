%x = alloca i32
store i32 3, i32* %x
%y = alloca i32
store i32 5, i32* %y
%reg1 = load i32, i32* %x
%reg2 = load i32, i32* %y
%reg3 = add i32 1, %reg2
%reg4 = mul i32 %reg1, 6
%reg5 = add i32 23, %reg4
%zvalue = alloca i32
store i32 41, i32* %zvalue
%reg6 = load i32, i32* %zvalue
41
%reg7 = load i32, i32* %x
%reg8 = load i32, i32* %y
%reg9 = load i32, i32* %zvalue
%reg10 = sub i32 %reg7, %reg8
%reg11 = sub i32 %reg10, %reg9
%k = alloca i32
store i32 -43, i32* %k
%reg12 = load i32, i32* %x
%reg13 = load i32, i32* %y
%reg14 = add i32 2, 5
%reg15 = mul i32 1, 7
%reg16 = mul i32 3, %reg13
%reg17 = mul i32 %reg16, 7
%reg18 = add i32 %reg12, %reg17
%k = alloca i32
store i32 108, i32* %k
%reg19 = load i32, i32* %k
%reg20 = add i32 %reg19, 1
109
