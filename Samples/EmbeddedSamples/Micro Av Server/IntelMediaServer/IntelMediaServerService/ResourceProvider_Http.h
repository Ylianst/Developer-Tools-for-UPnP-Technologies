#ifndef _RESOURCE_PROVIDER_HTTP_MACROS_H
#define _RESOURCE_PROVIDER_HTTP_MACROS_H

#define HTTP_MAX_URI_SIZE 1024
#define HTTP_URI_BASE_LEN 128

#define GetContentUri(title,creator,creatorLen,key,ext,virDirName,addr,port,uri,cp)	\
	if (creatorLen > 0)\
	{\
		if (ext[0] == '.')\
		{	\
			cp += sprintf(\
				uri,\
				"http://%d.%d.%d.%d:%d/%s/%ld/%s%%20-%%20%s%s",\
				(addr&0xFF), ((addr>>8)&0xFF), ((addr>>16)&0xFF), ((addr>>24)&0xFF), \
				port, virDirName, key, creator, title, ext\
				);\
		}\
		else\
		{\
			cp += sprintf(\
				uri,\
				"http://%d.%d.%d.%d:%d/%s/%ld/%s%%20-%%20%s.%s",\
				(addr&0xFF), ((addr>>8)&0xFF), ((addr>>16)&0xFF), ((addr>>24)&0xFF), \
				port, virDirName, key, creator, title, ext\
				);\
		}\
	}\
	else\
	{\
		if (ext[0] == '.')\
		{	\
			cp += sprintf(\
				uri, \
				"http://%d.%d.%d.%d:%d/%s/%ld/%s%s", \
				(addr&0xFF), ((addr>>8)&0xFF), ((addr>>16)&0xFF), ((addr>>24)&0xFF), \
				port, virDirName, key, title, ext\
				);\
		}\
		else\
		{\
			cp += sprintf(\
				uri, \
				"http://%d.%d.%d.%d:%d/%s/%ld/%s.%s", \
				(addr&0xFF), ((addr>>8)&0xFF), ((addr>>16)&0xFF), ((addr>>24)&0xFF), \
				port, virDirName, key, title, ext\
				);\
		}\
	}

#define UnescapeDbEntryFields(dbEntry,ext,title2,creator2,ext2)\
	memcpy(title2, dbEntry->Title, dbEntry->Title_length);\
	title2[dbEntry->Title_length] = '\0';\
	ILibInPlaceXmlUnEscape(title2);\
	memcpy(creator2, dbEntry->Creator, dbEntry->Creator_length);\
	creator2[dbEntry->Creator_length] = '\0';\
	ILibInPlaceXmlUnEscape(creator2);\
	strcpy(ext2, ext);\
	ILibInPlaceXmlUnEscape(ext2);\

#endif