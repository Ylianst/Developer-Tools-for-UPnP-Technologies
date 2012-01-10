/*   
Copyright 2006 - 2011 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "STBShell.h"
#include "ILibParsers.h"
#include "UPnPControlPointStructs.h"
//#include "WinSemaphore.h"

#include <stdio.h>
#include "Utility.h"

static void *AVR_KnownDevicesTable;
static void *MS_KnownDevicesTable;

static void *MS_ControlPoint;
static void *AVR_ControlPoint;

static char currMServerUDN[1024];
static char currMRendererUDN[1024];
static char currContainerObjID[1024];
static short currMRendererVolume;

static HANDLE callbackResponseSemaphore;

/********************************************************************
*********************************************************************

Callbacks to be used with the AVR and MS control point invoke 
functions.  

*********************************************************************
********************************************************************/

static void AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_RenderingControl_SetVolume(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: RenderingControl/SetVolume[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_RenderingControl_GetMute(struct UPnPService* Service,int ErrorCode,void *User,int CurrentMute)
{
	printf("AVRCP_ Invoke Response: RenderingControl/GetMute[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentMute);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_RenderingControl_SetMute(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: RenderingControl/SetMute[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_RenderingControl_GetVolume(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentVolume)
{
	printf("AVRCP_ Invoke Response: RenderingControl/GetVolume[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentVolume);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_GetCurrentTransportActions(struct UPnPService* Service,int ErrorCode,void *User,char* Actions)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetCurrentTransportActions[ErrorCode:%d](%s)\r\n",ErrorCode,Actions);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_Play(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Play[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_Previous(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Previous[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_Next(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Next[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_Stop(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Stop[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_GetTransportSettings(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMode,char* RecQualityMode)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetTransportSettings[ErrorCode:%d](%s,%s)\r\n",ErrorCode,PlayMode,RecQualityMode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_Seek(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Seek[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_Pause(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Pause[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_GetPositionInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Track,char* TrackDuration,char* TrackMetaData,char* TrackURI,char* RelTime,char* AbsTime,int RelCount,int AbsCount)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetPositionInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%d,%d)\r\n",ErrorCode,Track,TrackDuration,TrackMetaData,TrackURI,RelTime,AbsTime,RelCount,AbsCount);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_GetTransportInfo(struct UPnPService* Service,int ErrorCode,void *User,char* CurrentTransportState,char* CurrentTransportStatus,char* CurrentSpeed)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetTransportInfo[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,CurrentTransportState,CurrentTransportStatus,CurrentSpeed);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_SetAVTransportURI(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/SetAVTransportURI[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_GetDeviceCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMedia,char* RecMedia,char* RecQualityModes)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetDeviceCapabilities[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,PlayMedia,RecMedia,RecQualityModes);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_SetPlayMode(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/SetPlayMode[ErrorCode:%d]()\r\n",ErrorCode);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_ResponseSink_AVTransport_GetMediaInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int NrTracks,char* MediaDuration,char* CurrentURI,char* CurrentURIMetaData,char* NextURI,char* NextURIMetaData,char* PlayMedium,char* RecordMedium,char* WriteStatus)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetMediaInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%s,%s,%s)\r\n",ErrorCode,NrTracks,MediaDuration,CurrentURI,CurrentURIMetaData,NextURI,NextURIMetaData,PlayMedium,RecordMedium,WriteStatus);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("AVRCP_ Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("AVRCP_ Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("AVRCP_ Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_EventSink_RenderingControl_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("AVRCP_ Event from %s/RenderingControl/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void AVRCP_EventSink_AVTransport_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("AVRCP_ Event from %s/AVTransport/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_ResponseSink_ContentDirectory_Browse(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	printf("MSCP_ Invoke Response: ContentDirectory/Browse[ErrorCode:%d](%s,%u,%u,%u)\r\n",ErrorCode,Result,NumberReturned,TotalMatches,UpdateID);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_ResponseSink_ContentDirectory_GetSortCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SortCaps)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSortCapabilities[ErrorCode:%d](%s)\r\n",ErrorCode,SortCaps);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_ResponseSink_ContentDirectory_GetSystemUpdateID(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Id)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSystemUpdateID[ErrorCode:%d](%u)\r\n",ErrorCode,Id);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_ResponseSink_ContentDirectory_GetSearchCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SearchCaps)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSearchCapabilities[ErrorCode:%d](%s)\r\n",ErrorCode,SearchCaps);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("MSCP_ Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("MSCP_ Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("MSCP_ Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}

static void MSCP_EventSink_ContentDirectory_SystemUpdateID(struct UPnPService* Service,unsigned int SystemUpdateID)
{
	printf("MSCP_ Event from %s/ContentDirectory/SystemUpdateID: %u\r\n",Service->Parent->FriendlyName,SystemUpdateID);
	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}





static char *make_strcpy(char *string)
{
	char * ret;
	if(string == NULL)
	{
		ret = MALLOC(1);
		ret[0] = '\0';
	}
	else
	{
		ret = MALLOC(strlen(string)+1);
		strcpy(ret, string);
	}
	return ret;
}


// returns 1 if error
static int waitForResponse()
{
	if(WAIT_TIMEOUT == WaitForSingleObject(callbackResponseSemaphore, 30000))
	{
		printf("No response\n");
		return 1;
	}
	return 0;
}

static void MSCP_DeviceAdded(struct UPnPDevice *newDevice)
{
	printf("Server Device Added: %s\r\n", newDevice->FriendlyName);
	ILibHashTree_Lock(MS_KnownDevicesTable);

	ILibAddEntry(
		MS_KnownDevicesTable, 
		newDevice->UDN, 
		(int)strlen(newDevice->UDN), 
		make_strcpy(newDevice->FriendlyName));

	ILibHashTree_UnLock(MS_KnownDevicesTable);
}





static void MSCP_DeviceRemoved(struct UPnPDevice *goneDevice)
{
	printf("Server Device Removed: %s\r\n", goneDevice->FriendlyName);
	ILibHashTree_Lock(MS_KnownDevicesTable);

	ILibDeleteEntry(
		MS_KnownDevicesTable,
		goneDevice->UDN,
		(int)strlen(goneDevice->UDN));

	ILibHashTree_UnLock(MS_KnownDevicesTable);
}

static void AVRCP_DeviceAdded(struct UPnPDevice *newDevice)
{
	printf("AVRCP Device Added: %s\r\n", newDevice->FriendlyName);
	ILibHashTree_Lock(AVR_KnownDevicesTable);
	
	ILibAddEntry(
		AVR_KnownDevicesTable, 
		newDevice->UDN, 
		(int)strlen(newDevice->UDN), 
		make_strcpy(newDevice->FriendlyName));
	
	ILibHashTree_UnLock(AVR_KnownDevicesTable);
}

static void AVRCP_DeviceRemoved(struct UPnPDevice *goneDevice)
{
	printf("Server Device Removed: %s\r\n", goneDevice->FriendlyName);
	ILibHashTree_Lock(AVR_KnownDevicesTable);
	
	ILibDeleteEntry(
		AVR_KnownDevicesTable,
		goneDevice->UDN,
		(int)strlen(goneDevice->UDN));

	ILibHashTree_UnLock(AVR_KnownDevicesTable);
}




static void MSCP_BrowseResponded(void *serviceObj, 
								 struct MSCP_BrowseArgs *args, 
								 int errorCode, 
								 struct MSCP_ResultsList *results)
{
	struct MSCP_MediaObject *currObject;
	struct UPnPService *service = (struct UPnPService*)serviceObj;
	printf("A browse request came back...\n");
	printf("Error code: %d\n", errorCode);
	if(0 == errorCode)
	{
		printf("There were %d results\n", results->TotalMatches);
		currObject = results->FirstObject;

		while(currObject != NULL)
		{
			printf("Object: %s\n", currObject->Title);
			currObject = currObject->Next;
		}
	}

	ReleaseSemaphore(callbackResponseSemaphore, 1, NULL);
}



/***************************************************************************
****************************************************************************

End of callbacks

****************************************************************************
***************************************************************************/





static void str_chomp(char *s)
{
	while(*s != '\n' && *s != '\r' && *s != '\0') s++;
	*s = '\0';
}

















/* Make sure you release the renderer!!! */
static struct UPnPDevice *getCurrentMRenderer()
{
	struct UPnPDevice *ret;
	if('\0' == currMRendererUDN[0])
	{
		printf("No renderer selected, select one with setavr\n");
		ret = NULL;
	}
	else
	{
		ret = AVRCP_GetDeviceAtUDN(AVR_ControlPoint, currMRendererUDN);
		if(NULL == ret)
		{
			printf("The previously selected renderer has left the network\n");
			printf("Please select a new one with setavr\n");
			currMRendererUDN[0] = '\0';
		}
	}
	return ret;
}

/* Make sure you release the renderer!!! */
static struct UPnPDevice *getCurrentMServer()
{
	struct UPnPDevice *ret;
	if('\0' == currMServerUDN[0])
	{
		printf("No server selected, select one with setms\n");
		ret = NULL;
	}
	else
	{
		ret = MSCP_GetDeviceAtUDN(MS_ControlPoint, currMServerUDN);
		if(NULL == ret)
		{
			printf("The previously selected server has left the network\n");
			printf("Please select a new one with setms\n");
			currMServerUDN[0] = '\0';
			strcpy(currContainerObjID, "0");
		}
	}
	return ret;
}

/* Presents a list of devices to the user, allows the user to choose
 * one, and then returns the UDN of the device selected.  
 * Parameters:
 *		theILibHashTable: The hashtable containing the set of known nodes
 *                        from which the user should choose one.
 *      UDN: A buffer where the UDN of the chosen device can be stored.        
 */
static void chooseDeviceUDNFromTable(void *theILibHashTable, char *UDN)
{
	char *key;
	int keyLength;
	void *data;
	char *charData;
	void *enumerator;
	int i, numDevices, chosenIndex, attemptsLeft;
	struct UDN_To_Index * listOfUDNs = NULL;
	char buffer[1024];

	ILibHashTree_Lock(theILibHashTable);
	enumerator = ILibHashTree_GetEnumerator(theILibHashTable);

	printf("Select one of the following:\n");
	numDevices = 0;
	do
	{
		key = NULL;
		keyLength = 0;
		data = NULL;
		charData = NULL;

		ILibHashTree_GetValue(enumerator, &key, &keyLength, &data);
		charData = (char*)data;

		if(0 != keyLength)
		{
			char * tmp = MALLOC(keyLength + 1);
			memcpy(tmp, key, keyLength);
			tmp[keyLength] = '\0';
			printf("%d)\t%s (%s)\n", ++numDevices, charData, tmp);
			FREE(tmp);
		}
	}
	while(!ILibHashTree_MoveNext(enumerator));
	ILibHashTree_DestroyEnumerator(enumerator);

	if(numDevices == 0)
	{
		printf("None on network\n");
	}
	else
	{
		attemptsLeft = 3;
		while(attemptsLeft > 0)
		{
			fgets(buffer, 1024, stdin);
			str_chomp(buffer);
			if(1 != sscanf(buffer, "%d", &chosenIndex))
			{
				printf("Please enter a number\n");
			}
			else if(chosenIndex < 0 || chosenIndex > numDevices)
			{
				printf("Please choose one of the above, or 0 for none\n");
				attemptsLeft--;
			}
			else
			{	
				attemptsLeft = 0;
			}
		}
		if(0 == chosenIndex)
		{
			UDN[0] = '\0';
		}
		else
		{
			enumerator = ILibHashTree_GetEnumerator(theILibHashTable);
			for(i=0; i<chosenIndex; i++) ILibHashTree_MoveNext(enumerator);
			key = NULL;
			keyLength = 0;
			data = NULL;
			charData = NULL;

			ILibHashTree_GetValue(enumerator, &key, &keyLength, &data);

			memcpy(UDN, key, keyLength);
			UDN[keyLength] = '\0';
			ILibHashTree_DestroyEnumerator(enumerator);
		}


		ILibHashTree_UnLock(theILibHashTable);
	}
}


static void handleCmd_getms()
{
	struct UPnPDevice *dev = getCurrentMServer();
	if(NULL != dev)
	{
		printf("Current media server: %s\n", dev->FriendlyName);
		MSCP_Release(dev);
	}
}

static void handleCmd_getavr()
{
	struct UPnPDevice *dev = getCurrentMRenderer();
	if(dev != NULL)
	{
		printf("Current media renderer: %s\n", dev->FriendlyName);
		AVRCP_Release(dev);
	}
}

static void handleCmd_setms()
{
	chooseDeviceUDNFromTable(MS_KnownDevicesTable, currMServerUDN);
	if(currMServerUDN[0] == '\0')
	{
		strcpy(currContainerObjID, "0");
	}
}

static void handleCmd_setavr()
{
	chooseDeviceUDNFromTable(AVR_KnownDevicesTable, currMRendererUDN);
}

static void handleCmd_ls()
{
	struct UPnPService *service;
	struct MSCP_BrowseArgs browseArgs;
	struct UPnPDevice *dev = getCurrentMServer();

	if(NULL != dev)
	{
		browseArgs.BrowseFlag = MSCP_BrowseFlag_Children;
		browseArgs.Filter = "";
		browseArgs.ObjectID = currContainerObjID;
		browseArgs.RequestedCount = 256;
		browseArgs.SortCriteria = "";
		browseArgs.StartingIndex = 0;
		browseArgs.UserObject = NULL;

		service = MSCP_GetService_ContentDirectory(dev);
	
		MSCP_Invoke_Browse(service, &browseArgs);
		MSCP_Release(dev);
		waitForResponse();;
	}
}

static void handleCmd_cd()
{
	printf("Time to cd\n");
}

static void handleCmd_selCont()
{
	struct UPnPDevice *rendererDev = getCurrentMRenderer();
	struct UPnPDevice *serverDev = getCurrentMServer();

	if('\0' == currMRendererUDN[0])
	{
		printf("No currently selected renderer, select one with setavr\n");
		return;
	}

	printf("Time to selCont\n");



	if(NULL != rendererDev) AVRCP_Release(rendererDev);
	if(NULL != serverDev) MSCP_Release(serverDev);
}

static void handleCmd_play()
{
	struct UPnPService *service;
	struct UPnPDevice *dev = getCurrentMRenderer();
	
	if(NULL != dev)
	{
		service = AVRCP_GetService_AVTransport(dev);
		AVRCP_Invoke_AVTransport_Play(service, AVRCP_ResponseSink_AVTransport_Play, NULL, 0, "1");
		waitForResponse();;
		AVRCP_Release(dev);
	}
}

static void handleCmd_pause()
{
	struct UPnPService *service;
	struct UPnPDevice *dev = getCurrentMRenderer();
	
	if(NULL != dev)
	{
		service = AVRCP_GetService_AVTransport(dev);
		AVRCP_Invoke_AVTransport_Pause(service, AVRCP_ResponseSink_AVTransport_Pause, NULL, 0);
		waitForResponse();;
		AVRCP_Release(dev);
	}
}

static void handleCmd_stop()
{
	struct UPnPService *service;
	struct UPnPDevice *dev = getCurrentMRenderer();
	
	if(NULL != dev)
	{
		service = AVRCP_GetService_AVTransport(dev);
		AVRCP_Invoke_AVTransport_Stop(service, AVRCP_ResponseSink_AVTransport_Play, NULL, 0);
		waitForResponse();;
		AVRCP_Release(dev);
	}
}

static void handleCmd_volup()
{
	struct UPnPService *service;
	struct UPnPDevice *dev = getCurrentMRenderer();
	
	if(NULL != dev)
	{
		currMRendererVolume += 10;
		if(currMRendererVolume > 100) currMRendererVolume = 100;
		service = AVRCP_GetService_RenderingControl(dev);
		AVRCP_Invoke_RenderingControl_SetVolume(service, AVRCP_ResponseSink_RenderingControl_SetVolume, NULL, 0, "Master", currMRendererVolume);
		waitForResponse();;
		AVRCP_Release(dev);
	}
}

static void handleCmd_voldown()
{
	struct UPnPService *service;
	struct UPnPDevice *dev = getCurrentMRenderer();
	
	if(NULL != dev)
	{
		currMRendererVolume -= 10;
		if(currMRendererVolume < 0) currMRendererVolume = 0;
		service = AVRCP_GetService_RenderingControl(dev);
		AVRCP_Invoke_RenderingControl_SetVolume(service, AVRCP_ResponseSink_RenderingControl_SetVolume, NULL, 0, "Master", currMRendererVolume);
		waitForResponse();;
		AVRCP_Release(dev);
	}
}

static void handleCmd_help()
{
	printf("TODO: Write the help\n");
}



void STBS_Run(void)
{
	char currLine[2048];
	int pollAgain = 1;

	currLine[0] = '\0';
	while(pollAgain)
	{

		printf("command> ");
		fflush(stdout);
		fgets(currLine, 2048, stdin);
		str_chomp(currLine);

		printf("You asked me to: %s\n", currLine);

		if(0 == strcmp(currLine, "quit"))
		{
			pollAgain = 0;
		}
		else if(0 == strcmp(currLine, "setms"))
		{
			handleCmd_setms();
		}
		else if(0 == strcmp(currLine, "setavr"))
		{
			handleCmd_setavr();
		}
		else if(0 == strcmp(currLine, "getms"))
		{
			handleCmd_getms();
		}
		else if(0 == strcmp(currLine, "getavr"))
		{
			handleCmd_getavr();
		}
		else if(0 == strcmp(currLine, "ls"))
		{
			handleCmd_ls();
		}
		else if(0 == strcmp(currLine, "cd"))
		{
			handleCmd_cd();
		}
		else if(0 == strcmp(currLine, "selCont"))
		{
			handleCmd_selCont();
		}
		else if(0 == strcmp(currLine, "play"))
		{
			handleCmd_play();
		}
		else if(0 == strcmp(currLine, "pause"))
		{
			handleCmd_pause();
		}
		else if(0 == strcmp(currLine, "stop"))
		{
			handleCmd_stop();
		}
		else if(0 == strcmp(currLine, "volup"))
		{
			handleCmd_volup();
		}
		else if(0 == strcmp(currLine, "voldown"))
		{
			handleCmd_voldown();
		}
		else if(0 == strcmp(currLine, "help"))
		{
			handleCmd_help();
		}
		else 
		{
			printf("Unrecognized command: %s\n", currLine);
			handleCmd_help();
		}
	}
}

void STBS_Init(void *chain)
{
	callbackResponseSemaphore = CreateSemaphore(NULL,0, 2, "");
	strcpy(currContainerObjID,"0");
	currMServerUDN[0] = '\0';
	currMRendererUDN[0] = '\0';
	currMRendererVolume = 0;

	/* Event callback function registration code */
	MSCP_EventCallback_ConnectionManager_SourceProtocolInfo=MSCP_EventSink_ConnectionManager_SourceProtocolInfo;
	MSCP_EventCallback_ConnectionManager_SinkProtocolInfo=MSCP_EventSink_ConnectionManager_SinkProtocolInfo;
	MSCP_EventCallback_ConnectionManager_CurrentConnectionIDs=MSCP_EventSink_ConnectionManager_CurrentConnectionIDs;
	MSCP_EventCallback_ContentDirectory_SystemUpdateID=MSCP_EventSink_ContentDirectory_SystemUpdateID;

	AVRCP_EventCallback_ConnectionManager_SourceProtocolInfo=AVRCP_EventSink_ConnectionManager_SourceProtocolInfo;
	AVRCP_EventCallback_ConnectionManager_SinkProtocolInfo=AVRCP_EventSink_ConnectionManager_SinkProtocolInfo;
	AVRCP_EventCallback_ConnectionManager_CurrentConnectionIDs=AVRCP_EventSink_ConnectionManager_CurrentConnectionIDs;
	AVRCP_EventCallback_RenderingControl_LastChange=AVRCP_EventSink_RenderingControl_LastChange;
	AVRCP_EventCallback_AVTransport_LastChange=AVRCP_EventSink_AVTransport_LastChange;



	MS_KnownDevicesTable = ILibInitHashTree();
	AVR_KnownDevicesTable = ILibInitHashTree();
	
	MS_ControlPoint = MSCP_Init(chain, MSCP_BrowseResponded, MSCP_DeviceAdded, MSCP_DeviceRemoved);
	AVR_ControlPoint = AVRCP_CreateControlPoint(chain, AVRCP_DeviceAdded, AVRCP_DeviceRemoved);
}


void STBS_Uninit(void)
{
	ILibDestroyHashTree(MS_KnownDevicesTable);
	ILibDestroyHashTree(AVR_KnownDevicesTable);
	CloseHandle(callbackResponseSemaphore);
	MS_KnownDevicesTable = NULL;
	AVR_KnownDevicesTable = NULL;
}