# MockFileSystem
Implements and operates on a 128kb file system  
  
TORUN:
Compile: gcc ­std=c99 filesim.c ­o filesim Run ./filesim [inputfilename]

inputfilename is a list of commands followed by arguments to operate on the file system. See exinput.txt

DESIGN
The methods described in the project all work in a similar fashion. They open the file system and use: seek, fwrite and fread operations to manipulate the files and their contents.
  -The first 128 bytes are the availability of blocks.
  -The rest of the first block contains 16 inode structures (48 bytes each) that hold
  relevant file info.
  -The rest of the blocks contain data pointed to by inodes that have been allocated
  the block.

Below is a brief description of how the methods operate.

DELETE:
1. Locate the availability bytes and loads them into a 128 byte array.
2. Fread through inodes until the file name[8] is located.
3. Note the block pointers of the file to delete.
4. Create a new, empty inode to write into the spot containing the inode to delete, used = 0.
5. Set the availability of name’s block pointers to 0 and rewrite the availability array.
6. Close the file.
Note: We elected not to delete block contents in file deletion. Since there is no reference to this block and any new write will overwrite the existing data anyways.

CREATE:
1. Load the availability array in using fread to get the first 128 bytes.
2. Fread through inodes until the first inode with used = 0 is found.
3. Create an inode allocating it the first (size) blocks with availability = 0 in the availability
array.
4. Mark these blocks as unavailable.
5. Write the inode and availability back to the file.
6. Close the file.
READ/WRITE
1. Locate the inode and find the byte index of the blockNum.
2. Read/Write the buffer passed to the method.
3. Exit the file.

LS:
1. Iterate through the inodes that have names and print the names.
2. Close the file.

Main:
1. Iterates through lines in the input file provided to the file system.
2. Uses a switch to determine what method each line should call.
3. Performs the desired operation.
4. Stops when no new lines are encountered.
