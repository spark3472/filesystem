#include "vfs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "format.h"
#include <string.h>
#include <signal.h>

#define FAILURE -1
#define SUCCESS 0

int m_error;

//make tree root global in shell
vnode_t *root;
int num_open_files = 0;
int num_open_dir = 0;
int blockSize;
int data_start;
int inode_start;
FILE* firstDisk;
void* disk;
//create signal handle/register to clean up fp and buffer upon user ending program?
void sighandler(int signo)
{

    free(disk);
    //free vnode tree
}
vnode_t* find(char* path)
{
    vnode_t* traverse = root;

    if(root == NULL) {
        return NULL;
    }

    if (strcmp(root->name, path) == 0)
    {
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
            if(traverse != NULL) {
                for(traverse = traverse->child; traverse != NULL; traverse = traverse->next)
                {
                    if (strcmp(traverse->name, ptr) == 0)
                    {
                        //printf("%d\n", traverse->inode);
                        return traverse;
                    }
                }
                ptr = strtok(NULL, delim);
            } else {
                return NULL;
            }
            
        }
    }
    
}

int f_open( char* path, int flags)
{
    
    //error-checking to do:
        //wrong flag
        //file does not exist (flag based)

    if (num_open_files == MAX_FT_SIZE)
    {
        //set m_error to max files opened
    }

    vnode_t* vn = find(path);
    if(vn == NULL) {
        fprintf(stderr, "f_open: Error finding file\n");
        return FAILURE;
    }
    //printf("%d\n", vn->inode);
    
    fileEntry entry;
    entry.flag = flags;//permissions
    entry.offset = 0; //position in stream
    entry.vn = vn;
    for (int i = 0; i < MAX_FT_SIZE; i++)
    {
        fileEntry try = fileTable[i];
        if (try.vn == NULL)
        {
            fileTable[i] = entry;
            num_open_files++;// TO-DO: for checking if max number of files have been opened later
            return i;
        }
    }
}


size_t f_read(void *ptr, size_t size, int num, int fd)
{
    size_t data_to_read = size * num;
    fileEntry to_read = fileTable[fd];
    size_t readSize = size;
    int readNum = num;

    if(to_read.vn == NULL) {
        //fprintf(stderr, "f_read: no file found\n");
        return FAILURE;
    }
    
    void* node = disk;
    node += inode_start + fileTable[fd].vn->inode * sizeof(inode);
    inode* iNode = (inode*)node;

    node = disk + data_start + iNode->dblocks[0] * blockSize + to_read.offset;
    
    //printf("\noffset: %d, size: %d; to read: %ld * %d\n", to_read.offset, iNode->size, size, num);
    if(to_read.offset >= iNode->size) {
        return 0;
    } else if (to_read.offset + data_to_read > iNode->size) {
        void *end = disk + data_start + iNode->dblocks[0] * blockSize + (iNode->size - 1);
        for(int i = 0; i < 7; i++) {
            if(*((int*)end) == EOF) {
                //printf("EOF detecting %d from the end\n", i);
                fileTable[fd].offset = iNode->size;
                break;
            }
            end -= 1;
        }
    } else if((iNode->size - (to_read.offset + data_to_read)) < 8) {
        void *closeToEnd = disk + data_start + iNode->dblocks[0] * blockSize + to_read.offset;
        for(int i = 0; i < data_to_read; i++) {
            if(*((int*)closeToEnd) == EOF) {
                //printf("EOF detected %d from current location\n", i);
                fileTable[fd].offset = iNode->size;
                if(i == 0) {
                    return 0;
                }
                break;
            }
            closeToEnd += 1;
        }
    }
    
    //printf("Contents: %s\n", (char*)node);

    //ptr = malloc(data_to_read);
    //memccpy(ptr, node, readNum, readSize);
    memccpy(ptr, node, EOF, readSize*readNum);
    fileTable[fd].offset += data_to_read;
    return readNum*readSize;
}

size_t f_write(void *data, size_t size, int num, int fd)
{
    //how many blocks needed
    //check fd
    //

}


int f_close(int fd)
{
    if (num_open_files == 0)
    {
        //set m_error to EOF
    }
    fileEntry to_close = fileTable[fd];
    to_close.vn = NULL;
    num_open_files--;
    return 0;
}


int f_seek(int offset, int whence, int fd)
{
    if (num_open_files == 0 || fd > num_open_files || fd < 0)
    {
        //set m_error to file non existent
        return -1;
    }
    fileEntry to_seek = fileTable[fd];
    if (whence == SEEK_END)
    {
        void* node = disk;
        node += inode_start + to_seek.vn->inode * sizeof(inode);
        inode* iNode = (inode*)node;
        if (offset != 0)//can offset be negative?
        {
            //set m_error to attempt to access past end of file
            return -1;
        }
        to_seek.offset = iNode->size - offset;
    }else if (whence == SEEK_SET)
    {
        
    }
    


}

int f_rewind(int fd)
{
    if (num_open_files == 0 || fd > num_open_files || fd < 0)
    {
        //set m_error to file non existent
        return -1;
    }
    fileEntry to_rewind = fileTable[fd];
    to_rewind.offset = 0;

  
}

int f_stat(struct stat_t *buf, int fd)
{
    fileEntry to_stat = fileTable[fd];
    if (to_stat.vn == NULL)
    {
        //set m_error to EOF
        return -1;
    }

    buf->blocksize = blockSize;
    buf->inode = to_stat.vn->inode;

    void* node = disk;
    node += inode_start + fileTable[fd].vn->inode * sizeof(inode);
    inode* iNode = (inode*)node;

    buf->gid = iNode->gid;
    buf->n_links = iNode->nlink;
    buf->size = iNode->size;
    buf->uid = iNode->uid;
    buf->permissions = to_stat.vn->permissions;
    buf->mtime = iNode->mtime;

    return 0;
    
 
}

int f_remove(char *path)
{
  
}



int f_opendir(char *path)
{
    if (num_open_dir == MAX_DT_SIZE)
    {
        //set m_error to full
        return -1;
    }
    
    vnode_t* node = malloc(sizeof(vnode_t));
    node = find(path);
    dirent to_add;
    to_add.vn = node;
    to_add.where = 0;
    
    for (int i = 0; i < MAX_DT_SIZE; i++)
    {
        dirent try = dirTable[i];
        if (try.vn == NULL)
        {
            dirTable[i] = to_add;
            num_open_dir++;
            return i;
        }
    }

}

//function returns a pointer to a dirent structure representing 
//the next directory entry in the directory stream pointed to by dirp.
DirEntry* f_readdir(int dirp)
{
    if (dirp >= MAX_DT_SIZE )
    {
        //end of stream
        return NULL;

    }
    dirent find_entry = dirTable[dirp];
    vnode_t* find_inode = find_entry.vn;

    void* to_inode = disk + inode_start + find_inode->inode * blockSize;
    inode* iNode = (inode*)to_inode;

    void* to_data = disk + data_start + iNode->dblocks[0] * blockSize;
    DirEntry* child = (DirEntry*)to_data;

    for (int i = 0; i < find_entry.where; i++)
    {
        child = child->nextFile;
    }
    dirTable[dirp].where++;
    return child;


}

int f_closedir(int dirp)
{
    if (num_open_dir == 0)
    {
        //set m_error to no open files
        return -1;
    }
    dirent to_close = dirTable[dirp];
    to_close.vn = NULL;
    num_open_dir--;
    return 0;

}
int f_mkdir(char* path, char* filename, int mode)
{
    vnode_t* dircurrent = malloc(sizeof(vnode_t));
    dircurrent = find(path);
    superblock *super = (superblock*)(disk + 512);

    //make new vnode
    vnode_t* new = malloc(sizeof(vnode_t));
    new->inode = super->free_inode;
    strcpy(new->name, filename);
    new->permissions = mode;
    new->type = DIRECTORY_TYPE;
    new->next = NULL;
    new->child = NULL;

    vnode_t* find_end = dircurrent->child;
    while (find_end->next != NULL)
    {
        find_end = find_end->next;
    }
    
    find_end->next = new;

    //Find current Directory Entry on physical system
    void* node = disk + inode_start + dircurrent->inode * sizeof(inode);
    inode* iNode = (inode*)node;
    void* to_data = disk + data_start + iNode->dblocks[0] * blockSize;

    //find end of siblings
    DirEntry* traverse = (DirEntry*)to_data;
    while (traverse->nextFile != NULL)
    {
        traverse = traverse->nextFile;
    }

    //make new directory entry for physical system
    DirEntry* to_add = malloc(sizeof(DirEntry));
    strcpy(to_add->fileName, filename);
    to_add->inodeNum = super->free_inode;
    to_add->nextFile = NULL;
    traverse->nextFile = to_add;

    //update free inode list
    void* next_free = disk + inode_start + super->free_inode * sizeof(inode);
    inode* next = (inode*)next_free;
    super->free_inode = next->next_inode;

    
    return 0;

}
int f_rmdir(char* path)
{
    vnode_t* vn = malloc(sizeof(vnode_t));
    vn = find(path);

    void* node = disk;
    node += inode_start + vn->inode * sizeof(inode);
    inode* iNode = (inode*)node;
    int num_blocks_to_free = iNode->size/blockSize;
    int num_direct_blocks = 0;
    if (num_blocks_to_free > 10)
    {
        num_direct_blocks = 10;
        num_blocks_to_free -= 10;//indirect blocks
    }else
    {
        num_direct_blocks = num_blocks_to_free;
    }
    superblock* super = (superblock*)(disk+512);
    for (int i = 0; i < num_blocks_to_free; i++)
    {
        void* datablock = disk + data_start + iNode->dblocks[i] * blockSize;
        
        *(int*)datablock = super->free_block;

        super->free_block =  iNode->dblocks[0];
    }

    iNode->next_inode = super->free_inode;
    super->free_inode = iNode->next_inode;

    
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
    disk = malloc(size);
    fread(disk, 1, size, fp);
    //rewind(fp);
    //fclose(fp);

    //info from superblock
    superblock *super = (superblock*)(disk + 512);
    blockSize = super->block_size;
    int inode_offset = super->inode_offset;
    int data_offset = super->data_offset;
    inode_start = 1024 + inode_offset*blockSize;
    data_start = 1024 + data_offset*blockSize;

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
int f_unmount(const char *dir, int flags)
{
    //unmounting whole file system
    //corner case where other disk is mounted at root after the initial one - could maybe ignore
    if(strcmp(dir, "/") == 0) {
        printf("unmounting root\n");
        free(disk);
        free(root->child);
        free(root);
        int outcome = fclose(firstDisk);
        if(outcome != 0) {
            printf("Error unmounting disk\n");
            return FAILURE;
        }
    }
    return SUCCESS;
}

int main(){

    int mountOutcome = f_mount("DISK", "/");
    printf("Mount %d\n", mountOutcome);
    if(mountOutcome == -1) {
        fprintf(stderr, "Error mounting\n");
        exit(0);
    }
    //printf("Mount %d\n",f_mount("./DISK", "/"));
    int dirp = f_opendir("/");
    DirEntry* find = malloc(sizeof(DirEntry));
    find = f_readdir(dirp);
    f_mkdir("/", "new.txt", 0);
    find = f_readdir(dirp);
    int check = f_closedir(dirp);
    f_opendir("/new.txt");

    
    

    int fd = f_open("/letters.txt", ORDWR);
    if(fd == -1) {
        fprintf(stderr, "f_open error\n");
        exit(0);
    }

    char* ptr = malloc(sizeof(char)*4);
    f_read(ptr, 4, 1, fd);

    printf("File contents: %s\n", ptr);

    free(ptr);

    f_unmount("/", 0);

}
