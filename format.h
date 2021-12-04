
#define N_DBLOCKS 10
#define N_IBLOCKS 4

enum {DIRECTORY, FILE, MOUNT};

typedef struct superblock 
{
    int size;
    int super_user;
    int regular_user;
    int inode_offset;
    int data_offset;
    int swap_offset;
    int free_inode;
    int free_block;

}superblock;

typedef struct inode {
    int next_inode;
    int protect;
    int file_type;
    int nlink;
    int size;
    int uid;
    int gid;
    int ctime;
    int atime;
    int dblocks[N_DBLOCKS];
    int iblocks[N_IBLOCKS];
    int i2block;
    int i3block;
}inode;

