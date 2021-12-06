#include "vfs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "format.h"
#include <string.h>
//make tree root global in shell
vnode_t *root;
int num_open_files = 0;
FILE* fp;

vnode_t* find(char* path)
{
    //theoretical example of root directory 
    root = malloc(sizeof(vnode_t));
    vnode_t* child = malloc(sizeof(vnode_t));
    strcpy(child->name, "user0");
    strcpy(root->name, "/");
    root->child = child;
    vnode_t* correct_child = malloc(sizeof(vnode_t));
    strcpy(correct_child->name, "user1");
    child->next = correct_child;
    vnode_t* answer = malloc(sizeof(vnode_t));
    strcpy(answer->name, "a");
    correct_child->child = answer;


    
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
    printf("%s\n", vn->name);
    /*int inode_num = vn->inode;
    

    //get size of disk
    FILE* fp = fopen("DISK", "r");
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    //read into buffer
    void* disk = malloc(size);
    fread(disk, 1, size, fp);

    //find first inode
    void* ptr = disk;
    ptr += 1024 + 1 * 512;
    inode* node = (inode*)ptr;


    //print 
    if (node->file_type == FILE_TYPE)
    {
        fileEntry entry;
        entry.flag = flags;//permissions
        entry.offset = 0; //position in stream
        entry.vn = vn;
        fileTable[num_open_files] = entry;
        num_open_files++;

    }else if (node->file_type == DIRECTORY_TYPE)
    {
        //make a directory entry
    }
    
    //return handle of file/directory
    int handle; 
    return handle;*/
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



int f_opendir(vnode_t *vn, const char *filename){
  
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
int f_mount(char* filename)
{
    fp = fopen("DISK", "r+");
    if (fp == NULL)
    {
        return -1;
    }

}
int f_umount(vnode_t *vn, const char *dir, int flags)
{

}

int main(){


    f_open("/user1/a", ORDWR);

}
