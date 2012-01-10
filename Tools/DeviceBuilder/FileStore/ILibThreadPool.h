/*   
Copyright 2006 - 2010 Intel Corporation

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

#ifndef __ILIBTHREADPOOL__
#define __ILIBTHREADPOOL__

/*! \file ILibThreadPool.h 
	\brief MicroStack APIs for platform independent threadpooling capabilities
*/

/*! \defgroup ILibThreadPool ILibThreadPool Module
	\{
*/

/*! \typedef ILibThreadPool
	\brief Handle to an ILibThreadPool module
*/
typedef void* ILibThreadPool;
/*! \typedef ILibThreadPool_Handler
	\brief Handler for a thread pool work item
	\param sender The ILibThreadPool handle
	\param var State object
*/
typedef void(*ILibThreadPool_Handler)(ILibThreadPool sender, void *var);

ILibThreadPool ILibThreadPool_Create();
void ILibThreadPool_AddThread(ILibThreadPool pool);
void ILibThreadPool_QueueUserWorkItem(ILibThreadPool pool, void *var, ILibThreadPool_Handler callback);
void ILibThreadPool_Destroy(ILibThreadPool pool);
int ILibThreadPool_GetThreadCount(ILibThreadPool pool);


/*! \} */
#endif
