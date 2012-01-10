/*
 * Automated license insertion
 */

#ifndef CHAINADAPTOR_
#define CHAINADAPTOR_

#include "ILibChainDefs.h"

#ifdef __cplusplus
extern "C"
{
#endif
void* ILibChainAdaptor_CreateChainAdaptor(void *chain);
void* ILibChainAdaptor_GetChain(void *chainAdaptor);
void ILibChainAdaptor_SetTag(void *chainAdaptor, void *tag);
void *ILibChainAdaptor_GetTag(void *chainAdaptor);
void ILibChainAdaptor_StartChain(void *chainAdaptor, ILibChainEngine_OnPreSelect OnPreSelect, ILibChainEngine_OnPostSelect OnPostSelect,ILibChainEngine_OnDestroy OnDestroy);
void ILibChainAdaptor_StopChain(void *chainAdaptor);
void ILibChainAdaptor_ForceUnBlock(void *chainAdaptor);
void ILibChainAdaptor_Select(void *chainAdaptor, void *readset, void *writeset, void *errorset, int timeout);
#ifdef __cplusplus
}
#endif


#endif /*CHAINADAPTOR_*/
