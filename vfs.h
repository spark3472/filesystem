#include "directory.h"
#include <stdio.h>
#include <stddef.h>

#define MAX_FT_SIZE 10000



#define FAILURE -1
#define SUCCESS 0

#define TRUE  1
#define FALSE 0

//OREAD: open file for reading
//ORDWR: open file for reading and writing
//OWRITE: truncate file to zero or create file for writing
//OCREAT: open for for reading and writing. file is created if it does not exist
//OAPPEND: open for writing at end of file (stream is positioned at end). file is created if it does not exist
//ORDAD: open for reading or appending (end of file). file is created if it does not exist

enum {OREAD, ORDWR, OWRITE, OCREAT, OAPPEND, ORDAD};

//USERS
char *superuser = "bmcadmin";
char *superpwd = "bmcadmin";

char *regularuser = "bmcguest";
char *regularpwd = "bmcguest";

//stat struct taken from man fstat()
struct stat_t {
    int inode;
    int n_links;
    int uid;
    int gid;
    int size;
    int blocksize;
    int permissions;    //chmod permissions
    int mtime;
};

typedef struct vnode {
    int disk;
    char name[255];
    struct vnode* child;
    struct vnode* next;
    int permissions;    //holds read/write permissions
    int chmod;          //holds chmod permissions
    int type;
    //fs_driver_t* driver;  
    int inode;			/* hard coding for unix */
} vnode_t;

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

dirent *dirTable[MAX_DT_SIZE];


int f_open( char* path, char* filename, int flag);

size_t f_read(void *ptr, size_t size, int num, int fd);

size_t f_write(void *data, size_t size, int num, int fd);

int f_close(int fd);

int f_seek(int offset, int whence, int fd);

int f_rewind(int fd);

int f_rewinddir(int dirp);

int f_stat(struct stat_t *buf, int fd); 

int f_remove(char *path, char *filename);

int f_opendir(char *path);

DirEntry* f_readdir(int dirp);

int f_closedir(int dirp);

int f_mkdir(char* path, char* filename, int mode);

int f_rmdir(char* path, char *filename);

int f_mount(char* filename, char* path_to_put);

int f_unmount(const char *dir, int flags);

int remove_mention_of(char *path, char *filename);

int change_chmod(char* path, int chmod);





/*typedef struct fs_driver{
    int				    (*f_open)(char *path, char* filename, int flag);
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
}fs_driver_t;*/

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
