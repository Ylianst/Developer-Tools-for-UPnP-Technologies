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

#ifndef _WPS_REGISTRAR_
#define _WPS_REGISTRAR_

//OpenSSL includes
#include <openssl/bn.h>
#include <openssl/dh.h>

#include "WscHeaders.h"
#include "WscCommon.h"
#include "StateMachineInfo.h"
#include "RegProtoMsgs.h"
#include "RegProtocol.h"

class WPSRegistrar {
	int ProcessM1();
	int SendM2();
	int SendM2D();
	int ProcessM3();
	int SendM4();
	int ProcessM5();
	int SendM6();
	int ProcessM7();
	int SendM8();
	int ProcessDone();
	int SendACK();
	int AskForPin();
	void SetPIN(int pin);
	int m_defaultPin;
	BufferObj m_devicePassword;
	int m_devPwdId;
	void * m_peer;
	//void * m_callback;
	void (*m_callback) (struct UPnPService* Service,int ErrorCode,void *User,unsigned char* OutMessage,int OutMessageLength);
	int m_state;
	bool m_busy;
	BufferObj m_msgBuffer;
	S_REGISTRATION_DATA m_regInfo;

	enum {	err_process_m1 = -100, err_build_m2, err_process_m3, err_build_m4, err_process_m5,
			err_build_m6, err_process_m7, err_build_m8, err_process_Done, err_process_m2d,
			init = 0, device_info, sent_m2, sent_m2d, sent_m4, sent_m6, sent_m8, sent_ack, 
			got_nack_after_m2d };	

public:

	WPSRegistrar(GUID uuid, int PIN, int pwdId, BufferObj & pwd) : m_busy(false)
	{ 
		m_peer = NULL;
		m_defaultPin = PIN;
		m_devicePassword.Append(pwd.Length(),pwd.GetBuf()); 
		m_devPwdId = pwdId;
		m_state = init;
		memset(&m_regInfo, 0, sizeof(m_regInfo));  
		SetRegistrationProtocolInfo(uuid); 
	}
	~WPSRegistrar() { delete m_regInfo.p_enrolleeInfo; m_regInfo.p_enrolleeInfo = NULL;
					 delete m_regInfo.p_registrarInfo; m_regInfo.p_registrarInfo = NULL; }

	bool WaitingForPBCOnService( void *peerService ) {
		return ((peerService == m_peer) && (m_state == got_nack_after_m2d));
	}
	bool WaitingForPBC( ) {
		return (m_state == got_nack_after_m2d);
	}
	void PBCTimeout();
	void StartWPS(void *peerService, int PIN, int pwdId, void (*CallbackPtr) (struct UPnPService* Service,int ErrorCode,void *User,unsigned char* OutMessage,int OutMessageLength)) { 
		m_state = init; m_defaultPin = PIN, m_devPwdId = pwdId; 
		m_peer = peerService; m_busy = true; m_callback = CallbackPtr; 
		if (pwdId == WSC_DEVICEPWDID_PUSH_BTN) {
			// Signal intent to use PBC method by limiting config methods to this one method
			m_regInfo.p_registrarInfo->configMethods = WSC_CONFMET_PBC;
		} else {
			m_regInfo.p_registrarInfo->configMethods = WSC_CONFMET_KEYPAD | WSC_CONFMET_PBC | WSC_CONFMET_DISPLAY;
		}
	}
	void FinishWPS(GUID peerUUID) { m_busy = false; m_peer = NULL; }
	void SetRegistrationProtocolInfo(GUID uuid);
	void RunWPS();
	void SetBuffer(unsigned char * buf, int bufLen) { 
		m_msgBuffer.Reset();
		m_msgBuffer.Append(bufLen, buf);
		m_msgBuffer.Rewind();
	}
};

#endif // _WPS_REGISTRAR_
