/*
    Creates a simple sample disks with one file in the root that contains "abc"
    - currently just creates the file and doesn't deal with the root directory
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "format.h"
#include "directory.h"

int main(int argc, char *argv[]) {
    //opening existing disk
    char *filename = "./DISK";
    FILE *inputfile = fopen(filename, "rwb");
    if(!inputfile) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    fseek(inputfile, 0L, SEEK_END);
    int size = ftell(inputfile);
    rewind(inputfile);
    //reading disk into memory to modify
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
    int blockSize = super->block_size;
    int inode_offset = super->inode_offset;
    int data_offset = super->data_offset;
    int inode_start = 1024 + inode_offset*blockSize;
    int data_start = 1024 + data_offset*blockSize;


    inode *root = (inode*)(disk + inode_start);
    DirEntry *rootData = (DirEntry*)(disk + data_start + root->dblocks[0]*super->block_size);

    //creating a file and updating the free info
    inode *firstFree = (inode*)(disk + inode_start + super->free_inode*sizeof(inode));
    super->free_inode = firstFree->next_inode;

    char *contents = "abc";

    //putting in correct info in the inode
    firstFree->next_inode = -1;
    firstFree->size = sizeof(contents) / 8;
    firstFree->ctime = time(NULL);
    firstFree->dblocks[0] = *((int*)(disk + data_start + super->free_block * blockSize));
    //setting the first free block to the next one (which was stored as a pointer)
    super->free_block = *((int*)(disk + data_start + super->free_block * blockSize));
    //made "file"
    memcpy((disk + data_start + firstFree->dblocks[0]), contents, sizeof(contents));

    //set up directory entry
    strcpy(rootData->fileName,"letters.txt");
    rootData->inodeNum = 1;

    FILE *outputfile = fopen(filename, "wb");
    fwrite(disk, size, 1, outputfile);
    fclose(outputfile);

    free(disk);
}