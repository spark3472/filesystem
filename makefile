all: shell

shell: shell.c
	gcc -g -o shell shell.c -lreadline

clean:
	rm shell