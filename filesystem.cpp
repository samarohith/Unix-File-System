#include<bits/stdc++.h>
#include<stdio.h>
using namespace std;

#define BLOCKS_COUNT 128
#define FILES_COUNT 16

char fsName[10] = "skr";

struct inode{
	char name[8];
	int size;
	int blockPointers[8];
	int used=0;
};

int currInodeToBytes(int num){
   return 128+((num-1)*48);
}

int checkNumberOfFiles(){
  FILE* fp = fopen(fsName,"r+");
  char availStatus[128];
  fread(availStatus,128,1,fp); // first 128 BYTES is assigned for checking block's status
  int count = 0;
  inode curr;
  for(int i=0;i<FILES_COUNT;i++){
  	fread(&curr,sizeof(inode),1,fp);
  	if(curr.used == 1){
  		count++;
  	}
  }
  fclose(fp);
  return count;
}

void create(char filename[8],int filesize)
{
	
   FILE* fp = fopen(fsName,"r+");
   char availStatus[128];
   fseek(fp,0,SEEK_SET);
   fread(availStatus,BLOCKS_COUNT,1,fp); // first 128 BYTES is assigned for checking block's status
   cout<<endl;
   inode fileinode[16];
   int currInode = 0;
   for(currInode=1;currInode<FILES_COUNT;currInode++){
   	  fread(&fileinode[currInode],sizeof(inode),1,fp);
   	  if(fileinode[currInode].used == 0){
           break;
   	  }
   }
   if(currInode == 0){
   	  printf("FILESYSTEM reaches maximum count i.e., 16 files are present already \n ");
   	  printf("This New File can't be created \n");
   	  return;
   }
   fseek(fp,currInodeToBytes(currInode),SEEK_SET);

   inode newNode;

   newNode.size = filesize;
   newNode.used = 1;

   memcpy(&newNode.name,filename,8);
   
   int allottedblocks = 0;
   for(int i=1;i<BLOCKS_COUNT && filesize>0;i++){
   	  if(availStatus[i]=='0'){
   	  	availStatus[i] = '1';
   	  	newNode.blockPointers[allottedblocks] = 1024+((i-1)*1024);
   	  	allottedblocks++;
   	  	if(allottedblocks == filesize){
   	  		break;
   	  	}
   	  }
   }
   fwrite(&newNode,sizeof(inode),1,fp);
   fseek(fp,0,SEEK_SET);
   fwrite(availStatus,sizeof(availStatus),1,fp);
   fclose(fp);
   cout<<"file "<<filename<<" is created\n";
}

void read(char filename[8],int block,char buffer[1024]){
	FILE *fp = fopen(fsName,"r+");

	fseek(fp,BLOCKS_COUNT,SEEK_SET);
 
	inode currInode;
    bool fileFound = false;
	for(int i=0;i<FILES_COUNT;i++){
		fread(&currInode,sizeof(inode),1,fp);
		if(strcmp(currInode.name,filename)==0){
			fileFound = true;
			break;
		}
	}
	if(!fileFound){
		cout<<endl<<filename<<" doesn't exist in the FILESYSTEM, so can't able to perform read operation\n\n";
		fclose(fp);
		return;
	}
	if(currInode.size<block){
		cout<<"\nsorry, can't able to perform read operation, entered block num = "<<block<<" but current file size = "<<currInode.size<<endl;
        fclose(fp);
		return;
	}
	fseek(fp,currInode.blockPointers[block-1],SEEK_SET);
	fread(buffer,1024,1,fp);
	cout<<"\nReading operation from "<<filename<<" ,block num = "<<block<<" is done, so content of the buffer = \n";
	printf("\n\t%s\n\n",buffer);
	fclose(fp);
}

void write(char filename[8],int block,char buffer[1024]){

   FILE *fp = fopen(fsName,"r+");
   int seekpoint = fseek(fp,BLOCKS_COUNT,SEEK_SET);

   inode currInode;
   bool fileFound = false;
   for(int i=0;i<FILES_COUNT;i++){
   	  fread(&currInode,sizeof(inode),1,fp);
   	  if(strcmp(currInode.name,filename)==0){
   	  	 fileFound = true;
   	  	 break;
   	  }
   }
   if(!fileFound){
   	  cout<<endl<<filename<<" doesn't exist in the FILESYSTEM, so can't able to perform write operation \n\n";
      fclose(fp);
      return;
   }
   if(block<=0){
		cout<<"\nplease enter valid block number, greater than zero\n";
	}
   if(currInode.size<block){
   	    cout<<"\nsorry, can't able to perform write operation, entered block num = "<<block<<" but current file size = "<<currInode.size<<endl;
        fclose(fp);
		return;
   }
   cout<<"\nbuffer content "<<buffer <<" : is written into block number "<<block<<endl;
   fseek(fp,currInode.blockPointers[block-1],SEEK_SET);
   fwrite(buffer,1024,1,fp);
   fclose(fp);
}

void deleteF(char filename[8]){
	char availStatus[128];
	FILE *fp = fopen(fsName,"r+");
	fread(availStatus,BLOCKS_COUNT,1,fp);
    int seekpoint;
    inode currNode,newNode;
	bool fileFound = false;
	for(int i=1;i<=FILES_COUNT;i++){
        fread(&currNode,sizeof(inode),1,fp);
        if(strcmp(currNode.name,filename)==0){
        	fileFound = true;
        	seekpoint = currInodeToBytes(i);
        	break;
        }
	}
	if(!fileFound){
		cout<<endl<<filename<<" is not present in the FILESYSTEM, can't be deleted\n\n";
		fclose(fp);
		return;
	}
    for(int i=0;i<currNode.size;i++){
    	char *buffer;
    	buffer = (char *)calloc(1024,sizeof(char));
    	fseek(fp,currNode.blockPointers[i],SEEK_SET);
    	fwrite(buffer,1024,1,fp);
    	int blocknum = (currNode.blockPointers[i])/1024;
    	availStatus[blocknum] = '0';
    }
    newNode.used = 0;
    fseek(fp,seekpoint,SEEK_SET);
    fwrite(&newNode,sizeof(inode),1,fp);

    fseek(fp,0,SEEK_SET);
    fwrite(availStatus,128,1,fp);
    fclose(fp);
    printf("\n\n File , %s is deleted\n\n",filename);
    return;
}

void ls(){
   
  FILE* fp = fopen(fsName,"r+");
  char availStatus[128];
  fread(availStatus,128,1,fp); // first 128 BYTES is assigned for checking block's status

  inode curr;
  int files_total = 0;
  cout<<"\n\nCurrent status of FILESYSTEM is as follows :- \n\n";
  for(int i=0;i<FILES_COUNT;i++){
  	fread(&curr,sizeof(inode),1,fp);
  	if(curr.used == 1){
      files_total++;
  		cout<<"\nName of the FILE = "<<curr.name;
  		cout<<",\tSize of the FILE = "<<curr.size<<endl;
  		cout<<endl;
  	}
  }
   
  fclose(fp);
}

bool checkFileAlreadyExists(char filename[8]){
   bool flag = false;
   FILE* fp = fopen(fsName,"r+");
   char availStatus[128];
   fread(availStatus,128,1,fp); // first 128 BYTES is assigned for checking block's status

   inode curr;
   for(int i=0;i<FILES_COUNT;i++){
  	fread(&curr,sizeof(inode),1,fp);
  	if(curr.used == 1){
  		if(strcmp(curr.name,filename)==0){
  			flag = true;
  		}
  	}
   } 
   
  fclose(fp);
  return flag;

}

int findTotalAllotedBlocks(){
   int count = 0;
   FILE* fp = fopen(fsName,"r+");
   char availStatus[128];
   fread(availStatus,128,1,fp); // first 128 BYTES is assigned for checking block's status

   inode curr;
   for(int i=0;i<FILES_COUNT;i++){
  	fread(&curr,sizeof(inode),1,fp);
  	if(curr.used == 1){
  		count += curr.size;
  	}
   } 
   
  fclose(fp);
  return count;
}

int main(){

	cout<<"Choose among following :-\n 1) create a new FILESYSTEM or\n 2) continue with a already existed one\n";
	cout<<"\nyour choice = ";
	int choice;
	cin >> choice;
    FILE *fp;
	if(choice == 1){

		char *buffer;

        cout<<"\n Enter the desired name for FILESYSTEM = ";
		cin >> fsName;

        fp = fopen(fsName,"w");

        buffer = (char *)calloc(1024,sizeof(char));
        
        for(int i=1;i<128;i++){
        	buffer[i] = '0';
        }
        // as first datablock is superblock and first 128 Blocks are for block's status whether they are availabe or not except superblock
        buffer[0] = '1';

        fwrite(buffer,1024,1,fp);
        buffer = (char *)calloc(1024,sizeof(char));
        buffer[0] = '0';
        for(int i=1;i<BLOCKS_COUNT;i++){
            fwrite(buffer,1024,1,fp);
        }
        cout<<"FILE SYSTEM is created with name = "<<fsName<<endl;
        fclose(fp);

	}
	if(choice == 2){
		cout<<"\nEnter the FILESYSTEM name which already exists = ";
		cin >> fsName;
		fp = fopen(fsName,"r");
		if(!fp){
            cout<<"\nEntered FILESYSTEM isn't existed, please create a new one\n";
            exit(0);
		}
		fclose(fp);
	}

	char read_or_write_buffer[1024]="HELLO\0";
	char filename[8];
	int sizeofFN,blockNUM;
    bool flag = true;
    int curr_files_count=0,totalAllotedBlocks=0;
    while(flag){
    	printf("\nplease choose the following options : - \n");
    	int option;
    	cout << "1) CREATE,  2) DELETE,  3) READ,  4) WRITE,  5) LISTOUTFILES, 6) exit \n";
    	cout << "\nchoose option = ";
    	cin >> option;
    	cout<<endl;
    	switch(option){
            
            case 1 : cout<<"\nenter the file name to create = ";
                     cin >> filename;
                     cout<<"\nenter the size of this file = ";
                     cin >> sizeofFN;
                     if(sizeofFN > 8){
                     	cout<<"\nmaximum file size allowed is 8, please try again with smaller size\n";
                     	break;
                     }
                     curr_files_count = checkNumberOfFiles();
                     if(curr_files_count>=16){
   	                    cout<<"\nSorry maximum Limit of Files(16) already existed in this FILESYSTEM, can't able to create new one\n";
   	                    break;
                     }
                     totalAllotedBlocks = findTotalAllotedBlocks();
                     if(totalAllotedBlocks==120 && curr_files_count==15 && sizeofFN==8){
                     	cout<<"\nAs already 15 files are of maximum capacity i.e.,120 blocks already filled, hence only 7 blocks are availabe for new file\n";
                     	cout<<"\nplease try again with filesize less than 8\n\n";
                     	break;
                     }
                     if(!checkFileAlreadyExists(filename)){
                        create(filename,sizeofFN);
                     }
                     else{
                     	cout<<"\nEntered FileName already exists in the FILESYSTEM, please try with some other name\n";
                     }
                     break;
            case 2 : cout<<"\nenter the file name to delete = ";
                     cin >> filename;
                     deleteF(filename);
                     break;
            case 3 : cout<<"\nenter the file name to read from = ";
                     cin >> filename;
                     cout<<"\nenter the block number from where you want to read = ";
                     cin >> blockNUM;
                     read(filename,blockNUM,read_or_write_buffer);
                     break;

            case 4 : cout<<"\nenter the file name to write into = ";
                     cin >> filename;
                     cout<<"\nenter the block number to where you want to write = ";
                     cin >> blockNUM;
                     cin.ignore();
                     cout<<"\nenter the content you want to write = ";
                     cin.getline(read_or_write_buffer,1024);
                     write(filename,blockNUM,read_or_write_buffer);
                     break;         

            case 5 : ls(); 
                     break;

            case 6 : flag = false;
                     break;
            default :
                     cout<<"\nplease enter correct option\n";
                     break;                          

    	}
    }
	return 0;
}