make: microshell.c
	gcc -Wall -ansi -o shell microshell.c

clean: shell
	rm shell
