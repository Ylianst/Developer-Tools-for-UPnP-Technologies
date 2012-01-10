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

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "PortFn_MsTime.h"

#ifdef PORTFN_WINDOWS_IMPLEMENTATION
/* for timeval in Win32 */
#include <winsock.h>
#include <time.h>
#include <process.h>
#endif

#ifdef _UNIX
#include <sys/time.h>
#endif

void GetMillisecTime(struct MillisecTime* msTime)
{
#ifdef PORTFN_WINDOWS_IMPLEMENTATION
	msTime->val = GetTickCount();
#endif

#ifdef _UNIX
	gettimeofday(&(msTime->val), NULL);
#endif
}

long DiffMsTimes(struct MillisecTime* t1, struct MillisecTime* t2)
{
#ifdef PORTFN_WINDOWS_IMPLEMENTATION
	return (t1->val - t2->val);
#endif

#ifdef _UNIX
	long t1ms, t2ms;

	/* convert t1 to milliseconds */
	t1ms = (t1->val.tv_sec * 1000) + (t1->val.tv_usec / 1000);
	t2ms = (t2->val.tv_sec * 1000) + (t2->val.tv_usec / 1000);

	return (t1ms - t2ms);
#endif
}

void SetZeroMsTime(struct MillisecTime* makeZero)
{
#ifdef PORTFN_WINDOWS_IMPLEMENTATION
	makeZero->val = 0;
#endif

#ifdef _UNIX
	makeZero->val.tv_sec = 0;
	makeZero->val.tv_usec = 0;
#endif
}

void SleepMsTime(int millisecTime)
{
#ifdef PORTFN_WINDOWS_IMPLEMENTATION
	Sleep(millisecTime);
#endif

#ifdef _UNIX
	struct timeval delay;
	int seconds;

	seconds = millisecTime / 1000;
	delay.tv_sec = seconds;
	delay.tv_usec = (millisecTime - (seconds * 1000)) * 1000;

	select(0, NULL, NULL, NULL, &delay);
#endif
}

