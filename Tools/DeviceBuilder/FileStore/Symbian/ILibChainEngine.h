/*
 * Automated license insertion
 */


#ifndef CHAINENGINE_H_
#define CHAINENGINE_H_

#include "ILibChainDefs.h"
#include "ILibSocketWrapper.h"

#ifdef __cplusplus
#include <e32base.h>
#include <ES_SOCK.h>

class ILibChainLink;
class ILibChainTimer;
#endif

#ifdef __cplusplus
class ILibChainLinkObserver
{
	public:
	virtual void OnSelect(ILibChainLink *sender, int socketHandle, int Read, int Write, int Error)=NULL;
};
class ILibChainTimerObserver
{
	public:
	virtual void OnElapsed(ILibChainTimer *sender, void *tag)=NULL;
};
class ILibChainTimer : public CTimer
{
	public:
	ILibChainTimer();
	ILibChainTimer::~ILibChainTimer();
	static ILibChainTimer* NewL();
	static ILibChainTimer* NewLC();
	void ElapseAfter(int milliseconds, void *tag, ILibChainTimerObserver *obs);

	private:
	void ConstructL();
	void *UserTag;
	ILibChainTimerObserver *observer;
	virtual void RunL();
	virtual void DoCancel();
	virtual TInt RunError(TInt aError);
};

class ILibChainEngine : public ILibChainLinkObserver, public ILibChainTimerObserver
{
	public:
	ILibChainEngine();
	ILibChainEngine::~ILibChainEngine();
	
	void *Tag;
	void StopChain();
	void StartChain(ILibChainEngine_OnPreSelect OnPreSelect, ILibChainEngine_OnPostSelect OnPostSelect, ILibChainEngine_OnDestroy OnDestroy);
	void ForceUnBlock();
	void AddSelect(int socketHandle, int Read, int Write, int Error);
	void AddSelect_Start();
	void AddSelect_Done();
	void StartSelectTimeout(int timeout);
	virtual void OnSelect(ILibChainLink *sender, int socketHandle, int Read, int Write, int Error);
	virtual void OnElapsed(ILibChainTimer *sender, void *tag);
	
	void *chain;

	private:
	int StartStop;
	
	ILibChainTimer *chainTimer;
	int currentTimeout;
	
	void PreSelect();
	void PostSelect(int socketHandle, int Read, int Write, int Error);
	
	ILibChainEngine_OnPreSelect PreSelectPtr;
	ILibChainEngine_OnPostSelect PostSelectPtr;
	ILibChainEngine_OnDestroy DestroyPtr;
	
	ILibChainLink *Set[SOCKET_ARRAY_SIZE];
	int SetIndex;
};

class ILibChainLink : public CActive
{
	public:
	ILibChainLink();
	ILibChainLink::~ILibChainLink();
	static ILibChainLink* NewL();
	static ILibChainLink* NewLC();
	
	void Select(int socketHandle, int Read, int Write, int Error, ILibChainLinkObserver *obs);
	void CancelSelect();
	
	int Marked;
	int MatchesHandle(int socketHandle);
	int MatchesSelectFlags(int Read, int Write, int Error);
	
	private:
	void ConstructL();
	TPckgBuf<TUint> flags;
	RSocket *internalSocket;
	int internalSocketHandle;
	ILibChainLinkObserver *observer;
	
	virtual void RunL();
	virtual void DoCancel();
};
#endif
#endif /*CHAINENGINE_H_*/
