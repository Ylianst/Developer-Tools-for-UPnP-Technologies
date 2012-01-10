/*
 * Automated license insertion
 */


#include "ILibChainEngine.h"
#include "ILibChainAdaptor.h"
#include "ILibSocketWrapper.h"


void ILibChainAdaptor_Select(void *chainAdaptor, void *readset, void *writeset, void *errorset, int timeout)
{
	ILibChainEngine *engine = (ILibChainEngine*)chainAdaptor;
	int r,w,e;
	ILibSocketWrapper_struct_FDSET *fds;
	
	engine->AddSelect_Start();
	for(int i=0;i<SOCKET_ARRAY_SIZE;++i)
	{
		r=w=e=0;
		fds = (ILibSocketWrapper_struct_FDSET*)readset;
		if((*fds)[i]!=0)
		{
			r=1;
		}
		fds = (ILibSocketWrapper_struct_FDSET*)writeset;
		if((*fds)[i]!=0)
		{
			w=1;
		}
		fds = (ILibSocketWrapper_struct_FDSET*)errorset;
		if((*fds)[i]!=0)
		{
			e=1;
		}
		if(r!=0||w!=0||e!=0)
		{
			engine->AddSelect(i,r,w,e);
		}
	}
	engine->AddSelect_Done();
	engine->StartSelectTimeout(timeout);
}

void* ILibChainAdaptor_CreateChainAdaptor(void *chain)
{
	ILibChainEngine *RetVal = new ILibChainEngine();
	RetVal->chain = chain;
	return((void*)RetVal);
}
void ILibChainAdaptor_StartChain(void *chainAdaptor, ILibChainEngine_OnPreSelect OnPreSelect, ILibChainEngine_OnPostSelect OnPostSelect, ILibChainEngine_OnDestroy OnDestroy)
{
	ILibChainEngine *c = (ILibChainEngine*)chainAdaptor;
	c->StartChain(OnPreSelect,OnPostSelect,OnDestroy);
}
void ILibChainAdaptor_StopChain(void *chainAdaptor)
{
	ILibChainEngine *c = (ILibChainEngine*)chainAdaptor;
	c->StopChain();
}
void ILibChainAdaptor_ForceUnBlock(void *chainAdaptor)
{
	ILibChainEngine *c = (ILibChainEngine*)chainAdaptor;
	c->ForceUnBlock();	
}
void ILibChainAdaptor_SetTag(void *chainAdaptor, void *tag)
{
	ILibChainEngine *c = (ILibChainEngine*)chainAdaptor;
	c->Tag = tag;
}
void *ILibChainAdaptor_GetTag(void *chainAdaptor)
{
	ILibChainEngine *c = (ILibChainEngine*)chainAdaptor;
	return(c->Tag);
}
void* ILibChainAdaptor_GetChain(void *chainAdaptor)
{
	ILibChainEngine *c = (ILibChainEngine*)chainAdaptor;
	return(c->chain);
}
