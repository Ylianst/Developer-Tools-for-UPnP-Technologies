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

#include "ILibThreadPool.h"

#if defined(WINSOCK2)
#include <winsock2.h>
#elif defined(WINSOCK1)
#include <winsock.h>
#endif

#include "ILibParsers.h"

struct ILibThreadPool_WorkItem
{
	ILibThreadPool_Handler Callback;
	void *var;
};
struct ILibThreadPool_ThreadState
{
	int NumThreads;
	int Terminate;
	void *WorkItemQueue;
	sem_t SyncHandle;
	sem_t AbortHandle;
};

/*! \fn ILibThreadPool ILibThreadPool_Create()
	\brief Instantiate a new ILibThreadPool handle
	\returns Handle to a new ILibThreadPool module
*/
ILibThreadPool ILibThreadPool_Create()
{
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)malloc(sizeof(struct ILibThreadPool_ThreadState));
	memset(ts,0,sizeof(struct ILibThreadPool_ThreadState));

	ts->WorkItemQueue = ILibQueue_Create();
	sem_init(&(ts->SyncHandle),0,0);
	sem_init(&(ts->AbortHandle),0,0);
	return(ts);
}
int ILibThreadPool_GetThreadCount(ILibThreadPool pool)
{
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)pool;
	return(ts->NumThreads);
}
/*! \fn void ILibThreadPool_Destroy(ILibThreadPool pool)
	\brief Free the resources associated with an ILibThreadPool module
	\param pool Handle to free
*/
void ILibThreadPool_Destroy(ILibThreadPool pool)
{
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)pool;
	int ok = 0;
	int count = 0;

	ILibQueue_Lock(ts->WorkItemQueue);
	ts->Terminate=1;
	ok = count = ts->NumThreads;
	ILibQueue_UnLock(ts->WorkItemQueue);


	while(count!=0)
	{
		sem_post(&(ts->SyncHandle));
		--count;
	}

	if(ok!=0)
	{
		sem_wait(&(ts->AbortHandle));
	}

	sem_destroy(&(ts->SyncHandle));
	sem_destroy(&(ts->AbortHandle));
	ILibQueue_Destroy(ts->WorkItemQueue);

	free(pool);
}

/*! \fn void ILibThreadPool_AddThread(ILibThreadPool pool)
	\brief Gives ownership of the current thread to the pool.
	\par
	This method will not return until the ThreadPool is destroyed
	\param pool Handle to the ILibThreadPool module to add a thread to
*/
void ILibThreadPool_AddThread(ILibThreadPool pool)
{
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)pool;
	struct ILibThreadPool_WorkItem *wi = NULL;
	int abort = 0;
	int id;
	int ok=0;

	ILibQueue_Lock(ts->WorkItemQueue);
	++ts->NumThreads;
	id = ts->NumThreads;
	ILibQueue_UnLock(ts->WorkItemQueue);

	do
	{
		sem_wait(&(ts->SyncHandle));

		ILibQueue_Lock(ts->WorkItemQueue);
		wi = (struct ILibThreadPool_WorkItem*)ILibQueue_DeQueue(ts->WorkItemQueue);	
		abort = ts->Terminate;
		ILibQueue_UnLock(ts->WorkItemQueue);

		if(wi!=NULL && !abort)
		{
			wi->Callback(pool,wi->var);
			free(wi);
		}
		else if(wi!=NULL)
		{
			free(wi);
		}

		ILibQueue_Lock(ts->WorkItemQueue);
		abort = ts->Terminate;
		ILibQueue_UnLock(ts->WorkItemQueue);
		
	}while(!abort);

	ILibQueue_Lock(ts->WorkItemQueue);
	--ts->NumThreads;
	ok = ts->NumThreads;
	ILibQueue_UnLock(ts->WorkItemQueue);

	if(ok==0)
	{
		sem_post(&(ts->AbortHandle));
	}
}

/*! \fn void ILibThreadPool_QueueUserWorkItem(ILibThreadPool pool, void *var, ILibThreadPool_Handler callback)
	\brief Queues a new work item to the thread pool
	\param pool The ILibThreadPool handle to queue the work item to
	\param var State object to pass to the handler
	\param callback The handler to be called when an available thread is ready to process the work item
*/
void ILibThreadPool_QueueUserWorkItem(ILibThreadPool pool, void *var, ILibThreadPool_Handler callback)
{
	struct ILibThreadPool_WorkItem *wi = NULL;
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)pool;
	int NumThreads=0;

	wi = (struct ILibThreadPool_WorkItem*)malloc(sizeof(struct ILibThreadPool_WorkItem));
	memset(wi,0,sizeof(struct ILibThreadPool_WorkItem));
	wi->var = var;
	wi->Callback = callback;
	
	ILibQueue_Lock(ts->WorkItemQueue);
		
	if(ts->NumThreads!=0)
	{
		NumThreads = ts->NumThreads;
		ILibQueue_EnQueue(ts->WorkItemQueue,wi);
		sem_post(&(ts->SyncHandle));
	}
	ILibQueue_UnLock(ts->WorkItemQueue);

	if(NumThreads==0)
	{
		//
		// There are no threads in the Pool, so call this thing from here
		//
		wi->Callback(pool,wi->var);
		free(wi);
	}
}
