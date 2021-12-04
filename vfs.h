#include "directory.h"
#include "format.h"
#include <stdio.h>
#include <stddef.h>

#define MAX_FT_SIZE 10000

enum {OREAD, OWRITE, RDWR, APPEND};


fileEntry fileTable[MAX_FT_SIZE];

//f_open returns file handle
//f_opendir returns directory handle

typedef struct {
    int				    (*f_open)(vnode_t *vn, const char *filename, int flags);
    size_t 				(*f_read)(vnode_t *vn, void *data, size_t size, int num, int fd);
    size_t 				(*f_write)(vnode_t *vn, void *data, size_t size, int num, int fd);
    int 				(*f_close)(vnode_t *vn, int fd);
    int 				(*f_seek)(vnode_t *vn, int offset, int whence, int fd);
    int 				(*f_rewind)(vnode_t *vn, int fd);
    int					(*f_stat)(vnode_t *vn, struct stat_t *buf, int fd);
    int					(*f_remove)(vnode_t *vn, const char *filename);
    int				    (*f_opendir)(vnode_t *vn, const char *filename);
    struct _DirEntry*	(*f_readdir)(vnode_t *vn, int* directory);
    int					(*f_closedir)(vnode_t *vn, int* directory);
    int					(*f_mkdir)(vnode_t *vn, const char *filename, int mode);
    int					(*f_rmdir)(vnode_t *vn, const char *filename);
    int					(*f_mount)(vnode_t *vn, const char *type, const char *dir, int flags, void *data);
    int					(*f_umount)(vnode_t *vn, const char *dir, int flags);
} fs_driver_t;

typedef struct vnode {
    int vnode_number;
    char name[255];
    struct vnode* parent;
    int permissions;
    int type;
    fs_driver_t* driver;  
    int inode;			/* hard coding for unix */
} vnode_t;

// to track opened files
typedef struct fileEntry{
    int entry;
    vnode_t* vn;
    int offset;
    int flag;
}fileEntry;









