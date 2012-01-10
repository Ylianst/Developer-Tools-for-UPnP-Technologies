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

extern "C"
{
	#include "ILibParsers.h"
}

//{{{ControlPoint_Begin}}}
class CUPnP_ControlPoint_Device;
class CUPnP_ControlPoint_Service;
class CUPnP_Manager;
typedef void(*CUPnP_Manager_OnDeviceHandler)(CUPnP_Manager *sender, CUPnP_ControlPoint_Device *device);
//{{{ControlPoint_End}}}

class CUPnP_Manager
{
	//{{{Manager_Friends_BEGIN}}}friend class C{{{DEVICE}}};
	//{{{Manager_Friends_END}}}
	
public:
	CUPnP_Manager();
	~CUPnP_Manager();

	//{{{Manager_GetDevice_BEGIN}}}C{{{DEVICE}}} *Get_{{{DEVICE}}}();
	//{{{Manager_GetDevice_END}}}

	//{{{Manager_SetControlPoint_BEGIN}}}void Set_ControlPoint_Handler_{{{DEVICE}}}(CUPnP_Manager_OnDeviceHandler OnAdd, CUPnP_Manager_OnDeviceHandler OnRemove);
	//{{{Manager_SetControlPoint_END}}}

	void IPAddressListChanged();
	void *GetChain();
	int Start();
	int Stop();
protected:
	void *Chain;
	sem_t QuitLock;

	//{{{Manager_Device_BEGIN}}}C{{{DEVICE}}} *{{{DEVICE}}};
	//{{{Manager_Device_END}}}

	//{{{Manager_ProtectedCP_Stuff_BEGIN}}}void *ControlPoint_{{{DEVICE}}};
	CUPnP_Manager_OnDeviceHandler OnAdd_{{{DEVICE_ID}}};
	CUPnP_Manager_OnDeviceHandler OnRemove_{{{DEVICE_ID}}};
	static void DeviceDiscoverSink_{{{DEVICE_ID}}}(struct UPnPDevice *d);
	static void DeviceRemoveSink_{{{DEVICE_ID}}}(struct UPnPDevice *d);
	//{{{Manager_ProtectedCP_Stuff_END}}}
};

//{{{Service_Begin}}}
class C{{{SERVICE}}}
{
public:
	C{{{SERVICE}}}(C{{{DEVICE}}} *parent);
	virtual ~C{{{SERVICE}}}();

	//{{{Service_VirtualMethods}}}
	//{{{Service_VirtualMethods_Response}}}
	//{{{Service_Events_BEGIN}}}void SetStateValue_{{{VARNAME}}}({{{PARAMDEF}}});
	//{{{Service_Events_END}}}

	void Error(void *session, int errorCode, char *errorString);

	C{{{DEVICE}}} *ParentDevice;
};
//{{{Service_End}}}

//{{{Device_Begin}}}
class C{{{DEVICE}}}
{
	friend class CUPnP_Manager;
	//{{{ParentFriends}}}
public:
	void* GetToken();
	//{{{Device_ServiceList}}}
	//{{{Device_DeviceList}}}

protected:
	C{{{DEVICE}}}(CUPnP_Manager *pManager{{{EMBEDDED}}});
	~C{{{DEVICE}}}();
	void *MicrostackToken;
	void *MicrostackChain;

	//{{{Device_StaticSinks}}}
};
//{{{Device_End}}}


//{{{ControlPoint_Begin}}}
typedef enum CUPnP_SubscriptionResponse_Struct
{
	CUPnP_SubscriptionResponse_UNKNOWN = 0,
	CUPnP_SubscriptionResponse_OK = 1,
	CUPnP_SubscriptionResponse_SUBSCRIBE_FAILED = 2,
	CUPnP_SubscriptionResponse_RENEW_FAILED = 3
}CUPnP_SubscriptionResponse;

class CUPnP_ControlPoint_Device
{
	friend class CUPnP_Manager;
public:
	char *FriendlyName;
	char *URN;
	char *UDN;
	char *LocationURL;
	char *PresentationURL;
	char *InterfaceToHost;
	CUPnP_ControlPoint_Service **Services;
	int ServicesLength;
	void *User;
	struct UPnPDevice *m_Device;
	CUPnP_ControlPoint_Service *FindServiceByType(char *ServiceURN);
	
protected:
	CUPnP_ControlPoint_Device(struct UPnPDevice *device);
	~CUPnP_ControlPoint_Device();
};

class CUPnP_ControlPoint_Action
{
	friend class CUPnP_ControlPoint_Service;
public:
	char *Name;
	CUPnP_ControlPoint_Service *Parent;
protected:
	struct UPnPAction *m_Action;
	CUPnP_ControlPoint_Action(CUPnP_ControlPoint_Service *parent, struct UPnPAction *a);
};
class CUPnP_ControlPoint_StateVariable
{
	friend class CUPnP_ControlPoint_Service;
public:
	char *Name;
	char **AllowedValues;
	int AllowedValuesLength;

	char *Min;
	char *Max;
	char *Step;

	CUPnP_ControlPoint_Service *Parent;
	
protected:
	CUPnP_ControlPoint_StateVariable(CUPnP_ControlPoint_Service *parent, struct UPnPStateVariable *v);
	struct UPnPStateVariable *m_Variable;
};
class CUPnP_ControlPoint_Service
{
	friend class CUPnP_ControlPoint_Device;
public:
	char *URN;
	char *ID;
	struct UPnPService *m_Service;
	CUPnP_ControlPoint_Device *Parent;
	
	CUPnP_ControlPoint_Action **Actions;
	int ActionsLength;

	CUPnP_ControlPoint_StateVariable **Variables;
	int VariablesLength;
	void *User;
	
	typedef void(*CUPnP_SubscribeHandler)(CUPnP_ControlPoint_Service *sender, CUPnP_SubscriptionResponse status);
	virtual void Subscribe(CUPnP_SubscribeHandler OnSubscribe);
	virtual void UnSubscribe(CUPnP_SubscribeHandler OnSubscribe);

protected: 
	CUPnP_SubscribeHandler OnSubscribeHandler;
	static void CUPnP_ControlPoint_Service_OnSubscribe(struct UPnPService* service,int OK);
	CUPnP_ControlPoint_Service(CUPnP_ControlPoint_Device *parent, struct UPnPService *s);
	virtual ~CUPnP_ControlPoint_Service();
};

//{{{BEGIN_CP_SERVICE}}}
class CUPnP_ControlPoint_Service_{{{SERVICE}}};
#define CUPnP_ControlPoint_ServiceTypes_{{{SERVICE}}} "{{{URN}}}"

//{{{BEGIN_INVOKE_TYPEDEF}}}typedef void(*CUPnP_ControlPoint_Service_{{{SERVICE}}}_{{{ONACTION}}})(CUPnP_ControlPoint_Service_{{{SERVICE}}} *sender, int ErrorCode, void *User{{{OUTARGLIST}}});
//{{{END_INVOKE_TYPEDEF}}}
//{{{BEGIN_EVENT_TYPEDEF}}}typedef void(*CUPnP_ControlPoint_Service_{{{SERVICE}}}_{{{STATEVAR}}}_Handler)(CUPnP_ControlPoint_Service_{{{SERVICE}}} *sender, {{{ARGLIST}}});
//{{{END_EVENT_TYPEDEF}}}
class CUPnP_ControlPoint_Service_{{{SERVICE}}} : public CUPnP_ControlPoint_Service
{
	friend class CUPnP_ControlPoint_Device;

protected:
	CUPnP_ControlPoint_Service_{{{SERVICE}}}(CUPnP_ControlPoint_Device *parent, struct UPnPService *s);
	
	//{{{BEGIN_CP_INVOKESINK}}}static void InvokeSink_{{{ACTION}}}(struct UPnPService *sender,int ErrorCode,void *user{{{OUTARGLIST}}});
	//{{{END_CP_INVOKESINK}}}
	//{{{BEGIN_CP_EVENTSINK}}}static void EventSink_{{{STATEVAR}}}(struct UPnPService *sender,{{{ARGTYPE}}});
	//{{{END_CP_EVENTSINK}}}
public:
	virtual void Subscribe(CUPnP_SubscribeHandler OnSubscribe);

	//{{{BEGIN_EVENT}}}CUPnP_ControlPoint_Service_{{{SERVICE}}}_{{{STATEVAR}}}_Handler OnEvent_{{{STATEVAR}}};
	//{{{END_EVENT}}}
	//{{{BEGIN_CP_INVOKE}}}void Invoke_{{{ACTION}}}(CUPnP_ControlPoint_Service_{{{SERVICE}}}_{{{ONACTION}}} {{{ONACTION}}}, void *user{{{INARGLIST}}});
	//{{{END_CP_INVOKE}}}
};
//{{{END_CP_SERVICE}}}

//{{{ControlPoint_End}}}