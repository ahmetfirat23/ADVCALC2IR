file.ll:	advcalc2ir
		./advcalc2ir file.adv

advcalc2ir:	main.o
		gcc main.o -o advcalc2ir

main.o:		main.c
		gcc -c main.c
