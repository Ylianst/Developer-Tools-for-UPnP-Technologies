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

#ifndef MICROSTACK_NO_STDAFX
#include "stdafx.h"
#endif
#include <windows.h>
#include <math.h>
#include <winioctl.h>
#include <winbase.h>
#include <winerror.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <winsock.h>
#include <wininet.h>

#include "ILibParsers.h"
#define DEBUGSTATEMENT(x)
#define UPNP_MAX_WAIT 2000000000
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
static sem_t ILibChainLock = NULL;
static int ILibChainLock_RefCounter = 0;

static int malloc_counter = 0;
void* dbg_malloc(int sz)
{
	++malloc_counter;
	return((void*)malloc(sz));
}
void dbg_free(void* ptr)
{
	--malloc_counter;
	free(ptr);	
}
int dbg_GetCount()
{
	return(malloc_counter);
}

struct ILibStackNode
{
	void *Data;
	struct ILibStackNode *Next;
};
struct HashNode
{
	struct HashNode *Next;
	struct HashNode *Prev;
	int KeyHash;
	char *KeyValue;
	int KeyLength;
	void *Data;
};
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define gettimeofday(x,y) (x)->tv_sec = GetTickCount()/1000
int ILibGetLocalIPAddressList(int** pp_int)
{
	char name[256];
	int i = 0;
	int num = 0;
	struct hostent *entry;
	
	gethostname(name,256);
	entry = (struct hostent*)gethostbyname(name);
	
	if (entry->h_length != 4) return 0;
	while (entry->h_addr_list[num]!=0) ++num;
	*pp_int = (int*)MALLOC(sizeof(int)*num);
	
	for (i = 0;i < num;++i)
	{
		(*pp_int)[i] = *((u_long*)entry->h_addr_list[i]);
	}
	
	return num;
}

struct ILibChain
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
};

struct LifeTimeMonitorData
{
	long ExpirationTick;
	void *data;
	void (*CallbackPtr)(void *data);
	void (*DestroyPtr)(void *data);
	struct LifeTimeMonitorData *Next;
};
struct ILibLifeTime
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	struct LifeTimeMonitorData *LM;
	void *Chain;
	sem_t SyncLock;
};
struct ILibBaseChain
{
	SOCKET Terminate;
	int TerminateFlag;
	void *Object;
	void *Next;
};

void *ILibCreateChain()
{
	struct ILibBaseChain *RetVal = (struct ILibBaseChain*)MALLOC(sizeof(struct ILibBaseChain));
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 1, 1 );	
	if (WSAStartup( wVersionRequested, &wsaData ) != 0) {exit(1);}
	
	RetVal->Object = NULL;
	RetVal->Next = NULL;
	RetVal->Terminate = socket(AF_INET, SOCK_DGRAM, 0);
	RetVal->TerminateFlag = 0;
	
	if(ILibChainLock_RefCounter==0)
	{
		sem_init(&ILibChainLock,0,1);
	}
	return(RetVal);
}
void ILibAddToChain(void *Chain, void *object)
{
	struct ILibBaseChain *chain = (struct ILibBaseChain*)Chain;
	while(chain->Next!=NULL)
	{
		chain = chain->Next;
	}
	if(chain->Object!=NULL)
	{
		chain->Next = (struct ILibBaseChain*)MALLOC(sizeof(struct ILibBaseChain));
		chain = chain->Next;
	}
	chain->Object = object;
	chain->Next = NULL;
}
void ILibForceUnBlockChain(void *Chain)
{
	struct ILibBaseChain *c = (struct ILibBaseChain*)Chain;
	SOCKET temp;
	
	sem_wait(&ILibChainLock);
	
	temp = c->Terminate;
	c->Terminate = ~0;
	closesocket(temp);
	
	sem_post(&ILibChainLock);
	
}
void ILibStartChain(void *Chain)
{
	struct ILibBaseChain *c = (struct ILibBaseChain*)Chain;
	struct ILibBaseChain *temp;
	fd_set readset;
	fd_set errorset;
	fd_set writeset;
	struct timeval tv;
	int slct;
	
	srand(GetTickCount());
	FD_ZERO(&readset);
	FD_ZERO(&errorset);
	FD_ZERO(&writeset);
	while(((struct ILibBaseChain*)Chain)->TerminateFlag==0)
	{
		slct = 0;
		FD_ZERO(&readset);
		FD_ZERO(&errorset);
		FD_ZERO(&writeset);
		tv.tv_sec = UPNP_MAX_WAIT;
		tv.tv_usec = 0;
		
		sem_wait(&ILibChainLock);
		if(((struct ILibBaseChain*)Chain)->Terminate==~0)
		{
			slct = -1;
		}
		else
		{
			FD_SET(((struct ILibBaseChain*)Chain)->Terminate,&errorset);
		}
		sem_post(&ILibChainLock);
		
		c = (struct ILibBaseChain*)Chain;
		while(c!=NULL && c->Object!=NULL)
		{
			if(((struct ILibChain*)c->Object)->PreSelect!=NULL)
			{
				((struct ILibChain*)c->Object)->PreSelect(c->Object,&readset,&writeset,&errorset,(int*)&tv.tv_sec);
			}
			c = c->Next;
		}
		if(slct!=0)
		{
			tv.tv_sec = 0;
		}
		slct = select(FD_SETSIZE,&readset,&writeset,&errorset,&tv);
		if(slct==-1)
		{
			FD_ZERO(&readset);
			FD_ZERO(&writeset);
			FD_ZERO(&errorset);
		}
		
		if(((struct ILibBaseChain*)Chain)->Terminate==~0)
		{
			((struct ILibBaseChain*)Chain)->Terminate = socket(AF_INET,SOCK_DGRAM,0);
		}
		c = (struct ILibBaseChain*)Chain;
		while(c!=NULL && c->Object!=NULL)
		{
			if(((struct ILibChain*)c->Object)->PostSelect!=NULL)
			{
				((struct ILibChain*)c->Object)->PostSelect(c->Object,slct,&readset,&writeset,&errorset);
			}
			c = c->Next;
		}
	}
	c = (struct ILibBaseChain*)Chain;
	while(c!=NULL && c->Object!=NULL)
	{
		if(((struct ILibChain*)c->Object)->Destroy!=NULL)
		{
			((struct ILibChain*)c->Object)->Destroy(c->Object);
		}
		FREE(c->Object);
		c = c->Next;
	}
	
	c = (struct ILibBaseChain*)Chain;
	while(c!=NULL)
	{
		temp = c->Next;
		FREE(c);
		c = temp;
	}
	WSACleanup();
	if(ILibChainLock_RefCounter==1)
	{
		sem_destroy(&ILibChainLock);
	}
	--ILibChainLock_RefCounter;
}
void ILibStopChain(void *Chain)
{
	((struct ILibBaseChain*)Chain)->TerminateFlag = 1;
	ILibForceUnBlockChain(Chain);
}
void ILibDestructXMLNodeList(struct ILibXMLNode *node)
{
	struct ILibXMLNode *temp;
	while(node!=NULL)
	{
		temp = node->Next;
		FREE(node);
		node = temp;
	}
}
void ILibDestructXMLAttributeList(struct ILibXMLAttribute *attribute)
{
	struct ILibXMLAttribute *temp;
	while(attribute!=NULL)
	{
		temp = attribute->Next;
		FREE(attribute);
		attribute = temp;
	}
}
int ILibProcessXMLNodeList(struct ILibXMLNode *nodeList)
{
	int RetVal = 0;
	struct ILibXMLNode *current = nodeList;
	struct ILibXMLNode *temp;
	void *TagStack;
	
	ILibCreateStack(&TagStack);
	
	while(current!=NULL)
	{
		if(current->StartTag!=0)
		{
			// Start Tag
			current->Parent = ILibPeekStack(&TagStack);
			ILibPushStack(&TagStack,current);
		}
		else
		{
			// Close Tag
			temp = (struct ILibXMLNode*)ILibPopStack(&TagStack);
			if(temp!=NULL)
			{
				if(temp->NameLength==current->NameLength && memcmp(temp->Name,current->Name,current->NameLength)==0)
				{
					if(current->Next!=NULL)
					{
						if(current->Next->StartTag!=0)
						{
							temp->Peer = current->Next;
						}
					}
					temp->ClosingTag = current;
				}
				else
				{
					// Illegal Close Tag Order
					RetVal = -2;
					break;
				}
			}
			else
			{
				// Illegal Close Tag
				RetVal = -1;
				break;
			}
		}
		current = current->Next;
	}
	
	if(TagStack!=NULL)
	{
		// Incomplete XML
		RetVal = -3;
	}
	
	return(RetVal);
}
int ILibReadInnerXML(struct ILibXMLNode *node, char **RetVal)
{
	struct ILibXMLNode *x = node;
	int length = 0;
	void *TagStack;
	
	ILibCreateStack(&TagStack);
	do
	{
		if(x->StartTag!=0) {ILibPushStack(&TagStack,x);}
		x = x->Next;
	}while(!(x->StartTag==0 && ILibPopStack(&TagStack)==node && x->NameLength==node->NameLength && memcmp(x->Name,node->Name,node->NameLength)==0));
	
	length = (int)((char*)x->Reserved - (char*)node->Reserved - 1);
	if(length<0) {length=0;}
	*RetVal = (char*)node->Reserved;
	return(length);
}
struct ILibXMLAttribute *ILibGetXMLAttributes(struct ILibXMLNode *node)
{
	struct ILibXMLAttribute *RetVal = NULL;
	struct ILibXMLAttribute *current = NULL;
	char *c;
	int EndReserved = (node->EmptyTag==0)?1:2;
	
	struct parser_result *xml;
	struct parser_result_field *field;
	struct parser_result *temp2;
	struct parser_result *temp3;
	
	c = (char*)node->Reserved - 1;
	while(*c!='<')
	{
		c = c -1;
	}
	c = c +1;
	
	xml = ILibParseStringAdv(c,0,(int)((char*)node->Reserved - c -EndReserved)," ",1);
	field = xml->FirstResult;
	if(field!=NULL) {field = field->NextResult;}
	while(field!=NULL)
	{
		if(RetVal==NULL)
		{
			RetVal = (struct ILibXMLAttribute*)MALLOC(sizeof(struct ILibXMLAttribute));
			RetVal->Next = NULL;
		}
		else
		{
			current = (struct ILibXMLAttribute*)MALLOC(sizeof(struct ILibXMLAttribute));
			current->Next = RetVal;
			RetVal = current;
		}
		temp2 = ILibParseStringAdv(field->data,0,field->datalength,":",1);
		if(temp2->NumResults==1)
		{
			RetVal->Prefix = NULL;
			RetVal->PrefixLength = 0;
			temp3 = ILibParseStringAdv(field->data,0,field->datalength,"=",1);
		}
		else
		{
			RetVal->Prefix = temp2->FirstResult->data;
			RetVal->PrefixLength = temp2->FirstResult->datalength;
			temp3 = ILibParseStringAdv(field->data,RetVal->PrefixLength+1,field->datalength-RetVal->PrefixLength-1,"=",1);
		}
		ILibDestructParserResults(temp2);
		RetVal->Name = temp3->FirstResult->data;
		RetVal->NameLength = temp3->FirstResult->datalength;
		RetVal->Value = temp3->LastResult->data;
		RetVal->ValueLength = temp3->LastResult->datalength;
		ILibDestructParserResults(temp3);
		field = field->NextResult;
	}
	
	ILibDestructParserResults(xml);
	return(RetVal);
	
}
struct ILibXMLNode *ILibParseXML(char *buffer, int offset, int length)
{
	struct parser_result *xml;
	struct parser_result_field *field;
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	char* TagName;
	int TagNameLength;
	int StartTag;
	int EmptyTag;
	int i;
	
	struct ILibXMLNode *RetVal = NULL;
	struct ILibXMLNode *current = NULL;
	struct ILibXMLNode *x = NULL;
	
	char *NSTag;
	int NSTagLength;
	
	xml = ILibParseString(buffer,offset,length,"<",1);
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if(memcmp(field->data,"?",1)!=0)
		{
			EmptyTag = 0;
			if(memcmp(field->data,"/",1)==0)
			{
				StartTag = 0;
				field->data = field->data+1;
				field->datalength -= 1;
				temp2 = ILibParseString(field->data,0,field->datalength,">",1);
			}
			else
			{
				StartTag = -1;
				temp2 = ILibParseString(field->data,0,field->datalength,">",1);
				if(temp2->FirstResult->data[temp2->FirstResult->datalength-1]=='/')
				{
					EmptyTag = -1;
				}
			}
			temp = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength," ",1);
			temp3 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp3->NumResults==1)
			{
				NSTag = NULL;
				NSTagLength = 0;
				TagName = temp3->FirstResult->data;
				TagNameLength = temp3->FirstResult->datalength;
			}
			else
			{
				NSTag = temp3->FirstResult->data;
				NSTagLength = temp3->FirstResult->datalength;
				TagName = temp3->FirstResult->NextResult->data;
				TagNameLength = temp3->FirstResult->NextResult->datalength;
			}
			ILibDestructParserResults(temp3);
			
			for(i=0;i<TagNameLength;++i)
			{
				if( (TagName[i]==' ')||(TagName[i]=='/')||(TagName[i]=='>')||(TagName[i]=='\r')||(TagName[i]=='\n') )
				{
					if(i!=0)
					{
						if(TagName[i]=='/')
						{
							EmptyTag = -1;
						}
						TagNameLength = i;
						break;
					}
				}
			}
			
			if(TagNameLength!=0)
			{
				x = (struct ILibXMLNode*)MALLOC(sizeof(struct ILibXMLNode));
				x->Next = NULL;
				x->Name = TagName;
				x->NameLength = TagNameLength;
				x->StartTag = StartTag;
				x->NSTag = NSTag;
				x->NSLength = NSTagLength;
				
				x->Parent = NULL;
				x->Peer = NULL;
				x->ClosingTag = NULL;
				x->EmptyTag = 0;
				
				
				if(StartTag==0)
				{
					x->Reserved = field->data;
					do
					{
						(char*)x->Reserved -= 1;
					}while(*((char*)x->Reserved)=='<');
				}
				else
				{
					x->Reserved = temp2->LastResult->data;
				}
				
				if(RetVal==NULL)
				{
					RetVal = x;
				}
				else
				{
					current->Next = x;
				}
				current = x;
				if(EmptyTag!=0)
				{
					x = (struct ILibXMLNode*)MALLOC(sizeof(struct ILibXMLNode));
					x->Next = NULL;
					x->Name = TagName;
					x->NameLength = TagNameLength;
					x->StartTag = 0;
					x->NSTag = NSTag;
					x->NSLength = NSTagLength;
					
					x->Parent = NULL;
					x->Peer = NULL;
					x->ClosingTag = NULL;
					
					x->Reserved = current->Reserved;
					current->EmptyTag = -1;
					current->Next = x;
					current = x;
				}
			}
			
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	ILibDestructParserResults(xml);
	return(RetVal);
}
void ILibCreateStack(void **TheStack)
{
	*TheStack = NULL;
}
void ILibPushStack(void **TheStack, void *data)
{
	struct ILibStackNode *RetVal = (struct ILibStackNode*)MALLOC(sizeof(struct ILibStackNode));
	RetVal->Data = data;
	RetVal->Next = *TheStack;
	*TheStack = RetVal;
}
void *ILibPopStack(void **TheStack)
{
	void *RetVal = NULL;
	void *Temp;
	if(*TheStack!=NULL)
	{
		RetVal = ((struct ILibStackNode*)*TheStack)->Data;
		Temp = *TheStack;
		*TheStack = ((struct ILibStackNode*)*TheStack)->Next;
		FREE(Temp);
	}
	return(RetVal);
}
void *ILibPeekStack(void **TheStack)
{
	void *RetVal = NULL;
	if(*TheStack!=NULL)
	{
		RetVal = ((struct ILibStackNode*)*TheStack)->Data;
	}
	return(RetVal);
}
void ILibClearStack(void **TheStack)
{
	void *Temp = *TheStack;
	do
	{
		ILibPopStack(&Temp);
	}while(Temp!=NULL);
	*TheStack = NULL;
}
void ILibDestroyHashTree(void *tree)
{
	struct HashNode *c = (struct HashNode*)tree;
	struct HashNode *n;
	
	while(c!=NULL)
	{
		n = c->Next;
		if(c->KeyValue!=NULL) {FREE(c->KeyValue);}
		FREE(c);
		c = n;
	}
}
void* ILibInitHashTree()
{
	struct HashNode *RetVal = (struct HashNode*)MALLOC(sizeof(struct HashNode));
	memset(RetVal,0,sizeof(struct HashNode));
	return(RetVal);
}
int ILibGetHashValue(void *key, int keylength)
{
	int HashValue=0;
	char TempValue[4];
	
	if(keylength<=4)
	{
		memset(TempValue,0,4);
		memcpy(TempValue,key,keylength);
		HashValue = *((int*)TempValue);
	}
	else
	{
		memcpy(TempValue,key,4);
		HashValue = *((int*)TempValue);
		memcpy(TempValue,(char*)key+(keylength-4),4);
		HashValue = HashValue^(*((int*)TempValue));
		if(keylength>=10)
		{
			memcpy(TempValue,(char*)key+(keylength/2),4);
			HashValue = HashValue^(*((int*)TempValue));
		}
	}
	return(HashValue);
}
struct HashNode* ILibFindEntry(void *hashtree, void *key, int keylength, int create)
{
	struct HashNode *current = (struct HashNode*)hashtree;
	int HashValue = ILibGetHashValue(key,keylength);
	int done = 0;
	
	while(done==0)
	{
		if(current->KeyHash==HashValue)
		{
			if(current->KeyLength==keylength && memcmp(current->KeyValue,key,keylength)==0)
			{
				return(current);
			}
		}
		
		if(current->Next!=NULL)
		{
			current = current->Next;
		}
		else if(create!=0)
		{
			current->Next = (struct HashNode*)MALLOC(sizeof(struct HashNode));
			memset(current->Next,0,sizeof(struct HashNode));
			current->Next->Prev = current;
			current->Next->KeyHash = HashValue;
			current->Next->KeyValue = (void*)MALLOC(keylength);
			memcpy(current->Next->KeyValue,key,keylength);
			current->Next->KeyLength = keylength;
			return(current->Next);
		}
		else
		{
			return(NULL);
		}
	}
	return(NULL);
}
void ILibAddEntry(void* hashtree, char* key, int keylength, void *value)
{
	struct HashNode* n = ILibFindEntry(hashtree,key,keylength,1);
	n->Data = value;
}

void* ILibGetEntry(void *hashtree, char* key, int keylength)
{
	struct HashNode* n = ILibFindEntry(hashtree,key,keylength,0);
	if(n==NULL)
	{
		return(NULL);
	}
	else
	{
		return(n->Data);
	}
}
void ILibDeleteEntry(void *hashtree, char* key, int keylength)
{
	struct HashNode* n = ILibFindEntry(hashtree,key,keylength,0);
	if(n!=NULL)
	{
		n->Prev->Next = n->Next;
		if(n->Next!=NULL)
		{
			n->Next->Prev = n->Prev;
		}
		FREE(n->KeyValue);
		FREE(n);
	}
}
int ILibGetLong(char *TestValue, int TestValueLength, long* NumericValue)
{
	char* StopString;
	char* TempBuffer2 = (char*)MALLOC(1+sizeof(char)*19);
	char* TempBuffer = (char*)MALLOC(1+sizeof(char)*TestValueLength);
	memcpy(TempBuffer,TestValue,TestValueLength);
	TempBuffer[TestValueLength] = '\0';
	*NumericValue = strtol(TempBuffer,&StopString,10);
	if(*StopString!='\0')
	{
		FREE(TempBuffer);
		FREE(TempBuffer2);
		return(-1);
	}
	else
	{
		FREE(TempBuffer);
		FREE(TempBuffer2);
		/* No errno or ERANGE support on PPC2002 */		//if(errno!=ERANGE)
		//{
			return(0);
			//}
		//else
		//{
			//	return(-1);
			//}
	}
}
int ILibGetULong(const char *TestValue, const int TestValueLength, unsigned long* NumericValue){
	char* StopString;
	char* TempBuffer2 = (char*)MALLOC(1+sizeof(char)*19);
	char* TempBuffer = (char*)MALLOC(1+sizeof(char)*TestValueLength);
	memcpy(TempBuffer,TestValue,TestValueLength);
	TempBuffer[TestValueLength] = '\0';
	*NumericValue = strtoul(TempBuffer,&StopString,10);
	if(*StopString!='\0')
	{
		FREE(TempBuffer);
		FREE(TempBuffer2);
		return(-1);
	}
	else
	{
		FREE(TempBuffer);
		FREE(TempBuffer2);
		/* No errno or ERANGE support on PPC2002*/		//if(errno!=ERANGE)
		//{
			if(memcmp(TestValue,"-",1)==0)
			{
				return(-1);
			}
			return(0);
			//}
		//else
		//{
			//	return(-1);
			//}
	}
}
int ILibIsDelimiter(char* buffer, int offset, int buffersize, char* Delimiter, int DelimiterLength)
{
	int i=0;
	int RetVal = 1;
	if(offset+DelimiterLength>buffersize)
	{
		return(0);
	}
	
	for(i=0;i<DelimiterLength;++i)
	{
		if(buffer[offset+i]!=Delimiter[i])
		{
			RetVal = 0;
			break;
		}
	}
	return(RetVal);
}
struct parser_result* ILibParseStringAdv(char* buffer, int offset, int length, char* Delimiter, int DelimiterLength)
{
	struct parser_result* RetVal = (struct parser_result*)MALLOC(sizeof(struct parser_result));
	int i=0;	
	char* Token = NULL;
	int TokenLength = 0;
	struct parser_result_field *p_resultfield;
	int Ignore = 0;
	char StringDelimiter=0;
	
	RetVal->FirstResult = NULL;
	RetVal->NumResults = 0;
	
	Token = buffer + offset;
	for(i=offset;i<length;++i)
	{
		if(StringDelimiter==0)
		{
			if(buffer[i]=='"') 
			{
				StringDelimiter='"';
				Ignore=1;
			}
			else
			{
				if(buffer[i]=='\'')
				{
					StringDelimiter='\'';
					Ignore=1;
				}
			}
		}
		else
		{
			if(buffer[i]==StringDelimiter)
			{
				Ignore=((Ignore==0)?1:0);
			}
		}
		if(Ignore==0 && ILibIsDelimiter(buffer,i,length,Delimiter,DelimiterLength))
		{
			p_resultfield = (struct parser_result_field*)MALLOC(sizeof(struct parser_result_field));
			p_resultfield->data = Token;
			p_resultfield->datalength = TokenLength;
			p_resultfield->NextResult = NULL;
			if(RetVal->FirstResult != NULL)
			{
				RetVal->LastResult->NextResult = p_resultfield;
				RetVal->LastResult = p_resultfield;
			}
			else
			{
				RetVal->FirstResult = p_resultfield;
				RetVal->LastResult = p_resultfield;
			}
			
			++RetVal->NumResults;
			i = i + DelimiterLength -1;
			Token = Token + TokenLength + DelimiterLength;
			TokenLength = 0;	
		}
		else
		{
			++TokenLength;
		}
	}
	p_resultfield = (struct parser_result_field*)MALLOC(sizeof(struct parser_result_field));
	p_resultfield->data = Token;
	p_resultfield->datalength = TokenLength;
	p_resultfield->NextResult = NULL;
	if(RetVal->FirstResult != NULL)
	{
		RetVal->LastResult->NextResult = p_resultfield;
		RetVal->LastResult = p_resultfield;
	}
	else
	{
		RetVal->FirstResult = p_resultfield;
		RetVal->LastResult = p_resultfield;
	}	
	++RetVal->NumResults;
	
	return(RetVal);
}
struct parser_result* ILibParseString(char* buffer, int offset, int length, char* Delimiter, int DelimiterLength)
{
	struct parser_result* RetVal = (struct parser_result*)MALLOC(sizeof(struct parser_result));
	int i=0;	
	char* Token = NULL;
	int TokenLength = 0;
	struct parser_result_field *p_resultfield;
	
	RetVal->FirstResult = NULL;
	RetVal->NumResults = 0;
	
	Token = buffer + offset;
	for(i=offset;i<length;++i)
	{
		if(ILibIsDelimiter(buffer,i,length,Delimiter,DelimiterLength))
		{
			p_resultfield = (struct parser_result_field*)MALLOC(sizeof(struct parser_result_field));
			p_resultfield->data = Token;
			p_resultfield->datalength = TokenLength;
			p_resultfield->NextResult = NULL;
			if(RetVal->FirstResult != NULL)
			{
				RetVal->LastResult->NextResult = p_resultfield;
				RetVal->LastResult = p_resultfield;
			}
			else
			{
				RetVal->FirstResult = p_resultfield;
				RetVal->LastResult = p_resultfield;
			}
			
			++RetVal->NumResults;
			i = i + DelimiterLength -1;
			Token = Token + TokenLength + DelimiterLength;
			TokenLength = 0;	
		}
		else
		{
			++TokenLength;
		}
	}
	p_resultfield = (struct parser_result_field*)MALLOC(sizeof(struct parser_result_field));
	p_resultfield->data = Token;
	p_resultfield->datalength = TokenLength;
	p_resultfield->NextResult = NULL;
	if(RetVal->FirstResult != NULL)
	{
		RetVal->LastResult->NextResult = p_resultfield;
		RetVal->LastResult = p_resultfield;
	}
	else
	{
		RetVal->FirstResult = p_resultfield;
		RetVal->LastResult = p_resultfield;
	}	
	++RetVal->NumResults;
	
	return(RetVal);
}

void ILibDestructParserResults(struct parser_result *result)
{
	struct parser_result_field *node = result->FirstResult;
	struct parser_result_field *temp;
	
	while(node!=NULL)
	{
		temp = node->NextResult;
		FREE(node);
		node = temp;
	}
	FREE(result);
}

void ILibDestructPacket(struct packetheader *packet)
{
	struct packetheader_field_node *node = packet->FirstField;
	struct packetheader_field_node *nextnode;
	
	while(node!=NULL)
	{
		nextnode = node->NextField;
		if(node->UserAllocStrings!=0)
		{
			FREE(node->Field);
			FREE(node->FieldData);
		}
		FREE(node);
		node = nextnode;
	}
	if(packet->UserAllocStrings!=0)
	{
		if(packet->StatusData!=NULL) {FREE(packet->StatusData);}
		if(packet->Directive!=NULL) {FREE(packet->Directive);}
		if(packet->DirectiveObj!=NULL) {FREE(packet->DirectiveObj);}
		if(packet->Body!=NULL) FREE(packet->Body);
	}
	FREE(packet);
}

struct packetheader* ILibParsePacketHeader(char* buffer, int offset, int length)
{
	struct packetheader *RetVal = (struct packetheader*)MALLOC(sizeof(struct packetheader));
	struct parser_result *_packet;
	struct parser_result *p;
	struct parser_result *StartLine;
	struct parser_result_field *HeaderLine;
	struct parser_result_field *f;
	char* tempbuffer;
	struct packetheader_field_node *node;
	int i=0;
	int FLNWS = -1;
	int FTNWS = -1;
	RetVal->UserAllocStrings = 0;
	RetVal->Directive = NULL;
	RetVal->DirectiveLength = 0;
	RetVal->Body = NULL;
	RetVal->BodyLength = 0;
	RetVal->FirstField = NULL;
	RetVal->LastField = NULL;
	RetVal->Source = NULL;
	p = (struct parser_result*)ILibParseString(buffer,offset,length,"\r\n",2);
	_packet = p;
	f = p->FirstResult;
	StartLine = (struct parser_result*)ILibParseString(f->data,0,f->datalength," ",1);
	HeaderLine = f->NextResult;
	if(memcmp(StartLine->FirstResult->data,
	"HTTP/",
	5)==0)
	{
		/* Response Packet */
		p = (struct parser_result*)ILibParseString(StartLine->FirstResult->data,
		0,
		StartLine->FirstResult->datalength,
		"/",1);
		RetVal->Version = p->LastResult->data;
		RetVal->VersionLength = p->LastResult->datalength;
		ILibDestructParserResults(p);
		tempbuffer = (char*)MALLOC(1+sizeof(char)*(StartLine->FirstResult->NextResult->datalength));
		memcpy(tempbuffer,StartLine->FirstResult->NextResult->data,
		StartLine->FirstResult->NextResult->datalength);
		tempbuffer[StartLine->FirstResult->NextResult->datalength] = '\0';
		RetVal->StatusCode = (int)atoi(tempbuffer);
		FREE(tempbuffer);
		RetVal->StatusData = StartLine->FirstResult->NextResult->NextResult->data;
		RetVal->StatusDataLength = StartLine->FirstResult->NextResult->NextResult->datalength;
	}
	else
	{
		/* Request Packet */
		RetVal->Directive = StartLine->FirstResult->data;
		RetVal->DirectiveLength = StartLine->FirstResult->datalength;
		RetVal->DirectiveObj = StartLine->FirstResult->NextResult->data;
		RetVal->DirectiveObjLength = StartLine->FirstResult->NextResult->datalength;
		RetVal->StatusCode = -1;
		p = (struct parser_result*)ILibParseString(StartLine->LastResult->data,
		0,
		StartLine->LastResult->datalength,
		"/",1);
		RetVal->Version = p->LastResult->data;
		RetVal->VersionLength = p->LastResult->datalength;
		ILibDestructParserResults(p);
		
		RetVal->Directive[RetVal->DirectiveLength] = '\0';
		RetVal->DirectiveObj[RetVal->DirectiveObjLength] = '\0';
	}
	while(HeaderLine!=NULL)
	{
		if(HeaderLine->datalength==0)
		{
			break;
		}
		node = (struct packetheader_field_node*)MALLOC(sizeof(struct packetheader_field_node));
		memset(node,0,sizeof(struct packetheader_field_node));
		for(i=0;i<HeaderLine->datalength;++i)
		{
			if(*((HeaderLine->data)+i)==':')
			{
				node->Field = HeaderLine->data;
				node->FieldLength = i;
				node->FieldData = HeaderLine->data + i + 1;
				node->FieldDataLength = (HeaderLine->datalength)-i-1;
				break;
			}
		}
		if(node->Field==NULL)
		{
			FREE(RetVal);
			RetVal = NULL;
			break;
		}
		FLNWS = 0;
		FTNWS = node->FieldDataLength-1;
		for(i=0;i<node->FieldDataLength;++i)
		{
			if(*((node->FieldData)+i)!=' ')
			{
				FLNWS = i;
				break;
			}
		}
		for(i=(node->FieldDataLength)-1;i>=0;--i)
		{
			if(*((node->FieldData)+i)!=' ')
			{
				FTNWS = i;
				break;
			}
		}
		node->FieldData = (node->FieldData) + FLNWS;
		node->FieldDataLength = (FTNWS - FLNWS)+1;
		
		node->Field[node->FieldLength] = '\0';
		node->FieldData[node->FieldDataLength] = '\0';
		
		node->UserAllocStrings = 0;
		node->NextField = NULL;
		
		if(RetVal->FirstField==NULL)
		{
			RetVal->FirstField = node;
			RetVal->LastField = node;
		}
		else
		{
			RetVal->LastField->NextField = node;	
		}
		RetVal->LastField = node;
		HeaderLine = HeaderLine->NextResult;
	}
	ILibDestructParserResults(_packet);
	ILibDestructParserResults(StartLine);
	return(RetVal);
}
int ILibGetRawPacket(struct packetheader* packet,char **RetVal)
{
	int i;
	int BufferSize = 0;
	char* Buffer;
	struct packetheader_field_node *node;
	
	if(packet->StatusCode!=-1)
	{
		BufferSize = 12 + packet->VersionLength + packet->StatusDataLength;
		/* HTTP/1.1 200 OK\r\n */
	}
	else
	{
		BufferSize = packet->DirectiveLength + packet->DirectiveObjLength + 12;
		/* GET / HTTP/1.1\r\n */
	}
	
	node = packet->FirstField;
	while(node!=NULL)
	{
		BufferSize += node->FieldLength + node->FieldDataLength + 4;
		node = node->NextField;
	}
	BufferSize += (3+packet->BodyLength);
	
	*RetVal = (char*)MALLOC(BufferSize);
	Buffer = *RetVal;
	if(packet->StatusCode!=-1)
	{
		memcpy(Buffer,"HTTP/",5);
		memcpy(Buffer+5,packet->Version,packet->VersionLength);
		i = 5+packet->VersionLength;
		
		i+=sprintf(Buffer+i," %d ",packet->StatusCode);
		memcpy(Buffer+i,packet->StatusData,packet->StatusDataLength);
		i+=packet->StatusDataLength;
		
		memcpy(Buffer+i,"\r\n",2);
		i+=2;
		/* HTTP/1.1 200 OK\r\n */
	}
	else
	{
		memcpy(Buffer,packet->Directive,packet->DirectiveLength);
		i = packet->DirectiveLength;
		memcpy(Buffer+i," ",1);
		i+=1;
		memcpy(Buffer+i,packet->DirectiveObj,packet->DirectiveObjLength);
		i+=packet->DirectiveObjLength;
		memcpy(Buffer+i," HTTP/",6);
		i+=6;
		memcpy(Buffer+i,packet->Version,packet->VersionLength);
		i+=packet->VersionLength;
		memcpy(Buffer+i,"\r\n",2);
		i+=2;
		/* GET / HTTP/1.1\r\n */
	}
	
	node = packet->FirstField;
	while(node!=NULL)
	{
		memcpy(Buffer+i,node->Field,node->FieldLength);
		i+=node->FieldLength;
		memcpy(Buffer+i,": ",2);
		i+=2;
		memcpy(Buffer+i,node->FieldData,node->FieldDataLength);
		i+=node->FieldDataLength;
		memcpy(Buffer+i,"\r\n",2);
		i+=2;
		BufferSize += node->FieldLength + node->FieldDataLength + 4;
		node = node->NextField;
	}
	memcpy(Buffer+i,"\r\n",2);
	i+=2;
	
	memcpy(Buffer+i,packet->Body,packet->BodyLength);
	i+=packet->BodyLength;
	Buffer[i] = '\0';
	
	return(i);
}

unsigned short ILibGetDGramSocket(int local, SOCKET *TheSocket)
{
	unsigned short PortNum = -1;
	struct sockaddr_in addr;	
	memset((char *)&(addr), 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = local;
	
	*TheSocket = socket(AF_INET, SOCK_DGRAM, 0);
	do
	{
		PortNum = (unsigned short)(50000 + ((unsigned short)rand() % 15000));
		addr.sin_port = htons(PortNum);
	}
	while(bind(*TheSocket, (struct sockaddr *) &(addr), sizeof(addr)) < 0);
	return(PortNum);
}

unsigned short ILibGetStreamSocket(int local, SOCKET *TheSocket)
{
	unsigned short PortNum = -1;
	struct sockaddr_in addr;
	memset((char *)&(addr), 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = local;
	
	*TheSocket = socket(AF_INET, SOCK_STREAM, 0);
	do
	{
		PortNum = (unsigned short)(50000 + ((unsigned short)rand() % 15000));
		addr.sin_port = htons(PortNum);
	}
	while(bind(*TheSocket, (struct sockaddr *) &(addr), sizeof(addr)) < 0);
	return(PortNum);
}

void ILibParseUri(char* URI, char** IP, unsigned short* Port, char** Path)
{
	struct parser_result *result,*result2,*result3;
	char *TempString,*TempString2;
	int TempStringLength,TempStringLength2;
	
	result = ILibParseString(URI, 0, (int)strlen(URI), "://", 3);
	TempString = result->LastResult->data;
	TempStringLength = result->LastResult->datalength;
	
	/* Parse Path */
	result2 = ILibParseString(TempString,0,TempStringLength,"/",1);
	TempStringLength2 = TempStringLength-result2->FirstResult->datalength;
	*Path = (char*)MALLOC(TempStringLength2+1);
	memcpy(*Path,TempString+(result2->FirstResult->datalength),TempStringLength2);
	(*Path)[TempStringLength2] = '\0';
	
	/* Parse Port Number */
	result3 = ILibParseString(result2->FirstResult->data,0,result2->FirstResult->datalength,":",1);
	if(result3->NumResults==1)
	{
		*Port = 80;
	}
	else
	{
		TempString2 = (char*)MALLOC(result3->LastResult->datalength+1);
		memcpy(TempString2,result3->LastResult->data,result3->LastResult->datalength);
		TempString2[result3->LastResult->datalength] = '\0';
		*Port = (unsigned short)atoi(TempString2);
		FREE(TempString2);
	}
	/* Parse IP Address */
	TempStringLength2 = result3->FirstResult->datalength;
	*IP = (char*)MALLOC(TempStringLength2+1);
	memcpy(*IP,result3->FirstResult->data,TempStringLength2);
	(*IP)[TempStringLength2] = '\0';
	ILibDestructParserResults(result3);
	ILibDestructParserResults(result2);
	ILibDestructParserResults(result);
}
struct packetheader *ILibCreateEmptyPacket()
{
	struct packetheader *RetVal = (struct packetheader*)MALLOC(sizeof(struct packetheader));
	
	RetVal->UserAllocStrings = -1;
	RetVal->Directive = NULL;
	RetVal->DirectiveLength = 0;
	
	RetVal->DirectiveObj = NULL;
	RetVal->DirectiveObjLength = 0;
	
	RetVal->StatusCode = -1;
	RetVal->StatusData = NULL;
	RetVal->StatusDataLength = 0;
	RetVal->Version = "1.0";
	RetVal->VersionLength = 3;
	RetVal->Body = NULL;
	RetVal->BodyLength = 0;
	
	RetVal->FirstField = NULL;
	RetVal->LastField = NULL;
	
	RetVal->Source = NULL;
	RetVal->ReceivingAddress = 0;
	
	return(RetVal);
}
void ILibSetStatusCode(struct packetheader *packet, int StatusCode, char *StatusData, int StatusDataLength)
{
	packet->StatusCode = StatusCode;
	packet->StatusData = (char*)MALLOC(StatusDataLength+1);
	memcpy(packet->StatusData,StatusData,StatusDataLength);
	packet->StatusData[StatusDataLength] = '\0';
	packet->StatusDataLength = StatusDataLength;
}
void ILibSetDirective(struct packetheader *packet, char* Directive, int DirectiveLength, char* DirectiveObj, int DirectiveObjLength)
{
	packet->Directive = (char*)MALLOC(DirectiveLength+1);
	memcpy(packet->Directive,Directive,DirectiveLength);
	packet->Directive[DirectiveLength] = '\0';
	packet->DirectiveLength = DirectiveLength;
	
	packet->DirectiveObj = (char*)MALLOC(DirectiveObjLength+1);
	memcpy(packet->DirectiveObj,DirectiveObj,DirectiveObjLength);
	packet->DirectiveObj[DirectiveObjLength] = '\0';
	packet->DirectiveObjLength = DirectiveObjLength;
	packet->UserAllocStrings = -1;
}
void ILibAddHeaderLine(struct packetheader *packet, char* FieldName, int FieldNameLength, char* FieldData, int FieldDataLength)
{
	struct packetheader_field_node *node;
	
	node = (struct packetheader_field_node*)MALLOC(sizeof(struct packetheader_field_node));
	node->UserAllocStrings = -1;
	node->Field = (char*)MALLOC(FieldNameLength+1);
	memcpy(node->Field,FieldName,FieldNameLength);
	node->Field[FieldNameLength] = '\0';
	node->FieldLength = FieldNameLength;
	
	node->FieldData = (char*)MALLOC(FieldDataLength+1);
	memcpy(node->FieldData,FieldData,FieldDataLength);
	node->FieldData[FieldDataLength] = '\0';
	node->FieldDataLength = FieldDataLength;
	
	node->NextField = NULL;
	
	if(packet->LastField!=NULL)
	{
		packet->LastField->NextField = node;
		packet->LastField = node;
	}
	else
	{
		packet->LastField = node;
		packet->FirstField = node;
	}
}
char* ILibGetHeaderLine(struct packetheader *packet, char* FieldName, int FieldNameLength)
{
	char* RetVal = NULL;
	struct packetheader_field_node *node = packet->FirstField;
	int i;
	
	while(node!=NULL)
	{
		if(strncasecmp(FieldName,node->Field,FieldNameLength)==0)
		{
			RetVal = (char*)MALLOC(node->FieldDataLength+1);
			
			for(i=0;i<node->FieldDataLength;++i)
			{
				if(node->FieldData[i]!=' ') {break;}
			}
			if(i==node->FieldDataLength-1) {i = 0;}
			memcpy(RetVal,node->FieldData+i,node->FieldDataLength-i);
			RetVal[node->FieldDataLength-i] = '\0';
			break;
		}
		node = node->NextField;
	}
	
	return(RetVal);
}

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/* encode 3 8-bit binary bytes as 4 '6-bit' characters */
void ILibencodeblock( unsigned char in[3], unsigned char out[4], int len )
{
	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/* Base64 encode a stream adding padding and line breaks as per spec. */
int ILibBase64Encode(unsigned char* input, const int inputlen, unsigned char** output)
{
	unsigned char* out;
	unsigned char* in;
	
	*output = (unsigned char*)MALLOC(((inputlen * 4) / 3) + 5);
	out = *output;
	in  = input;
	
	if (input == NULL || inputlen == 0)
	{
		*output = NULL;
		return 0;
	}
	
	while ((in+3) <= (input+inputlen))
	{
		ILibencodeblock(in, out, 3);
		in += 3;
		out += 4;
	}
	if ((input+inputlen)-in == 1)
	{
		ILibencodeblock(in, out, 1);
		out += 4;
	}
	else
	if ((input+inputlen)-in == 2)
	{
		ILibencodeblock(in, out, 2);
		out += 4;
	}
	*out = 0;
	
	return (int)(out-*output);
}

/* Decode 4 '6-bit' characters into 3 8-bit binary bytes */
void ILibdecodeblock( unsigned char in[4], unsigned char out[3] )
{
	out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
	out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
	out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

/* decode a base64 encoded stream discarding padding, line breaks and noise */
int ILibBase64Decode(unsigned char* input, const int inputlen, unsigned char** output)
{
	unsigned char* inptr;
	unsigned char* out;
	unsigned char v;
	unsigned char in[4];
	int i, len;
	
	if (input == NULL || inputlen == 0)
	{
		*output = NULL;
		return 0;
	}
	
	*output = (unsigned char*)MALLOC(((inputlen * 3) / 4) + 4);
	out = *output;
	inptr = input;
	
	while( inptr <= (input+inputlen) )
	{
		for( len = 0, i = 0; i < 4 && inptr <= (input+inputlen); i++ )
		{
			v = 0;
			while( inptr <= (input+inputlen) && v == 0 ) {
				v = (unsigned char) *inptr;
				inptr++;
				v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
				if( v ) {
					v = (unsigned char) ((v == '$') ? 0 : v - 61);
				}
			}
			if( inptr <= (input+inputlen) ) {
				len++;
				if( v ) {
					in[ i ] = (unsigned char) (v - 1);
				}
			}
			else {
				in[i] = 0;
			}
		}
		if( len )
		{
			ILibdecodeblock( in, out );
			out += len-1;
		}
	}
	*out = 0;
	return (int)(out-*output);
}

int ILibInPlaceXmlUnEscape(char* data)
{
	char* end = data+strlen(data);
	char* i = data;              /* src */
	char* j = data;              /* dest */
	while (j < end)
	{
		if (j[0] == '&' && j[1] == 'q' && j[2] == 'u' && j[3] == 'o' && j[4] == 't' && j[5] == ';')   // &quot;
		{
			i[0] = '"';
			j += 5;
		}
		else if (j[0] == '&' && j[1] == 'a' && j[2] == 'p' && j[3] == 'o' && j[4] == 's' && j[5] == ';')   // &apos;
		{
			i[0] = '\'';
			j += 5;
		}
		else if (j[0] == '&' && j[1] == 'a' && j[2] == 'm' && j[3] == 'p' && j[4] == ';')   // &amp;
		{
			i[0] = '&';
			j += 4;
		}
		else if (j[0] == '&' && j[1] == 'l' && j[2] == 't' && j[3] == ';')   // &lt;
		{
			i[0] = '<';
			j += 3;
		}
		else if (j[0] == '&' && j[1] == 'g' && j[2] == 't' && j[3] == ';')   // &gt;
		{
			i[0] = '>';
			j += 3;
		}
		else
		{
			i[0] = j[0];
		}
		i++;
		j++;
	}
	i[0] = '\0';
	return (int)(i - data);
}

int ILibXmlEscapeLength(const char* data)
{
	int i = 0, j = 0;
	while (data[i] != 0)
	{
		switch (data[i])
		{
			case '"':
			j += 6;
			break;
			case '\'':
			j += 6;
			break;
			case '<':
			j += 4;
			break;
			case '>':
			j += 4;
			break;
			case '&':
			j += 5;
			break;
			default:
			j++;
		}
		i++;
	}
	return j;
}

int ILibXmlEscape(char* outdata, const char* indata)
{
	int i=0;
	int inlen;
	char* out;
	
	out = outdata;
	inlen = (int)strlen(indata);
	
	for (i=0; i < inlen; i++)
	{
		if (indata[i] == '"')
		{
			memcpy(out, "&quot;", 6);
			out = out + 6;
		}
		else
		if (indata[i] == '\'')
		{
			memcpy(out, "&apos;", 6);
			out = out + 6;
		}
		else
		if (indata[i] == '<')
		{
			memcpy(out, "&lt;", 4);
			out = out + 4;
		}
		else
		if (indata[i] == '>')
		{
			memcpy(out, "&gt;", 4);
			out = out + 4;
		}
		else
		if (indata[i] == '&')
		{
			memcpy(out, "&amp;", 5);
			out = out + 5;
		}
		else
		{
			out[0] = indata[i];
			out++;
		}
	}
	
	out[0] = 0;
	
	return (int)(out - outdata);
}

void ILibLifeTime_Add(void *LifetimeMonitorObject,void *data, int seconds, void* Callback, void* Destroy)
{
	int NeedUnBlock = 0;
	struct timeval tv;
	struct LifeTimeMonitorData *temp,*temp2;
	struct LifeTimeMonitorData *ltms = (struct LifeTimeMonitorData*)MALLOC(sizeof(struct LifeTimeMonitorData));
	struct ILibLifeTime *UPnPLifeTime = (struct ILibLifeTime*)LifetimeMonitorObject;
	
	gettimeofday(&tv,NULL);
	
	ltms->data = data;
	ltms->ExpirationTick = tv.tv_sec + seconds;
	ltms->CallbackPtr = Callback;
	ltms->DestroyPtr = Destroy;
	ltms->Next = NULL;
	
	sem_wait(&(UPnPLifeTime->SyncLock));
	if(UPnPLifeTime->LM==NULL)
	{
		UPnPLifeTime->LM = ltms;
		NeedUnBlock = 1;
	}
	else
	{
		temp = UPnPLifeTime->LM;
		temp2 = temp;
		while(temp!=NULL)
		{
			if(ltms->ExpirationTick<=temp->ExpirationTick){break;}
			temp2 = temp;
			temp = temp->Next;
		}
		ltms->Next = temp;
		if(temp!=temp2) 
		{
			temp2->Next = ltms;
		}
		else
		{
			UPnPLifeTime->LM = ltms;
			NeedUnBlock = 1;
		}
	}
	if(NeedUnBlock!=0) {ILibForceUnBlockChain(UPnPLifeTime->Chain);}
	sem_post(&(UPnPLifeTime->SyncLock));
}

void ILibLifeTime_Check(void *LifeTimeMonitorObject,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	struct timeval tv;
	long CurrentTick;
	struct LifeTimeMonitorData *Temp,*EVT,*Last=NULL;
	struct ILibLifeTime *UPnPLifeTime = (struct ILibLifeTime*)LifeTimeMonitorObject;
	int nexttick;
	
	EVT = NULL;
	sem_wait(&(UPnPLifeTime->SyncLock));
	if(UPnPLifeTime->LM!=NULL)
	{
		gettimeofday(&tv,NULL);
		CurrentTick = tv.tv_sec;
		Temp = UPnPLifeTime->LM;
		while(Temp!=NULL && Temp->ExpirationTick<=CurrentTick)
		{
			EVT = UPnPLifeTime->LM;
			Last = Temp;
			Temp = Temp->Next;
		}
		if(EVT != NULL)
		{
			if(Temp!=NULL)
			{
				UPnPLifeTime->LM = Temp;
				Last->Next = NULL;
			}
			else
			{
				UPnPLifeTime->LM = NULL;
			}
		}
		sem_post(&(UPnPLifeTime->SyncLock));
		
		while(EVT!=NULL)
		{
			EVT->CallbackPtr(EVT->data);
			Temp = EVT;
			EVT = EVT->Next;
			FREE(Temp);
		}
		
		if(UPnPLifeTime->LM!=NULL)
		{
			nexttick = UPnPLifeTime->LM->ExpirationTick-CurrentTick;
			if(nexttick<*blocktime) {*blocktime=nexttick;}
		}
	}
	else
	{
		sem_post(&(UPnPLifeTime->SyncLock));
	}
}

void ILibLifeTime_Remove(void *LifeTimeToken, void *data)
{
	struct ILibLifeTime *UPnPLifeTime = (struct ILibLifeTime*)LifeTimeToken;
	struct LifeTimeMonitorData *first,*second;
	
	sem_wait(&(UPnPLifeTime->SyncLock));
	
	first = UPnPLifeTime->LM;
	if(first==NULL) 
	{
		sem_post(&(UPnPLifeTime->SyncLock));
		return;
	}
	second = first->Next;
	if(first->data==data)
	{
		UPnPLifeTime->LM = first->Next;
		if(first->DestroyPtr!=NULL) {first->DestroyPtr(first->data);}
		FREE(first);
	}
	else
	{
		while(second!=NULL)
		{
			if(second->data==data)
			{
				first->Next = second->Next;
				if(second->DestroyPtr!=NULL) {second->DestroyPtr(second->data);}
				FREE(second);
				second = first->Next;
			}
			else
			{
				first = first->Next;
				second = second->Next;
			}
		}
	}
	sem_post(&(UPnPLifeTime->SyncLock));
}
void ILibLifeTime_Flush(void *LifeTimeToken)
{
	struct ILibLifeTime *UPnPLifeTime = (struct ILibLifeTime*)LifeTimeToken;
	struct LifeTimeMonitorData *temp,*temp2;
	
	sem_wait(&(UPnPLifeTime->SyncLock));
	
	temp = UPnPLifeTime->LM;
	while(temp!=NULL)
	{
		temp2 = temp->Next;
		if(temp->DestroyPtr!=NULL) {temp->DestroyPtr(temp->data);}
		FREE(temp);
		temp = temp2;
	}
	UPnPLifeTime->LM = NULL;
	sem_post(&(UPnPLifeTime->SyncLock));
}
void ILibLifeTime_Destroy(void *LifeTimeToken)
{
	struct ILibLifeTime *UPnPLifeTime = (struct ILibLifeTime*)LifeTimeToken;
	ILibLifeTime_Flush(LifeTimeToken);
	sem_destroy(&(UPnPLifeTime->SyncLock));
}
void *ILibCreateLifeTime(void *Chain)
{
	struct ILibLifeTime *RetVal = (struct ILibLifeTime*)MALLOC(sizeof(struct ILibLifeTime));
	RetVal->LM = NULL;
	RetVal->PreSelect = &ILibLifeTime_Check;
	RetVal->PostSelect = NULL;
	RetVal->Destroy = &ILibLifeTime_Destroy;
	RetVal->Chain = Chain;
	sem_init(&(RetVal->SyncLock),0,1);
	ILibAddToChain(Chain,RetVal);
	return((void*)RetVal);
}
int ILibFindEntryInTable(char *Entry, char **Table)
{
	int i = 0;
	
	while(Table[i]!=NULL)
	{
		if(strcmp(Entry,Table[i])==0)
		{
			return(i);
		}
		++i;
	}
	
	return(-1);
}
