/*
 * Automated license insertion
 */


#ifndef SYMBIANSEMAPHORE_H_
#define SYMBIANSEMAPHORE_H_

#ifdef __cplusplus
extern "C" 
{
#endif
	void ILibSymbian_CreateSemaphore(void **sem, int initVal);
	void ILibSymbian_DestroySemaphore(void** sem);
	void ILibSymbian_WaitSemaphore(void **sem);
	void ILibSymbian_SignalSemaphore(void **sem);
	int ILibSymbian_TryWaitSemaphore(void **sem);
#ifdef __cplusplus
}
#endif
#endif /*SYMBIANSEMAPHORE_H_*/
