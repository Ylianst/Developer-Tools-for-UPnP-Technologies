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

#ifndef _WPS_ENROLLEE_
#define _WPS_ENROLLEE_

//OpenSSL includes
#include <openssl/bn.h>
#include <openssl/dh.h>

#include "WscHeaders.h"
#include "WscCommon.h"
#include "StateMachineInfo.h"
#include "RegProtoMsgs.h"
#include "RegProtocol.h"

extern void SetRegistrationProtocolInfo(bool isEnrollee, GUID uuid);

class WPSEnrollee {
	int SendM1();
	int ProcessM2();
	int SendM3();
	int SendACK();	
	int SendNACK();
	int ProcessM4();
	int SendM5();
	int ProcessM6();
	int SendM7();
	int ProcessM8();
	int SendDone();
	int m_defaultPin;
	BufferObj m_devicePassword;
	int m_devPwdId;
	int m_state;
	BufferObj m_msgBuffer; 
	void * m_upnptoken;
	bool m_PBCVerificationNeeded;
	S_REGISTRATION_DATA m_regInfo;
	GUID m_uuid;

public:
	WPSEnrollee(GUID uuid, int PIN, int pwdId, BufferObj & pwd) : m_defaultPin(PIN), 
		m_devPwdId(pwdId), m_PBCVerificationNeeded(false), m_state(init) { 
			memcpy(& m_uuid, & uuid, sizeof(GUID));
			memset(&m_regInfo, 0, sizeof(m_regInfo)); 
			m_devicePassword.Append(pwd.Length(),pwd.GetBuf()); 
			SetRegistrationProtocolInfo(m_uuid); 
	}
	~WPSEnrollee() { delete m_regInfo.p_enrolleeInfo; m_regInfo.p_enrolleeInfo = NULL;
					 delete m_regInfo.p_registrarInfo; m_regInfo.p_registrarInfo = NULL; }

	enum {	err_build_m1 = -100, err_process_m2, err_process_m2d, err_build_m3, err_process_m4,
			err_build_m5, err_process_m6, err_build_m7, err_process_m8, err_build_Done, err_build_Ack, 
			err_reject_PBC,
			init = 0, sent_m1, sent_m3, sent_m5, sent_m7, sent_done, sent_ack, sent_nack, 
			waiting_for_pbc };	

	void SetRegistrationProtocolInfo(GUID uuid);
	void SetBuffer(void * token, unsigned char * buf, int bufLen) { 
		m_msgBuffer.Reset();
		m_msgBuffer.Append(bufLen, buf);
		m_msgBuffer.Rewind();
		m_upnptoken = token;
	}
	bool RequestPBC();
	// called by UI thread to check if it should request PBC button press
	bool PBCVerificationNeeded() { return m_PBCVerificationNeeded; }
	void DoPBCVerification();

	void Reset() {
		m_state = init;
		delete m_regInfo.p_enrolleeInfo; 
		delete m_regInfo.p_registrarInfo;
		memset(&m_regInfo, 0, sizeof(m_regInfo)); 
		SetRegistrationProtocolInfo(m_uuid); 
	}
	void RunWPS();
};

#endif // _WPS_ENROLLEE_
