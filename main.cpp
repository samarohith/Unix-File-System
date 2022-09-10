#include <bits/stdc++.h>
using namespace std;

#define BLOCKS_COUNT 128
#define FILES_COUNT 16

int main()
{
    FILE *fp;
    char *buffer;

    fp = fopen("disk0","w");

    buffer = (char *)calloc(1024,sizeof(char));

    // as first datablock is superblock and first 128 Blocks are for block's status whether they are availabe or not except superblock
    buffer[0] = 1;

    fwrite(buffer,1024,1,fp);
    buffer[0] = 0;
    for(int i=1;i<BLOCKS_COUNT;i++){
      fwrite(buffer,1024,1,fp);
    }
    printf("FILE SYSTEM is created with name disk0\n");
    fclose(fp);
    return 0;
}
