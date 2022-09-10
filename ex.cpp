#include<bits/stdc++.h>
using namespace std;
int main(){
	char *buffer;

	buffer = (char *)calloc(1024,sizeof(char));
    
    cout<<"size of char = "<<sizeof(char)<<endl;

    for(int i=0;i<1023;i++){
    	buffer[i] = '0';
    }

    cout<<buffer[5]<<endl;

	return 0;
}