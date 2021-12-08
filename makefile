all: library shell format sampleDisk

setup:
	./format DISK 
	./sampleDisk

library: vfs.c vfs.h
	gcc -g -Wall -fpic -c vfs.c 
	gcc -shared -o libvfs.so vfs.o

shell: shell.c
	gcc -g -o shell shell.c -L. -lvfs -lreadline

shell2.0: shell.c
	gcc -Wall -g -o shell shell.c -lreadline

format: format.c
	gcc -g -o format format.c

sampleDisk: sampleDisk.c 
	gcc -g -o sampleDisk sampleDisk.c
	
clean:
	rm shell
	rm format
	rm sampleDisk
	rm vfs.o
	rm libvfs.so