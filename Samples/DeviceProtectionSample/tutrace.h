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

#ifndef _TUTRACE_H
#define _TUTRACE_H

#ifdef __cplusplus
extern "C" {
#endif


// include this preprocessor directive in your project/make file
#ifdef _TUDEBUGTRACE

// Set Debug Trace level here
#define TUTRACELEVEL    (TUINFO | TUERR | TUVERBOSE)
//#define TUTRACELEVEL    (TUERR)
//#define TUTRACELEVEL    (0)

// trace levels
#define TUINFO  0x0001  
#define TUVERBOSE 0x0002
#define TUERR   0x0010

#define TUTRACE_ERR        TUERR, __FILE__, __LINE__
#define TUTRACE_INFO       TUINFO, __FILE__, __LINE__
#define TUTRACE_VERBOSE    TUVERBOSE, __FILE__, __LINE__

#define TUTRACE(VARGLST)   PrintTraceMsg VARGLST

void PrintTraceMsg(int level, char *lpszFile,
                   int nLine, char *lpszFormat, ...);

#else //_TUDEBUGTRACE

#define TUTRACE(VARGLST)    ((void)0)

#endif //_TUDEBUGTRACE

#ifdef __cplusplus
}
#endif


#endif // _TUTRACE_H
