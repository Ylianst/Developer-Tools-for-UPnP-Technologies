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

#include <string.h>
#include "MyString.h"

#ifdef UNDER_CE
#define strnicmp _strnicmp
#endif

#ifdef _POSIX
#define strnicmp strncasecmp
#endif

int EndsWith(char* str, char* endsWith, int ignoreCase)
{
	int strLen, ewLen, offset;
	int cmp = 0;

	strLen = (int)strlen(str);
	ewLen = (int)strlen(endsWith);

	if (ewLen > strLen)
	{
		return 0;
	}

	offset = strLen - ewLen;

	if (ignoreCase != 0)
	{
		cmp = strnicmp(str+offset, endsWith, ewLen);
	}
	else
	{
		cmp = strncmp(str+offset, endsWith, ewLen);
	}

	if (cmp == 0)
	{
		return 1;
	}

	return 0;
}

int IndexOf(char* str, char* findThis)
{
	int i,j, strLen, ftLen;
	
	strLen = (int)strlen(str);
	ftLen = (int)strlen(findThis);

	if (ftLen <= strLen)
	{
		for (i=0; i < strLen; i++)
		{
			for (j=0; j < ftLen; j++)
			{
				int ij = i+j;
				if (str[ij] != findThis[j])
				{
					break;
				}
			}
			if (j == ftLen)
			{
				return i;
			}
		}
	}

	return -1;
}

int LastIndexOf(char* str, char* findThis)
{
	int i,j,strLen,ftLen;

	strLen = (int)strlen(str);
	ftLen = (int)strlen(findThis);

	if (ftLen <= strLen)
	{
		for (i=strLen-ftLen; i >= 0; i--)
		{
			for (j=0; j < ftLen; j++)
			{
				int ij = i+j;
				if (str[ij] != findThis[j])
				{
					break;
				}
			}
			if (j == ftLen)
			{
				return i;
			}
		}
	}

	return -1;
}

int StartsWith(char* str, char* startsWith, int ignoreCase)
{
	int cmp;

	if (ignoreCase != 0)
	{
		cmp = strnicmp(str, startsWith, strlen(startsWith));
	}
	else
	{
		cmp = strncmp(str, startsWith, strlen(startsWith));
	}

	if (cmp == 0)
	{
		return 1;
	}

	return 0;

}

int Utf8ToAnsi(char *dest, const char *src, int destLen)
{
	int di,si,si2,si3;
	int conv;

	conv = 0;
	di = 0;
	si = 0;
	destLen--;

	while ((src[si] != '\0') && (di < destLen))
	{
		si2 = si+1;
		si3 = si2+1;

		if ((unsigned char)src[si] <= 0x7F)
		{
			dest[di++] = src[si++];
			conv++;
		}
		else if ((unsigned char)src[si] <= 0xDF)
		{
			if (((unsigned char)src[si] & 0x1C) == 0)
			{
				dest[di++] = ((src[si] & 0x03) << 6) | (src[si2] & 0x3F);
				conv++;
			}
			si+=2;
		}
		else if ((unsigned char)src[si] <= 0xEF)
		{
			/* outside ansi char set */
			si+=3;
		}
		else if ((unsigned char)src[si] <= 0xF7)
		{
			/* outside ansi char set */
			si+=4;
		}
	}
	dest[di] = '\0';

	return conv;
}

int Utf8ToWide(unsigned short *dest, const char *src, int destLen)
{
	int di,si,si2,si3;
	int conv;

	conv = 0;
	di = 0;
	si = 0;
	destLen--;

	while ((src[si] != '\0') && (di < destLen))
	{
		si2 = si+1;
		si3 = si2+1;

		if ((unsigned char)src[si] <= 0x7F)
		{
			dest[di++] = src[si++];
			conv++;
		}
		else if ((unsigned char)src[si] <= 0xDF)
		{
			dest[di++] = ((src[si] & 0x1F) << 6) | (src[si2] & 0x3F);
			conv++;
			si+=2;
		}
		else if ((unsigned char)src[si] <= 0xEF)
		{
			dest[di++] = ((src[si] & 0x0F) << 12) | ((src[si2] & 0x3F) << 6) | (src[si3] & 0x3F);
			conv++;
			si+=3;
		}
		else if ((unsigned char)src[si] <= 0xF7)
		{
			/* code should never execute because it's for unicode characters greater than 16 bits */
			/*
			dest[di++] = ((src[si] & 0x07) << 18) | ((src[si2] & 0x3F) << 6) | ((src[si2] & 0x3F) << 6) | (src[si3] & 0x3F);
			conv++;
			 */
			si+=4;
		}
	}
	dest[di] = '\0';

	return conv;
}

int strToUtf8(char *dest, const char *src, int destSize, int isWide)
{
	unsigned short wcv;		/* wide char scalar */
	int i,j;
	int cc;			/* char count */
	int destLen;

	destLen = destSize - 1;
	cc = 0;
	i = 0;
	wcv = 0;
	j=0;

	
	do
	{
		wcv =  (unsigned char) src[i++];

		if (isWide != 0)
		{
			wcv |= (((unsigned char) src[i++]) << 8);
		}

		if ((wcv < 0x80) && (j < destLen))
		{
			dest[j++] = (char) wcv;
			cc++;
		}
		else if ((wcv < 0x0800) && (j < destLen - 1))
		{
			dest[j++] = (char)((wcv >> 6) | 0xC0);
			dest[j++] = (char)((wcv & 0x3F) | 0x80);
			cc++;
		}
		else if ((wcv < 0x10000) && (j < destLen - 2))
		{
			dest[j++] = (char)((wcv >> 12) | 0xF0);
			dest[j++] = (char)(((wcv >> 6) & 0x3F) | 0x80);
			dest[j++] = (char)((wcv & 0x3F) | 0x80);
			cc++;
		}
		else if (j < destLen - 3)
		{
			/*
			 *	this code should never execute with unicode chars with
			 *	more than 2 bytes because the unicode scalar won't fit in 16 bits.
			 *
			dest[j++] = (char)(wcv >> 18 | 0xF0);
			dest[j++] = (char)(wcv >> 12 & 0x3F | 0x80);
			dest[j++] = (char)(wcv >> 6 & 0x3F | 0x80);
			dest[j++] = (char)(wcv & 0x3F | 0x80);
			cc++;
			 */
		}
		else
		{
			/* no more room in dest buffer */
			break;
		}
	}
	while (wcv != 0);


	dest[j] = '\0';
	return cc;
}
