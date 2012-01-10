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

#ifdef WPS_REGISTRAR

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

#include "WPSRegistrar.h"

extern CRegProtocol g_regProtocol;

extern void SendWPSMessage(void * registrar, void * peer, 
						   void (*CallbackPtr) (struct UPnPService* Service,int ErrorCode,void *User,unsigned char* OutMessage,int OutMessageLength),
						   BufferObj & outBuf);

extern void WPSDeviceWasAuthenticated(const GUID & peerGUID);

CTlvEsM8Sta * CreateTlvEsM8Sta(bool isWirelessWPS = false )
{
	char    *cp_data;
    uint16    data16;
	uint8 mac[] = { 0, 1, 2, 3, 4, 5 };

    CTlvEsM8Sta * mp_tlvEsM8Sta = new CTlvEsM8Sta();

	if (isWirelessWPS) { // only include a wireless Credential if run in wireless mode.
		// credential
		CTlvCredential *p_tlvCred = new CTlvCredential();
		// Fill in credential items
		// nwIndex
		p_tlvCred->nwIndex.Set( WSC_ID_NW_INDEX, 1 );
		// ssid
		data16 = 3;
		p_tlvCred->ssid.Set( WSC_ID_SSID, (uint8 *)"foo", data16 );
		// authType
		data16 = WSC_AUTHTYPE_WPAPSK;
		p_tlvCred->authType.Set( WSC_ID_AUTH_TYPE, data16 );
		// encrType
		data16 = WSC_ENCRTYPE_TKIP;
		p_tlvCred->encrType.Set( WSC_ID_ENCR_TYPE, data16 );
		// nwKeyIndex
		p_tlvCred->nwKeyIndex.Set( WSC_ID_NW_KEY_INDEX, 1 );
		// nwKey
		p_tlvCred->nwKey.Set( WSC_ID_NW_KEY, "foo", 3);
		// macAddr
		data16 = SIZE_MAC_ADDR;
		p_tlvCred->macAddr.Set( WSC_ID_MAC_ADDR, mac, data16 );
		ListAddItem( mp_tlvEsM8Sta->credential, p_tlvCred );
	}

    // New pwd
    // TODO

    // PwdId
    // TODO

	return mp_tlvEsM8Sta;
} // CreateTlvEsM8Sta

void WPSRegistrar::SetPIN(int pin)
{
	char buf[40];
	m_defaultPin = pin;
	m_regInfo.p_registrarInfo->devPwdId = (pin) ? WSC_DEVICEPWDID_DEFAULT : WSC_DEVICEPWDID_PUSH_BTN;
	m_devPwdId = m_regInfo.p_registrarInfo->devPwdId;

	sprintf(buf,"%08d", pin);
	m_regInfo.password.Reset();
	m_regInfo.password.Append(strlen(buf),(uint8*) buf);
}

void WPSRegistrar::RunWPS()
{
	switch(m_state) {
	case init:
		{
		char buf[40];
		m_regInfo.password.Reset();
		if (m_devPwdId == WSC_DEVICEPWDID_DEFAULT || m_devPwdId == WSC_DEVICEPWDID_PUSH_BTN) {
			sprintf(buf,"%08d", m_defaultPin);
			m_regInfo.password.Append(strlen(buf),(uint8*) buf);
			m_regInfo.p_registrarInfo->devPwdId = m_devPwdId;
		} else { // one of the other password types such as user-defined or machine-defined
			m_regInfo.password.Append(m_devicePassword.Length(), m_devicePassword.GetBuf());
			//m_regInfo.enrolleePwdId = m_devPwdId;
		}
		BufferObj outBuf;
		//outBuf.Append(1, (uint8 *) "");
		SendWPSMessage(this, m_peer, m_callback, outBuf);
		m_state = device_info;
		}
	break;
	case device_info:
		m_state = ProcessM1();
	break;
	case sent_m2d: // The Device has been asked to display its PIN number.  Prompt user to enter it.
		{
		// Should get here only if Device is now ready to proceed (either its PBC button has been pushed,
		// or it has a label-based PIN, or it is currently displaying its dynamic PIN).
		uint32 message_type;
		int err = g_regProtocol.GetMsgType(message_type, m_msgBuffer);
		if(WSC_SUCCESS != err)
		{
			m_state = init; // reset the state machine to recover from error
			return;
		}
		if (message_type == WSC_ID_MESSAGE_ACK) { // Device is ready to run protocol
			if (m_devPwdId == WSC_DEVICEPWDID_PUSH_BTN && 
				m_devPwdId == m_regInfo.p_enrolleeInfo->devPwdId) {
				printf("\n*********\nUsing push-button method...\n");
				m_state = SendM2();
			} else if (m_regInfo.p_enrolleeInfo->devPwdId == WSC_DEVICEPWDID_DEFAULT) {
				m_state = AskForPin(); // Device is using a PIN for its password ID
			} else if (m_regInfo.p_enrolleeInfo->devPwdId == WSC_DEVICEPWDID_PUSH_BTN) { 
				// Device is in PBC mode but Registrar is not. 
				// Automatically switch into PBC mode
				printf("\n******** Device wants to use PBC mode. Is this okay? (Y/N) ");
				char c;
				c = getc(stdin);
				if (c == 'y' || c == 'Y') {
					m_devPwdId = WSC_DEVICEPWDID_PUSH_BTN;
					m_regInfo.p_registrarInfo->devPwdId = m_devPwdId;
					m_regInfo.password.Reset();
					m_regInfo.password.Append(8,(uint8*) "00000000");
					printf("\n*********\nUsing push-button method...\n");
					m_state = SendM2();
				} else {
					m_state = err_build_m2;
					return;
				}
			} else { // Device wants to use some other password type that the Registrar doesn't support
				m_state = err_build_m2;
				return;
			}
		} else if (message_type == WSC_ID_MESSAGE_NACK) { // Device is not ready to run protocol
			printf("got NACK from Device\n");
			if (m_devPwdId == WSC_DEVICEPWDID_PUSH_BTN) {
				printf("\n*********\nUsing push-button method. Please press button on Device now.\n");
				m_state = got_nack_after_m2d; // stay in this state for now
			}
		}
		}
	break;

	// Registrar will not send another message until it sees the Device transition its SetupReady
	// state variable to 1.  At that point, it will call back into the state machine here and continue
	// the setup operation.
	case got_nack_after_m2d: 
		printf("Waiting for button press on Device\n");
		m_state = SendM2();
	break;

	case sent_m2:
		m_state = ProcessM3();
	break;
	case sent_m4:
		m_state = ProcessM5();
	break;
	case sent_m6:
		m_state = ProcessM7();
	break;
	case sent_m8:
		m_state = ProcessDone();
	break;
	case sent_ack:
		m_state = init;
	break;
	}
	if (m_state < init) { // error condition
		printf("Message Processing Error in Registrar state %d\n",m_state); 
		m_state = init;
	}
}

void WPSRegistrar::PBCTimeout()
{
	printf("\nTimeout waiting for PushButton event on Device\n");
	m_state = init;
}

int WPSRegistrar::AskForPin()
{
	if (g_regProtocol.UseStaticPIN()) { // in this mode, use static PIN value known to Registrar already
		return SendM2();
	}
	printf("\n*********\nThe Device should be displaying its PIN.  Please enter it now: ");
	unsigned long pin;
	int res = scanf("%u",&pin);
	if (res == 1 && pin < 100000000 && g_regProtocol.ValidateChecksum(pin)) { // okay
		SetPIN(pin);
		return SendM2();
	} else {
		printf("Invalid PIN number entered %d\n",pin); 
		return(init);
	}
}

int WPSRegistrar::ProcessM1()
{
	uint32 err;

	err = g_regProtocol.ProcessMessageM1(&m_regInfo, m_msgBuffer );
    if(WSC_SUCCESS != err)
    {
		return err_process_m1;
    }
	return SendM2D(); // Tell Device the WPS method the Registrar wants to use
}

int WPSRegistrar::SendM2D()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageM2D(&m_regInfo, tempBuf);
    if(WSC_SUCCESS != err)
    {
		return err_build_m2;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending M2D\n");
	SendWPSMessage(this, m_peer, m_callback, outBuf);
	return sent_m2d;
}

int WPSRegistrar::SendM2()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageM2(&m_regInfo, tempBuf, NULL);
    if(WSC_SUCCESS != err)
    {
		return err_build_m2;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	char buf[9];
	memcpy(buf, m_regInfo.password.GetBuf(), 8);
	buf[8] = 0;
	printf("sending M2 with password %s\n", buf);
	SendWPSMessage(this, m_peer, m_callback, outBuf);
	return sent_m2;
}

int WPSRegistrar::ProcessM3()
{
	uint32 err;

	err = g_regProtocol.ProcessMessageM3(&m_regInfo, m_msgBuffer );
    if(WSC_SUCCESS != err)
    {
		return err_process_m3;
    }
	return SendM4();
}
int WPSRegistrar::SendM4()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageM4(&m_regInfo, tempBuf);
    if(WSC_SUCCESS != err)
    {
		return err_build_m4;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending M4\n");
	SendWPSMessage(this, m_peer, m_callback, outBuf);
	return sent_m4;
}
int WPSRegistrar::SendACK()
{
    BufferObj outBuf((unsigned char *) "WPS Message ACK", strlen("WPS Message ACK") + 1);
	SendWPSMessage(this, m_peer, m_callback, outBuf);
	return sent_ack;
}

int WPSRegistrar::ProcessM5()
{
	uint32 err;

	err = g_regProtocol.ProcessMessageM5(&m_regInfo, m_msgBuffer );
    if(WSC_SUCCESS != err)
    {
		return err_process_m5;
    }
	return SendM6();
}
int WPSRegistrar::SendM6()
{
	uint32 err;
    BufferObj outBuf, tempBuf;

	err = g_regProtocol.BuildMessageM6(&m_regInfo, tempBuf);
    if(WSC_SUCCESS != err)
    {
		return err_build_m6;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending M6\n");
	SendWPSMessage(this, m_peer, m_callback, outBuf);
	return sent_m6;
}

int WPSRegistrar::ProcessM7()
{
	uint32 err;
    void *encrSettings = NULL;

	err = g_regProtocol.ProcessMessageM7(&m_regInfo, m_msgBuffer, &encrSettings );
    if(WSC_SUCCESS != err)
    {
		return err_process_m7;
    }
	return SendM8();
}
int WPSRegistrar::SendM8()
{
	uint32 err;
    BufferObj outBuf, tempBuf;
	//CTlvEsM8Sta * apEncrSettings = CreateTlvEsM8Sta( true ); // to include Credential
	CTlvEsM8Sta * apEncrSettings = CreateTlvEsM8Sta( );

	err = g_regProtocol.BuildMessageM8(&m_regInfo, tempBuf, apEncrSettings);
    if(WSC_SUCCESS != err)
    {
		return err_build_m8;
    }
    outBuf.Append(tempBuf.Length(), tempBuf.GetBuf());
    
	printf("sending M8\n");
	SendWPSMessage(this, m_peer, m_callback, outBuf);
	return sent_m8;
}
int WPSRegistrar::ProcessDone()
{
	uint32 err;

	err = g_regProtocol.ProcessMessageDone(&m_regInfo, m_msgBuffer );
    if(WSC_SUCCESS != err)
    {
		return err_process_Done;
    }
	printf("\n*****  Successfully processed Message Done\n");
	if (m_regInfo.p_enrolleeInfo) {
		WPSDeviceWasAuthenticated(* ((GUID*) & (m_regInfo.p_enrolleeInfo->uuid)));
	}

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

	return init;
}

void WPSRegistrar::SetRegistrationProtocolInfo(GUID uuid)
{
	S_DEVICE_INFO * di = new S_DEVICE_INFO;
	if (m_regInfo.p_registrarInfo) {
		delete m_regInfo.p_registrarInfo;
	}
	memset((void*) di, 0, sizeof(S_DEVICE_INFO));
	m_regInfo.p_registrarInfo = di;
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
	strcpy(di->modelName,"DPCP");
	strcpy(di->modelNumber,"Model 123");
	strcpy(di->serialNumber,"SN: 456");
	di->rfBand = 1;
	di->osVersion = 0x80000000;
	di->featureId = 0x80000000;
	di->assocState = WSC_ASSOC_NOT_ASSOCIATED;
	di->configError = 0; // No error
	di->devPwdId = m_devPwdId; 
	di->b_ap = false;
	strcpy(di->ssid,"WscNewAP");
	strcpy(di->keyMgmt,"WPA-EAP");
}

#endif WPS_REGISTRAR