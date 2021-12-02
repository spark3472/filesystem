#include <stdio.h>
#include <stdlib.h>
#include "format.h"

int main(int argc, char *argv[]) {
    char *filename = "./DISK";
    FILE *disk = fopen(filename, "rwb");
    if(!disk) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    superblock *super = (superblock*)(disk + 512);
    int inode_offset = super->inode_offset;
    int data_offset = super->data_offset;

    fclose(disk);

}