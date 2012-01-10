/*
 * Automated license insertion
 */


#include "ILibChainEngine.h"
#include "ILibSocketWrapper.h"

extern "C"
{
	#include "ILibParsers.h"
}

void ILibChainLink::Select(int socketHandle, int Read, int Write, int Error, ILibChainLinkObserver *obs)
{
	RSocket *s = (RSocket*)ILibSocketWrapper_GetSocketFromHandle(socketHandle);
	flags() = 0;
   	
   	if(Read)
   	{
   		flags() |= KSockSelectRead;
   	}
   	if(Write)
   	{
   		flags() |= KSockSelectWrite;
   	}
   	if(Error)
   	{
   		flags() |= KSockSelectExcept;
   	}
   	
   	internalSocket = s;
   	internalSocketHandle = socketHandle;
   	observer = obs;
   	if(s!=NULL)
   	{
		s->Ioctl(KIOctlSelect,iStatus ,&flags, KSOLSocket); 
		SetActive();
   	}
   	else
   	{
   		//
   		// Why is this null?
   		//
   		internalSocket = NULL;
   	}
}
void ILibChainLink::CancelSelect()
{
	if(ILibSocketWrapper_GetSocketFromHandle(internalSocketHandle)!=0)
	{
		internalSocket->CancelIoctl();
	}
}
void ILibChainLink::DoCancel()
{
	CancelSelect();
}
void ILibChainLink::RunL()
{
	int f = flags();
	int Read = flags() & KSockSelectRead;
	int Write = flags() & KSockSelectWrite;
	int Error = flags() & KSockSelectExcept;
	
	observer->OnSelect(this,internalSocketHandle,Read,Write,Error);
}


ILibChainTimer *ILibChainTimer::NewLC()
{
	ILibChainTimer *obj = new (ELeave) ILibChainTimer;
	CleanupStack::PushL(obj);
	obj->ConstructL();
	return(obj);
}
ILibChainTimer *ILibChainTimer::NewL()
{
	ILibChainTimer *obj = ILibChainTimer::NewLC();
	CleanupStack::Pop(obj);
	return(obj);
}
void ILibChainTimer::ConstructL()
{
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
}
ILibChainTimer::ILibChainTimer():CTimer(EPriorityStandard)
{
}
ILibChainTimer::~ILibChainTimer()
{
}
void ILibChainTimer::ElapseAfter(int milliseconds, void *tag, ILibChainTimerObserver *obs)
{
	this->UserTag = tag;
	this->observer = obs;
	this->After(milliseconds*1000);
}
void ILibChainTimer::RunL()
{
	this->observer->OnElapsed(this,this->UserTag);
}
void ILibChainTimer::DoCancel()
{
	CTimer::DoCancel();
}
TInt ILibChainTimer::RunError(TInt aError)
{
	return(aError);
}
ILibChainEngine::~ILibChainEngine()
{
}
ILibChainEngine::ILibChainEngine()
{
	for(int i=0;i<SOCKET_ARRAY_SIZE;++i)
	{
		Set[i]=NULL;
	}
	SetIndex=0;
	StartStop=0;
}
ILibChainLink::ILibChainLink():CActive(EPriorityStandard)
{
	iStatus = KRequestPending;
	Marked = 0;
}
void ILibChainLink::ConstructL()
{
	CActiveScheduler::Add(this);
}
ILibChainLink *ILibChainLink::NewLC()
{
	ILibChainLink *obj = new (ELeave) ILibChainLink;
	CleanupStack::PushL(obj);
	obj->ConstructL();
	return(obj);
}
ILibChainLink *ILibChainLink::NewL()
{
	ILibChainLink *obj = ILibChainLink::NewLC();
	CleanupStack::Pop(obj);
	return(obj);
}
ILibChainLink::~ILibChainLink()
{
	if(this->IsActive())
	{
		this->Cancel();
	}
}

int ILibChainLink::MatchesHandle(int socketHandle)
{
	return(socketHandle==this->internalSocketHandle?1:0);
}
int ILibChainLink::MatchesSelectFlags(int Read, int Write, int Error)
{
	if(Read)
   	{
   		if(!(flags() & KSockSelectRead))
   		{
   			return(0);
   		}
   	}
   	if(Write)
   	{
   		if(!(flags() & KSockSelectWrite))
   		{
   			return(0);
   		}
   	}
   	if(Error)
   	{
   		if(!(flags() & KSockSelectExcept))
   		{
   			return(0);
   		}
   	}
   	if((flags() & KSockSelectRead))
	{
		if(Read==0)
		{
			return(0);
		}
	}
	if((flags() & KSockSelectWrite))
	{
		if(Write==0)
		{
			return(0);
		}
	}	
	if((flags() & KSockSelectExcept))
	{
		if(Error==0)
		{
			return(0);
		}
	}
   	return(1);
}


void ILibChainEngine::StartSelectTimeout(int timeout)
{
	if(chainTimer->IsActive())
	{
		// There is already a timeout, so lets see if it's short enough
		if(this->currentTimeout>timeout)
		{
			// Nope
			chainTimer->Cancel();
			chainTimer->ElapseAfter(timeout,this,this);
			this->currentTimeout = timeout;
		}
	}
	else
	{
		// Set timeout, because there isn't one
		chainTimer->ElapseAfter(timeout,this,this);
		this->currentTimeout = timeout;
	}
}
void ILibChainEngine::ForceUnBlock()
{
	int i;
	//ToDo: Look into whether or not we need to be locked

	if(this->chainTimer->IsActive())
	{
		this->chainTimer->Cancel();
	}
	this->currentTimeout = 0;
	chainTimer->ElapseAfter(0,this,this);
}

void ILibChainEngine::AddSelect_Start()
{
	int i=0;
	
	this->StartStop = 1;
	for(i=0;i<SetIndex;++i)
	{
		Set[i]->Marked = 0;
	}
}
void ILibChainEngine::AddSelect_Done()
{
	int i,i2;
	StartStop = 0;
	
	for(i=0;i<SetIndex;++i)
	{
		if(Set[i]->Marked==0)
		{
			if(Set[i]->IsActive())
			{
				Set[i]->Cancel();
			}
			delete Set[i];
			for(i2=i;i2<SetIndex-1;++i2)
			{
				Set[i2] = Set[i2+1];
			}
			--SetIndex;
		}
	}
	
}

void ILibChainEngine::AddSelect(int socketHandle, int Read, int Write, int Error)
{
	if(StartStop==0)
	{
		ILibChainLink *link = ILibChainLink::NewL();
		Set[SetIndex] = link;
		++SetIndex;
		
		//ToDo: Add link to the Active Scheduler
		link->Select(socketHandle,Read,Write,Error,this);
	}
	else
	{
		int i;
		for(i=0;i<SetIndex;++i)
		{
			if(Set[i]->MatchesHandle(socketHandle)!=0)
			{
				//
				// Same handle
				//
				if(Set[i]->MatchesSelectFlags(Read,Write,Error)!=0)
				{
					//
					// Matches, so jus tmark the existing one
					//
					Set[i]->Marked = 1;
				}
				else
				{
					//
					// Different set of criteria, so me must delete the old one first
					//
					Set[i]->Cancel();
					delete Set[i];
					Set[i] = ILibChainLink::NewL();
					Set[i]->Select(socketHandle,Read,Write,Error,this);
					Set[i]->Marked = 1;
				}
				break;
			}
		}
		if(!(i<SetIndex))
		{
			//
			// There were no matches, so add a new one
			//
			Set[SetIndex] = ILibChainLink::NewL();
			Set[SetIndex]->Select(socketHandle,Read,Write,Error,this);
			Set[SetIndex]->Marked = 1;
			++SetIndex;
		}
	}
}

void ILibChainEngine::OnElapsed(ILibChainTimer *sender, void *tag)
{
	//
	// Timeout condition
	//
	int i = 0;
	/*
	for(i=0;i<SetIndex;++i)
	{
		if(Set[i]->IsActive())
		{
			Set[i]->CancelSelect();
		}
	}
	*/
	
	PreSelect();
}
void ILibChainEngine::PreSelect()
{
	if(PreSelectPtr!=NULL)
	{
		PreSelectPtr((void*)this);
	}
}
void ILibChainEngine::PostSelect(int socketHandle, int Read, int Write, int Error)
{
	ILibSocketWrapper_struct_FDSET readset;
	ILibSocketWrapper_struct_FDSET writeset;
	ILibSocketWrapper_struct_FDSET errorset;
	
	ILibSocketWrapper_FDZERO(&readset);
	ILibSocketWrapper_FDZERO(&writeset);
	ILibSocketWrapper_FDZERO(&errorset);
	
	if(Read!=0)
	{
		ILibSocketWrapper_FDSET(socketHandle,&readset);
	}
	if(Write!=0)
	{
		ILibSocketWrapper_FDSET(socketHandle,&writeset);
	}
	if(Error!=0)
	{
		ILibSocketWrapper_FDSET(socketHandle,&errorset);
	}
	if(PostSelectPtr!=NULL)
	{
		PostSelectPtr((void*)this,&readset,&writeset,&errorset);
	}
	
	//
	// Remove the Link ActiveObjects
	//
	/*
	for(int i=0;i<SetIndex;++i)
	{
		delete Set[i];
	}
	SetIndex=0;
	*/
}
void ILibChainEngine::OnSelect(ILibChainLink *sender, int socketHandle, int Read, int Write, int Error)
{
	//
	// Cancel all the other pending Selects, and deque
	//
	/*
	int i=0;
	for(i=0;i<SetIndex;++i)
	{
		if(Set[i]!=sender)
		{
			Set[i]->Cancel();
		}
	}
	*/
	
	//
	// Cancel the timeout
	//
	this->chainTimer->Cancel();
	
	//
	// Remove ourselves from the list of ActiveObjects
	//
	int i=0;
	int i2;
	for(i=0;i<SetIndex;++i)
	{
		if(Set[i]==sender)
		{
			delete Set[i];
			for(i2=i;i2<SetIndex-1;++i2)
			{
				Set[i2] = Set[i2+1];
			}
			--SetIndex;
			break;
		}
	}
	
	//
	// Trigger Event
	//
	PostSelect(socketHandle,Read,Write,Error);
	PreSelect();
}
void ILibChainEngine::StopChain()
{
	//
	// Underlying stack has destroyed the chain,
	// so we need to clear all the active objects
	//
	
	int i;
	for(i=0;i<SetIndex;++i)
	{
		if(Set[i]->IsActive())
		{
			Set[i]->Cancel();
		}
		delete Set[i];
	}
	if(this->chainTimer->IsActive())
	{
		chainTimer->Cancel();
	}
	delete chainTimer;
	delete this;
}
void ILibChainEngine::StartChain(ILibChainEngine_OnPreSelect OnPreSelect, ILibChainEngine_OnPostSelect OnPostSelect, ILibChainEngine_OnDestroy OnDestroy)
{
	PreSelectPtr = OnPreSelect;
	PostSelectPtr = OnPostSelect;
	DestroyPtr = OnDestroy;
	
	// Create and install the active scheduler
    //CActiveScheduler* myScheduler=new (ELeave) CActiveScheduler;
    //CleanupStack::PushL(myScheduler);
    //CActiveScheduler::Install(myScheduler);

	chainTimer = ILibChainTimer::NewL();
	chainTimer->ElapseAfter(0,this,this);
    //CActiveScheduler::Start();

    // Remove the exampleScheduler and other
    // objects from cleanup stack and destroy them
    //CleanupStack::PopAndDestroy(2);
}
