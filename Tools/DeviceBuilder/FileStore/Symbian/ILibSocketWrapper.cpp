/*
 * Automated license insertion
 */


#include "ILibSocketWrapper.h"

#include <e32cmn.h>
#include <ES_SOCK.h>
#include <e32des8.h>
#include <e32base.h>
#include <ES_SOCK.h>
#include <in_sock.h>
#include <CommDbConnPref.h>
///@todo change SocketArray to an array of structs including both socket & connection instances
RConnection* ConnectionArray[SOCKET_ARRAY_SIZE];

int CurrentSocketIndex = 0;
int SocketArray[SOCKET_ARRAY_SIZE];
int Initialized = 0;

void ILibSocketWrapper_FDZERO(ILibSocketWrapper_struct_FDSET *fds)
{
	for(int i=0;i<SOCKET_ARRAY_SIZE;++i)
	{
		(*fds)[i] = 0;
	}
}
void ILibSocketWrapper_FDSET(int obj, ILibSocketWrapper_struct_FDSET *fds)
{
	(*fds)[obj] = 1;
}
int ILibSocketWrapper_FDISSET(int obj, ILibSocketWrapper_struct_FDSET *fds)
{
	return((*fds)[obj]);
}
int ILibSocketWrapper_GetSocketFromHandle(int socketHandle)
{
	RSocket *s = (RSocket*)SocketArray[socketHandle];
	return((int)s);
}

class CSocketWrapperEvent : CActive
{
	public:

	CSocketWrapperEvent();
	CSocketWrapperEvent::~CSocketWrapperEvent();
	TRequestStatus *GetIStatus();
	static CSocketWrapperEvent* NewL();
	static CSocketWrapperEvent* NewLC();
	
	void Activate();
	
	void *Reserved_dest;
	void *Reserved_buf;
		
	private:
	
	void ConstructL();
	virtual void RunL();
	virtual void DoCancel();
	virtual TInt RunError(TInt aError);
};

TRequestStatus* CSocketWrapperEvent::GetIStatus()
{
	return(&iStatus);
}
CSocketWrapperEvent *CSocketWrapperEvent::NewLC()
{
	CSocketWrapperEvent *obj = new (ELeave) CSocketWrapperEvent;
	CleanupStack::PushL(obj);
	obj->ConstructL();
	return(obj);
}
CSocketWrapperEvent *CSocketWrapperEvent::NewL()
{
	CSocketWrapperEvent *obj = CSocketWrapperEvent::NewLC();
	CleanupStack::Pop(obj);
	return(obj);
}
void CSocketWrapperEvent::Activate()
{
	SetActive();
}
void CSocketWrapperEvent::ConstructL()
{
	Reserved_dest = NULL;
	Reserved_buf = NULL;
	CActiveScheduler::Add(this);
}
CSocketWrapperEvent::CSocketWrapperEvent():CActive(EPriorityStandard)
{
	iStatus = KRequestPending;
}
CSocketWrapperEvent::~CSocketWrapperEvent()
{
}
TInt CSocketWrapperEvent::RunError(TInt aError)
{
	return(aError);
}
void CSocketWrapperEvent::RunL()
{
	//
	// Free some resources if necessary
	//
	if(this->Reserved_buf!=NULL)
	{
		delete Reserved_buf;
		Reserved_buf = NULL;
	}
	if(this->Reserved_dest!=NULL)
	{
		delete Reserved_dest;
		Reserved_dest=NULL;
	}
	int ecode = iStatus.Int();
  // finished with the AO
	delete this;
}
void CSocketWrapperEvent::DoCancel()
{
	//
	// Do Nothing
	//
}

RSocketServ socketServer;
int socketInit = 0;

HBufC8 *pchar2HBufC8(char *inbuffer, int inbufferLength)
{
	HBufC8* retVal = HBufC8::NewMaxL(inbufferLength);
	Mem::Copy((void*)(retVal->Ptr()),inbuffer,inbufferLength);
	return(retVal);
}

int ILibSocketWrapper_GetNextAvailableHandle()
{
	int errorCheck = 0;
	do
	{
		++errorCheck;
		CurrentSocketIndex = (CurrentSocketIndex+1)==SOCKET_ARRAY_SIZE?1:CurrentSocketIndex+1;
	}while(errorCheck < SOCKET_ARRAY_SIZE && SocketArray[CurrentSocketIndex]!=0);
	
	if(errorCheck<SOCKET_ARRAY_SIZE)
	{
		return(CurrentSocketIndex);
	}
	else
	{
		return(-1);
	}
}
int ILibSocketWrapper_getsockname(int socketObject, struct sockaddr* local, int* length)
{
  struct sockaddr_in* localAddr = (struct sockaddr_in*)local;
	RSocket *s = (RSocket*)SocketArray[socketObject];
  TInetAddr sockAddr;

  // get the local name
  s->LocalName(sockAddr);

  // convert from Symbian
  localAddr->sin_family = sockAddr.Family();
  localAddr->sin_port = sockAddr.Port();
  localAddr->sin_addr.s_addr = ntohl(sockAddr.Address());
  
  return 0;
}
int ILibSocketWrapper_socket(int socketType)
{
	int RetVal;

  // create a new connection for the socket
  RConnection* pConnection = new RConnection;

  // open a connection
  pConnection->Open(socketServer);

  TCommDbConnPref prefs;
  // set the preference for the requested interface
  prefs.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
  pConnection->Start(prefs);	

	if ( (socketType == SOCK_DGRAM) ||
       (socketType == SOCK_STREAM) )
	{
    RSocket *pSocket = new RSocket();

    if ( socketType == SOCK_DGRAM )
    {
      pSocket->Open(socketServer,
                    KAfInet,
                    KSockDatagram,
                    KProtocolInetUdp,
                    *pConnection);
    }
    else
    {
      pSocket->Open(socketServer,
                    KAfInet,
                    KSockStream,
                    KProtocolInetTcp,
                    *pConnection);
    }

    RetVal = ILibSocketWrapper_GetNextAvailableHandle();

    if( RetVal >= 0 )
    {
      SocketArray[RetVal] = (int)pSocket;
      ConnectionArray[RetVal] = pConnection;
    }
    else
    {
      pSocket->Close();
      delete pSocket;
      pConnection->Close();
      delete pConnection;
      RetVal = -1;
    }
  }

  return RetVal;
}
int ILibSocketWrapper_bind(int socketObject, struct sockaddr *local)
{
	TInetAddr addr = TInetAddr(htonl(((struct in_addr*)local->sa_data)->s_addr),ntohs(local->sa_port));
	
	RSocket *s = (RSocket*)SocketArray[socketObject];
	
	if(s->Bind(addr)==KErrNone)
	{
		return(0);
	}
	else
	{
		return(-1);
	}
}
int ILibSocketWrapper_listen(int socketObject,int backlog)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];
	if(s->Listen(backlog)==KErrNone)
	{
		return(0);
	}
	else
	{
		return(-1);
	}
}
int ILibSocketWrapper_accept(int socketObject)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];
	TRequestStatus rs;
	RSocket *newSocket;
	int RetVal = ILibSocketWrapper_GetNextAvailableHandle();
	
	if(RetVal>0)
	{
		newSocket = new RSocket();
		newSocket->Open(socketServer);
	
		//
		// ToDo: Verify this works. For now, I'm assuming that if we
		// call this after we get notification tha the listen socket is writable
		// that this call will complete immediately.
		//
		s->Accept(*newSocket,rs);
		User::WaitForRequest(rs);
		SocketArray[RetVal] = (int)newSocket;
		return(RetVal);
	}
	return(-1);
}
int ILibSocketWrapper_joinmulticastgroup(int socketObject, long multicastAddress, long multicastInterface)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];

	int RetVal;
	TInetAddr dst;
	
	dst.SetAddress(htonl(multicastAddress));
	dst.SetPort(0);
	dst.ConvertToV4Mapped();
	
	TPckgBuf<TIp6Mreq> req;
	
	req().iAddr = dst.Ip6Address();
	req().iInterface = ILibSocketWrapper_GetInterfaceIndex(multicastInterface);
   	RetVal = s->SetOpt(KSoIp6JoinGroup,KSolInetIp,req);
	if(RetVal==KErrNone)
	{
		return(0);
	}
	else
	{
		s->SetOpt(KSoIp6LeaveGroup,KSolInetIp,req);
		return(-1);
	}
}
int ILibSocketWrapper_sendto(int socketObject, char *buffer, int bufferLength, struct sockaddr *dest)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];
	HBufC8 *buf = pchar2HBufC8(buffer,bufferLength);
	CSocketWrapperEvent *e = CSocketWrapperEvent::NewL();
//	CSocketWrapperEvent *e2 = CSocketWrapperEvent::NewL();
//	TPckgBuf<TUint> *ioctlResult = new TPckgBuf<TUint>(KSockSelectWrite|KSockSelectExcept);
	
//	e2->Reserved = 1;
	
	TInetAddr *dst = new TInetAddr(htonl(((struct in_addr*)dest->sa_data)->s_addr),ntohs(dest->sa_port));
	
	e->Reserved_dest = dst;
	e->Reserved_buf = buf;
	
	s->SendTo(*buf,*dst,0,*(e->GetIStatus()));	
	e->Activate();
	
	
//   	s->Ioctl(KIOctlSelect,*(e2->GetIStatus()) ,ioctlResult, KSOLSocket); 
//   	e2->Activate();
   	
   	
	return(-1);
	
}
int ILibSocketWrapper_send(int socketObject, char *buffer, int bufferLength)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];
	HBufC8 *buf = pchar2HBufC8(buffer,bufferLength);
	CSocketWrapperEvent *e = CSocketWrapperEvent::NewL();
	
	e->Reserved_dest = NULL;
	e->Reserved_buf = buf;
	s->Send(*buf,0,*(e->GetIStatus()));
	e->Activate();
		
	return(-1);
}
int ILibSocketWrapper_recv(int socketObject, char *buffer, int bufferLength)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];
	RBuf8 *buf = new RBuf8();
	
	TRequestStatus status;
	TSockXfrLength aLen;
	int RetVal=0;
	
	if(buf->Create(bufferLength)==KErrNone)
	{
		s->RecvOneOrMore(*buf,0,status,aLen);
		User::WaitForRequest(status);
		if(status!=KErrNone)
		{
			RetVal = 0;
		}
		else
		{
			RetVal = aLen();
			Mem::Copy(buffer,(void*)buf->Ptr(),RetVal);
		}
	}
	buf->Close();
	delete buf;
	return(RetVal);
}
int ILibSocketWrapper_recvfrom(int socketObject, char *buffer, int bufferLength, struct sockaddr *src)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];
	TRequestStatus status;
	TInetAddr addr;
	int RetVal=0;
	RBuf8 *buf = new RBuf8();
	
	if(buf->Create(bufferLength)==KErrNone)
	{
		TProtocolDesc aProtocol;
		
		s->Info(aProtocol);
		if(aProtocol.iSockType==KSockStream)
		{
			s->RemoteName(addr);
			((struct in_addr*)src->sa_data)->s_addr = ntohl(addr.Address());
			src->sa_port = htons(addr.Port());
			RetVal = ILibSocketWrapper_recv(socketObject, buffer, bufferLength);
		}
		else
		{
			s->RecvFrom(*buf,addr,(unsigned int)0,status);
			User::WaitForRequest(status);
			if(status!=KErrNone)
			{
				RetVal = 0;
			}
			else
			{
				((struct in_addr*)src->sa_data)->s_addr = ntohl(addr.Address());
				src->sa_port = htons(addr.Port());
				Mem::Copy(buffer,buf->Ptr(),buf->Length());
				RetVal = buf->Length();
			}
		}
	}
	buf->Close();
	delete buf;
	return(RetVal);
}
int ILibSocketWrapper_close(int socketObject)
{
  // lookup the socket handle and close and delete if exists
	RSocket *s = (RSocket*)SocketArray[socketObject];
  if ( s != NULL )
  {
    s->Close();
    delete s;
    SocketArray[socketObject] = 0;
  }

  // lookup the connection handle and close and delete if exists
  RConnection *c = ConnectionArray[socketObject];
  if ( c != NULL )
  {
    c->Close();
    delete c;
    ConnectionArray[socketObject] = NULL;
  }

	return(0);
}
int ILibSocketWrapper_connect(int socketObject,struct sockaddr *dest)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];
	CSocketWrapperEvent *e = CSocketWrapperEvent::NewL();

	TInetAddr *dst = new TInetAddr(htonl(((struct in_addr*)dest->sa_data)->s_addr),ntohs(dest->sa_port));
	
	//TInetAddr *dst = new TInetAddr(htonl(inet_addr("172.16.0.37")),55860);
	e->Reserved_dest = dst;
	
	s->Connect(*dst,*(e->GetIStatus()));
	e->Activate();
	return(0);
}
int ILibSocketWrapper_SetReuseAddr(int socketObject, int enabled)
{
	RSocket *s = (RSocket*)SocketArray[socketObject];
	return(s->SetOpt(KSoReuseAddr,KSolInetIp,enabled)==KErrNone?0:-1);
}
int ILibSocketWrapper_GetInterfaceIndex(long localAddr)
{
	int RetVal = -1;
	int idx = 1;
	
	int socketObject = ILibSocketWrapper_socket(SOCK_STREAM);
	RSocket *sock = (RSocket*)SocketArray[socketObject];
	TPckgBuf<TSoInetInterfaceInfo> item;
	
	// Initialize the iterator to start getting Interface list
	TInt result = sock->SetOpt(KSoInetEnumInterfaces, KSolInetIfCtrl);

	// if the above line fails, then this will just fail too. 
	result = sock->GetOpt(KSoInetNextInterface, KSolInetIfCtrl, item);
	while (result == KErrNone)
	{
	
		TSoInetInterfaceInfo& ifInfo = item();
		if (ifInfo.iState == EIfUp && ifInfo.iAddress.Address()!=0)
		{
			if(ntohl(ifInfo.iAddress.Address())==localAddr)
			{
				RetVal = idx;
				break;
			}
		}
		++idx;
		result = sock->GetOpt(KSoInetNextInterface, KSolInetIfCtrl, item);
	}
	ILibSocketWrapper_close(socketObject);
	return(RetVal);
}
int ILibSocketWrapper_GetLocalIPAddressList(int iplist[])
{
	int socketObject = ILibSocketWrapper_socket(SOCK_STREAM);
	RSocket *sock = (RSocket*)SocketArray[socketObject];
	TPckgBuf<TSoInetInterfaceInfo> item;
	int results=0;
	
	// Initialize the iterator to start getting Interface list
	TInt result = sock->SetOpt(KSoInetEnumInterfaces, KSolInetIfCtrl);

	// if the above line fails, then this will just fail too. 
	result = sock->GetOpt(KSoInetNextInterface, KSolInetIfCtrl, item);
	while (result == KErrNone)
	{
	
		TSoInetInterfaceInfo& ifInfo = item();
		if (ifInfo.iState == EIfUp && ifInfo.iAddress.Address()!=0)
		{
			iplist[results] = ntohl(ifInfo.iAddress.Address());
			++results;
		}
		result = sock->GetOpt(KSoInetNextInterface, KSolInetIfCtrl, item);
	}
	ILibSocketWrapper_close(socketObject);
	return(results);
}
// methods to allow for the closing of the socket server when the chain is stopped
void ILibSocketWrapper_Create()
{
	if(Initialized==0)
	{
  		socketServer.Connect(16);
  		Initialized = 1;
	}
}
void ILibSocketWrapper_Destroy()
{
  // make sure all sockets and connections are closed
  for (int i = 0; i < SOCKET_ARRAY_SIZE; i++)
  {
    ILibSocketWrapper_close(i);
  }

  // release the socket server
  socketServer.Close();
}
