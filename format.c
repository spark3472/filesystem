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

    void* ptr = disk;
    ptr += 512;
    superblock* super = (superblock*)ptr;
    super->size = 512;
    super->inode_offset = 1;

    ptr += 512 + 1 * 512;//bootblock + superblock + blocksize * inode_offset
    
    int count = 0;


    for (int i = 0; i < 64; i++)
    {
        inode* node = (inode*)ptr;
        for (int j = 0; j < 10; j++)
        {
            count+=512;
            void* data_block = disk + 1024 + 1 * 512 + 64 * 100 + 256 + i * 512 + count;
            node->dblocks[j] = *((int*)data_block);
        }
         
        node->size = 0;
        ptr+=100;
    }

    FILE* fp = fopen("DISK", "w");

    size_t size = fwrite(disk, 1, MEGABYTE*num_megabytes, fp);

    fclose(fp);
    
    
    /*superblock* block = (superblock*)(disk + 512);
    inode* node = (inode*)(disk + 1024 + 1*512);
    printf("%d %d\n", block->size, node->size);*/

    /*FILE *check = fopen("DISK", "r");
    void* file = malloc(MEGABYTE*num_megabytes+1);
    
    size_t read = fread(file, 1, MEGABYTE*num_megabytes, check);

    superblock* to_check = file + 512;
    printf("superblock size: %d\n", to_check->size);

    printf("%zu %zu\n", size, read);
    if (fp != NULL)
    {
        printf("SUCCESS\n");
    }*/
}