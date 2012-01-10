/*
 * Automated license insertion
 */


#ifndef CHAINDEFS_H_
#define CHAINDEFS_H_

typedef void(*ILibChainEngine_OnPreSelect)(void* sender);
typedef void(*ILibChainEngine_OnPostSelect)(void* sender, void *fds_read, void *fds_write, void *fds_error);
typedef void(*ILibChainEngine_OnDestroy)(void* sender);

#endif /*CHAINDEFS_H_*/
