#include "directory.h"
#include <stdio.h>
#include <stddef.h>

#define MAX_FT_SIZE 10000

//OREAD: open file for reading
//ORDWR: open file for reading and writing
//OWRITE: truncate file to zero or create file for writing
//OCREAT: open for for reading and writing. file is created if it does not exist
//OAPPEND: open for writing at end of file (stream is positioned at end). file is created if it does not exist
//ORDAD: open for reading or appending (end of file). file is created if it does not exist

enum {OREAD, ORDWR, OWRITE, OCREAT, OAPPEND, ORDAD};

//f_open returns file handle
//f_opendir returns directory handle


//stat struct taken from man fstat()
struct stat_t {
    int inode;
    int n_links;
    int uid;
    int gid;
    int size;
    int blocksize;
    int permissions;
    int mtime;
};

/*struct stat {
    //dev_t     st_dev;     /* ID of device containing file */
    //ino_t     st_ino;     /* inode number */
    //mode_t    st_mode;    /* protection */
    //nlink_t   st_nlink;   /* number of hard links */
    //uid_t     st_uid;     /* user ID of owner */
    //gid_t     st_gid;     /* group ID of owner */
    //dev_t     st_rdev;    /* device ID (if special file) */
    //off_t     st_size;    /* total size, in bytes */
    //blksize_t st_blksize; /* blocksize for file system I/O */
    //blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
    //time_t    st_atime;   /* time of last access */
    //time_t    st_mtime;   /* time of last modification */
    //time_t    st_ctime;   /* time of last status change */
//};

typedef struct vnode {
    int vnode_number;
    char name[255];
    struct vnode* child;
    struct vnode* next;
    int permissions;
    int type;
    //fs_driver_t* driver;  
    int inode;			/* hard coding for unix */
} vnode_t;

typedef struct fs_driver{
    int				    (*f_open)(char *path, int flags);
    size_t 				(*f_read)(void *ptr, size_t size, int num, int fd);
    size_t 				(*f_write)(vnode_t *vn, void *data, size_t size, int num, int fd);
    int 				(*f_close)(int fd);
    int 				(*f_seek)(vnode_t *vn, int offset, int whence, int fd);
    int 				(*f_rewind)(vnode_t *vn, int fd);
    int					(*f_stat)(vnode_t *vn, struct stat_t *buf, int fd);
    int					(*f_remove)(vnode_t *vn, const char *filename);
    int				    (*f_opendir)(vnode_t *vn, const char *filename);
    struct _DirEntry*	(*f_readdir)(vnode_t *vn, int* directory);
    int					(*f_closedir)(vnode_t *vn, int* directory);
    int					(*f_mkdir)(vnode_t *vn, const char *filename, int mode);
    int					(*f_rmdir)(vnode_t *vn, const char *filename);
    int					(*f_mount)(char* src, char* target);
    int					(*f_umount)(vnode_t *vn, const char *dir, int flags);
}fs_driver_t;



// to track opened files
typedef struct fileEntry{
    vnode_t* vn;
    int offset;
    int flag;
}fileEntry;

fileEntry fileTable[MAX_FT_SIZE];

typedef struct dirent{
    vnode_t* vn;
    int where;
    
}dirent;

/*struct dirent {
        //ino_t          d_ino;       /* Inode number */
        //off_t          d_off;       /* Not an offset; see below */
        //unsigned short d_reclen;    /* Length of this record */
        //unsigned char  d_type;      /* Type of file; not supported
                                        //by all filesystem types */
        //char           d_name[256]; /* Null-terminated filename */
//};

dirent dirTable[MAX_DT_SIZE];


int f_open( char* path, int flags);

size_t f_read(void *ptr, size_t size, int num, int fd);

size_t f_write(void *data, size_t size, int num, int fd);

int f_close(int fd);

int f_seek(int offset, int whence, int fd);

int f_rewind(int fd);

int f_stat(struct stat_t *buf, int fd); 

int f_remove(char *path);

int f_opendir(char *path);

DirEntry* f_readdir(int dirp);

int f_closedir(int dirp);

int f_mkdir(char* path, char* filename, int mode);

int f_rmdir(char* path);

int f_mount(char* filename, char* path_to_put);

int f_unmount(const char *dir, int flags);







