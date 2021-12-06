all: shell format sampleDisk library

library: vfs.c
	gcc -g -Wall -fpic -c vfs.c 

	gcc -o -libvfs.so vfs.o -shared

shell2.0: shell.c
	gcc -g -o shell shell.c -L. -lvfs -lreadline

shell: shell.c
	gcc -Wall -g -o shell shell.c -lreadline

format: format.c
	gcc -g -o format format.c

sampleDisk: sampleDisk.c 
	gcc -g -o sampleDisk sampleDisk.c
	
clean:
	rm shell
	rm format
	rm sampleDisk