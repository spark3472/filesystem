#define MAX_DT_SIZE 1000
/* Directory entry that is contained within a directory */
typedef struct _DirEntry {
    char fileName[256];	        //the file name (max 256 characters)
    int inodeNum;		        //the inode number of the file
    struct _DirEntry *nextFile;	//the next file in the directory
} DirEntry;

DirEntry dirTree[MAX_DT_SIZE];