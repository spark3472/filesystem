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
    //int diskSize = MEGABYTE*num_megabytes+1;
    int diskSize = MEGABYTE*num_megabytes;
    void* disk = malloc(diskSize);

    void* ptr = disk;

    //parameterize number of inodes so can adjust for bigger disks
    int numInodes = 64;

    //getting to start of superblock
    ptr += 512;
    superblock* super = (superblock*)ptr;
    super->block_size = 512;
    //why is it offset by 1?
    super->inode_offset = 1;
    super->data_offset = ((numInodes * sizeof(inode)) / super->block_size) + 1 + super->inode_offset;

    ptr += 512 + super->inode_offset * super->block_size; //bootblock + superblock + blocksize * inode_offset

    //setting up inode region
    /*int count = 0;
    inode* current = (inode*)ptr;
    inode* next = (inode*)(ptr + sizeof(inode));
    while (count < numInodes - 1) {
        current->nlink = 0;
        current->next_inode = *((int*)next);

        current++;
        next++;
        count++;
    }
    current->nlink = 0;
    current->next_inode = -1;
    */

    //setting up inode region attempt two - next_inode contains the number of the next
        //free inode instead of a pointer (what hw 6 does I think)
    int count = 0;
    inode* current = (inode*)ptr;
    while (count < numInodes-1) {
        current->nlink = 0;
        current->next_inode = count + 1;

        current++;
        count++;
    }
    current->nlink = 0;
    current->next_inode = -1;
    
    //setting up the free block region
    void* dataBlock = disk + super->data_offset * super->block_size;
    int blocks = 0;
    //while not exceding the disk size, each free block points to the next
    while ((dataBlock - disk) <= (diskSize - (super->block_size)*2)){
        *((int*)dataBlock) = blocks+1;
        blocks++;
        dataBlock += super->block_size;
    }
    //the last block points to 0
    *((int*)dataBlock) = -1;

    //setting up the root directory
    inode *root = disk + 1024 + super->inode_offset+super->block_size;
    root->next_inode = -1;
    root->nlink = 1;
    root->file_type = DIRECTORY_TYPE;
    //first data block
    root->dblocks[0] = 0;
    //reset to empty
    //*((int*)(disk + 1024 + super->data_offset+super->block_size)) = (int*)NULL;

    //start of frees are now first and second
    super->free_block = 1;
    super->free_inode = 1;

    FILE* fp = fopen("DISK", "wb");

    size_t size = fwrite(disk, 1, diskSize, fp);

    fclose(fp);
    
    free(disk);
    
    
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