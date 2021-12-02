all: shell format


shell: shell.c
	gcc -g -o shell shell.c -lreadline

format: format.c
	gcc -g -o format format.c
	
clean:
	rm shell
	rm format