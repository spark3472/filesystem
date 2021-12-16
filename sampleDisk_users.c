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
    DirEntry *firstUser = rootData;
    DirEntry *secondUser = rootData + 1;

    //creating a file and updating the free info
    inode *firstFree = (inode*)(disk + inode_start + super->free_inode*sizeof(inode));
    super->free_inode++;

    //putting in correct info in the inode
    firstFree->next_inode = -1;
    firstFree->size = 4096;
    firstFree->mtime = time(NULL);
    firstFree->file_type = DIRECTORY_TYPE;

    firstFree->dblocks[0] = super->free_block;
    super->free_block++;
    void* free_block = disk + data_start + firstFree->dblocks[0] * blockSize;//find block
    int ptr = *(int*)free_block;//get ptr to next block 
    super->free_block = ptr;//assign next free block ptr to super->free_block

    //set it up with a directory structure
    DirEntry* first_entry = (DirEntry*)(disk + data_start + firstFree->dblocks[0] * blockSize);
    first_entry->inodeNum = -1;
    first_entry = NULL;


    //second user
    //creating a file and updating the free info
    inode *secondFree = (inode*)(disk + inode_start + super->free_inode*sizeof(inode));
    super->free_inode++;

    //putting in correct info in the inode
    secondFree->next_inode = -1;
    secondFree->size = 4096;
    secondFree->mtime = time(NULL);
    secondFree->file_type = DIRECTORY_TYPE;

    secondFree->dblocks[0] = super->free_block;
    super->free_block++;
    void* free_block2 = disk + data_start + secondFree->dblocks[0] * blockSize;//find block
    int ptr2 = *(int*)free_block2;//get ptr to next block 
    super->free_block = ptr2;//assign next free block ptr to super->free_block

    //set it up with a directory structure
    DirEntry* first_entry2 = (DirEntry*)(disk + data_start + secondFree->dblocks[0] * blockSize);
    first_entry2->inodeNum = -1;
    first_entry2 = NULL;

    //set up directory entries
    strcpy(firstUser->fileName,"bmcadmin");
    firstUser->inodeNum = 1;

    strcpy(secondUser->fileName,"bmcguest");
    secondUser->inodeNum = 2;
    secondUser->nextFile = NULL;
    firstUser->nextFile = secondUser;

    rootData = firstUser;

    FILE *outputfile = fopen(filename, "wb");
    fwrite(disk, size, 1, outputfile);
    fclose(outputfile);

    free(disk);
}