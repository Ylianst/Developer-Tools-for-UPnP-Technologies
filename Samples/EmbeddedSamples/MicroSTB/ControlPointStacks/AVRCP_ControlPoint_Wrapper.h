#ifndef AVRCP_CONTROL_POINT_WRAPPER_H
#define AVRCP_CONTROL_POINT_WRAPPER_H

#include "UPnPControlPointStructs.h"
#include "AVRCP_ControlPoint.h"
#include "ControlPoint_Wrapper_Common.h"


/*
 *	Must call this method once at the very beginning.
 *
 *	Caller registers callbacks for Browse responses and when MediaServers enter/leave the UPnP network.
 *		chain			: thread chain, obtained from ILibCreateChain
 *		callbackDeviceAdd : execute this method when a MediaRenderer enters the UPnP network
 *		callbackDeviceRemove : execute this method when a MediaRenderer leaves the UPnP network
 *
 */
void AVRCP_Init(void *chain, 
				CP_Fn_Device_Add callbackDeviceAdd,
				CP_Fn_Device_Remove callbackDeviceRemove);



/*
 *	Call this method for cleanup.
 */
void AVRCP_Uninit();

#endif