/*
    Creates a simple sample disks with one file in the root that contains "1 2 3"
    - currently just creates the file and doesn't deal with the root directory
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "format.h"

int main(int argc, char *argv[]) {
    //opening existing disk
    char *filename = "./DISK";
    FILE *inputfile = fopen(filename, "rwb");
    FILE *outputfile = fopen("SIMPLE_DISK", "wb");
    if(!inputfile) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    fseek(inputfile, 0L, SEEK_END);
    int size = ftell(inputfile);
    printf("File size is %d\n", size);
    rewind(inputfile);
    //reading disk into memory to modift
    void *disk = malloc(size);
    
    size_t bytes;
    bytes = fread(disk, 1, size, inputfile);
    if (bytes != size) {
        fprintf(stderr, "fread() failed: %zu\n", bytes);
        exit(EXIT_FAILURE);
    }
    fclose(inputfile);

    //info from superblock
    superblock *super = (superblock*)(disk + 512);
    int blockSize = super->size;
    int inode_offset = super->inode_offset;
    int data_offset = super->data_offset;
    int inode_start = 1024 + inode_offset*blockSize;
    int data_start = 1024 + data_offset*blockSize;

    //creating a file and updating the free info
    inode *firstInode = (inode*)(disk + inode_start);
    inode *firstFree = (inode*)(disk + super->free_inode);
    super->inode_offset++;
    super->free_inode = firstFree->next_inode;

    char *contents = "1 2 3";

    //putting in correct info in the inode
    firstFree->next_inode = -1;
    firstFree->size = sizeof(contents) / 8;
    firstFree->ctime = time(NULL);
    firstFree->dblocks[0] = *((int*)(disk + data_start + super->free_block * blockSize));
    //setting the first free block to the next one
    super->free_block = *((int*)(disk + data_start + super->free_block * blockSize));
    //made "file"
    memcpy((disk + data_start + firstFree->dblocks[0]), contents, sizeof(contents));

    //go to root directory
        //add a directory entry for "numbers"

    fwrite(disk, size, 1, outputfile);
    fclose(outputfile);

    free(disk);
}