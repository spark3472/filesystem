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

    printf("Change chmod from 777 to 300\n");

    int fd = f_open("/","letters.txt", ORDWR);
    if (fd == -1)
    {
        fprintf(stderr, "FNF\n");
        exit(EXIT_FAILURE);
    }

    vnode_t* first = fileTable[fd].vn;
    printf("chmod: %d\n", first->chmod);

    int change = change_chmod("/letters.txt", 300);
    if (change == -1)
    {
        fprintf(stderr, "did not change chmod\n");
        exit(EXIT_FAILURE);
    }else{
        printf("changed chmod!\n");
    }


    vnode_t* check = fileTable[fd].vn;
    printf("chmod after change: %d\n", check->chmod);

    void* ptr = malloc(4);
    size_t read = f_read(ptr, 1, 4, fd);
    if (read == -1)
    {
        printf("E_CHMOD, user does not have permission to read from file\n");
    }
   

    change = change_chmod("/letters.txt", 500);
    if (change == -1)
    {
        fprintf(stderr, "did not change chmod\n");
        exit(EXIT_FAILURE);
    }else{
        printf("changed chmod!\n");
    }

    size_t write = f_write(ptr, 1, 4, fd);
    if (write == -1)
    {
        printf("E_CHMOD, user does not have permission to write to file\n");
    }

     free(ptr);

}