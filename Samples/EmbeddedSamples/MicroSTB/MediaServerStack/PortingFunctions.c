/*   
Copyright 2006 - 2011 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MyString.h"
#include "MicroMediaServer.h"

#include "PortingFunctions.h"
#include "Utility.h"

/* Windows 32 */
#ifdef WIN32
#include <windows.h>
#include <time.h>
#include <crtdbg.h>
#endif

/* Win CE */
#ifdef UNDER_CE
#include <Winbase.h>
#endif

/* POSIX */
#ifdef _POSIX
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#define MAX_PATH_LENGTH 1024

void PCRandomize()
{
#ifdef WIN32
	srand((int)GetTickCount());
#endif

#ifdef UNDER_CE
	srand((int)GetTickCount());
#endif

#ifdef _POSIX
	struct timeval tv;
	gettimeofday(&tv,NULL);
	srand((int)tv.tv_sec);
#endif
}

// Windows Version
void PCCloseDir(void* handle)
{
#ifdef WIN32
	FindClose(*((HANDLE*)handle));
	FREE(handle);
#endif

#ifdef UNDER_CE
	FindClose(*((HANDLE*)handle));
	FREE(handle);
#endif

#ifdef _POSIX
	DIR* dirObj = (DIR*) handle;
	closedir(dirObj);
#endif
}

int PCFileClose(void *fileHandle)
{
	/* TODO: Modify to enable unicode char support */
	return fclose((FILE*)fileHandle);
}

void* PCFileOpen(const char* fullPath, const char *mode)
{
	/* TODO: Modify to enable unicode char support */
	return (void*) fopen(fullPath, mode);
}

int PCFileRead(void*dest, int itemSize, int itemCount, void *fileHandle)
{
	/* TODO: Modify to enable unicode char support */
	return (int) fread(dest, itemSize, itemCount, (FILE*)fileHandle);
}

int PCFileSeek(void *fileHandle, long offset, int origin)
{
	/* TODO: Modify to enable unicode char support */
	return fseek((FILE*)fileHandle, offset, origin);
}

int PCFileTell(void *fileHandle)
{
	/* TODO: Modify to enable unicode char support */
	return ftell((FILE*)fileHandle);
}

void* PCGetDirFirstFile(const char* directory, char* filename, int filenamelength, int* filesize)
{
#ifdef WIN32
	WIN32_FIND_DATA FileData;
	HANDLE* hSearch;
	char* direx;

	hSearch = MALLOC(sizeof(HANDLE));
	direx = MALLOC(filenamelength + 5);

	if (directory[(int) strlen(directory) - 1] == '\\')
	{
		sprintf(direx,"%s*.*",directory);
	}
	else
	{
		sprintf(direx,"%s\\*.*",directory);
	}

	*hSearch = FindFirstFile(direx, &FileData);
	FREE(direx);

	if (*hSearch == INVALID_HANDLE_VALUE)
	{
		FREE(hSearch);
		hSearch = NULL;
	}
	else
	{
		if (filename != NULL)
		{
			strToUtf8(filename,FileData.cFileName, filenamelength, 0, NULL);
		}

		if (filesize != NULL)
		{
			*filesize = FileData.nFileSizeLow;
		}
	}

	return hSearch;
#endif


#ifdef UNDER_CE
	WIN32_FIND_DATA FileData;
	HANDLE* hSearch;			/* must MMS_FREE */
	wchar_t* wdirectory;		/* must MMS_FREE */
	int wDirLen;
	int wDirSize;
	int mbDirLen;
	int mbDirSize;
	char* direx;				/* must MMS_FREE */

	hSearch = MALLOC(sizeof(HANDLE));
	direx = MALLOC(filenamelength + 5);

	if (directory[(int) strlen(directory) - 1] == '\\')
	{
		sprintf(direx,"%s*.*",directory);
	}
	else
	{
		sprintf(direx,"%s\\*.*",directory);
	}

	mbDirLen = (int) strlen(direx);
	mbDirSize = mbDirLen+1;
	wDirLen = mbDirLen * 2;
	wDirSize = mbDirSize * 2;
	wdirectory = (wchar_t*)MALLOC(wDirSize);

	if (mbstowcs(wdirectory,direx,wDirSize) == -1)
	{
		FREE(hSearch);
		hSearch = NULL;
	}
	else
	{
		*hSearch = FindFirstFile(wdirectory, &FileData);
		if (*hSearch == INVALID_HANDLE_VALUE)
		{
			FREE(hSearch);
			hSearch = NULL;
		}
		else
		{
			if (filename != NULL)
			{
				if (strToUtf8(filename,(char*)FileData.cFileName,filenamelength, 1, NULL) == -1)
				{
					FindClose(*hSearch);
					FREE(hSearch);
					hSearch = NULL;
				}
				else
				{
					if (filesize != NULL)
					{
						*filesize = FileData.nFileSizeLow;
					}
				}
			}
		}
	}

	FREE(direx);
	FREE(wdirectory);

	return hSearch;
#endif

#ifdef _POSIX
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
				strToUtf8(filename, dirEntry->d_name, filenamelength, 0, NULL);
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
int PCGetDirNextFile(void* handle, const char* dirName, char* filename, int filenamelength, int* filesize)
{
#ifdef WIN32
	WIN32_FIND_DATA FileData;
	
	if (FindNextFile(*((HANDLE*)handle), &FileData) == 0) {return 0;}
	strToUtf8(filename,FileData.cFileName,filenamelength,0, NULL);

	if (filesize != NULL)
	{
		*filesize = FileData.nFileSizeLow;
	}

	return 1;
#endif

#ifdef UNDER_CE
    WIN32_FIND_DATA FileData;
    int fnf = 0;
    int conv = -1;
    
    fnf = FindNextFile(*((HANDLE*)handle), &FileData);
    if (fnf == 0) {return 0;}

    conv = strToUtf8(filename, (char*)FileData.cFileName, filenamelength, 1, NULL);
    if (conv == -1) {return 0;}

	if (filesize != NULL)
	{
		*filesize = FileData.nFileSizeLow;
	}
    return 1;
#endif

#ifdef _POSIX
	DIR* dirObj;
	struct dirent* dirEntry;	/* dirEntry is a pointer to static memory in the C runtime lib for readdir()*/
	struct stat _si;
	char fullPath[1024];

	dirObj = (DIR*) handle;
	dirEntry = readdir(dirObj);

	if ((dirEntry != NULL) && ((int) strlen(dirEntry->d_name) < filenamelength))
	{
		strToUtf8(filename, dirEntry->d_name, filenamelength, 0, NULL);
		sprintf(fullPath, "%s%s", dirName, dirEntry->d_name);

		if (filesize != NULL)
		{
			/* WTF? Cygwin has a memory leak with stat. */
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

int PCGetFileDirType(char* directory)
{
#ifdef WIN32
	DWORD _si;
	int dirLen,dirSize;
	char *fullpath;

	dirLen = (int) strlen(directory);
	dirSize = dirLen+1;
	fullpath = (char*) MALLOC(dirSize);
	Utf8ToAnsi(fullpath, directory, dirSize);

	_si = GetFileAttributes(fullpath);
	
	FREE(fullpath);
	
	if (_si == 0xFFFFFFFF)
	{
		return 0;
	}

	if ((_si & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return 1;
	}
	else 
	{
		return 2;
	}
#endif

#ifdef UNDER_CE
	wchar_t* wfullPath;
	DWORD _si;
	int mbDirSize;
	int wPathSize;
	int dirLen,dirSize;
	char *fullpath;
	int retVal = 0;

	dirLen = (int) strlen(directory);
	dirSize = dirLen+1;
	fullpath = (char*) MALLOC(dirSize);
	Utf8ToAnsi(fullpath, directory, dirSize);

	mbDirSize = (int) strlen(fullpath) + 1;
	wPathSize = mbDirSize * 2;

	wfullPath = (wchar_t*)MALLOC(wPathSize);
	if (mbstowcs(wfullPath,fullpath,wPathSize) == -1)
	{
		retVal = 0;
	}
	else
	{
		_si = GetFileAttributes(wfullPath);
		if (_si == 0xFFFFFFFF)
		{
			retVal = 0;
		}
		else
		{
			if ((_si & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				retVal = 1;
			}
			else 
			{
				retVal = 2;
			}
		}
	}

	FREE(fullpath);
	FREE(wfullPath);

	return retVal;
#endif

#ifdef _POSIX
	struct stat _si;

	int dirLen,dirSize;
	char *fullpath;
	int pathExists;
	int retVal = 0;

	dirLen = (int) strlen(directory);
	dirSize = dirLen+1;
	fullpath = (char*) MALLOC(dirSize);
	Utf8ToAnsi(fullpath, directory, dirSize);

	pathExists = stat (fullpath, &_si);

	FREE(fullpath);

	if (pathExists != -1)
	{
		if ((_si.st_mode & S_IFDIR) == S_IFDIR)
		{
			retVal = 2;
		}
		else
		{
			retVal = 1;
		}
	}

	return retVal;
#endif
}

#ifdef _POSIX
/* only needed for posix because readdir returns statically allocated values */
pthread_mutex_t BrowseLock;
#endif

void InitBrowseLock()
{
#ifdef _POSIX
	pthread_mutex_init(&BrowseLock, NULL);
#endif
}

void LockBrowse()
{
#ifdef _POSIX
	pthread_mutex_lock(&BrowseLock);
#endif
}

void UnlockBrowse()
{
#ifdef _POSIX
	pthread_mutex_unlock(&BrowseLock);
#endif
}

void DestroyBrowseLock()
{
#ifdef _POSIX
	pthread_mutex_destroy(&BrowseLock);
#endif
}

void EndThisThread()
{
#ifdef _POSIX
	pthread_exit(NULL);
#endif

#ifdef WIN32
	ExitThread(0);
#endif

#ifdef UNDER_CE
	ExitThread(0);
#endif
}

void* SpawnNormalThread(void* method, void* arg)
{
#ifdef _POSIX
	int result;
	void* (*fptr) (void* a);
	pthread_t newThread;
	fptr = method;
	result = pthread_create(&newThread, NULL, fptr, arg);
	pthread_detach(newThread);
	return (void*) result;
#endif

#ifdef WIN32
	return CreateThread(NULL, 0, method, arg, 0, NULL );
#endif

#ifdef UNDER_CE
	return CreateThread(NULL, 0, method, arg, 0, NULL );
#endif
}

int PCGetFileSize(const char* fullPath)
{
	int filesize = -1;

#ifdef _POSIX
	struct stat _si;

	int pathLen,pathSize;
	char *fp;

	pathLen = (int) strlen(fullPath);
	pathSize = pathLen+1;
	fp = (char*) MALLOC(pathSize);
	Utf8ToAnsi(fp, fullPath, pathSize);

	if (stat(fp, &_si) != -1)
	{
		if (!((_si.st_mode & S_IFDIR) == S_IFDIR))
		{
			filesize = _si.st_size;
		}
	}

	FREE(fp);
#endif

#ifdef WIN32
	WIN32_FIND_DATA FileData;
	HANDLE* hSearch;			/* must MMS_FREE */
	int pathLen,pathSize;
	char *fp;

	pathLen = (int) strlen(fullPath);
	pathSize = pathLen+1;
	fp = (char*) MALLOC(pathSize);
	Utf8ToAnsi(fp, fullPath, pathSize);

	hSearch = MALLOC(sizeof(HANDLE));

	*hSearch = FindFirstFile(fp, &FileData);
	FREE(fp);

	if (*hSearch == INVALID_HANDLE_VALUE)
	{
		filesize = 0;
	}
	else
	{
		filesize = FileData.nFileSizeLow;
	}

	FindClose(*hSearch);
	FREE(hSearch);
#endif

#ifdef UNDER_CE
	WIN32_FIND_DATA FileData;
	HANDLE* hSearch;			/* must MMS_FREE */
	wchar_t* wdirectory;		/* must MMS_FREE */
	int wPathLen;
	int wPathSize;
	int fullPathLen;
	int fullPathSize;
	char* fp;

	fullPathLen = (int) strlen(fullPath);
	fullPathSize = fullPathLen + 1;
	fp = (char*) MALLOC(fullPathSize);
	Utf8ToAnsi(fp, fullPath, fullPathSize);

	hSearch = MALLOC(sizeof(HANDLE));

	wPathLen = fullPathLen * 2;
	wPathSize = fullPathSize * 2;
	wdirectory = (wchar_t*)MALLOC(wPathSize);

	if (mbstowcs(wdirectory,fp,wPathSize) == -1)
	{
		filesize = -1;
	}
	else
	{
		*hSearch = FindFirstFile(wdirectory, &FileData);
		if (*hSearch == INVALID_HANDLE_VALUE)
		{
			filesize = -1;
		}
		else
		{
			FindClose(*hSearch);
			filesize = FileData.nFileSizeLow;
		}
	}

	FREE(fp);
	FREE(wdirectory);
	FREE(hSearch);
#endif

	return filesize;
}

int PCGetGetDirEntryCount(const char* fullPath, char *dirDelimiter)
{
	char fn[MAX_PATH_LENGTH];
	void *dirObj;
	int retVal = 0;
	int ewDD;
	int nextFile;

	dirObj = PCGetDirFirstFile(fullPath, fn, MAX_PATH_LENGTH, NULL);

	if (dirObj != NULL)
	{
		ewDD = EndsWith(fullPath, dirDelimiter, 0);

		do
		{
			if (ProceedWithDirEntry(fullPath, fn, MAX_PATH_LENGTH) != 0)
			{
				retVal++;
			}

			nextFile = PCGetDirNextFile(dirObj,fullPath,fn,MAX_PATH_LENGTH, NULL);
		}
		while (nextFile != 0);

		PCCloseDir(dirObj);
	}

	return retVal;
}

/*
	 returns 0 if directory entry should not be processed.
	 returns nonzero if directory entry should be processed
*/
int ProceedWithDirEntry(const char* dirName, const char* filename, int maxPathLength)
{
	int dirLen;
	int fnLen;
	int val;

	char *fullpath;

	dirLen = (int) strlen(dirName);
	fnLen = (int) strlen(filename);

	if ((strcmp(filename, ".") == 0) || (strcmp(filename, "..") == 0))
	{
		/* NOP */
		return 0;
	}
	
	if ((dirLen+fnLen+2) > maxPathLength)
	{
		/* directory is too long */
		return 0;
	}

	/* prevent hidden files from showing up */
	fullpath = (char*) MALLOC(maxPathLength);
	memcpy(fullpath, dirName, dirLen);
	memcpy(fullpath+dirLen, filename, fnLen);
	fullpath[ dirLen+fnLen ] = '\0';
	val = PCGetFileDirType(fullpath);
	FREE(fullpath);
	if (val == 0)
	{
		return 0;
	}

		/*
		#ifdef UNDER_CE
		#define CF_CARD "CF Card"
		#define SD_CARD "SD Card"
		#define MMC_CARD "MMC Card"
		#define BUILT_IN_STORAGE "Built-in Storage"
		#define STORAGE "Storage"

				if (strcmp(dirName, "\\") == 0)
				{
					// only reveal directories that are storage cards from the root

					if (StartsWith(filename, CF_CARD, 1) != 0)
					{
						return 1;
					}
					if (StartsWith(filename, SD_CARD, 1) != 0)
					{
						return 1;
					}
					if (StartsWith(filename, MMC_CARD, 1) != 0)
					{
						return 1;
					}
					if (StartsWith(filename, BUILT_IN_STORAGE, 1) != 0)
					{
						return 1;
					}
					if (StartsWith(filename, STORAGE, 1) != 0)
					{
						return 1;
					}

					return 0;
				}
		#endif
		*/

		return 1;
}

unsigned int PCGetSystemUpdateID()
{
	/*
	 *	TODO: Return a number indicating the state of the metadata store.
	 *	Whenever the metadata in the CDS changes, this value should
	 *	increase monotomically.
	 *
	 *	For file systems, the most reliable method to obtain this value would likely
	 *	be the most recent file system date.
	 */

	return 0;
}

unsigned int PCGetContainerUpdateID(const char* path)
{
	/*
	 *	TODO: Return a number indicating the state of the directory.
	 *	Whenever the entries in the directory change for any reason, this value should
	 *	increase monotomically for that directory. Each directory should have
	 *	its own UpdateID.
	 *
	 *	For file systems, the most reliable to obtain this value would likely
	 *	be the most recent file system date of a file/dir in the provided directory.
	 */

	return 0;
}
