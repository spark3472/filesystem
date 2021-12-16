all: library shell format sampleDisk

setup:
	./format DISK 
	./sampleDisk

library: vfs.c vfs.h
	gcc -g -fpic -c vfs.c -lm
	gcc -shared -o libvfs.so vfs.o -lm

shell: shell.c
	gcc -g -o shell shell.c -L. -lvfs -lreadline

shell2.0: shell.c
	gcc -Wall -g -o shell shell.c -lreadline

format: format.c
	gcc -g -o format format.c

sampleDisk: sampleDisk.c 
	gcc -g -o sampleDisk sampleDisk.c

sampleDisk_users: sampleDisk_users.c 
	gcc -g -o sampleDisk_users sampleDisk_users.c
	
vfs: vfs.c vfs.h
	gcc -g -o vfs vfs.c -lm

librarySetup:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.

clean:
	rm shell
	rm format
	rm sampleDisk
	rm vfs.o
	rm libvfs.so