/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002-2005 Intel Corporation.  All rights reserved.
 * 
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel
 * Corporation or its suppliers or licensors.  Title to the
 * Material remains with Intel Corporation or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and
 * licensors. The Material is protected by worldwide copyright and
 * trade secret laws and treaty provisions.  No part of the Material
 * may be used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed, or disclosed in any way without
 * Intel's prior express written permission.
 
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by Intel in writing.
 * 
 * $Workfile: <FILE>
 * $Revision: #1.0.1607.17215
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Monday, August 1, 2005
 *
 */


#include "UPnPAbstraction.h"

extern "C"
{
#if defined(WINSOCK1)
#include<winsock.h>
#elif defined(WINSOCK2)
#include<winsock2.h>
#endif
	//{{{MicroStackInclude_Begin}}}	#include "{{{PREFIX}}}MicroStack.h"
	//{{{MicroStackInclude_End}}}
	//{{{CPMicroStackInclude_Begin}}}	#include "{{{PREFIX}}}ControlPoint.h"
	//{{{CPMicroStackInclude_End}}}
	#include "ILibParsers.h"
	#include "ILibWebServer.h"
}

//{{{CP_BEGIN}}}
struct CUPnP_InvokeData
{
	void *sender;
	void *callback;
	void *user;
};
//{{{CP_END}}}

CUPnP_Manager::CUPnP_Manager()
{
	this->Chain = ILibCreateChain();
	sem_init(&(this->QuitLock),0,0);
	//{{{Manager_Constructor_Begin}}}this->{{{DEVICE}}} = new C{{{DEVICE}}}(this);
	//{{{Manager_Constructor_End}}}

	//{{{Manager_CPConstructor_Begin}}}this->ControlPoint_{{{DEVICE}}} = {{{PREFIX}}}CreateControlPoint(Chain,&CUPnP_Manager::DeviceDiscoverSink_{{{DEVICEID}}},&CUPnP_Manager::DeviceRemoveSink_{{{DEVICEID}}});
	{{{PREFIX}}}SetUser(ControlPoint_{{{DEVICE}}},this);
	//{{{Manager_CPConstructor_End}}}

}
int CUPnP_Manager::Start()
{
	ILibStartChain(Chain);
	Chain = NULL;
	sem_post(&(this->QuitLock));
	return(0);
}
int CUPnP_Manager::Stop()
{
	ILibStopChain(Chain);
	return(0);
}
void* CUPnP_Manager::GetChain()
{
	return(Chain);
}
//{{{BEGIN_CPDiscoverSink}}}
void CUPnP_Manager::DeviceDiscoverSink_{{{DEVICEID}}}(struct UPnPDevice *d)
{
	CUPnP_ControlPoint_Device *dv = new CUPnP_ControlPoint_Device(d);
	CUPnP_Manager *m = (CUPnP_Manager*){{{PREFIX}}}GetUser(d->CP);

	d->Tag = dv;
	if(m->OnAdd_{{{DEVICEID}}}!=NULL)
	{
		m->OnAdd_{{{DEVICEID}}}(m,dv);
	}
}
void CUPnP_Manager::DeviceRemoveSink_{{{DEVICEID}}}(struct UPnPDevice *d)
{
	CUPnP_Manager *m = (CUPnP_Manager*){{{PREFIX}}}GetUser(d->CP);
	CUPnP_ControlPoint_Device *dv = (CUPnP_ControlPoint_Device*)d->Tag;

	if(m->OnRemove_{{{DEVICEID}}}!=NULL)
	{
		m->OnRemove_{{{DEVICEID}}}(m,dv);
	}
	delete dv;
}
//{{{END_CPDiscoverSink}}}

//{{{Manager_GetDevice_Begin}}}
C{{{DEVICE}}} *CUPnP_Manager::Get_{{{DEVICE}}}()
{
	return({{{DEVICE}}});
}
//{{{Manager_GetDevice_End}}}

CUPnP_Manager::~CUPnP_Manager()
{
	if(Chain!=NULL && !ILibIsChainBeingDestroyed(Chain))
	{
		ILibStopChain(Chain);
		sem_wait(&(this->QuitLock));
	}
	sem_destroy(&(this->QuitLock));
	//{{{Manager_Destructor_Begin}}}delete {{{DEVICE}}};
	//{{{Manager_Destructor_End}}}
}
void CUPnP_Manager::IPAddressListChanged()
{
	//{{{IPADDRESS_HANDLER_BEGIN}}}{{{PREFIX}}}IPAddressListChanged({{{DEVICE}}}->GetToken());
	//{{{IPADDRESS_HANDLER_END}}}
}

//{{{Device_Begin}}}
C{{{DEVICE}}}::C{{{DEVICE}}}(CUPnP_Manager *pManager{{{EMBEDDED}}})
{
	this->MicrostackChain = pManager->Chain;
	//{{{SetToken_Begin}}}this->MicrostackToken = parentDevice->GetToken();
	//{{{SetToken_End}}}
	//{{{CreateMicroStack}}}
	//{{{Service_Instantiation_Begin}}}this->m_{{{SERVICE}}} = new CUPnP_Service_{{{SERVICE}}}(this);
	//{{{Service_Instantiation_End}}}
	//{{{Device_Instantiation_Begin}}}this->m_{{{DEVICE}}} = new CUPnP_{{{DEVICE}}}(pManager,this);
	//{{{Device_Instantiation_End}}}

	//{{{Device_Root_Begin}}}ILibWebServer_SetTag({{{Prefix}}}GetWebServerToken(MicrostackToken),(void*)this);
	//{{{Device_Root_End}}}

	//{{{SinkList_Begin}}}{{{Prefix}}}FP_{{{SERVICE_SHORT_NAME}}}_{{{ACTION_NAME}}} = ({{{Prefix}}}_ActionHandler_{{{SERVICE_SHORT_NAME}}}_{{{ACTION_NAME}}})&(C{{{DEVICE}}}::{{{SERVICE_SHORT_NAME}}}_{{{ACTION_NAME}}}_Sink);
	//{{{SinkList_End}}}
}
C{{{DEVICE}}}::~C{{{DEVICE}}}()
{
	//{{{Destructor_Begin}}}
	delete m_{{{NAME}}};
	m_{{{NAME}}} = NULL;
	//{{{Destructor_End}}}
}
void *C{{{DEVICE}}}::GetToken()
{
	return(MicrostackToken);
}
//{{{Dispatch_Begin}}}
void C{{{DEVICE}}}::{{{SERVICE_SHORT_NAME}}}_{{{ACTION_NAME}}}_Sink({{{PARAM_LIST}}})
{
	C{{{ROOTDEVICE}}} *d = (C{{{ROOTDEVICE}}}*)ILibWebServer_GetTag(((struct ILibWebServer_Session*)session)->Parent);
	d->{{{DEVICELIST}}}m_{{{SERVICE_SHORT_NAME}}}->{{{ACTION_NAME}}}({{{PARAM_LIST_DISPATCH}}});
}
//{{{Dispatch_End}}}
//{{{Device_End}}}
//{{{Service_Begin}}}
CUPnP_Service_{{{SERVICE_NAME}}}::CUPnP_Service_{{{SERVICE_NAME}}}(C{{{DEVICE}}} *parent)
{
	ParentDevice = parent;
}
CUPnP_Service_{{{SERVICE_NAME}}}::~CUPnP_Service_{{{SERVICE_NAME}}}()
{
}
void CUPnP_Service_{{{SERVICE_NAME}}}::Error(void *session, int errorCode, char *errorString)
{
	{{{PREFIX}}}Response_Error(session,errorCode,errorString);
}
//{{{SERVICE_VIRTUAL_METHOD_BASE_IMPLEMENTATION_BEGIN}}}
void CUPnP_Service_{{{SERVICE_NAME}}}::{{{ACTION_NAME}}}({{{PARAM_LIST}}})
{
	Error(session,501,"No Handler");
}
void CUPnP_Service_{{{SERVICE_NAME}}}::Response_{{{ACTION_NAME}}}({{{OUTPUT_PARAM_LIST}}})
{
	{{{PREFIX}}}Response_{{{SERVICE_NAME}}}_{{{ACTION_NAME}}}({{{OUTPUT_PARAM_LIST_DISPATCH}}});
}
//(((FragmentedResponse_Begin}}}
void CUPnP_Service_{{{SERVICE_NAME}}}::Response_Async_{{{ACTION_NAME}}}(void *session, char *ArgName, char *ArgValue, int ArgValueLength, int ArgStart, int ArgDone, int ResponseDone)
{
	struct ILibWebServer_Session *s = (struct ILibWebServer_Session*)session;

	if(s->User2==NULL)
	{
		{{{PREFIX}}}AsyncResponse_START(session, "{{{ACTION_NAME}}}", "{{{URN}}}");
		s->User2 = (void*)~0;
	}
	{{{PREFIX}}}AsyncResponse_OUT(session, ArgName, ArgValue, ArgValueLength, ILibAsyncSocket_MemoryOwnership_USER,ArgStart, ArgDone);
	if(ResponseDone)
	{
		{{{PREFIX}}}AsyncResponse_DONE(session, "{{{ACTION_NAME}}}");
		s->User2 = NULL;
	}
}
//(((FragmentedResponse_End}}}
//{{{SERVICE_VIRTUAL_METHOD_BASE_IMPLEMENTATION_END}}}
//{{{SERVICE_EVENTS_BEGIN}}}
void CUPnP_Service_{{{SERVICE_NAME}}}::SetStateValue_{{{VARNAME}}}({{{PARAMDEF}}})
{
	{{{PREFIX}}}SetState_{{{SERVICE_NAME}}}_{{{VARNAME}}}({{{PARAMLIST}}});
}
//{{{SERVICE_EVENTS_END}}}
//{{{Service_End}}}



//{{{CP_BEGIN}}}
//{{{BEGIN_CPEVENT_SINK}}}void CUPnP_ControlPoint_Service_{{{SERVICE_NAME}}}::EventSink_{{{STATEVAR}}}(struct UPnPService *sender,{{{ARGTYPE}}})
{
	CUPnP_ControlPoint_Service_{{{SERVICE_NAME}}} *service = (CUPnP_ControlPoint_Service_{{{SERVICE_NAME}}}*)sender->Tag;
	if(service->OnEvent_{{{STATEVAR}}}!=NULL)
	{
		service->OnEvent_{{{STATEVAR}}}(service,{{{ARGLIST}}});
	}
}
//{{{END_CPEVENT_SINK}}}
//{{{BEGIN_CPEVENT_SUBSCRIBE}}}void CUPnP_ControlPoint_Service_{{{SERVICE_NAME}}}::Subscribe(CUPnP_ControlPoint_Service::CUPnP_SubscribeHandler OnSubscribe)
{
	m_Service->Tag = this;
	//{{{REGISTER}}}
	CUPnP_ControlPoint_Service::Subscribe(OnSubscribe);
}
//{{{END_CPEVENT_SUBSCRIBE}}}
void CUPnP_ControlPoint_Service::CUPnP_ControlPoint_Service_OnSubscribe(struct UPnPService* service,int OK)
{
	CUPnP_ControlPoint_Service *s = (CUPnP_ControlPoint_Service*)service->Tag;
	if(s->OnSubscribeHandler!=NULL)
	{
		if(OK==TRUE)
		{
			s->OnSubscribeHandler(s,CUPnP_SubscriptionResponse_OK);
		}
		else
		{
			s->OnSubscribeHandler(s,CUPnP_SubscriptionResponse_SUBSCRIBE_FAILED);
		}
	}
}
void CUPnP_ControlPoint_Service::Subscribe(CUPnP_ControlPoint_Service::CUPnP_SubscribeHandler OnSubscribe)
{
	m_Service->Tag = this;
	this->OnSubscribeHandler = OnSubscribe;
	UPnPSubscribeForUPnPEvents(this->m_Service,&CUPnP_ControlPoint_Service::CUPnP_ControlPoint_Service_OnSubscribe);
}
void CUPnP_ControlPoint_Service::UnSubscribe(CUPnP_ControlPoint_Service::CUPnP_SubscribeHandler OnSubscribe)
{
	UPnPUnSubscribeUPnPEvents(this->m_Service);
}
CUPnP_ControlPoint_Action::CUPnP_ControlPoint_Action(CUPnP_ControlPoint_Service *parent, struct UPnPAction *a)
{
	this->Name = a->Name;
	this->Parent = parent;
	this->m_Action = a;
}
CUPnP_ControlPoint_StateVariable::CUPnP_ControlPoint_StateVariable(CUPnP_ControlPoint_Service *parent, UPnPStateVariable *v)
{
	this->m_Variable = v;
	this->Name = v->Name;
	this->AllowedValuesLength = v->NumAllowedValues;
	this->AllowedValues = v->AllowedValues;
	this->Min = v->Min;
	this->Max = v->Max;
	this->Step = v->Step;
	this->Parent = parent;
}
CUPnP_ControlPoint_Service::~CUPnP_ControlPoint_Service()
{
	int i;
	for(i=0;i<this->VariablesLength;++i)
	{
		delete this->Variables[i];
	}
	if(this->VariablesLength!=0)
	{
		free(this->Variables);
	}

	for(i=0;i<this->ActionsLength;++i)
	{
		delete this->Actions[i];
	}
	if(this->ActionsLength!=0)
	{
		free(this->Actions);
	}
}
CUPnP_ControlPoint_Service::CUPnP_ControlPoint_Service(CUPnP_ControlPoint_Device *parent, struct UPnPService*s)
{
	int i;
	struct UPnPAction *a;
	struct UPnPStateVariable *v;

	this->Parent = parent;
	this->URN = s->ServiceType;
	this->ID = s->ServiceId;
	this->m_Service = s;

	i=0;
	a = s->Actions;
	while(a!=NULL)
	{
		++i;
		a = a->Next;
	}
	if(i!=0)
	{
		this->ActionsLength = i;
		this->Actions = (CUPnP_ControlPoint_Action**)malloc(i*sizeof(CUPnP_ControlPoint_Action*));
		i=0;
		a = s->Actions;
		do
		{
			Actions[i] = new CUPnP_ControlPoint_Action(this,a);
			++i;
			a = a->Next;
		}while(a!=NULL);
	}

	i=0;
	v = s->Variables;
	while(v!=NULL)
	{
		++i;
		v = v->Next;
	}
	if(i!=0)
	{
		this->VariablesLength = i;
		this->Variables = (CUPnP_ControlPoint_StateVariable**)malloc(i*sizeof(CUPnP_ControlPoint_StateVariable*));
		i=0;
		v = s->Variables;
		do
		{
			this->Variables[i] = new CUPnP_ControlPoint_StateVariable(this,v);
			++i;
			v = v->Next;
		}while(v!=NULL);
	}
}
CUPnP_ControlPoint_Device::~CUPnP_ControlPoint_Device()
{
	int i;
	for(i=0;i<this->ServicesLength;++i)
	{
		delete this->Services[i];
	}
	if(this->ServicesLength!=0)
	{
		free(this->Services);
	}
}
CUPnP_ControlPoint_Service * CUPnP_ControlPoint_Device::FindServiceByType(char *ServiceURN)
{
	int i;
	int ServiceURNLength = (int)strlen(ServiceURN);
	int ServiceURNColon = ILibString_LastIndexOf(ServiceURN,ServiceURNLength,":",1);
	int ServiceURNVersion = atoi(ServiceURN+ServiceURNColon+1);

	int i2,ilen;
	for(i=0;i<this->ServicesLength;++i)
	{
		ilen = (int)strlen(Services[i]->URN);
		i2 = ILibString_LastIndexOf(Services[i]->URN,ilen,":",1);

		if(i2==ServiceURNColon && memcmp(Services[i]->URN,ServiceURN,i2)==0 && atoi(Services[i]->URN+i2+1)>=ServiceURNVersion)
		{
			return(Services[i]);
		}
	}
	return(NULL);
}
CUPnP_ControlPoint_Device::CUPnP_ControlPoint_Device(struct UPnPDevice *device)
{
	int i;
	struct UPnPService *s;
	int ok = 0;

	this->m_Device = device;
	this->FriendlyName = device->FriendlyName;
	this->InterfaceToHost = device->InterfaceToHost;
	this->LocationURL = device->LocationURL;
	this->PresentationURL = device->PresentationURL;
	this->UDN = device->UDN;
	this->URN = device->DeviceType;

	i=0;
	s = device->Services;
	while(s!=NULL)
	{
		++i;
		s = s->Next;
	}

	if(i!=0)
	{
		this->ServicesLength = i;
		this->Services = (CUPnP_ControlPoint_Service**)malloc(sizeof(CUPnP_ControlPoint_Service*)*i);
		i=0;
		s = device->Services;
		do
		{	
			//{{{BEGIN_ServiceCheck}}}{{{COMPARESTRING}}}
			{
				Services[i] = new CUPnP_ControlPoint_Service_{{{SERVICE}}}(this,s);
				ok=1;
			}
			//{{{END_ServiceCheck}}}
			if(ok==0)
			{
				Services[i] = new CUPnP_ControlPoint_Service(this,s);
			}
			++i;
			s = s->Next;
		}while(s!=NULL);
	}
}
//{{{BEGIN_SetControlPoint}}}void CUPnP_Manager::Set_ControlPoint_Handler_{{{DEVICE}}}(CUPnP_Manager_OnDeviceHandler OnAdd, CUPnP_Manager_OnDeviceHandler OnRemove)
{
	this->OnAdd_{{{DEVICEID}}} = OnAdd;
	this->OnRemove_{{{DEVICEID}}} = OnRemove;
}
//{{{END_SetControlPoint}}}
//{{{BEGIN_CP_Constructor}}}CUPnP_ControlPoint_Service_{{{SERVICE}}}::CUPnP_ControlPoint_Service_{{{SERVICE}}}(CUPnP_ControlPoint_Device *parent, struct UPnPService *s) : CUPnP_ControlPoint_Service(parent,s)
{
	//{{{BEGIN_EVENT}}}OnEvent_{{{STATEVAR}}} = NULL;
	//{{{END_EVENT}}}
}
//{{{END_CP_Constructor}}}

//{{{BEGIN_CP_Invoke}}}void CUPnP_ControlPoint_Service_{{{SERVICE}}}::Invoke_{{{ACTION}}}(CUPnP_ControlPoint_Service_{{{SERVICE}}}_{{{ONACTION}}} OnResult, void *user{{{INARGS}}})
{
	struct CUPnP_InvokeData *d = (struct CUPnP_InvokeData*)malloc(sizeof(struct CUPnP_InvokeData));
	
	d->callback = (void*)OnResult;
	d->sender = this;
	d->user = user;

	{{{PREFIX}}}Invoke_{{{SERVICE}}}_{{{ACTION}}}(m_Service,&InvokeSink_{{{ACTION}}},d{{{INARGS_Values}}});
}
void CUPnP_ControlPoint_Service_{{{SERVICE}}}::InvokeSink_{{{ACTION}}}(struct UPnPService *sender,int ErrorCode,void *user{{{OUTARGS}}})
{
	struct CUPnP_InvokeData *d = (struct CUPnP_InvokeData*)user;

	if(d->callback!=NULL)
	{
		((CUPnP_ControlPoint_Service_{{{SERVICE}}}_{{{ONACTION}}})d->callback)((CUPnP_ControlPoint_Service_{{{SERVICE}}}*)d->sender,ErrorCode, d->user{{{OUTARGS_Values}}});
	}
	free(d);
}
//{{{END_CP_Invoke}}}
//{{{CP_END}}}