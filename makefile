all:		myexec
		./myexec

myexec:		file.s
		clang file.s -o myexec

file.s: 	file.ll
		llc file.ll -o file.s

file.ll:	advcalc2ir
		./advcalc2ir file.adv

advcalc2ir:	main.o
		gcc main.o -o advcalc2ir

main.o:		main.c
		gcc -c main.c
		
clean:
	rm -f *.o advcalc2ir *.ll *.s myexec
