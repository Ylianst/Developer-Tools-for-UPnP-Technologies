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


#ifndef PORTFN_MS_TIME_H
#define PORTFN_MS_TIME_H

#ifdef WIN32
#include <windows.h>
#define PORTFN_WINDOWS_IMPLEMENTATION
#endif

#ifdef UNDER_CE
#include <winbase.h>
#define PORTFN_WINDOWS_IMPLEMENTATION
#endif

#ifdef _UNIX
#include <sys/time.h>
#include <unistd.h>
#endif

/*
 *	[IMPLEMENTATION SPECIFIC CODE]
 *
 *	Different platforms have different means
 *	of calculating high resolution times.
 *	For example, Win32 supports ftime() but
 *	WinCE does not. ftime() and timeval are 
 *	supported in UNIX, but neither is really
 *	a POSIX.1 convention.
 *
 *	The goal of this library is really to provide
 *	a fast way for an app to determine if
 *	a particular amount of milliseconds have elapsed.
 *	It does not intend to abstract high-resolution
 *	date/time functions.
 *
 *	This header file assumes that Win32, WinCe,
 *	and Unix are used in the implementation. 
 *	External modules should #define
 *		WIN32
 *		UNDER_CE
 *		_UNIX
 *	to indicate which implementation they want
 *	to use. 
 *
 *	The Win32 and WinCE implementations use
 *	GetTickCount() as the basis for calculating
 *	high resolution times.
 *
 *	Unix uses gettimeofday(struct timeval *tv, struct timezone *tz)
 *	to achieve the same effect.
 */	

/*
 *	MsTime is understood to be opaque
 *	representation of a millisecond
 *	time value.
 */
struct MillisecTime
{
#ifdef PORTFN_WINDOWS_IMPLEMENTATION
	DWORD val;
#endif

#ifdef _UNIX
	struct timeval val;
#endif
};

/*
 *	Stores the high resolution representation
 *	of the current time in the provided
 *	HrTime.
 */
void GetMillisecTime(struct MillisecTime* millisecTime);

/*
 *	Returns millisecond difference of t1-t2.
 */
long DiffMsTimes(struct MillisecTime* t1, struct MillisecTime* t2);

/*
 *	Sets the specified millisecond time to zero.
 */
void SetZeroMsTime(struct MillisecTime* makeZero);

/*
 *	Sleeps for a number of milliseconds
 */
void SleepMsTime(int millisecTime);

#endif
