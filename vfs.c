#include "vfs.h"
#include <fcntl.h>
#include <stdio.h>
//make tree root global in shell
vnode_t *root;
enum {OREAD, OREADCREAT, OWRITE, OWRITECREATE, OAPPEND, OAPPENDCREAT};
int f_open(vnode_t *vn, char* path, int flag)
{
    //error-checking to do:
        //wrong flag
        //file does not exist (flag based)


    //for now, assume file exists and is a simple read flag
}


size_t f_read(vnode_t *vn, void *data, size_t size, int num, int fd)
{

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
int f_mount(vnode_t *vn, const char *type, const char *dir, int flags, void *data)
{

}
int f_umount(vnode_t *vn, const char *dir, int flags)
{

}
