#ifndef WIN_SEMAPHORE_H
#define WIN_SEMAPHORE_H

#define sem_t HANDLE
#define sem_init(p_semaphore,y,initialValue) *p_semaphore=CreateSemaphore(NULL,initialValue,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#define strncasecmp(x,y,z) _strnicmp(x,y,z)

#endif
