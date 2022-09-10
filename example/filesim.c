#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_NODES 16
#define NUM_BLOCK_POINTERS 128

/* One for each file in file sys (max 16) */
typedef struct inode {
	char name[8]; // file name
	int size; // file size (in number of blocks)
	int blockPointers[8]; // direct block pointers
	int used; // 0 = free, 1 = in use
} inode; // total = 48 bytes in size

/* Returns inode start byte position. */
int nodeToByte(int i ) {
	return NUM_BLOCK_POINTERS + (i * 48);
}
void delete(char name[8]) {
	FILE* inFile = fopen("disk0", "r+");

	/* Create availability array. */
	char *availability = malloc(NUM_BLOCK_POINTERS);
	fread(availability, NUM_BLOCK_POINTERS, 1, inFile);

	/* Read in nodes until name is found. Assumes name exists in FS. */
	inode node[16];
	inode oldNode;
	int seekPoint;
	for (int i = 0; i < NUM_NODES; i++) {
		fread(&node[i], sizeof(inode), 1, inFile);
		if (strncmp(node[i].name, name, 8) == 0) {
			printf("Inode %d contains the file to delete.\n", i);
			oldNode = node[i];
			fseek(inFile, nodeToByte(i), SEEK_SET);
			seekPoint = nodeToByte(i);
			break;
		}
	}
	/* Now, make the block pointers available. */
	int *blockPointers = malloc(sizeof(int)*oldNode.size);
	memcpy(blockPointers, oldNode.blockPointers, oldNode.size*sizeof(int));
	/* Create a new inode to overwrite deleted one. */
	inode newNode = {
		.used = 0 };
	fwrite(&newNode, sizeof(newNode), 1, inFile);
	/* Update availability array and write it to file. */
	for (int i = 0; i < oldNode.size; i++) {
		int index = blockPointers[i] / 1024;
		availability[index] = 0;
	}
	rewind(inFile);
	fwrite(availability, sizeof(availability), 1, inFile);
	/* Upkeep */
	fclose(inFile);
	free(blockPointers);
	free(availability);
}
void create(char name[8], int size) {
	FILE* inFile = fopen("disk0", "r+");
    //printf("hi\n");
	/* Create availability array. */
	char *availability = malloc(NUM_BLOCK_POINTERS);
	fread(availability, NUM_BLOCK_POINTERS, 1, inFile);

	/* Find the first available inode and assign file to it. */
	inode node[16];
	for (int i = 0; i < NUM_NODES; i++) {
		fread(&node[i], sizeof(inode), 1, inFile);
		if (node[i].used == 0) {
			printf("Node %d not in use. Creating file...\n", i + 1);
			int seekPoint = fseek(inFile, nodeToByte(i), SEEK_SET);
			break;
		}
	}
	/* Node to replace the current empty inode. */
	inode newNode = {
		.size = size,
		.used = 1
	};
	memcpy(&newNode.name, name, 8);



	int found = 0; // Count of blocks found to assign to new inode.
	/* Assign # size blocks to inode. */
	for (int i = 1; i < NUM_BLOCK_POINTERS; i++) {
		if (availability[i] == 0) {	// can assign this block
			newNode.blockPointers[found] = 1024 + (1024*(i-1));
			found++;
			availability[i] = 1;
			if (found == size) { // found enough available blocks. done.
				break;
			}
		}
	}

	/* Write the new availability and inode to disk. Perform upkeep. */
	fwrite(&newNode, sizeof(inode), 1, inFile);
	rewind(inFile);
	fwrite(availability, sizeof(availability), 1, inFile);
	fclose(inFile);
	free(availability);
}
void write(char name[8], int blockNum, char buf[1024]) {
	FILE* inFile = fopen("disk0", "r+");

	/* Create availability array. */
	char *availability = malloc(NUM_BLOCK_POINTERS);
	fread(availability, NUM_BLOCK_POINTERS, 1, inFile);

	/* Find the inode, find the byte location of the block to write. */
	inode node;
	int availIndex = -1;
	for (int i = 0; i < NUM_NODES; i++) {
		fread(&node, sizeof(inode), 1, inFile);
		if (strcmp(node.name, name) == 0) {
			printf("Found the file. Writing block...\n");
			printf("Writing to....%d", node.blockPointers[blockNum]);
			availIndex = node.blockPointers[blockNum];
			break;
		}
	}

	/* Write the block. */
	fseek(inFile, availIndex, SEEK_SET);
    fwrite(buf, 1024, 1, inFile);
    /* Make sure the block is set to unavailable. */
    rewind(inFile);
	availability[availIndex/1024] = 1;
	fwrite(availability, sizeof(availability), 1, inFile);
	/* Upkeep */
	free(availability);
	fclose(inFile);
}

int read(char name[8], int blockNum, char buf[1024]){
    //read the specified block from this file into the
    //specified buffer; blockNum can range from 0 to 7.
    FILE* inFile = fopen("disk0", "r+");
    fseek(inFile, NUM_BLOCK_POINTERS, SEEK_SET);
    inode node;
    int found = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        fread(&node, sizeof(inode), 1, inFile);
        if (strcmp(node.name, name) == 0) {
            break;
            }
        }

    //rewind(inFile);
    fseek(inFile, node.blockPointers[blockNum], SEEK_SET);
    fread(buf, 1024, 1, inFile);
    fclose(inFile);

    printf("Buffer contents after read: %s\n", buf);
   
    return !found;
}

void ls() {
	FILE* inFile = fopen("disk0", "r+");
	/* Seek through availability. */ 
	char *availability = malloc(NUM_BLOCK_POINTERS);
	fread(availability, NUM_BLOCK_POINTERS, 1, inFile);

	/* Print all allocated inodes and their size. */
	inode node;
	for (int i = 0; i < NUM_NODES; i++) {
		fread(&node, sizeof(inode), 1, inFile);
		if (node.used == 1) {
			printf("Filename: %s", node.name);
			printf(" Size: %d\n", node.size);
		}
	}
	/* Upkeep */
	free(availability); 
}


int main(int argc, char* argv[]) {
    /*TODO: add functions. */
    FILE *fp;
    fp = fopen(argv[1], "r");
 
    char dummy[1024]="Dummy\0";
    char buffer[100];
    char type;
    char fname[8];
    char name[8];
    int s = 0;
 
    /* Read each line and call appropriate function. Output described in function calls... 
    ASSUMPTION: No error handling & file to read in is properly formatted. */ 
    while(fgets(buffer, sizeof(buffer), fp)) {
        if (sscanf(buffer, "%c %s %d\n", &type, fname, &s)){ //successfull read of at least one parameter i.e L,D etc
            memcpy(&name, &fname, 8);
 
        printf("Name: %c %s id: %d\n", type, fname, s);
 
        switch(type) {
            case 'C':
                printf("Create:\n");
                create(name, s);
                break;
            case 'L':
                printf("List:\n");
                ls();
                break;
            case 'W':
                printf("Write:\n");
                write(name, s, dummy);
                break;
            case 'D':
                printf("Delete:\n");
                printf("%s\n", name);
                delete(name);
                break;
            case 'R':
                printf("Read\n");
                read(name, s, dummy);
                break;
            default:
                printf("not sure here.\n");
        }
    	}
	}
 
    fclose(fp);
 
 
    return 0;
}
