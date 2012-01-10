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

#ifndef _REGPROT_
#define _REGPROT_

using namespace std;

#include "time.h"

#pragma pack(push, 1)

class CRegProtocol
{
private:
    uint8 version;
	bool m_useStaticPIN;
	time_t m_devButtonPressTime;
	time_t m_CPButtonPressTime;
	time_t m_devLastMessageTime;

public:
    CRegProtocol();
    ~CRegProtocol();

    uint32 SetMCCallback(IN CALLBACK_FN p_mcCallbackFn, IN void* cookie);

    // build message methods
    uint32 BuildMessageM1(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 BuildMessageM2(S_REGISTRATION_DATA *regInfo, BufferObj &msg, void *encrSettings);
    uint32 BuildMessageM2D(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 BuildMessageM3(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 BuildMessageM4(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 BuildMessageM5(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 BuildMessageM6(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 BuildMessageM7(S_REGISTRATION_DATA *regInfo, BufferObj &msg, void *encrSettings);
    uint32 BuildMessageM8(S_REGISTRATION_DATA *regInfo, BufferObj &msg, void *encrSettings);
    uint32 BuildMessageAck(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 BuildMessageNack(S_REGISTRATION_DATA *regInfo, BufferObj &msg, uint16 configError);
    uint32 BuildMessageDone(S_REGISTRATION_DATA *regInfo, BufferObj &msg);


    // process message methods
    uint32 ProcessMessageM1(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 ProcessMessageM2(S_REGISTRATION_DATA *regInfo, BufferObj &msg, void **encrSettings);
    uint32 ProcessMessageM2D(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 ProcessMessageM3(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 ProcessMessageM4(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 ProcessMessageM5(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 ProcessMessageM6(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 ProcessMessageM7(S_REGISTRATION_DATA *regInfo, BufferObj &msg, void **encrSettings);
    uint32 ProcessMessageM8(S_REGISTRATION_DATA *regInfo, BufferObj &msg, void **encrSettings);
    uint32 ProcessMessageAck(S_REGISTRATION_DATA *regInfo, BufferObj &msg);
    uint32 ProcessMessageNack(S_REGISTRATION_DATA *regInfo, BufferObj &msg, uint16 *configError);
    uint32 ProcessMessageDone(S_REGISTRATION_DATA *regInfo, BufferObj &msg);

    //utility methods
    uint32 GenerateDHKeyPair(DH **DHKeyPair, BufferObj &pubKey);
    void GenerateSHA256Hash(BufferObj &inBuf, BufferObj &outBuf);
    void DeriveKey(BufferObj &KDK, BufferObj &prsnlString, uint32 keyBits, BufferObj &key);
    bool ValidateMac(BufferObj &data, uint8 *hmac, BufferObj &key);
    bool ValidateKeyWrapAuth(BufferObj &data, uint8 *hmac, BufferObj &key);
    void EncryptData(BufferObj &plainText, 
                        BufferObj &encrKey, 
                        BufferObj &authKey, 
                        BufferObj &cipherText, 
                        BufferObj &iv);
    void DecryptData(BufferObj &cipherText, 
                          BufferObj &iv,
                          BufferObj &encrKey, 
                          BufferObj &authKey, 
                          BufferObj &plainText);
	void SetUseStaticPIN( bool val ) { m_useStaticPIN = val; }
	void DevPBCButtonWasPressed() { time (&m_devButtonPressTime); }
	void DevMessageWasSent() { time (&m_devLastMessageTime); }
	void CPPBCButtonWasPressed() { time (&m_CPButtonPressTime); }
	void ClearPBCButtonPress() { m_devButtonPressTime = 0; }
	int SecondsSinceDevPBCButtonPress() {
			time_t now; time (&now);
			return (int) difftime (now,m_devButtonPressTime);
	}
	int SecondsSinceCPPBCButtonPress() {
			time_t now; time (&now);
			return (int) difftime (now,m_CPButtonPressTime);
	}
	int SecondsSinceDevLastMessage() {
			time_t now; time (&now);
			return (int) difftime (now,m_devLastMessageTime);
	}
	bool UseStaticPIN() { return m_useStaticPIN; }
	uint32 NewPIN();
    uint32 GeneratePSK(IN uint32 length, OUT BufferObj &PSK);
    uint32 CheckNonce(IN uint8 *nonce, IN BufferObj &msg, IN int nonceType);
    uint32 GetMsgType(uint32 &msgType, BufferObj &msg);
    bool ValidatePeerPubKeyHash(uint8 *pubKeyMsg, uint8 *pubKeyHash);
	bool ValidateChecksum( IN unsigned long int PIN );
	uint32 ComputeChecksum( IN unsigned long int PIN );

	enum { pbc_walk_time = 120 };	

/*    uint32 CreatePrivateKey(char *name, EVP_PKEY **key);
    uint32 GetPublicKey(char *Name, EVP_PKEY **key);
    uint32 GetPrivateKey(char *Name, EVP_PKEY **key);
*/
}; // CRegProtocol

#pragma pack(pop)
#endif // _REGPROT_



