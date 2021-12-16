# README
CS355 Final Project HW7

<h1> Implementation of a Filesystem </h1>

<h2>To Compile and Run</h2>
Library: <br >
make <br  >

<h2>Introduction</h2>


<h2> Virtual File System </h2>
<h3>Data Structures</h3>
arrays: to keep track of the file table and the directory table <br >
vnode: linked list where it stores it's children but also a sibling <br >
Testing for:
    - file to mount doesn't exist
    - file to open doesn't exist
    - file to read doesn't exist

<h2> Shell </h2>


<h2> Disk Format </h2>
<h3>Superblock</h3>
<img width="174" alt="Screen Shot 2021-12-13 at 2 04 56 PM" src="https://user-images.githubusercontent.com/55250326/145872642-a3531555-1027-4304-b193-c472574efee9.png">
<h3>Inode</h3>
<img width="186" alt="Screen Shot 2021-12-13 at 2 05 04 PM" src="https://user-images.githubusercontent.com/55250326/145872702-70eec2c0-57ef-4cac-95ae-777e024973dd.png">
<h3>Physical Directory Entry</h3>
<img width="421" alt="Screen Shot 2021-12-13 at 2 07 16 PM" src="https://user-images.githubusercontent.com/55250326/145872918-615daf71-5392-4018-863f-b4dd8f80a8b8.png">


<h2>Features</h2>

<h3>Fully Implemented</h3>
<h4>Virtual File System</h4>
<h5>f_open</h5>
open the specified file with the specified access (read, write, read/write, append). If the file does not exist, handle accordingly. (rule of thumb: create file if writing/appending, return error if reading is involved). Returns a file handle if successful <br >
<h5>f_read</h5>
read the specified number of bytes from a file handle at the current position. Returns the number of bytes read, or an error. <br >
<h5>f_write</h5>
write some bytes to a file handle at the current position. Returns the number of bytes written, or an error. <br >
<h5>f_close</h5>
close a file handle <br >
<h5>f_seek</h5>
move to a specified position in a file
<h5>f_rewind</h5>
move to the start of the file
<h5>f_stat</h5>
retrieve information about a file
<h5>f_remove</h5> 
delete a file
<h5>f_opendir</h5>
recall that directories are handled as special cases of files. open a “directory file” for reading, and return a directory handle.
<h5>f_readdir</h5> 
returns a pointer to a “directory entry” structure representing the next directory entry in the directory file specified.
<h5>f_closedir</h5> 
close an open directory file
<h5>f_mkdir</h5>
make a new directory at the specified location
<h5>f_rmdir</h5>
delete a specified directory. Be sure to remove entire contents and the contents of all subdirectories from the filesystem. Do NOT simply remove pointers to directory.
<h5>f_mount</h5>
mount a specified file system into your directory tree at a specified location. (cannot mount more than one disk)
<h5>f_umount</h5>
unmount a specified file system
 
<h4>Shell</h4>


<h3>Partially Implemented</h3>


<h3>Not Implemented</h3>


<h3>Known Bugs and Limitations</h3>
Only pressing enter on the shell will cause a memory error.
