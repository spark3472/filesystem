#include "vfs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "format.h"
#include <string.h>
#include <signal.h>
//make tree root global in shell
vnode_t *root;
int num_open_files = 0;
FILE* firstDisk;
void* disk;
//create signal handle/register to clean up fp and buffer upon user ending program?
void sighandler(int signo)
{

    free(disk);
}
vnode_t* find(char* path)
{
    vnode_t* traverse = root;
    if (strcmp(root->name, path) == 0)
    {
        printf("is root directory\n");
        return traverse;
    }else{
        char to_seperate[255];
        strcpy(to_seperate, path);
        const char delim[2] = "/";

        char* ptr = strtok(to_seperate, delim);
        for (traverse = root->child; traverse != NULL; traverse = traverse->next)
        {
            if (strcmp (traverse->name, ptr) == 0)
            {
                ptr = strtok(NULL, delim);
                if (ptr == NULL)
                {
                    return traverse;
                }else
                {
                    break;
                }
            }

        }
        while(ptr != NULL)
        {

            for(traverse = traverse->child; traverse != NULL; traverse = traverse->next)
            {
                if (strcmp(traverse->name, ptr) == 0)
                {
                    return traverse;
                }
            }
            ptr = strtok(NULL, delim);
            
        }
    }
    
}

int f_open( char* path, int flags)
{
    
    //error-checking to do:
        //wrong flag
        //file does not exist (flag based)
    vnode_t* vn = find(path);

    
    //find first inode
    void* ptr = disk;
    ptr += 1024 + 1 * 512;
    inode* node = (inode*)ptr;


   
    fileEntry entry;
    entry.flag = flags;//permissions
    entry.offset = 0; //position in stream
    entry.vn = vn;
    fileTable[num_open_files] = entry;
    
    //return handle of file/directory
    int handle = num_open_files;
    num_open_files++;
    return handle;
}


size_t f_read(vnode_t *vn, void *data, size_t size, int num, int fd)
{
    size_t to_read = size * num;



}

size_t f_write(vnode_t *vn, void *data, size_t size, int num, int fd)
{

}


int f_close(vnode_t *vn, int fd)
{
  
}


int f_seek(int offset, int whence, int fd)
{

}

int f_rewind(int fd)
{
  
}

int f_stat(struct stat_t *buf, int fd)
{
 
}

int f_remove(vnode_t *vn, const char *filename)
{
  
}



int f_opendir(char *path)
{
    
    vnode_t* node = malloc(sizeof(vnode_t));
    node = find(path);


}


struct _DirEntry* f_readdir(vnode_t *vn, int* dirp)
{

}
int f_closedir(vnode_t *vn, int* dirp)
{

}
int f_mkdir(vnode_t *vn, const char *filename, int mode)
{

}
int f_rmdir(vnode_t *vn, const char *filename)
{

}
int f_mount(char* filename, char* path_to_put)
{
    FILE *fp = fopen(filename, "r+");
    if (fp == NULL)
    {
        return -1;
    }
    //get size of disk
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    //read into buffer
    void* disk = malloc(size);
    fread(disk, 1, size, fp);
    rewind(fp);
    fclose(fp);

    //info from superblock
    superblock *super = (superblock*)(disk + 512);
    int blockSize = super->block_size;
    int inode_offset = super->inode_offset;
    int data_offset = super->data_offset;
    int inode_start = 1024 + inode_offset*blockSize;
    int data_start = 1024 + data_offset*blockSize;

    //find first inode
    void* ptr = disk;
    ptr += 1024 + inode_offset * blockSize;
    inode* node = (inode*)ptr;

    void *rootData = disk + data_start + (node->dblocks[0] * blockSize);
    if(strcmp(path_to_put, "/") == 0) {
        //make vnode for root
        firstDisk = fp;
        root = malloc(sizeof(vnode_t));
        strcpy(root->name, "/"); 
        DirEntry* dir = (DirEntry*)rootData;
        root->inode = 0;
        root->child = malloc(sizeof(vnode_t));
        strcpy(root->child->name, dir->fileName);
        root->child->inode = dir->inodeNum;

        vnode_t* temp = root->child;
        for (dir = dir->nextFile; dir != NULL; dir = dir->nextFile)
        {
            temp->next = malloc(sizeof(vnode_t));
            temp = temp->next;
            strcpy(temp->name, dir->fileName);
            temp->inode = dir->inodeNum;
        }
    }else{
        vnode_t* to_put = find(path_to_put);
        //mount at path_to_put
    }

    return 0;


}
int f_umount(vnode_t *vn, const char *dir, int flags)
{

}

int main(){

    printf("Mount %d\n",f_mount("DISK", "/"));
    printf("Open %d\n", f_open("/letters.txt", ORDWR));

    free(disk);
    free(root->child->next);
    free(root->child);
    free(root);

}
