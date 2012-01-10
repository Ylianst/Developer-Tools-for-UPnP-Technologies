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

#ifdef WPS_ENROLLEE

#if defined(WIN32)
	#ifndef MICROSTACK_NO_STDAFX
		#include "stdafx.h"
	#endif
	#define _CRTDBG_MAP_ALLOC
	#include <TCHAR.h>
#endif

#if defined(WINSOCK2)
	#include <winsock2.h>
	#include <ws2tcpip.h>
#elif defined(WINSOCK1)
	#include <winsock.h>
	#include <wininet.h>
#endif

extern "C" {
#include "DVMicroStack.h"
};

#include "WPSEnrollee.h"

extern CRegProtocol g_regProtocol;

extern void SendWPSResponse(void * upnptoken, BufferObj & outBuf);
extern void SendWPSResponseError(void * upnptoken, int code, const char * mess);
extern void RegistrationReady(int value);
extern void WPS_CPWasAuthenticated(void * upnptoken, const GUID & peerGUID);

void WPSEnrollee::RunWPS()
{
	printf("Running WPS as Enrollee: state is %d\n", m_state);

	// Always allow a GetDeviceInfo to reset the state machine to its initial
	// state.
	char errorbuf[128];

	if (m_msgBuffer.Length() <= 1) {
		printf("GetDeviceInfo called\n");

		if (m_state > sent_m1) { // An introduction is already underway
			// Check to see if the introduction that was underway is "stale" and should be aborted. 
			if (g_regProtocol.SecondsSinceDevLastMessage() < g_regProtocol.pbc_walk_time ||
				(m_state == waiting_for_pbc && 
				g_regProtocol.SecondsSinceDevPBCButtonPress() < g_regProtocol.pbc_walk_time)) {
				// Tell Registrar we are busy for now
				sprintf(errorbuf,"Message Processing Error in Enrollee state %d\n",m_state); 
				SendWPSResponseError(m_upnptoken, 708, errorbuf); // Device busy error
				RegistrationReady(0);	// Tell Registrars we are busy.
				return;
			}
			// Else the current introduction should be aborted and the new introduction should be
			// allowed to proceed.
			Reset(); // Reset all of the parameters to get ready for another Control Point introduction
			m_state = init;
		}

		int pin = m_defaultPin;

		if ( (m_regInfo.p_enrolleeInfo->configMethods & WSC_CONFMET_DISPLAY) &&
			! g_regProtocol.UseStaticPIN()) {
			// Device has a display, so generate a new PIN if not configured to use static PIN
			pin = g_regProtocol.NewPIN(); 
		} 

		char buf[40];
		sprintf(buf,"%08d", pin);
		m_regInfo.password.Reset();
		m_regInfo.password.Append(strlen(buf),(uint8*)buf);
		m_regInfo.p_enrolleeInfo->devPwdId = (pin) ? WSC_DEVICEPWDID_DEFAULT : WSC_DEVICEPWDID_PUSH_BTN;

		m_state = SendM1();
		if (m_state < init) { // error condition
			sprintf(errorbuf,"Message Processing Error in Enrollee state %d\n",m_state); 
			SendWPSResponseError(m_upnptoken, 704, errorbuf);
			m_state = init;
		}
		return;
	}
	switch(m_state) {
	case sent_m1:
	case sent_ack:
		// If Device has been waiting for PBC to commence, but the Registrar has been
		// too slow, then ask user to press the button again or cancel.
		if (m_regInfo.p_enrolleeInfo->devPwdId == WSC_DEVICEPWDID_PUSH_BTN && 
			g_regProtocol.SecondsSinceDevPBCButtonPress() > g_regProtocol.pbc_walk_time) {
			// delay has been too long since button press
			if (RequestPBC()) { // PBC button pressed
				m_state = ProcessM2();
			} else {
				m_state = err_process_m2;
			}
		} else {
			m_state = ProcessM2();
		}
	break;
	case waiting_for_pbc: 
		{ int diff = g_regProtocol.SecondsSinceDevPBCButtonPress();
		printf("time since pbc is %d\n", diff);
		if (g_regProtocol.SecondsSinceDevPBCButtonPress() <= g_regProtocol.pbc_walk_time) {
			m_state = ProcessM2();
		} else {
			m_state = err_process_m2; // delay has been too long since button press
		}
		}
	break;
	case sent_m3:
		m_state = ProcessM4();
	break;
	case sent_m5:
		m_state = ProcessM6();
	break;
	case sent_m7:
		m_state = ProcessM8();
		// According to the WPS spec, the exchange is complete after the Device (Enrollee) sends
		// its Done message.  This occurs as part of the logic of ProcessM8().  Therefore, the
		// state machine needs to be re-initilized here to be ready for another instance of the
		// Registration protocol.  However, only re-initialize here if there was no error in 
		// processing M8.
		if (m_state == sent_done) { // re-initialize if successfully processed M8
			m_state = init; 
		}
	break;
	case sent_done:
		m_state = init;
	break;
	/*
	case sent_ack:
		m_state = init;
	break;
	*/
	default:
		sprintf(errorbuf,"*** Message Processing Error in Enrollee state %d\n",m_state); 
		printf(errorbuf);
		SendWPSResponseError(m_upnptoken, 704, errorbuf);
		RegistrationReady(1);	// we are ready for a new connection now
		m_state = init;
		return;
	}
	if (m_state < init) { // error condition
		sprintf(errorbuf,"*** Message Processing Error in Enrollee state %d\n",m_state); 
		printf(errorbuf);
		SendWPSResponseError(m_upnptoken, 704, errorbuf);
		RegistrationReady(1);	// we are ready for a new connection now
		m_state = init;
	}
}


int WPSEnrollee::SendM1()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageM1(&m_regInfo, tempBuf);
    if(WSC_SUCCESS != err)
    {
		return err_build_m1;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending M1\n");
	SendWPSResponse(m_upnptoken, outBuf);
	g_regProtocol.DevMessageWasSent();
	return sent_m1;
}

#define STDIN_FILENO 0

int KeyboardInput()
{
	fd_set selectset;
	struct timeval timeout = {20,0};  //timeout of 20 secs.
	int ret;
	FD_ZERO(&selectset);
	FD_SET(0,&selectset);
	ret = 1; // for now, just disable the timeout logic
	//ret = select(1,&selectset,NULL,NULL,&timeout);
	// note:  this select call fails immediately and returns -1, so the timout doesn't really work...
	if (ret == 0 || ret == -1) { //timeout or error
		printf("PBC was cancelled\n");
		return 'c';
	} else {
		int c = getc(stdin);
		if (c == 'c' || c == 'C') {
			c = getc(stdin); // eat up the <Enter> character
			printf("PBC was cancelled\n");
			fflush(stdout);
			return 'c'; 
		} else { 
			printf("PushButton pressed\n");
			fflush(stdout);
			return 0;
		}
	}
}

bool WPSEnrollee::RequestPBC()
{
	printf("Press <Enter> now to push the PBC button, or 'c' to cancel\n");
	fflush(stdout);
	if (KeyboardInput() == 'c') {
		return false;
	}
	g_regProtocol.DevPBCButtonWasPressed();
	return true;
}

void WPSEnrollee::DoPBCVerification()
{
	m_PBCVerificationNeeded = false; // turn off flag
	if (! RequestPBC()) {
		printf("sending RegistrationReady=1\n");
		RegistrationReady(1);
		m_state = init; 
		return;
	}

	// Change the password to PBC mode
	m_regInfo.p_enrolleeInfo->devPwdId = WSC_DEVICEPWDID_PUSH_BTN;
	m_regInfo.password.Reset();
	m_regInfo.password.Append(8,(uint8*)"00000000");

	printf("sending RegistrationReady=1\n");
	RegistrationReady(1);
	m_state = waiting_for_pbc; 
}

int WPSEnrollee::ProcessM2()
{
	uint32 err;

	// need to get actual message size, not msg buffer size here.  Othewise, HMAC fails
	// because the code assumes the end of the message contains the authenticator TLV.

	uint32 message_type;
	err = g_regProtocol.GetMsgType(message_type, m_msgBuffer);
    if(WSC_SUCCESS != err)
    {
		return err_process_m2;
    }
	if (message_type == WSC_ID_MESSAGE_M2D) { // begin displaying the PIN number now and send an ACK
		err = g_regProtocol.ProcessMessageM2D(&m_regInfo, m_msgBuffer);
		if(WSC_SUCCESS != err)
		{
			return err_process_m2d;
		}

		if (m_regInfo.p_enrolleeInfo->devPwdId == WSC_DEVICEPWDID_PUSH_BTN && 
			m_regInfo.p_registrarInfo->configMethods == WSC_CONFMET_PBC &&
			g_regProtocol.SecondsSinceDevPBCButtonPress() <= g_regProtocol.pbc_walk_time) { 
			// if PBC has already been pressed on Device.
			SendACK();
			return waiting_for_pbc;
		} else if ( m_regInfo.p_registrarInfo->configMethods != WSC_CONFMET_PBC ) {
			char buf[9];
			memcpy(buf, m_regInfo.password.GetBuf(), 8);
			buf[8] = 0;
			printf("PIN number is: %8s\n", buf );
			return SendACK();
		} 

		// This code will be reached if the Registrar wants to use the PBC method but the Device's PBC
		// button has not been pressed.  This is indicated by the Device's password ID being DEFAULT
		// whereas the Registrar's configMethods value is only PBC.

		m_PBCVerificationNeeded = true; // signal UI thread that verification is needed

		return SendNACK(); // NACK here tells the Registrar that the Device isn't ready to proceed yet
		// allow UI thread to decide if the PBC is pushed.  In meantime, reject all messages.
	}

	err = g_regProtocol.ProcessMessageM2(&m_regInfo, m_msgBuffer, NULL );
    if(WSC_SUCCESS != err)
    {
		return err_process_m2;
    }

	// Signal an error if the Registrar is not using the same password type as the Device
	if (m_regInfo.p_registrarInfo->devPwdId != m_regInfo.p_enrolleeInfo->devPwdId) {
		return err_process_m2;
	}

	char buf[9];
	memcpy(buf, m_regInfo.password.GetBuf(), 8);
	buf[8] = 0;
	printf("PIN %8s pwdid %d\n", buf, m_regInfo.p_enrolleeInfo->devPwdId);
	g_regProtocol.ClearPBCButtonPress(); // only allow one run of protocol for a given button press
	
	RegistrationReady(0);	// Once an M2 message has been processed, the Device declares itself busy
	
	return SendM3();
}

int WPSEnrollee::SendM3()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageM3(&m_regInfo, tempBuf);
    if(WSC_SUCCESS != err)
    {
		return err_build_m3;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending M3\n");
	SendWPSResponse(m_upnptoken, outBuf);
	g_regProtocol.DevMessageWasSent();
	return sent_m3;
}

int WPSEnrollee::SendACK()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageAck(&m_regInfo, tempBuf);
    if(WSC_SUCCESS != err)
    {
		return err_build_Ack;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending ACK\n");
	SendWPSResponse(m_upnptoken, outBuf);
	g_regProtocol.DevMessageWasSent();
	return sent_ack;
}

int WPSEnrollee::SendNACK()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageNack(&m_regInfo, tempBuf, WSC_ERROR_DEVICE_BUSY);
    if(WSC_SUCCESS != err)
    {
		return err_build_Ack;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending NACK\n");
	SendWPSResponse(m_upnptoken, outBuf);
	g_regProtocol.DevMessageWasSent();
	return sent_nack;
}

int WPSEnrollee::ProcessM4()
{
	uint32 err;

	err = g_regProtocol.ProcessMessageM4(&m_regInfo, m_msgBuffer );
    if(WSC_SUCCESS != err)
    {
		return err_process_m4;
    }
	return SendM5();
}

int WPSEnrollee::SendM5()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageM5(&m_regInfo, tempBuf);
    if(WSC_SUCCESS != err)
    {
		return err_build_m5;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending M5\n");
	SendWPSResponse(m_upnptoken, outBuf);
	g_regProtocol.DevMessageWasSent();
	return sent_m5;
}
int WPSEnrollee::ProcessM6()
{
	uint32 err;

	err = g_regProtocol.ProcessMessageM6(&m_regInfo, m_msgBuffer );
    if(WSC_SUCCESS != err)
    {
		return err_process_m6;
    }
	return SendM7();
}

int WPSEnrollee::SendM7()
{
	uint32 err;
    BufferObj outBuf, tempBuf;
	CTlvEsM7Enr * encrSettings = NULL;

	err = g_regProtocol.BuildMessageM7(&m_regInfo, tempBuf, encrSettings);
    if(WSC_SUCCESS != err)
    {
		return err_build_m7;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending M7\n");
	SendWPSResponse(m_upnptoken, outBuf);
	g_regProtocol.DevMessageWasSent();
	return sent_m7;
}
int WPSEnrollee::ProcessM8()
{
	uint32 err;
	CTlvEsM8Sta *esSta;

	err = g_regProtocol.ProcessMessageM8(&m_regInfo, m_msgBuffer, (void **) &esSta );
    if(WSC_SUCCESS != err)
    {
		return err_process_m8;
    }
	printf("\n***************\n**** Successfully processed M8 ****\n\n");
	return SendDone();
}
int WPSEnrollee::SendDone()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageDone(&m_regInfo, tempBuf);
    if(WSC_SUCCESS != err)
    {
		return err_build_Done;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending Done\n");
	SendWPSResponse(m_upnptoken, outBuf);

	//****** Derivation of UPnP Protected Setup AuthKey and KeyWrapKey ******
    //1. declare and initialize the appropriate buffer objects
	BufferObj kdkBuf(m_regInfo.emsk.GetBuf(), SIZE_256_BITS);
    BufferObj pString((uint8 *)UPNP_PERSONALIZATION_STRING, 
                        strlen(UPNP_PERSONALIZATION_STRING));
    BufferObj keys;

    //2. call the key derivation function
    g_regProtocol.DeriveKey(kdkBuf, pString, 256 + 128, keys);

    //3. split the key into the component keys and store them
    keys.Rewind(keys.Length());
    m_regInfo.UPnPPSauthKey.Reset();
	m_regInfo.UPnPPSkeyWrapKey.Reset();

    m_regInfo.UPnPPSauthKey.Append(SIZE_256_BITS, keys.Pos());
    keys.Advance(SIZE_256_BITS);

    m_regInfo.UPnPPSkeyWrapKey.Append(SIZE_128_BITS, keys.Pos());
	// **** End of key derivation code

	if (m_regInfo.p_registrarInfo) {
		WPS_CPWasAuthenticated(m_upnptoken, * ((GUID*) & (m_regInfo.p_registrarInfo->uuid)));
	}

	Reset(); // Reset all of the parameters to get ready for another Control Point introduction

	g_regProtocol.DevMessageWasSent();
	return sent_done;
}

void WPSEnrollee::SetRegistrationProtocolInfo(GUID uuid)
{
	S_DEVICE_INFO * di = new S_DEVICE_INFO;
	if (m_regInfo.p_enrolleeInfo) {
		delete m_regInfo.p_enrolleeInfo;
	}
	memset((void*) di, 0, sizeof(S_DEVICE_INFO));
	m_regInfo.p_enrolleeInfo = di;
	di->version = 0x10;
	memcpy(di->uuid,&uuid,sizeof(GUID));
	// di->macAddr ... currently NULL
	strcpy(di->deviceName,"My Device");
	di->primDeviceCategory = WSC_DEVICE_TYPE_CAT_COMPUTER; 
	di->primDeviceOui = 0x0050f204;
	di->primDeviceSubCategory = WSC_DEVICE_TYPE_SUB_CAT_COMP_PC;  
	di->authTypeFlags = 0x1;
	di->encrTypeFlags = 0x4;
	di->connTypeFlags = 0x1;
	di->configMethods = WSC_CONFMET_KEYPAD | WSC_CONFMET_PBC | WSC_CONFMET_DISPLAY;
	di->scState = WSC_SCSTATE_UNCONFIGURED; 
	strcpy(di->manufacturer,"Intel");
	strcpy(di->modelName,"gizmo");
	strcpy(di->modelNumber,"Model 123");
	strcpy(di->serialNumber,"SN: 456");
	di->rfBand = 1;
	di->osVersion = 0x80000000;
	di->featureId = 0x80000000;
	di->assocState = WSC_ASSOC_NOT_ASSOCIATED;
	di->configError = 0; // No error
	di->devPwdId = m_devPwdId; // this is the password ID that will be used in the M1 message
	di->b_ap = false;
	strcpy(di->ssid,"WscNewAP");
	strcpy(di->keyMgmt,"WPA-EAP");
}

#endif WPS_ENROLLEE