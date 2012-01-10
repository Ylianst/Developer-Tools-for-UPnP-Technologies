#ifndef MICRO_STB_H
#define MICRO_STB_H

#include <stdlib.h>
#include "WinSemaphore.h"

sem_t UpnpIPAddressListLock;

#ifdef _POSIX
void *UpnpMonitor;
int UpnpIPAddressListLength;
int *UpnpIPAddressList;
#endif

#ifdef _WINSOCK1
void *UpnpMonitor;
int UpnpIPAddressListLength;
int *UpnpIPAddressList;
#endif

#ifdef _WINSOCK2
DWORD UpnpMonitorSocketReserved;
WSAOVERLAPPED UpnpMonitorSocketStateObject;
SOCKET UpnpMonitorSocket;
#endif

#endif
