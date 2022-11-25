// Including required libraries.
#include<iostream>
#include<unordered_map>
#include<vector>
#include<stdlib.h>
#include<sstream>
#include<fstream>
#include<cstring>
#include<stack>
#include <sys/stat.h>
#include <sys/types.h>
#include<unistd.h>
using namespace std;

//structure of Inode
struct Inode{
	int Id;
	string filename;
	vector<string> childfilenames;     // stores all childs(sub files) of the file
	vector<struct Inode> indirect;    // indirect block ptrs
};


//structure of Directory
struct Directory{
	string name;
	unordered_map<string,struct Inode> files;             // files in the directory
	unordered_map<string,struct Directory *> directories;  // directories inside the directory
};

struct Directory *global;     // global directory
stack<struct Directory*> globalStack;   // stack to keep track of the directories we are inside of
unordered_map<int,int> InodeId;  //checks if InodeId is unique or not
unordered_map<string,int> InodeChildNames; //checks if InodeChildNames is unique or not


//Will initialise disk block pointers and push all available block pointers into map.
//here we are assuming 1MB space.

void initialiseDiskBlockPointers()
{
	//total of 2 power 18 disk blocks.
   for(int i = 0; i<262144; i++)
   {
       string s=to_string(i+1);
       s+=".txt";
       InodeChildNames[s]=1;
   }
}

//The below function will create a file.
void makeFile(){

	struct Inode newInode;
	struct Inode realInode;
    
	string fileName;
	//cout<<"Maximum file size is 1KB(1024)"<<endl;
	cin>>fileName;
	char filecontent[10300];
	cin.getline(filecontent,sizeof(filecontent));
	int r=rand();
	while(InodeId[r]!=0){
		r=rand();
	}
	InodeId[r]=1;
	newInode.Id=r;
	if(global->files.find(fileName)!= global->files.end()){
			cout<<"Error : Filename already exists"<<endl;
			cout<<"--------------------------------------"<<endl;
			InodeId[r]=0;
			return;
	}
	newInode.filename=fileName;
	string s1;
	int count=0;
	int count1=0;
	if(strlen(filecontent)>1026)
	{
		cout<<"File size limit exceeded:- Max File size is 1KB"<<endl;
		return;
	}
	for(int i=2;i<(int)strlen(filecontent)-1;i++){
		s1+=filecontent[i];
		if((i-1)%4==0){
			string r1=(InodeChildNames.begin())->first;
			string childFileName=r1;
			InodeChildNames.erase(InodeChildNames.begin()->first);
			newInode.childfilenames.push_back(childFileName);
			count++;
			if(count==64)
			{
				struct Inode temp;
				int r=rand();
				while(InodeId[r]!=0){
					r=rand();
				}
				InodeId[r]=1;
				temp.Id=r;
				string r1=(InodeChildNames.begin())->first;
				childFileName=r1;
				InodeChildNames.erase(InodeChildNames.begin()->first);
	            if(realInode.indirect.size()<3)
	            {
	           	       if(count1==0)
	           	       {
	           	       	  realInode=newInode;
	           	       }
	           	       else{
	           	       		realInode.indirect.push_back(newInode);	
                       }
                       newInode=temp;
                       count1++;
	           }
	           count=0;
			}
			fstream file;
			file.open(childFileName,ios::out);
			file<<s1;
			s1="";
		}
	}
	if(s1!=""){
		string r1=(InodeChildNames.begin())->first;
		string childFileName;
	 	childFileName=r1;
	    InodeChildNames.erase(InodeChildNames.begin()->first);
		newInode.childfilenames.push_back(childFileName);
		count++;
		fstream file;
		file.open(childFileName,ios::out);
		file<<s1;
		s1="";
	}
	if(count!=0){
		struct Inode temp;
		int r=rand();
		while(InodeId[r]!=0)
		{
		    r=rand();
	    }
	    InodeId[r]=1;
	    temp.Id=r;
	    if(realInode.indirect.size()<3)
	    {
	        if(count1==0)
	        {
	           	realInode=newInode;
	        }
	        else{
	           realInode.indirect.push_back(newInode);               	
            }
            newInode=temp;
            count1++;
		}
		count=0;
	}
	global->files[fileName]=realInode;
}

//The below function will delete the required file.
void deleteFile(){
	string fileName;
	cin>>fileName;
	if(global->files.find(fileName)!=global->files.end()){
		InodeId[global->files[fileName].Id]=0;
		for(int i=0;i<(int)global->files[fileName].childfilenames.size();i++){
			InodeChildNames[global->files[fileName].childfilenames[i]]=0;
			string s2=global->files[fileName].childfilenames[i];
			const char* s1=global->files[fileName].childfilenames[i].c_str();
			//This is to physically delete the file
			remove(s1);
			InodeChildNames[s2]=1;
		}
		for(int i=0;i<(int)global->files[fileName].indirect.size();i++){
			InodeId[global->files[fileName].indirect[i].Id]=0;
			for(int j=0;j<(int)global->files[fileName].indirect[i].childfilenames.size();j++){
				InodeChildNames[global->files[fileName].indirect[i].childfilenames[j]]=0;
				string s2=global->files[fileName].indirect[i].childfilenames[j];
				const char* s1=global->files[fileName].childfilenames[j].c_str();
				//This is to physically delete the file
				remove(s1);
				InodeChildNames[s2]=1;
			}	
		}
		global->files.erase(fileName);
	}
	else{
		cout<<"This file not present"<<endl;
	}
}

//The below function will rename the given file to given name.
void renameFile(){
	string fileName1;
	cin>>fileName1;
	string fileName2;
	cin>>fileName2;
	if(global->files.find(fileName1)==global->files.end()){
		cout<<"This "<<fileName1<<" is not present"<<endl;
		cout<<"----------------------------------------"<<endl;
		return;
	}
	if(global->files.find(fileName2)!=global->files.end()){
		cout<<"This fileName already exist please give another fileName Next time."<<endl;
		cout<<"-------------------------------------"<<endl;
		return;
	}
	else{
		struct Inode tempInode=global->files[fileName1];
		tempInode.filename=fileName2;
		global->files[fileName2]=tempInode;
		global->files.erase(fileName1);
		cout<<"Changed Sussesfully"<<endl;
	}
}

//The below function will print the content of the file.
void printFile(){
	string fileName;
	cin>>fileName;
	int count=0;

	if(global->files.find(fileName)!=global->files.end()){
		struct Inode temp=global->files[fileName];
		struct Inode real=temp;
		int count1=0;
		int indirectchilds=real.indirect.size();
		for(int i=0;i<(int)temp.childfilenames.size();i++){
			char s1[100];
			fstream file;
			file.open(temp.childfilenames[i],ios::in);
			file.getline(s1,sizeof(s1));
			cout<<s1;
			count++;
		}
		for(int i=0;i<indirectchilds;i++){
			temp=real.indirect[i];
			for(int j=0;j<(int)temp.childfilenames.size();j++){
				char s1[100];
				fstream file;
				file.open(temp.childfilenames[j],ios::in);
				file.getline(s1,sizeof(s1));
				cout<<s1;
				count++;
			}
		}
		cout<<endl;	
	}
	else{
		cout<<"File Not exist"<<endl;
	}
}

//The below function will list all the files present in the current directory.
void listFiles(){
	for(auto Inodes:global->files){
		cout<<"Inode Id:"<<Inodes.second.Id<<" FileName:"<<Inodes.second.filename<<endl;
	}
	for(auto directory:global->directories){
		cout<<"Directory Name:"<<directory.second->name<<endl;
	}
}

//The below function will create a directory with given name.
void makeDirectory(){
	char directoryName[200];
	cin>>directoryName;
	if(global->directories.find(directoryName)==global->directories.end()){
		int check=mkdir(directoryName,0777);
		if(check==0){
			struct Directory* temp=new Directory();
			temp->name=directoryName;
			global->directories[directoryName]=temp;
			cout<<"Directory Created!!!!!!"<<endl;
			cout<<"----------------------------------------"<<endl;
		}
		else{
			cout<<"Unable to create directory"<<endl;
			cout<<"------------------------------------------"<<endl;
		}
	}
	else{
		cout<<"Directory Already Exist"<<endl;
	}
}

//The below function will implement cd command.
void changeDirectory(){
	char path[100];
	cin>>path;
	char back[10]="..";
	if(strcmp(path,back)==0){
		if(globalStack.empty()){
			cout<<"You are at the top most level"<<endl;
			return;
		}
		cout<<"you are going back"<<endl;
		int t=chdir(path);
		if(t<0){
			cout<<"NOT Sussesfull"<<endl;
		}
		else{
			struct Directory* CurrentDirectory=globalStack.top();
			globalStack.pop();
			global=CurrentDirectory;
			cout<<global->directories.size()<<endl;
			cout<<global->name<<endl;
		}
	}
	else{
		if(global->directories.find(path)!=global->directories.end()){
			int t=chdir(path);	
			if(t<0){
				cout<<"NOT Sussesfull"<<endl;
			}
			else{
				globalStack.push(global);
				global=global->directories[path];
				cout<<global->directories.size()<<endl;
				cout<<global->name<<endl;
			}	
		}
		else{
			cout<<"No such Directory exist"<<endl;
		}
	}
	
}

//The below function will delete the given directory.
void deleteDirectory(){
	char directoryName[200];
	cin>>directoryName;
	if(global->directories.find(directoryName)!=global->directories.end()){
		rmdir(directoryName);
		global->directories.erase(directoryName);
	}
	else{
		cout<<"No such Directory exist"<<endl;
	}

}

//The below function will append the data at the end of the given file. 
void appendFile()
{
     string fileName;
     string s1;
	//cout<<"Maximum file size is 1KB(1024)"<<endl;
	cin>>fileName;
	char filecontent[10300];
	cin.getline(filecontent,sizeof(filecontent));
	if(global->files.find(fileName)==global->files.end()){
			cout<<"This file doesn't exists!!"<<endl;
			cout<<"--------------------------------------"<<endl;
			return;
	}
	struct Inode temp=global->files[fileName];
	int size1=0;
	size1+=temp.childfilenames.size()*4;
	for(int i=0;i<temp.indirect.size();i++)
	{
		i+=temp.indirect[i].childfilenames.size()*4;
	}
	size1=1024-size1;
	if(size1<strlen(filecontent))
	{
		cout<<"File size is more that expected(Max file size=1KB)"<<endl;
	}
	else{
		//cout<<"sdfsdgdsfgdfsgsdf"<<endl;
		int count1=temp.indirect.size();
		//cout<<count1<<endl;
		struct Inode realInode=temp;
		struct Inode newInode;
		if(count1==0){
			newInode=realInode;
		}
		else{
			newInode=realInode.indirect[count1-1];
		}
		int count=newInode.childfilenames.size();
		for(int i=2;i<(int)strlen(filecontent)-1;i++){
			s1+=filecontent[i];
			if((i-1)%4==0){
				string r1=(InodeChildNames.begin())->first;
				string childFileName=r1;
				InodeChildNames.erase(InodeChildNames.begin()->first);
				newInode.childfilenames.push_back(childFileName);
				
				count++;

				if(count==64)
				{
					struct Inode temp1;
					int r=rand();
					while(InodeId[r]!=0){
						r=rand();
					}
					InodeId[r]=1;
					temp1.Id=r;
					string r1=(InodeChildNames.begin())->first;
					InodeChildNames.erase(InodeChildNames.begin()->first);
		           if(realInode.indirect.size()<3)
		           {
		           	       if(count1==0)
		           	       {
		           	       	  realInode=newInode;
		           	       }
		           	       else{
		           	       		realInode.indirect.push_back(newInode);	
	                       }
	                       newInode=temp1;
	                       count1++;
		           }
		           count=0;
				}
				fstream file;
				file.open(childFileName,ios::out);
				file<<s1;
				s1="";
			}
		}
		if(s1!=""){
			string r1=(InodeChildNames.begin())->first;
			string childFileName=r1;
		    InodeChildNames.erase(InodeChildNames.begin()->first);
			newInode.childfilenames.push_back(childFileName);
			count++;
			fstream file;
			file.open(childFileName,ios::out);
			file<<s1;
			s1="";
		}
		//Inodes.push_back(newInode);
		if(count!=0){
			struct Inode temp1;
			int r=rand();
			while(InodeId[r]!=0){
				r=rand();
			}
			InodeId[r]=1;
			temp1.Id=r;
			string r1=(InodeChildNames.begin())->first;
			InodeChildNames.erase(InodeChildNames.begin()->first);
		    if(realInode.indirect.size()<3)
		    {
		        if(count1==0)
		        {
		           	realInode=newInode;
		        }
		        else{
	            	realInode.indirect.push_back(newInode);       	
	            }
	            newInode=temp1;
	            count1++;
			}
			count=0;
		}
		global->files[fileName]=realInode;

	}
}

//The below function will act as a helper function to print the contents of a file.
void printFile(fstream &file1,struct Inode t,bool f){
	if(!f)
		file1<<t.filename<<endl;
	file1<<t.Id<<endl;
	file1<<t.childfilenames.size()<<endl;
	for(int i=0;i<t.childfilenames.size();i++){
		file1<<t.childfilenames[i]<<endl;
	}
	file1<<t.indirect.size()<<endl;
	for(int i=0;i<t.indirect.size();i++){
		printFile(file1,t.indirect[i],1);
	}
}

//The below function will store the meta data of the files when the programe is terminated.
void helperStoreData(struct Directory* t)
{
	fstream file1;	
	file1.open("restore.txt",ios::out);
	file1<<t->files.size()<<endl;
	for(auto it:t->files){
		printFile(file1,it.second,0);		
	}
	file1<<t->directories.size()<<endl;
	for(auto it:t->directories){
		file1<<it.first<<endl;

	}
    file1.close();
}

//The below function will store the meta data.
 void storeData(struct Directory* t)
 {
         helperStoreData(t);
         //cout<<t->name<<" oops "<<endl;
          for(auto it:t->directories)
         {
    		
         	while(chdir(it.first.c_str())!=0);
         	globalStack.push(t);
         	t=it.second;
         	storeData(t);
         	while(chdir("..")!=0);
         	t=globalStack.top();
         	globalStack.pop();
         }
 }


//The below function will store the data of the inodes.
void readInode(fstream &file1,struct Inode &temp,bool f){
	if(!f){
		string fileName;
		file1>>fileName;
		temp.filename=fileName;
	}
	int Id;
	file1>>Id;
	temp.Id=Id;
	InodeId[Id]=1;
	int noOfChildFiles;
	file1>>noOfChildFiles;
	for(int i=0;i<noOfChildFiles;i++){
		string t;
		file1>>t;
		InodeChildNames.erase(t);
		temp.childfilenames.push_back(t);
	}
	int noOfIndirectInodes;
	file1>>noOfIndirectInodes;
	for(int i=0;i<noOfIndirectInodes;i++){
		struct Inode t;
		readInode(file1,t,1);
		temp.indirect.push_back(t);
	}
}

//The below function will load the meta data of the files at the begining of the programe. 
void LoadData(){
	fstream file1;
	file1.open("restore.txt",ios::in);
	int numberOfFiles;
	file1>>numberOfFiles;
	for(int i=0;i<numberOfFiles;i++){
		struct Inode temp;
		readInode(file1,temp,0);
		global->files[temp.filename]=temp;
	}
	int numberOfDirectories;
	file1>>numberOfDirectories;
	for(int i=0;i<numberOfDirectories;i++){
		struct Directory *temp=new Directory();
		string t;
		file1>>t;
		temp->name=t;
		while(chdir(t.c_str())!=0);
        globalStack.push(global);
        global=temp;
        LoadData();
        while(chdir("..")!=0);
        global=globalStack.top();
        globalStack.pop();
        global->directories[t]=temp;

	}
}

//The below function will act as entry point of the programe.
int main(){
	initialiseDiskBlockPointers();
	chdir("global");
	global=new Directory();
	global->name="global";
	LoadData();
	while(1){
		string s;
		cout<<"Commands Available:"<<endl;
		cout<<"mkdir foldername"<<endl;
		cout<<"cd foldername"<<endl;
		cout<<"del foldername"<<endl;
		cout<<"mf fileName \"file content\" ->create file"<<endl;
		cout<<"df fileName ->delete file"<<endl;
		cout<<"rf fileName1 fileName2 ->rename file"<<endl;
		cout<<"pf fileName ->display contents of file"<<endl;
		cout<<"ap fileName->to append content at the end of file"<<endl;
		cout<<"ls ->dispaly inode number and filename"<<endl;
		cout<<"exit->to come out of program"<<endl;
		cin>>s;

		if(s[0]=='m' && s[1]=='k'){
			makeDirectory();
		}
		else if(s[0]=='c' && s[1]=='d'){
			changeDirectory();
		}
		else if(s[0]=='d' && s[1]=='e'){
			deleteDirectory();
		}
		//MakeFile
		else if(s[0]=='m' && s[1]=='f'){
			makeFile();
		}

		//Delete File
		else if(s[0]=='d' && s[1]=='f'){
			deleteFile();
		}

		//Rename File
		else if(s[0]=='r'&&s[1]=='f'){
			renameFile();
		}
		//Print File
		else if(s[0]=='p' && s[1]=='f'){
			printFile();
		}
		//To list all files in directory
		else if(s[0]=='l' && s[1]=='s'){
			listFiles();
		}
		else if(s[0]=='a' and s[1]=='p'){
			appendFile();
		}
		//To exit from the program
		else if(s=="exit"){
          	while(!globalStack.empty())
         	{
         		global=globalStack.top();
         		globalStack.pop();
         		while(chdir("..")!=0);
         	}
			storeData(global);
			break;
		}
		cout<<"---------------------------------"<<endl;
	}
	return 0;
}
