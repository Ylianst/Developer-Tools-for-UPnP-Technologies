#ifndef MY_STRING_H

/*
 *	EndsWith()
 *		str					: the string to analyze
 *		endsWith			: the token to find at the end of str
 *
 *	If "str" ends with "endsWith", then we return nonzero.
 */
int EndsWith(char* str, char* endsWith, int ignoreCase);

/*
 *	IndexOf()
 *		str					: the string to analyze
 *		findThis			: the token to find 
 *
 *	Returns the first index where findThis can be found in str.
 *	Returns -1 if not found.
 */
int IndexOf(char* str, char* findThis);

int LastIndexOf(char* str, char* findThis);

int StartsWith(char* str, char* startsWith, int ignoreCase);

int Utf8ToAnsi(char *dest, const char *src, int destLen);
int Utf8ToWide(unsigned short *dest, const char *src, int destLen);
int strToUtf8(char *dest, const char *src, int destSize, int isWide);

#endif 
