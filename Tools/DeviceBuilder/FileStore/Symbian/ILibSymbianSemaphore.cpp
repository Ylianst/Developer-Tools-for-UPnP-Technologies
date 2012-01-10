/*
 * Automated license insertion
 */


#include "ILibSymbianSemaphore.h"
#include "e32cmn.h"


void ILibSymbian_CreateSemaphore(void **sem, int initVal)
{
	RSemaphore *RetVal = new RSemaphore();
	RetVal->CreateLocal(initVal);
	*sem = (void*)RetVal;
}
void ILibSymbian_DestroySemaphore(void** sem)
{
	(*((RSemaphore**)sem))->Close();
  delete *sem;
  *sem = NULL;
}
void ILibSymbian_WaitSemaphore(void **sem)
{
	(*((RSemaphore**)sem))->Wait();
}
void ILibSymbian_SignalSemaphore(void **sem)
{
	(*((RSemaphore**)sem))->Signal();
}
int ILibSymbian_TryWaitSemaphore(void **sem)
{
	int v=(*((RSemaphore**)sem))->Wait(0);
	if(v!=KErrNone)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

