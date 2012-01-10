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

#ifndef __linux__
#include <windows.h>
#endif
#ifdef __linux__
#include <stdarg.h>
#endif
#include <stdio.h>
#include <time.h>
#include "tutrace.h"

#ifdef _TUDEBUGTRACE

extern "C" bool g_verbose;

void PrintTraceMsg(int level, char *lpszFile, 
                   int nLine, char *lpszFormat, ...)
{
    char     szTraceMsg[2000];
    char *   lpszPfx;
    int      cbMsg;
    va_list  lpArgv;
	/*
    char     datetimebuffer[256];
    time_t   ltime;
	*/
#ifdef _WIN32_WCE
    TCHAR    szMsgW[2000];
#endif

    if ( ! (TUTRACELEVEL & level))
    {
        return;
    }

    /*
    lpszPfx = "[%08X] %s %s(%d) : ";

    time( &ltime );
    sprintf(datetimebuffer, "%s", ctime(&ltime));
    datetimebuffer[strlen(datetimebuffer) - 1] = 0;

    // Format trace msg prefix
    cbMsg = sprintf(szTraceMsg, 
                    lpszPfx, 
                    GetCurrentThreadId(),
                    datetimebuffer,
                    lpszFile, 
                    nLine);

	// on Windows, this approach can be used.
   va_list args;
   int len;
   char * buffer;

   va_start( args, format );
   len = _vscprintf( format, args ) // _vscprintf doesn't count
                               + 1; // terminating '\0'
   buffer = malloc( len * sizeof(char) );
   vsprintf_s( buffer, len, format, args );
   puts( buffer );
   free( buffer );

    */
   
    lpszPfx = "%s(%d):";

    // Format trace msg prefix
    cbMsg = sprintf_s(szTraceMsg, 
                    lpszPfx, 
                    lpszFile, 
                    nLine);
    // Append trace msg to prefix.
    va_start(lpArgv, lpszFormat);
    cbMsg = vsprintf(szTraceMsg + cbMsg, lpszFormat, lpArgv);
    va_end(lpArgv);  

	szTraceMsg[sizeof(szTraceMsg) - 1] = 0;

#ifndef _WIN32_WCE
#ifdef WIN32
    OutputDebugString(szTraceMsg);
	if (g_verbose) {
		printf("%s", szTraceMsg);
	}
#else // Linux
    fprintf(stderr, szTraceMsg);
#endif // WIN32
#else // _WIN32_WCE
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTraceMsg, -1, szMsgW, strlen(szTraceMsg)+1);
    RETAILMSG(1, (szMsgW));
#endif // _WIN32_WCE
}

#endif //_TUDEBUGTRACE

