all: shell format sampleDisk


shell: shell.c
	gcc -g -o shell shell.c -lreadline

format: format.c
	gcc -g -o format format.c

sampleDisk: sampleDisk.c 
	gcc -g -o sampleDisk sampleDisk.c
	
clean:
	rm shell
	rm format
	rm sampleDisk