/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002, 2003 Intel Corporation.  All rights reserved.
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
 * $Workfile: UPnPControlPointStructs.h
 * $Revision: #1.0.1775.28223
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, November 11, 2004
 *
 *
 *
 */
#ifndef __UPNP_CONTROLPOINT_STRUCTS__
#define __UPNP_CONTROLPOINT_STRUCTS__

#define UPNP_ERROR_SCPD_NOT_WELL_FORMED 5

struct UPnPDevice;
typedef void(*UPnPDeviceHandler)(struct UPnPDevice *device);
typedef void(*UPnPDeviceDiscoveryErrorHandler)(char *UDN, char *LocationURL, int StatusCode);

typedef enum
{
	UPnPSSDP_MSEARCH = 1,
	UPnPSSDP_NOTIFY = 2
} UPnPSSDP_MESSAGE;

struct UPnPIcon
{
	int width;
	int height;
	int colorDepth;
	char *mimeType;
	char *url;
};

struct UPnPDevice
{
	void* CP;
	char* DeviceType;
	char* UDN;
	UPnPDeviceHandler fpDestroy;
	
	char* LocationURL;
	struct sockaddr_in6 LocationAddr;
	struct sockaddr_in6 InterfaceToHostAddr;
	struct UPnPIcon *Icons;
	int IconsLength;
	char* PresentationURL;
	char* FriendlyName;
	char* ManufacturerName;
	char* ManufacturerURL;
	char* ModelName;
	char* ModelDescription;
	char* ModelNumber;
	char* ModelURL;
	
	int MaxVersion;
	int SCPDError;
	int SCPDLeft;
	int ReferenceCount;
	int ReferenceTiedToEvents;
	char* InterfaceToHost;
	int CacheTime;
	void *Tag;

	int Reserved;
	long Reserved2;
	char *Reserved3;
	int ReservedID;
	void *CustomTagTable;
	
	struct UPnPDevice *Parent;
	struct UPnPDevice *EmbeddedDevices;
	struct UPnPService *Services;
	struct UPnPDevice *Next;
};

struct UPnPService
{
	char* ServiceType;
	char* ServiceId;
	char* ControlURL;
	char* SubscriptionURL;
	char* SCPDURL;
	char* SubscriptionID;
	int MaxVersion;
	
	struct UPnPAction *Actions;
	struct UPnPStateVariable *Variables;
	struct UPnPDevice *Parent;
	struct UPnPService *Next;
};

struct UPnPStateVariable
{
	struct UPnPStateVariable *Next;
	struct UPnPService *Parent;
	
	char* Name;
	char **AllowedValues;
	int NumAllowedValues;
	char* Min;
	char* Max;
	char* Step;
};

struct UPnPAction
{
	char* Name;
	struct UPnPAction *Next;
};

struct UPnPAllowedValue
{
	struct UPnPAllowedValue *Next;
	char* Value;
};
#endif
