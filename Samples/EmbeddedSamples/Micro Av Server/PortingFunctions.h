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

#ifndef PORTINGFUNCTIONS_H
#define PORTINGFUNCTIONS

void PCRandomize();

/*
 *	Use to a close a file after streaming is complete.
 *		fileHandle returned from PCFileOpen. 
 */
int PCFileClose(void *fileHandle);

/*
 *	Use to open a file for streaming.
 *		Returns a void* fileHandle for use with other PCFilexxx methods.
 *		Arguments behave exactly like fopen().
 */
void* PCFileOpen(const char* fullPath, const char *mode);

/*
 *	Use to read a file. 
 *		fileHandle returned from PCFileOpen.
 *		Other arguments behave exactly like fread().
 */
int PCFileRead(void *dest, int itemSize, int itemCount, void *fileHandle);

/*
 *	Use to move the file pointer to a specific offset.
 *		fileHandle returned from PCFileOpen.
 *		Other arguments behave exactly like fread().
 */
int PCFileSeek(void *fileHandle, long offset, int origin);

/*
 *	Use to obtain the current file pointer position.
 *		fileHandle returned from PCFileOpen.
 */
int PCFileTell(void *fileHandle);

/*
 *	Returns a state number for the file system.
 *	Ideally, we would return the most recent file-system
 *	date to indicate the latest state. Currently we only
 *	return 0.
 */
unsigned int PCGetSystemUpdateID();

/*
 *	Returns a state number for a directory, given a path.
 *	Ideally, we would return the msot recent date for an entry
 *	within the directory, but currently we only return 0.
 */
unsigned int PCGetContainerUpdateID(const char* path);

/*
 *	Returns an integer describing the type of content at this path.
 *		0 = Does Not Exist
 *		1 = Is a File
 *		2 = Is a Directory
 *
 *	Provided path is a UTF8-encoded path into the file system.
 */
int PCGetFileDirType(char* path);

/*
 *	Closes a directory or specific path.
 *
 *	dirHandle returned from PCGetDirFirstFile.
 */
void PCCloseDir(void* dirHandle);

/*
 *	Opens a directory for enumeration or a specific file.
 *		Returns void* dirHandle.
 *
 *		directory		is the directory portion of the path
 *		filename		allocated byte array, contains the first filename in that directory path upon return
 *		filenamelength	the number of bytes available in filename
 *		fileSize		the size of the file (specified by dirName+filename)
 */
void* PCGetDirFirstFile(const char* dirName, /*INOUT*/char* filename, /*IN*/int filenamelength, /*INOUT*/int* fileSize);

/*
 *	Obtains the next entry in the directory.
 *		dirHandle		the void* returned in PCGetDirFirstFile
 *		dirName			the directory name
 *		filename		allocated byte array, contains the next filename in that directory path upon return
 *		filenamelength	the number of bytes available in filename
 *		fileSize		the size of the file (specified by dirName+filename)
 */
int PCGetDirNextFile(void* dirHandle, const char* dirName, /*INOUT*/char* filename, int filenamelength, int* fileSize);

/*
 *	Obtains the filesize of the file specified by fullpath.
 */
int PCGetFileSize(const char* fullPath);

/*
 *	Returns the number of entries specified in fullPath. 
 *
 *		dirDelimiter	single character to identify the character used to delimit directories within a path.
 *						Posix usually uses '/' whereas windows usually uses '\'
 */
int PCGetGetDirEntryCount(const char* fullPath, char *dirDelimiter);


void EndThisThread();
void LockBrowse();
void UnlockBrowse();
void InitBrowseLock();
void DestroyBrowseLock();
void* SpawnNormalThread(void* method, void* arg);
int ProceedWithDirEntry(const char* dirName, const char* filename, int maxPathLength);
#endif
