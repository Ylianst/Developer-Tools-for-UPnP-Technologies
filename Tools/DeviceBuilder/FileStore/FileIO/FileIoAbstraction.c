
#if defined(WIN32)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#if !defined(_WIN32_WCE)
		#include <crtdbg.h>
	#endif
#else
	#include <stdlib.h>
#endif

#include <stdio.h>
#include <string.h>


/* Windows 32 */
#if defined(WIN32)
	#include <windows.h>
	#include <time.h>
	#include <direct.h>
	#define getcwd(path, sizeOfBuffers) _getcwd(path,sizeOfBuffers)
#endif


/* POSIX */
#if defined(_POSIX)
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <dirent.h>
	#include <sys/time.h>
	#include <unistd.h>
#endif

#if defined(WIN32) || defined(_WIN32_WCE)
	#define strncasecmp(x,y,z) _strnicmp(x,y,z)
	#define strcasecmp(x,y) _stricmp(x,y)
#endif


#if defined(_WIN32_WCE)
	#include <Winbase.h>
#endif
#include "FileIoAbstraction.h"


int EndsWith(/*INOUT*/ const char* str, const char* endsWith, int ignoreCase)
{
	int strLen, ewLen, offset;
	int cmp = 0;

	strLen = (int) strlen(str);
	ewLen = (int) strlen(endsWith);
	if (ewLen > strLen) return 0;
	offset = strLen - ewLen;

	if (ignoreCase != 0)
	{
		cmp = strncasecmp(str+offset, endsWith, ewLen);
	}
	else
	{
		cmp = strncmp(str+offset, endsWith, ewLen);
	}

	return cmp == 0?1:0;
}

void ILibFileDir_CloseDir(void* handle)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	FindClose(*((HANDLE*)handle));
	free(handle);
#else
	DIR* dirObj = (DIR*) handle;
	closedir(dirObj);
#endif
}


void* ILibFileDir_GetDirFirstFile(const char* directory, /*INOUT*/ char* filename, int filenamelength, /*INOUT*/ int* filesize)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	WIN32_FIND_DATA FileData;
	HANDLE* hSearch;
	char* direx;
	#if defined(_WIN32_WCE)
		wchar_t *tempChar;
		int tempCharLength;
	#endif


	hSearch = malloc(sizeof(HANDLE));
	direx = malloc(filenamelength + 5);

	if (directory[(int) strlen(directory) - 1] == '\\')
	{
		sprintf(direx,"%s*.*",directory);
	}
	else
	{
		sprintf(direx,"%s\\*.*",directory);
	}

	#if defined(_WIN32_WCE)
		tempCharLength = MultiByteToWideChar(CP_UTF8,0,direx,-1,NULL,0);
		tempChar = (wchar_t*)malloc(sizeof(wchar_t)*tempCharLength);
		MultiByteToWideChar(CP_UTF8,0,direx,-1,tempChar,tempCharLength);
		*hSearch = FindFirstFile(tempChar, &FileData);
		free(direx);
		free(tempChar);
	#else
		*hSearch = FindFirstFile(direx, &FileData);
		free(direx);
	#endif

	if (*hSearch == INVALID_HANDLE_VALUE)
	{
		free(hSearch);
		hSearch = NULL;
	}
	else
	{
		if (filename != NULL)
		{
#if defined(UNICODE)
			WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)FileData.cFileName,-1,filename,filenamelength,NULL,NULL);
#else
			memcpy(filename,FileData.cFileName,1+(int)strlen(FileData.cFileName));
#endif
		}

		if (filesize != NULL)
		{
			*filesize = FileData.nFileSizeLow;
		}
	}

	return hSearch;
#else
	DIR* dirObj;
	struct dirent* dirEntry;	/* dirEntry is a pointer to static memory in the C runtime lib for readdir()*/
	struct stat _si;
	char fullPath[1024];
	
	dirObj = opendir(directory);

	if (dirObj != NULL)
	{
		dirEntry = readdir(dirObj);

		if ((dirEntry != NULL) && ((int) strlen(dirEntry->d_name) < filenamelength))
		{
			if (filename != NULL)
			{
				strcpy(filename,dirEntry->d_name);
				sprintf(fullPath, "%s%s", directory, dirEntry->d_name);

				if (filesize != NULL)
				{
					if (stat(fullPath, &_si) != -1)
					{
						if ((_si.st_mode & S_IFDIR) == S_IFDIR)
						{
							*filesize = 0;
						}
						else
						{
							*filesize = _si.st_size;
						}
					}
				}
			}
		}
	}

	return dirObj;
#endif
}




// Windows Version
// 0 = No More Files
// 1 = Next File
int ILibFileDir_GetDirNextFile(void* handle, const char* dirName, char* filename, int filenamelength, int* filesize)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	WIN32_FIND_DATA FileData;
	
	if (FindNextFile(*((HANDLE*)handle), &FileData) == 0) {return 0;}
	#if defined(UNICODE)
		WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)FileData.cFileName,-1,filename,filenamelength,NULL,NULL);
	#else
		memcpy(filename,FileData.cFileName,1+(int)strlen(FileData.cFileName));
	#endif
	if (filesize != NULL)
	{
		*filesize = FileData.nFileSizeLow;
	}

	return 1;
#else
	DIR* dirObj;
	struct dirent* dirEntry;	/* dirEntry is a pointer to static memory in the C runtime lib for readdir()*/
	struct stat _si;
	char fullPath[1024];

	dirObj = (DIR*) handle;
	dirEntry = readdir(dirObj);

	if ((dirEntry != NULL) && ((int) strlen(dirEntry->d_name) < filenamelength))
	{
		strcpy(filename,dirEntry->d_name);
		sprintf(fullPath, "%s%s", dirName, dirEntry->d_name);

		if (filesize != NULL)
		{
			/* ? Cygwin has a memory leak with stat. */
			if (stat(fullPath, &_si) != -1)
			{
				if ((_si.st_mode & S_IFDIR) == S_IFDIR)
				{
					*filesize = 0;
				}
				else
				{
					*filesize = _si.st_size;
				}
			}
		}
		return 1;
	}

	return 0;
#endif
}

enum ILibFileDir_Type ILibFileDir_GetType(char* directory)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	DWORD _si;
	int dirLen,dirSize;
	#if defined(_WIN32_WCE)
		wchar_t *tempChar;
		int tempCharLength;
	#endif

	dirLen = (int) strlen(directory);
	dirSize = dirLen+1;

#if defined(_WIN32_WCE)
	tempCharLength = MultiByteToWideChar(CP_UTF8,0,directory,-1,NULL,0);
	tempChar = (wchar_t*)malloc(sizeof(wchar_t)*tempCharLength);
	MultiByteToWideChar(CP_UTF8,0,directory,-1,tempChar,tempCharLength);
	_si = GetFileAttributes(tempChar);
#else
	_si = GetFileAttributes(directory);
#endif
	
	if (_si == 0xFFFFFFFF)
	{
		return ILibFileDir_Type_NOT_FOUND_ERROR;
	}

	if ((_si & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return ILibFileDir_Type_FILE;
	}
	else 
	{
		return ILibFileDir_Type_DIRECTORY;
	}
#else
	struct stat _si;

	int dirLen,dirSize;
	char *fullpath;
	int pathExists;
	enum ILibFileDir_Type retVal = ILibFileDir_Type_NOT_FOUND_ERROR;

	dirLen = (int) strlen(directory);
	dirSize = dirLen+1;
	fullpath = (char*) malloc(dirSize);

	pathExists = stat (directory, &_si);

	if (pathExists != -1)
	{
		if ((_si.st_mode & S_IFDIR) == S_IFDIR)
		{
			retVal = ILibFileDir_Type_DIRECTORY;
		}
		else
		{
			retVal = ILibFileDir_Type_FILE;
		}
	}
	return retVal;
#endif
}

int ILibFileDir_GetDirEntryCount(const char* fullPath, char *dirDelimiter)
{
	char fn[MAX_PATH_LENGTH];
	void *dirObj;
	int retVal = 0;
	int ewDD;
	int nextFile;

	dirObj = ILibFileDir_GetDirFirstFile(fullPath, fn, MAX_PATH_LENGTH, NULL);

	if (dirObj != NULL)
	{
		ewDD = EndsWith(fullPath, dirDelimiter, 0);

		do
		{
			retVal++;
			nextFile = ILibFileDir_GetDirNextFile(dirObj,fullPath,fn,MAX_PATH_LENGTH, NULL);
		}
		while (nextFile != 0);

		ILibFileDir_CloseDir(dirObj);
	}

	return retVal;
}

char* ILibFileDir_GetWorkingDir(char *path, size_t sizeOfBuf)
{
#if defined(_WIN32_WCE)
	if(path==NULL)
	{
		path = (char*)malloc(2);
	}
	sprintf(path,"/");
	return(path);
#else
	return getcwd(path, (int)sizeOfBuf);
#endif
}
int ILibFileDir_DeleteFile(char *FileName)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	return(!DeleteFile((LPCTSTR)FileName));
#else
	return(remove(FileName));
#endif
}
int ILibFileDir_DeleteDir(char *path)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	return(!RemoveDirectory((LPCTSTR)path));
#else
	return(rmdir(path));
#endif
}
int ILibFileDir_CreateDir(char *path)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	return(CreateDirectory((LPCTSTR)path,NULL));
#else
	return(mkdir(path));
#endif
}
long ILibFileDir_GetFileSize(char *FileName)
{
	char *buffer;
	long SourceFileLength;
	FILE *SourceFile = fopen(FileName,"rb");

	if(SourceFile==NULL)
	{
		return(-1);
	}

	fseek(SourceFile,0,SEEK_END);
	
	SourceFileLength = ftell(SourceFile);
	fclose(SourceFile);
	return(SourceFileLength);
}
