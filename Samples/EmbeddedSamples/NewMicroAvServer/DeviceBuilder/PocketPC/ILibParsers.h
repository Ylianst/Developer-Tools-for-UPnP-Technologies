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

#ifndef __ILibParsers__
#define __ILibParsers__

#include <winsock.h>

#define UPnPMIN(a,b) (((a)<(b))?(a):(b))
#define MALLOC malloc
#define FREE free

struct parser_result_field
{
	char* data;
	int datalength;
	struct parser_result_field *NextResult;
};
struct parser_result
{
	struct parser_result_field *FirstResult;
	struct parser_result_field *LastResult;
	int NumResults;
};
struct packetheader_field_node
{
	char* Field;
	int FieldLength;
	char* FieldData;
	int FieldDataLength;
	int UserAllocStrings;
	struct packetheader_field_node* NextField;
};
struct packetheader
{
	char* Directive;
	int DirectiveLength;
	char* DirectiveObj;
	int DirectiveObjLength;
	int StatusCode;
	char* StatusData;
	int StatusDataLength;
	char* Version;
	int VersionLength;
	char* Body;
	int BodyLength;
	int UserAllocStrings;
	
	struct packetheader_field_node* FirstField;
	struct packetheader_field_node* LastField;
	struct sockaddr_in *Source;
	int ReceivingAddress;
};
struct ILibXMLNode
{
	char* Name;
	int NameLength;
	
	char* NSTag;
	int NSLength;
	int StartTag;
	int EmptyTag;
	
	void *Reserved;
	struct ILibXMLNode *Next;
	struct ILibXMLNode *Parent;
	struct ILibXMLNode *Peer;
	struct ILibXMLNode *ClosingTag;
};
struct ILibXMLAttribute
{
	char* Name;
	int NameLength;
	
	char* Prefix;
	int PrefixLength;
	
	char* Value;
	int ValueLength;
	struct ILibXMLAttribute *Next;
};

int ILibFindEntryInTable(char *Entry, char **Table);

/* Stack Methods */
void ILibCreateStack(void **TheStack);
void ILibPushStack(void **TheStack, void *data);
void *ILibPopStack(void **TheStack);
void *ILibPeekStack(void **TheStack);
void ILibClearStack(void **TheStack);

/* XML Parsing Methods */
int ILibReadInnerXML(struct ILibXMLNode *node, char **RetVal);
struct ILibXMLNode *ILibParseXML(char *buffer, int offset, int length);
struct ILibXMLAttribute *ILibGetXMLAttributes(struct ILibXMLNode *node);
int ILibProcessXMLNodeList(struct ILibXMLNode *nodeList);
void ILibDestructXMLNodeList(struct ILibXMLNode *node);
void ILibDestructXMLAttributeList(struct ILibXMLAttribute *attribute);

/* Chaining Methods */
void *ILibCreateChain();
void ILibAddToChain(void *chain, void *object);
void ILibStartChain(void *chain);
void ILibStopChain(void *chain);
void ILibForceUnBlockChain(void *Chain);

/* HashTree Methods */
void* ILibInitHashTree();
void ILibDestroyHashTree(void *tree);
void ILibAddEntry(void* hashtree, char* key, int keylength, void *value);
void* ILibGetEntry(void *hashtree, char* key, int keylength);
void ILibDeleteEntry(void *hashtree, char* key, int keylength);

/* LifeTimeMonitor Methods */
void ILibLifeTime_Add(void *LifetimeMonitorObject,void *data, int seconds, void* Callback, void* Destroy);
void ILibLifeTime_Remove(void *LifeTimeToken, void *data);
void ILibLifeTime_Flush();
void *ILibCreateLifeTime(void *Chain);

/* String Parsing Methods */
struct parser_result* ILibParseString(char* buffer, int offset, int length, char* Delimiter, int DelimiterLength);
struct parser_result* ILibParseStringAdv(char* buffer, int offset, int length, char* Delimiter, int DelimiterLength);
void ILibDestructParserResults(struct parser_result *result);
void ILibParseUri(char* URI, char** IP, unsigned short* Port, char** Path);
int ILibGetLong(char *TestValue, int TestValueLength, long* NumericValue);
int ILibGetULong(const char *TestValue, const int TestValueLength, unsigned long* NumericValue);

/* Packet Methods */
struct packetheader *ILibCreateEmptyPacket();
void ILibAddHeaderLine(struct packetheader *packet, char* FieldName, int FieldNameLength, char* FieldData, int FieldDataLength);
char* ILibGetHeaderLine(struct packetheader *packet, char* FieldName, int FieldNameLength);
void ILibSetStatusCode(struct packetheader *packet, int StatusCode, char* StatusData, int StatusDataLength);
void ILibSetDirective(struct packetheader *packet, char* Directive, int DirectiveLength, char* DirectiveObj, int DirectiveObjLength);
void ILibDestructPacket(struct packetheader *packet);
struct packetheader* ILibParsePacketHeader(char* buffer, int offset, int length);
int ILibGetRawPacket(struct packetheader *packet,char **buffer);

/* Network Helper Methods */
int ILibGetLocalIPAddressList(int** pp_int);
unsigned short ILibGetDGramSocket(int local, SOCKET *TheSocket);
unsigned short ILibGetStreamSocket(int local, SOCKET *TheSocket);

void* dbg_malloc(int sz);
void dbg_free(void* ptr);
int dbg_GetCount();

/* XML escaping methods */
int ILibXmlEscape(char* outdata, const char* indata);
int ILibXmlEscapeLength(const char* data);
int ILibInPlaceXmlUnEscape(char* data);

/* Base64 handling methods */
int ILibBase64Encode(unsigned char* input, const int inputlen, unsigned char** output);
int ILibBase64Decode(unsigned char* input, const int inputlen, unsigned char** output);

#endif
