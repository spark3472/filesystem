#include "vfs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "format.h"
#include <string.h>
#include <signal.h>
#include <math.h>


int main()
{
    int first_disk = f_mount("DISK", "/");
    if (first_disk == -1)
    {
        fprintf(stderr, "mounting of first disk failed\n");
        exit(FAILURE);
    }else{
        printf("mounting of first disk successful!\n");
    }


    int fd = f_open("/","letters.txt", ORDWR);
    if (fd == -1)
    {
        fprintf(stderr, "FNF\n");
        exit(EXIT_FAILURE);
    }

    struct stat_t* buf = malloc(sizeof(struct stat_t));

    int check = f_stat(buf, fd);
    printf("INODE: %d\n", buf->inode);
    printf("CHMOD: %d\n", buf->permissions);

    free(buf);

}