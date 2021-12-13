#include "vfs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "format.h"
#include <string.h>
#include <signal.h>
#include <math.h>

#define FAILURE -1
#define SUCCESS 0

int m_error;
enum errors{E_BADARGS, E_EOF, E_FNF, E_DNF, FT_FULL};
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

int f_open(char* path, char* filename, int flag)
{
    
    if (flag != OREAD && flag != ORDWR && flag != ORDAD && flag != OWRITE && flag != OCREAT && flag != OAPPEND)
    {
        m_error = E_BADARGS;
        return -1;
    }

    if (num_open_files == MAX_FT_SIZE)
    {
        m_error = FT_FULL;
        return -1;
    }

    vnode_t* vn = find(path);

    vnode_t* find_file = vn->child;
    while (find_file != NULL && strcmp(find_file->name, filename) != 0)
    {
        find_file = find_file->next;
    }

    if (find_file == NULL)
    {
        if (flag == OREAD || flag == ORDWR)//FNF error
        {
            m_error = E_FNF;
            return -1;
        }else //create file
        {
            vnode_t* new_file = malloc(sizeof(vnode_t));
            superblock* super = (superblock*)(disk+blockSize);
            new_file->child = NULL;
            strcpy(new_file->name, filename);
            new_file->permissions = flag;
            new_file->type = FILE_TYPE;
            
            new_file->inode = super->free_inode; //assign inode for file

            //assign free block to inode
            void* to_inode = disk + inode_start + super->free_inode * sizeof(inode);
            inode* node = (inode*)to_inode;
            node->dblocks[0] = super->free_block;


            //update free lists
            super->free_inode = node->next_inode;
            void* to_data = disk + data_start + super->free_block * blockSize;
            int ptr = *(int*)to_data;
            super->free_block = ptr;

            //make it a child of current directory
            find_file = new_file;

        }
    }

    void* to_inode = disk + inode_start + find_file->inode * sizeof(inode);
    inode* find_size = (inode*)to_inode;
   
    
    fileEntry entry;
    entry.flag = flag;//permissions
    if (flag == OAPPEND || flag == ORDAD)//position in stream
    {
        entry.offset = find_size->size;

    }else
    {
        entry.offset = 0;
    }
    entry.vn = find_file;

    //add file to file table
    for (int i = 0; i < MAX_FT_SIZE; i++)
    {
        fileEntry try = fileTable[i];
        if (try.vn == NULL)//emptyyyy
        {
            fileTable[i] = entry;
            num_open_files++;// TO-DO: for checking if max number of files have been opened later
            return i;
        }else if (try.vn->inode == entry.vn->inode)
        {
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

    //ahh what if offset is bigger than blocksize......... update
    node = disk + data_start + iNode->dblocks[0] * blockSize + to_read.offset;
    
    //printf("\noffset: %d, size: %d; to read: %ld * %d\n", to_read.offset, iNode->size, size, num);
    if(to_read.offset >= iNode->size) {
        return 0;
    } else if (to_read.offset + data_to_read > iNode->size) {
        fileTable[fd].offset = iNode->size;
        readSize = 1;
        readNum = iNode->size - to_read.offset;
    } 
    
    //printf("Contents: %s\n", (char*)node);

    //ptr = malloc(data_to_read);
    memcpy(ptr, node, readNum*readSize);
    //memccpy(ptr, node, EOF, readSize*readNum);
    fileTable[fd].offset += data_to_read;
    if(to_read.offset > iNode->size) {
        fileTable[fd].offset = iNode->size;
    }
    return readNum*readSize;
}

size_t f_write(void *data, size_t size, int num, int fd)
{
    //how many blocks needed
    //check fd
    //
    size_t data_to_write = size * num;
    fileEntry to_write = fileTable[fd];
    size_t writeSize = size;
    int writeNum = num;

    superblock *super = (superblock*)(disk + 512);

    //file doesn't exist
    if(to_write.vn == NULL) {
        //fprintf(stderr, "f_write: no file found\n");
        return FAILURE;
    }

    //MODIFY make error if no free space left on disk and trying to write over file size
    
    void* node = (disk + inode_start + fileTable[fd].vn->inode*sizeof(inode));
    inode* iNode = (inode*)node;

    int currentBlocksUsed = (int)ceil((double)(iNode->size)/blockSize);
    int totalBlocksNeeded = (int)ceil((double)(to_write.offset + data_to_write)/blockSize);

    //if over current size of file
    if(to_write.offset > iNode->size) {
        return FAILURE;
    }

    //CHECK is this the right way to assign a new data block?
    if(iNode->size == 0) {
        iNode->dblocks[0] = super->free_block;
        super->free_block = *(int*)(disk + data_start + super->free_block*blockSize);
    }

    if(totalBlocksNeeded - currentBlocksUsed != 0) {
        int diff = totalBlocksNeeded - currentBlocksUsed;
        if(totalBlocksNeeded > (N_DBLOCKS + N_IBLOCKS*(blockSize/sizeof(int)))) {
            fprintf(stderr, "We don't support above singly indirect blocks for files currently. Try something smaller\n");
            return FAILURE;
        }
        while(totalBlocksNeeded - currentBlocksUsed > 0) {
            if(currentBlocksUsed < N_DBLOCKS) {
                iNode->dblocks[currentBlocksUsed];
            } else {
                //allocate block for indirects like in defrag
            }

            currentBlocksUsed++;
        }
    }

    int offset = 0;
    int blocksIn = 0;
    //check for rounding error
    int inBlockOffset = to_write.offset % blockSize;

    if(blocksIn < N_DBLOCKS) {
        offset = iNode->dblocks[blocksIn];
    }
    
    node = disk + data_start + offset*blockSize + inBlockOffset;

    //adjust inode size (consider if overwriting existing memory)
    iNode->size += data_to_write;

    memcpy(node, data, data_to_write);
    fileTable[fd].offset += data_to_write;
    return data_to_write;
}


int f_close(int fd)
{
    if (num_open_files == 0)
    {
        //set m_error to EOF
    }
    fileEntry to_close = fileTable[fd];
    fileTable[fd].offset = 0;
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
    void* node = disk;
    node += inode_start + to_seek.vn->inode * sizeof(inode);
    inode* iNode = (inode*)node;
    
    if (whence == SEEK_END)
    {
        if (offset > 0 || offset < iNode->size * -1)//has to be negative number between size of file and 0
        {
            //set m_error to attempt to access past end of file
            return -1;
        }
        
        to_seek.offset = iNode->size + offset;

    }else if (whence == SEEK_SET)
    {
        if (offset < 0 || offset > iNode->size)//has to be positive number that is less than the size of the file
        {
            //set m_error to bad args cannot access before file
            return -1;
        }
        to_seek.offset = offset;

        
    }else if (whence == SEEK_CUR)
    {
        if(offset + to_seek.offset > iNode->size || offset + to_seek.offset < 0 )
        {
            //set m_error to bad args
            return -1;
        }
        to_seek.offset += offset;
    }else
    {
        //set m_error to bad args
        return -1;
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

    vnode_t* vn = malloc(sizeof(vnode_t));
    vn = find(path);

    void* node = disk + inode_start + vn->inode * sizeof(inode);
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
    //how to free indirect blocks???
    superblock* super = (superblock*)(disk+512);
    for (int i = 0; i < num_blocks_to_free; i++)
    {
        void* datablock = disk + data_start + iNode->dblocks[i] * blockSize;
        
        *(int*)datablock = super->free_block;

        super->free_block =  iNode->dblocks[i];
    }

    iNode->next_inode = super->free_inode;
    super->free_inode = vn->inode;


    
    
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

    if (dircurrent == NULL)
    {
        //set m_error to directory not found
        return -1;
    }
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

    //assign a block to the new directory
    void* free_block = disk + data_start + super->free_block * sizeof(inode);//find block
    int ptr = *(int*)free_block;//get ptr to next block
    next->dblocks[0] = super->free_block;//assign block to inode
    super->free_block = ptr;//assign next free block ptr to super->free_block



    
    return 0;

}
int f_rmdir(char* path)
{
    vnode_t* vn = malloc(sizeof(vnode_t));
    vn = find(path);

    void* node = disk + inode_start + vn->inode * sizeof(inode);
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

        super->free_block =  iNode->dblocks[i];
    }

    iNode->next_inode = super->free_inode;
    super->free_inode = vn->inode;

    
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

    
    

    int fd = f_open("/", "letters.txt", OCREAT);
    if(fd == -1) {
        fprintf(stderr, "f_open error\n");
        exit(0);
    }

    char* ptr = malloc(sizeof(char)*3);
    f_read(ptr, 3, 1, fd);
    printf("File contents: %s\n", ptr);
    free(ptr);

    char *newText = "defg";
    int outcome = f_write(newText, strlen(newText)+1, 1, fd);
    if(outcome == -1) {
        fprintf(stderr, "f_write error\n");
        exit(0);
    }

    f_close(fd);
    fd = f_open("/letters.txt", ORDWR);
    if(fd == -1) {
        fprintf(stderr, "f_open error\n");
        exit(0);
    }

    ptr = malloc(sizeof(char)*9);
    f_read(ptr, 9, 1, fd);
    printf("File contents now: %s\n", ptr);
    free(ptr);

    f_close(fd);

    /*int fd2 = f_open("/new.txt", OCREAT);
    if(fd2 == -1) {
        fprintf(stderr, "f_open error\n");
        exit(0);
    }
    char *newText = "New text";
    int outcome = f_write(newText, strlen(newText), 1, fd2);
    if(outcome == -1) {
        fprintf(stderr, "f_write error\n");
        exit(0);
    }*/


    f_unmount("/", 0);

}
