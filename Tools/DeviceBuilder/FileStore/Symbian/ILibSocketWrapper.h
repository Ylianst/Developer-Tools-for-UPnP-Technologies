/*
 * Automated license insertion
 */


#ifndef SOCKETWRAPPER_H_
#define SOCKETWRAPPER_H_
#endif /*SOCKETWRAPPER_H_*/


#define SOCKET_ARRAY_SIZE 256
#define MAX_SOCKET_BUFFER_SIZE 8192

#ifdef __cplusplus
extern "C" 
{
#endif

#include <libc\stddef.h>
#include <libc\sys\types.h>
#include <libc\sys\socket.h>
#include <libc\netinet\in.h>
#include <libc\arpa\inet.h>

typedef int ILibSocketWrapper_struct_FDSET[SOCKET_ARRAY_SIZE];

void ILibSocketWrapper_Create();
void ILibSocketWrapper_Destroy();
int ILibSocketWrapper_socket(int socketType);
int ILibSocketWrapper_close(int socketObject);
int ILibSocketWrapper_bind(int socketObject, struct sockaddr *local);
int ILibSocketWrapper_getsockname(int socketObject, struct sockaddr* local, int* length);
int ILibSocketWrapper_GetLocalIPAddressList(int iplist[]);
int ILibSocketWrapper_GetInterfaceIndex(long localAddr);
int ILibSocketWrapper_GetSocketFromHandle(int socketHandle);

int ILibSocketWrapper_SetReuseAddr(int socketObject, int enabled);
int ILibSocketWrapper_connect(int socketObject,struct sockaddr *dest);
int ILibSocketWrapper_listen(int socketObject,int backlog);
int ILibSocketWrapper_accept(int socketObject);
int ILibSocketWrapper_joinmulticastgroup(int socketObject, long multicastAddress, long multicastInterface);
int ILibSocketWrapper_sendto(int socketObject, char *buffer, int bufferLength, struct sockaddr *dest);
int ILibSocketWrapper_send(int socketObject, char *buffer, int bufferLength);
int ILibSocketWrapper_recv(int socketObject, char *buffer, int bufferLength);
int ILibSocketWrapper_recvfrom(int socketObject, char *buffer, int bufferLength, struct sockaddr *src);

void ILibSocketWrapper_FDZERO(ILibSocketWrapper_struct_FDSET *fds);
void ILibSocketWrapper_FDSET(int obj, ILibSocketWrapper_struct_FDSET *fds);
int ILibSocketWrapper_FDISSET(int obj, ILibSocketWrapper_struct_FDSET *fds);

#ifdef __cplusplus
}
#endif