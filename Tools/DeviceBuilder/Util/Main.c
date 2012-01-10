
#ifndef MICROSTACK_NO_STDAFX
#include "stdafx.h"
#endif
#define _CRTDBG_MAP_ALLOC
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>
#include <winsock2.h>
#include "ILibParsers.h"



/* Main entry point to the sample application */
int _tmain(int argc, _TCHAR* argv[])
{
	char *targetFile;

	char *tempBuffer;
	int tempBufferLength;

	char *sourceBuffer;
	char *buffer;
	char *delimiter;
	int delimiterLength;

	int x;

	FILE* TargetFile;
	int TargetFileLength;
	int SourceFileLength;
	struct parser_result *pr;

	HANDLE target,src;
	FILETIME targetFT,srcFT;

	DWORD ptid=0;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		
	targetFile = (char*)malloc(2+strlen(argv[1]));
	strcpy(targetFile,argv[1]);
	targetFile[strlen(argv[1])] = '~';
	targetFile[strlen(argv[1])+1] = 0;

	if(argc==2)
	{
		TargetFileLength = ILibReadFileFromDiskEx(&buffer,targetFile);
		if(TargetFileLength!=0)
		{
			ILibWriteStringToDiskEx(argv[1],buffer,TargetFileLength);
			free(buffer);
			ILibDeleteFileFromDisk(targetFile);
		}
		return;
	}

	target = CreateFile(argv[1],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	src = CreateFile(argv[2],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	GetFileTime(target,NULL,NULL,&targetFT);
	GetFileTime(src,NULL,NULL,&srcFT);

	CloseHandle(target);
	CloseHandle(src);

	if(srcFT.dwHighDateTime < targetFT.dwHighDateTime ||
		(srcFT.dwHighDateTime == targetFT.dwHighDateTime && srcFT.dwLowDateTime < targetFT.dwLowDateTime))
	{
		//return;
	}

	printf("SourceCodeInjector:\r\n");
	printf("	TargetFile: %s\r\n",argv[1]);
	printf("	SourceFile: %s\r\n",argv[2]);
	printf("	VariableName: %s\r\n",argv[3]);
	if(argc==5)
	{
		printf("	Delimiter: %s\r\n",argv[4]);
	}
	// argv[1] = TargetFile
	// argv[2] = SourceFile
	// argv[3] = VariableName

	if(argc==5)
	{
		delimiter = (char*)malloc((int)strlen(argv[4])+1);
		delimiterLength=sprintf(delimiter,"%s",argv[4]);
	}
	else
	{
		delimiter = (char*)malloc((int)strlen(argv[3])+20);
		delimiterLength=sprintf(delimiter,"// -=S3P4R470R=- {%s}",argv[3]);
	}

	TargetFileLength = ILibReadFileFromDiskEx(&buffer,targetFile);
	SourceFileLength = ILibReadFileFromDiskEx(&sourceBuffer,argv[2]);

	if(TargetFileLength == 0)
	{
		TargetFileLength = ILibReadFileFromDiskEx(&buffer,argv[1]);
	}

	if(TargetFileLength == NULL || SourceFileLength == 0)
	{
		exit(0);
	}
	

	tempBufferLength = ILibBase64Encode(sourceBuffer,SourceFileLength,&tempBuffer);
	free(sourceBuffer);
	sourceBuffer = tempBuffer;
	SourceFileLength = tempBufferLength;

	pr = ILibParseString(buffer,0,TargetFileLength,delimiter,delimiterLength);

	TargetFile = fopen(targetFile,"wb");
	fwrite(pr->FirstResult->data,sizeof(char),pr->FirstResult->datalength,TargetFile);
	fwrite(delimiter,sizeof(char),delimiterLength,TargetFile);
	fwrite("\n",sizeof(char),1,TargetFile);

	//fwrite("private static byte[] ",sizeof(char),22,TargetFile);
	//fwrite(argv[3],sizeof(char),strlen(argv[3]),TargetFile);
	//fwrite(" = {",sizeof(char),4,TargetFile);

	fwrite("private static string ",sizeof(char),22,TargetFile);
	fwrite(argv[3],sizeof(char),strlen(argv[3]),TargetFile);
	fwrite(" = \"",sizeof(char),4,TargetFile);

	for(x=0;x<SourceFileLength;++x)
	{
		if(argv[3] != '-')
		{
			if(sourceBuffer[x] == '"')
			{
				fwrite("\\\"",sizeof(char),2,TargetFile);
			}
			else if(sourceBuffer[x] == '\\')
			{
				fwrite("\\\\",sizeof(char),2,TargetFile);
			}
			else if(sourceBuffer[x] == '\r')
			{
				fwrite("\\r",sizeof(char),2,TargetFile);
			}
			else if(sourceBuffer[x] == '\n')
			{
				fwrite("\\n",sizeof(char),2,TargetFile);
			}
			else
			{
				fwrite(&sourceBuffer[x],sizeof(char),1,TargetFile);
			}

			if(x!=0 && x%100==0)
			{
				fwrite("\"\n",sizeof(char),2,TargetFile);
				//if(x!=SourceFileLength-1)
				//{
					fwrite("+\"",sizeof(char),2,TargetFile);
				//}
			}
		}
		else
		{
			fwrite(&sourceBuffer[x],sizeof(char),1,TargetFile);
		}
	}

	if(argv[3]!='-')
	{
		fwrite("\";\n",sizeof(char),3,TargetFile);
	}

	fwrite(delimiter,sizeof(char),delimiterLength,TargetFile);
	fwrite(pr->LastResult->data,sizeof(char),pr->LastResult->datalength,TargetFile);
	fflush(TargetFile);
	fclose(TargetFile);


	ILibDestructParserResults(pr);
	free(sourceBuffer);
	free(buffer);
	free(delimiter);
	free(targetFile);
	return 0;
}

