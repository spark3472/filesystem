#include "format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define MEGABYTE 1024 * 1024

int main(int argc, char** argv)
{
    int num_megabytes = 1;

    if (argc == 1)//too few arguments
    {
        printf("too few arguments\n");
        exit(EXIT_FAILURE);
    }else if (strcmp(argv[1], "DISK") != 0)//file name must be disk
    {
        printf("file name must be disk\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 3)//1: file command, 2: disk name, 3: flag, 3: size of file in MB
    {
        exit(EXIT_FAILURE);
    }

    if (argc > 3)
    {
        if (strcmp(argv[2], "-s") == 0)
        {
            num_megabytes = atoi(argv[3]);
        }else
        {
            printf("%s is not an accepted flag\n", argv[2]);
        }
        
    }

    void* disk = malloc(MEGABYTE*num_megabytes+1);

    /*super->size;
    super->inode_offset;//inode region
    super->data_offset;//data region
    super->swap_offset;//end of file
    super->free_inode;//link to first inode
    super->free_block;//link to head of free block list*/

    void* ptr = disk;
    ptr += 512;
    superblock* super = (superblock*)ptr;
    super->size = 512;
    super->inode_offset = 1;



    FILE* fp = fopen("DISK", "w");
    int check = ftruncate(fileno(fp), MEGABYTE*num_megabytes);
    
    
    superblock* block = (superblock*)(disk + 512);

    void* for_inode = disk;
    for_inode += 1024 + 1 * 512;//bootblock + superblock + blocksize * inode_offset

    inode* node = (inode*)for_inode;
    

    printf("%d\n", block->size);
    

    if (fp != NULL && check == 0)
    {
        printf("SUCCESS\n");
    }
}