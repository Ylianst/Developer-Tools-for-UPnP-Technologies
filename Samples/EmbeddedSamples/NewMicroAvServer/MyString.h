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
int Utf8ToWide(wchar_t *dest, const char *src, int destLen);
int strToUtf8(char *dest, const char *src, int destSize, int isWide, int *charactersConverted);

int strUtf8Len(char *src, int isWide, int asEscapedUri);

/*
 *	Stores UTF8-compliant escaped URI in 'dest'.
 *	Returns number of bytes used in dest.
 */
int strToEscapedUri(char *dest, const char *src, int destSize, int isWide, int *charactersConverted);

#endif 
