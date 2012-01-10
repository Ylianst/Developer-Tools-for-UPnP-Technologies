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

// EnrolleeSM.cpp
#ifdef WIN32
#include <windows.h>
#endif

#include <stdexcept>
#include <string.h>

//OpenSSL includes
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "tutrace.h"
#include "WscHeaders.h"
#include "WscCommon.h"
#include "WscError.h"
#include "Portability.h"
// #include "WscQueue.h"
#include "WscTlvBase.h"
#include "RegProtoTlv.h"
#include "RegProtoMsgs.h"
#include "StateMachineInfo.h"
#include "RegProtocol.h"
//#include "OobUfd.h"

extern "C" bool g_verbose;

using namespace std;

// ****************************
// public methods
// ****************************

/*
 * Name        : CEnrolleeSM
 * Description : Class constructor. Initialize member variables, set 
 *                    callback function.
 * Arguments   : none
 * Return type : none
 */
CRegProtocol::CRegProtocol() : m_devButtonPressTime(0), 
				m_CPButtonPressTime(0), m_useStaticPIN(false) // by default, use dynamic PIN
{
    version = WSC_VERSION;
    
} // constructor

/*
 * Name        : ~CEnrolleeSM
 * Description : Class destructor. Cleanup if necessary.
 * Arguments   : none
 * Return type : none
 */
CRegProtocol::~CRegProtocol() 
{
 
} // destructor


/*
 * Name        : SetMCCallback
 * Description : Set callback information for MC
 * Arguments   : IN CALLBACK_FN p_mcCallbackFn - pointer to callback function
 *               IN void *cookie - pointer that we pass back in the callback fn
 * Return type : none
 */
uint32 
CRegProtocol::SetMCCallback(IN CALLBACK_FN p_mcCallbackFn, IN void* cookie)
{
    return WSC_SUCCESS;
} // SetMCCallback

#ifdef WPS_ENROLLEE
uint32 
CRegProtocol::BuildMessageM1(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    uint8 message;

    try
    {
        //First generate/gather all the required data.
        message = WSC_ID_MESSAGE_M1;

        //Enrollee nonce
        RAND_bytes(regInfo->enrolleeNonce, SIZE_128_BITS);

        if(!regInfo->DHSecret)
        {
            BufferObj pubKey;
            GenerateDHKeyPair(&regInfo->DHSecret, pubKey);
        }

        //Extract the DH public key
        int len = BN_bn2bin(regInfo->DHSecret->pub_key, regInfo->pke);
        if(0 == len)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: BN_bn2bin: %s", 
                    ERR_error_string(ERR_get_error(), NULL)));
            throw RPROT_ERR_CRYPTO;
        }
 
        //Now start composing the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvUuid(
                        WSC_ID_UUID_E, 
                        msg, 
                        regInfo->p_enrolleeInfo->uuid, 
                        SIZE_UUID);
        CTlvMacAddr(
                        WSC_ID_MAC_ADDR,
                        msg, 
                        regInfo->p_enrolleeInfo->macAddr, 
                        SIZE_MAC_ADDR);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvPublicKey(
                        WSC_ID_PUBLIC_KEY, 
                        msg,
                        regInfo->pke, 
                        SIZE_PUB_KEY);
        CTlvAuthTypeFlags(
                        WSC_ID_AUTH_TYPE_FLAGS, 
                        msg,
                        &regInfo->p_enrolleeInfo->authTypeFlags);
        CTlvEncrTypeFlags(
                        WSC_ID_ENCR_TYPE_FLAGS, 
                        msg,
                        &regInfo->p_enrolleeInfo->encrTypeFlags);
        CTlvConnTypeFlags(
                        WSC_ID_CONN_TYPE_FLAGS,
                        msg,
                        &regInfo->p_enrolleeInfo->connTypeFlags);
        CTlvConfigMethods(
                        WSC_ID_CONFIG_METHODS,
                        msg,
                        &regInfo->p_enrolleeInfo->configMethods);
        CTlvScState(
                        WSC_ID_SC_STATE,
                        msg,
                        &regInfo->p_enrolleeInfo->scState);

		len = strlen(regInfo->p_enrolleeInfo->manufacturer) + 1;
        if (len > SIZE_64_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Manufacturer name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvManufacturer(
                        WSC_ID_MANUFACTURER,
                        msg,
                        regInfo->p_enrolleeInfo->manufacturer, 
                        len);
		len = strlen(regInfo->p_enrolleeInfo->modelName) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Model name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvModelName(
                        WSC_ID_MODEL_NAME,
                        msg,
                        regInfo->p_enrolleeInfo->modelName, 
                        len);
 		len = strlen(regInfo->p_enrolleeInfo->modelNumber) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Model number too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
       CTlvModelNumber(
                        WSC_ID_MODEL_NUMBER,
                        msg,
                        regInfo->p_enrolleeInfo->modelNumber, 
                        len);
 		len = strlen(regInfo->p_enrolleeInfo->serialNumber) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Serial number too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvSerialNum(
                        WSC_ID_SERIAL_NUM, 
                        msg,
                        regInfo->p_enrolleeInfo->serialNumber, 
                        len);

        CTlvPrimDeviceType primDev;
        primDev.categoryId = regInfo->p_enrolleeInfo->primDeviceCategory;
        primDev.oui = regInfo->p_enrolleeInfo->primDeviceOui;
        primDev.subCategoryId = regInfo->p_enrolleeInfo->primDeviceSubCategory;
        primDev.write(msg);

		len = strlen(regInfo->p_enrolleeInfo->deviceName) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Device name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvDeviceName(
                        WSC_ID_DEVICE_NAME,
                        msg,
                        regInfo->p_enrolleeInfo->deviceName, 
                        len); 
		CTlvRfBand(
                        WSC_ID_RF_BAND, 
                        msg,
                        &regInfo->p_enrolleeInfo->rfBand);
        CTlvAssocState(
                        WSC_ID_ASSOC_STATE,
                        msg,
                        &regInfo->p_enrolleeInfo->assocState);
        CTlvDevicePwdId(
                        WSC_ID_DEVICE_PWD_ID, 
                        msg,
                        &regInfo->p_enrolleeInfo->devPwdId);
        CTlvConfigError(
                        WSC_ID_CONFIG_ERROR,
                        msg,
                        &regInfo->p_enrolleeInfo->configError);
        CTlvOsVersion(
                        WSC_ID_OS_VERSION,
                        msg,
                        &regInfo->p_enrolleeInfo->osVersion);
        //skip optional attributes

        //copy message to outMsg buffer
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());
		int enrolleeMsgLen = regInfo->outMsg.Length();

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM1 built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM1 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM1 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM1 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM1
#endif // WPS_ENROLLEE

#ifdef WPS_REGISTRAR
uint32 CRegProtocol::ProcessMessageM1(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    S_WSC_M1 m1;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM1: %d byte message\n",
                                msg.Length()));
	int msglen = msg.Length();
    try
    {
        //First, deserialize (parse) the message.
        m1.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m1.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //If the major version number matches, assume we can parse it successfully.
        if((version & 0xF0) != (m1.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M1 != m1.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m1.uuid          = CTlvUuid(WSC_ID_UUID_E, msg, SIZE_UUID);
        m1.macAddr       = CTlvMacAddr(WSC_ID_MAC_ADDR, msg, SIZE_MAC_ADDR);
        m1.enrolleeNonce = CTlvEnrolleeNonce(WSC_ID_ENROLLEE_NONCE, 
                                             msg, SIZE_128_BITS);
        m1.publicKey     = CTlvPublicKey(WSC_ID_PUBLIC_KEY, msg, SIZE_PUB_KEY);
        m1.authTypeFlags = CTlvAuthTypeFlags(WSC_ID_AUTH_TYPE_FLAGS,msg);
        m1.encrTypeFlags = CTlvEncrTypeFlags(WSC_ID_ENCR_TYPE_FLAGS,msg);
        m1.connTypeFlags = CTlvConnTypeFlags(WSC_ID_CONN_TYPE_FLAGS,msg);
        m1.configMethods = CTlvConfigMethods(WSC_ID_CONFIG_METHODS, msg);
        m1.scState       = CTlvScState(WSC_ID_SC_STATE, msg);
        m1.manufacturer  = CTlvManufacturer(WSC_ID_MANUFACTURER, 
                                            msg, SIZE_64_BYTES);
        m1.modelName     = CTlvModelName(WSC_ID_MODEL_NAME, msg, SIZE_32_BYTES);
        m1.modelNumber   = CTlvModelNumber(WSC_ID_MODEL_NUMBER, 
                                           msg, SIZE_32_BYTES);
        m1.serialNumber  = CTlvSerialNum(WSC_ID_SERIAL_NUM, msg, SIZE_32_BYTES);
        m1.primDeviceType.parse(msg);
        m1.deviceName    = CTlvDeviceName(WSC_ID_DEVICE_NAME, 
                                          msg, SIZE_32_BYTES);
        m1.rfBand        = CTlvRfBand(WSC_ID_RF_BAND, msg);
        m1.assocState    = CTlvAssocState(WSC_ID_ASSOC_STATE, msg);
        m1.devPwdId      = CTlvDevicePwdId(WSC_ID_DEVICE_PWD_ID, msg);
        m1.configError   = CTlvConfigError(WSC_ID_CONFIG_ERROR, msg);
        m1.osVersion     = CTlvOsVersion(WSC_ID_OS_VERSION, msg);

        //skip the optional attributes
        //Check if the Identity field is present
        //if(WSC_ID_IDENTITY == msg.NextType())
        //{
        //    m1.identity = CTlvIdentity(WSC_ID_IDENTITY, msg,SIZE_80_BYTES);
        //}
        //ignore any vendor extensions

        //Now start processing the message

        //Before we do anyting else, check if we need to allocate enrolleeInfo
        //Master Control might not have allocated memory if it didn't have
        //any info about the enrollee
        if(!regInfo->p_enrolleeInfo)
            regInfo->p_enrolleeInfo = new S_DEVICE_INFO;

        memcpy(regInfo->p_enrolleeInfo->uuid, 
            m1.uuid.Value(), 
            m1.uuid.Length());
        memcpy(regInfo->p_enrolleeInfo->macAddr, 
            m1.macAddr.Value(), 
            m1.macAddr.Length());
        memcpy(regInfo->enrolleeNonce, 
            m1.enrolleeNonce.Value(),
            m1.enrolleeNonce.Length());

        //Extract the peer's public key.
        //First store the raw public key (to be used for e/rhash computation)
        memcpy(regInfo->pke, m1.publicKey.Value(), SIZE_PUB_KEY);

        if (regInfo->oobMode && !ValidatePeerPubKeyHash(
                    regInfo->pke, regInfo->peerPubKeyHash)) {
            throw "Public Key Hash validation failed";
        }

        //Next, allocate memory for the pub key
        regInfo->DH_PubKey_Peer = BN_new(); 
        if(!regInfo->DH_PubKey_Peer)
            throw WSC_ERR_OUTOFMEMORY;

        //Finally, import the raw key into the bignum datastructure
        if(BN_bin2bn(regInfo->pke, 
                    SIZE_PUB_KEY, 
                    regInfo->DH_PubKey_Peer) == NULL)
        {
            throw RPROT_ERR_CRYPTO;
        }

        regInfo->p_enrolleeInfo->authTypeFlags = m1.authTypeFlags.Value();
        regInfo->p_enrolleeInfo->encrTypeFlags = m1.encrTypeFlags.Value();
        regInfo->p_enrolleeInfo->connTypeFlags = m1.connTypeFlags.Value();
        regInfo->p_enrolleeInfo->configMethods = m1.configMethods.Value();
        regInfo->p_enrolleeInfo->scState       = m1.scState.Value();
        strncpy(regInfo->p_enrolleeInfo->manufacturer,
                m1.manufacturer.Value(), 
                SIZE_64_BYTES);
        strncpy(regInfo->p_enrolleeInfo->modelName,
                m1.modelName.Value(),
                SIZE_32_BYTES);
        strncpy(regInfo->p_enrolleeInfo->modelNumber,
                m1.modelNumber.Value(),
                SIZE_32_BYTES);
        strncpy(regInfo->p_enrolleeInfo->serialNumber,
                m1.serialNumber.Value(),
                SIZE_32_BYTES);
        regInfo->p_enrolleeInfo->primDeviceCategory = 
                                            m1.primDeviceType.categoryId;
        regInfo->p_enrolleeInfo->primDeviceOui = 
                                            m1.primDeviceType.oui;
        regInfo->p_enrolleeInfo->primDeviceSubCategory = 
                                            m1.primDeviceType.subCategoryId;
        strncpy(regInfo->p_enrolleeInfo->deviceName,
                m1.deviceName.Value(),
                SIZE_32_BYTES);
        regInfo->p_enrolleeInfo->rfBand = m1.rfBand.Value();
        regInfo->p_enrolleeInfo->assocState = m1.assocState.Value();
        regInfo->p_enrolleeInfo->devPwdId = m1.devPwdId.Value();

		// Verify that the device password ID indicated by the Enrollee corresponds 
		// to a supported password type.  For now, exclude rekey passwords and 
		// machine-specified passwords.
		if (WSC_DEVICEPWDID_MACHINE_SPEC == regInfo->p_enrolleeInfo->devPwdId ||
			WSC_DEVICEPWDID_REKEY == regInfo->p_enrolleeInfo->devPwdId) {
			regInfo->p_enrolleeInfo->configError = m1.configError.Value();
			// should probably define a config error for unknown Device Password.
			throw WSC_ERR_NOT_IMPLEMENTED;
		}

        regInfo->p_enrolleeInfo->configError = m1.configError.Value();
        regInfo->p_enrolleeInfo->osVersion = m1.osVersion.Value();

        //Check if the enrollee is an AP and set the b_ap flag accordingly
        //We need to set the flag only if the AP indicates that it is 
        //unconfigured so that we don't send it different configuration
        //parameters
        if((m1.primDeviceType.categoryId == WSC_DEVICE_TYPE_CAT_NW_INFRA) &&
           (m1.primDeviceType.subCategoryId == WSC_DEVICE_TYPE_SUB_CAT_NW_AP)&&
           (m1.scState.Value() == WSC_SCSTATE_UNCONFIGURED))
        {
            regInfo->p_enrolleeInfo->b_ap = true;
        }
        else
        {
            regInfo->p_enrolleeInfo->b_ap = false;
        }

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM1 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM1 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM1 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }

}//ProcessMessageM1

uint32 
CRegProtocol::BuildMessageM2(S_REGISTRATION_DATA *regInfo, 
                             BufferObj &msg,
                             void *encrSettings)
{
    uint8 message;
    
    //First, generate or gather the required data
    try
    {
        message = WSC_ID_MESSAGE_M2;

        //Registrar nonce
        RAND_bytes(regInfo->registrarNonce, SIZE_128_BITS);

        if(!regInfo->DHSecret)
        {
            BufferObj pubKey;
            GenerateDHKeyPair(&regInfo->DHSecret, pubKey);
        }



        //extract the DH public key
        int len = BN_bn2bin(regInfo->DHSecret->pub_key, regInfo->pkr);
        if(0 == len)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: BN_bn2bin: %s", 
                    ERR_error_string(ERR_get_error(), NULL)));
            throw RPROT_ERR_CRYPTO;
        }

        //****** KDK generation ******
        //1. generate the DH shared secret
        uint8 secret[SIZE_PUB_KEY];
        int secretLen = DH_compute_key(secret, 
                               regInfo->DH_PubKey_Peer, 
                               regInfo->DHSecret);
        if(secretLen == -1)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: DH_compute_key: %s", 
                    ERR_error_string(ERR_get_error(), NULL)));
            throw RPROT_ERR_CRYPTO;
        }        
        
        //2. compute the DHKey based on the DH secret
        uint8 DHKey[SIZE_256_BITS];
        if(SHA256(secret, secretLen, DHKey) == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: SHA256 calculation failed\n"));
            throw RPROT_ERR_CRYPTO;
        }

        //3.Append the enrollee nonce(N1), enrollee mac and registrar nonce(N2)
        BufferObj kdkData;
        kdkData.Append(SIZE_128_BITS, regInfo->enrolleeNonce);
        kdkData.Append(SIZE_MAC_ADDR, regInfo->p_enrolleeInfo->macAddr);
        kdkData.Append(SIZE_128_BITS, regInfo->registrarNonce);

        //4. now generate the KDK
        uint8 kdk[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), DHKey, SIZE_256_BITS, 
                kdkData.GetBuf(), kdkData.Length(), kdk, NULL) == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        //****** KDK generation ******

        //****** Derivation of AuthKey, KeyWrapKey and EMSK ******
        //1. declare and initialize the appropriate buffer objects
        BufferObj kdkBuf(kdk, SIZE_256_BITS);
        BufferObj pString((uint8 *)PERSONALIZATION_STRING, 
                          strlen(PERSONALIZATION_STRING));
        BufferObj keys;

        //2. call the key derivation function
        DeriveKey(kdkBuf, pString, KDF_KEY_BITS, keys);

        //3. split the key into the component keys and store them
        keys.Rewind(keys.Length());
		regInfo->authKey.Reset();
        regInfo->authKey.Append(SIZE_256_BITS, keys.Pos());
        keys.Advance(SIZE_256_BITS);

		regInfo->keyWrapKey.Reset();
        regInfo->keyWrapKey.Append(SIZE_128_BITS, keys.Pos());
        keys.Advance(SIZE_128_BITS);

		regInfo->emsk.Reset();
        regInfo->emsk.Append(SIZE_256_BITS, keys.Pos());
        //****** Derivation of AuthKey, KeyWrapKey and EMSK ******

        //****** Encrypted settings ******
        //encrypted settings.
        BufferObj esBuf;
        BufferObj cipherText, iv;

        if(encrSettings)
        {
#ifdef WSC_AP_DEFINED
            if(regInfo->p_enrolleeInfo->b_ap)
            {
                CTlvEsM8Ap *apEs = (CTlvEsM8Ap *)encrSettings;
                apEs->write(esBuf, regInfo->authKey);
            }
            else
#endif
            {
                CTlvEsM8Sta *staEs = (CTlvEsM8Sta *)encrSettings;
                staEs->write(esBuf, regInfo->authKey);
            }
            //Now encrypt the serialize Encrypted settings buffer
            EncryptData(esBuf, 
                        regInfo->keyWrapKey, 
                        regInfo->authKey, 
                        cipherText, 
                        iv);
        }

        //****** Encrypted settings ******

        //start assembling the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvRegistrarNonce(
                        WSC_ID_REGISTRAR_NONCE,
                        msg,
                        regInfo->registrarNonce, 
                        SIZE_128_BITS);
        CTlvUuid(
                        WSC_ID_UUID_R, 
                        msg, 
                        regInfo->p_registrarInfo->uuid, 
                        SIZE_UUID);
        CTlvPublicKey(
                        WSC_ID_PUBLIC_KEY, 
                        msg,
                        regInfo->pkr, 
                        SIZE_PUB_KEY);
        CTlvAuthTypeFlags(
                        WSC_ID_AUTH_TYPE_FLAGS, 
                        msg,
                        &regInfo->p_registrarInfo->authTypeFlags);
        CTlvEncrTypeFlags(
                        WSC_ID_ENCR_TYPE_FLAGS, 
                        msg,
                        &regInfo->p_registrarInfo->encrTypeFlags);
        CTlvConnTypeFlags(
                        WSC_ID_CONN_TYPE_FLAGS,
                        msg,
                        &regInfo->p_registrarInfo->connTypeFlags);
        CTlvConfigMethods(
                        WSC_ID_CONFIG_METHODS,
                        msg,
                        &regInfo->p_registrarInfo->configMethods);
		len = strlen(regInfo->p_registrarInfo->manufacturer) + 1;
        if (len > SIZE_64_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Manufacturer name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvManufacturer(
                        WSC_ID_MANUFACTURER,
                        msg,
                        regInfo->p_registrarInfo->manufacturer, 
                        len);
		len = strlen(regInfo->p_registrarInfo->modelName) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Model name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvModelName(
                        WSC_ID_MODEL_NAME,
                        msg,
                        regInfo->p_registrarInfo->modelName, 
                        len);
 		len = strlen(regInfo->p_registrarInfo->modelNumber) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Model number too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
       CTlvModelNumber(
                        WSC_ID_MODEL_NUMBER,
                        msg,
                        regInfo->p_registrarInfo->modelNumber, 
                        len);
 		len = strlen(regInfo->p_registrarInfo->serialNumber) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Serial number too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvSerialNum(
                        WSC_ID_SERIAL_NUM, 
                        msg,
                        regInfo->p_registrarInfo->serialNumber, 
                        len);

        CTlvPrimDeviceType primDev;
        primDev.categoryId = regInfo->p_registrarInfo->primDeviceCategory;
        primDev.oui = regInfo->p_registrarInfo->primDeviceOui;
        primDev.subCategoryId = regInfo->p_registrarInfo->primDeviceSubCategory;
        primDev.write(msg);

		len = strlen(regInfo->p_registrarInfo->deviceName) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Device name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvDeviceName(
                        WSC_ID_DEVICE_NAME,
                        msg,
                        regInfo->p_registrarInfo->deviceName, 
                        len);
        CTlvRfBand(
                        WSC_ID_RF_BAND, 
                        msg,
                        &regInfo->p_registrarInfo->rfBand);
        CTlvAssocState(
                        WSC_ID_ASSOC_STATE,
                        msg,
                        &regInfo->p_registrarInfo->assocState);
        CTlvConfigError(
                        WSC_ID_CONFIG_ERROR,
                        msg,
                        &regInfo->p_registrarInfo->configError);
        CTlvDevicePwdId(
                        WSC_ID_DEVICE_PWD_ID, 
                        msg,
                        &regInfo->p_registrarInfo->devPwdId);
        CTlvOsVersion(
                        WSC_ID_OS_VERSION,
                        msg,
                        &regInfo->p_registrarInfo->osVersion);
        //Skip optional attributes
        //Encrypted settings
        if(encrSettings)
        {
            CTlvEncrSettings encrSettings;
            encrSettings.iv = iv.GetBuf();
            encrSettings.ip_encryptedData = cipherText.GetBuf();
            encrSettings.encrDataLength = cipherText.Length();
            encrSettings.write(msg);
        }
        //No vendor extensions

        //Now calculate the hmac
        BufferObj hmacData;
		uint8 * buf = regInfo->inMsg.GetBuf();
		int buflen = regInfo->inMsg.Length();
        hmacData.Append(regInfo->inMsg.Length(), regInfo->inMsg.GetBuf());
        hmacData.Append(msg.Length(), msg.GetBuf());
		buflen = hmacData.Length();

        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                hmacData.GetBuf(), hmacData.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_AUTHENTICATOR,
                        msg,
                        hmac,
                        SIZE_64_BITS);

        //Store the outgoing message 
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM2 built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM2 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM2 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM2 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM2

//M2D generation

uint32 
CRegProtocol::BuildMessageM2D(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    uint8 message;
    
    //First, generate or gather the required data
    try
    {
        message = WSC_ID_MESSAGE_M2D;

        //Registrar nonce
        RAND_bytes(regInfo->registrarNonce, SIZE_128_BITS);

        //start assembling the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvRegistrarNonce(
                        WSC_ID_REGISTRAR_NONCE,
                        msg,
                        regInfo->registrarNonce, 
                        SIZE_128_BITS);
        CTlvUuid(
                        WSC_ID_UUID_R, 
                        msg, 
                        regInfo->p_registrarInfo->uuid, 
                        SIZE_UUID);
        CTlvAuthTypeFlags(
                        WSC_ID_AUTH_TYPE_FLAGS, 
                        msg,
                        &regInfo->p_registrarInfo->authTypeFlags);
        CTlvEncrTypeFlags(
                        WSC_ID_ENCR_TYPE_FLAGS, 
                        msg,
                        &regInfo->p_registrarInfo->encrTypeFlags);
        CTlvConnTypeFlags(
                        WSC_ID_CONN_TYPE_FLAGS,
                        msg,
                        &regInfo->p_registrarInfo->connTypeFlags);
        CTlvConfigMethods(
                        WSC_ID_CONFIG_METHODS,
                        msg,
                        &regInfo->p_registrarInfo->configMethods);
		int len = strlen(regInfo->p_registrarInfo->manufacturer) + 1;
        if (len > SIZE_64_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Manufacturer name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvManufacturer(
                        WSC_ID_MANUFACTURER,
                        msg,
                        regInfo->p_registrarInfo->manufacturer, 
                        len);
		len = strlen(regInfo->p_registrarInfo->modelName) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Model name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvModelName(
                        WSC_ID_MODEL_NAME,
                        msg,
                        regInfo->p_registrarInfo->modelName, 
                        len);
 		len = strlen(regInfo->p_registrarInfo->modelNumber) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Model number too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
       CTlvModelNumber(
                        WSC_ID_MODEL_NUMBER,
                        msg,
                        regInfo->p_registrarInfo->modelNumber, 
                        len);
 		len = strlen(regInfo->p_registrarInfo->serialNumber) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Serial number too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvSerialNum(
                        WSC_ID_SERIAL_NUM, 
                        msg,
                        regInfo->p_registrarInfo->serialNumber, 
                        len);

        CTlvPrimDeviceType primDev;
        primDev.categoryId = regInfo->p_registrarInfo->primDeviceCategory;
        primDev.oui = regInfo->p_registrarInfo->primDeviceOui;
        primDev.subCategoryId = regInfo->p_registrarInfo->primDeviceSubCategory;
        primDev.write(msg);

		len = strlen(regInfo->p_registrarInfo->deviceName) + 1;
        if (len > SIZE_32_BYTES) {
            TUTRACE((TUTRACE_ERR, "RPROTO: Device name too big\n"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        CTlvDeviceName(
                        WSC_ID_DEVICE_NAME,
                        msg,
                        regInfo->p_registrarInfo->deviceName, 
                        len);        
		CTlvRfBand(
                        WSC_ID_RF_BAND, 
                        msg,
                        &regInfo->p_registrarInfo->rfBand);
        CTlvAssocState(
                        WSC_ID_ASSOC_STATE,
                        msg,
                        &regInfo->p_registrarInfo->assocState);
        CTlvConfigError(
                        WSC_ID_CONFIG_ERROR,
                        msg,
                        &regInfo->p_registrarInfo->configError);

		// Per 1.0b spec, M2D does not include Device Password Id.
        //CTlvDevicePwdId(
        //                WSC_ID_DEVICE_PWD_ID, 
        //                msg,
        //                &regInfo->p_registrarInfo->devPwdId);

        CTlvOsVersion(
                        WSC_ID_OS_VERSION,
                        msg,
                        &regInfo->p_registrarInfo->osVersion);
        
		//No optional attributes

        //Store the outgoing message 
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM2D built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM2 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM2 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM2 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM2D

#endif // WPS_REGISTRAR 

char * NokiaM2 = "EEoAAQEQIgABBRAaABB1Fpt52UL4SNlNBqAmVQGFEDkAEOTDAnJT\
3gm9fQ/BPRyxug4QSAAQ4WBFpB5WfE6h1BVQ4y8qOxAyAMC+aAUB\
v/Qkt3A9g2ojJhIrSHLML2tGZjiKr9tntmrM/OTAj+JmoC9ldSh6\
9Xo5lpvLnpFiscNXI4KWRLvcrdPHzUjYTDPj43sE9+JqDaRmE6pX\
Mkmu/v2YnXysIXkpnb1llrhfxNCteXMKzi+HZP+BKU8LKEGXWzS9\
np6zxVk29OhlD/lmxmbgoQdpEM0mRQOCXwPsuxK/ZBBmLTk5Inws\
RLMpUvl/wmVv7/fyeJFegHBn+ucFrak9Tp7gAjRv9qQQBAACAAIQ\
EAACAAQQDQABAhAhAAlVbmRlZmluZWQQIwAJVW5kZWZpbmVkECQA\
CVVuZGVmaW5lZBBCAAlVbmRlZmluZWQQVAAIMAAAAAAAAAAQEQAJ\
VW5kZWZpbmVkEAgAAgAEEDwAAQEQAgACAAUQCQACAAAQEgACAAAQ\
LQAEAQIDBBAFAAgllvruYpbBtA==";

unsigned char *NokiaM2Buf;

extern "C" {
int ILibBase64Decode(unsigned char* input, const int inputlen, unsigned char** output);
}

#ifdef WPS_ENROLLEE
uint32 CRegProtocol::ProcessMessageM2(S_REGISTRATION_DATA *regInfo, 
                                      BufferObj &msg, 
                                      void **encrSettings)
{
    S_WSC_M2 m2;
    uint8 *Pos;

	/* enable this code to manually insert a M2 message
	int i = ILibBase64Decode((unsigned char *) NokiaM2, strlen(NokiaM2) + 1, &NokiaM2Buf); // TEST
	msg.Reset(); // TEST
	msg.Append(i,(uint8 *) NokiaM2Buf); // TEST
	int j = msg.Length();
	msg.Rewind();
	*/

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM2: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        m2.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m2.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //Don't deserialize incompatible messages!
        if((version & 0xF0) != (m2.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M2 != m2.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m2.enrolleeNonce = CTlvEnrolleeNonce(WSC_ID_ENROLLEE_NONCE, 
                                             msg, SIZE_128_BITS);
        m2.registrarNonce = CTlvRegistrarNonce(WSC_ID_REGISTRAR_NONCE, 
                                               msg, SIZE_128_BITS);
        m2.uuid          = CTlvUuid(WSC_ID_UUID_R, msg, SIZE_UUID);
        m2.publicKey     = CTlvPublicKey(WSC_ID_PUBLIC_KEY, msg, SIZE_PUB_KEY);
        m2.authTypeFlags = CTlvAuthTypeFlags(WSC_ID_AUTH_TYPE_FLAGS,msg);
        m2.encrTypeFlags = CTlvEncrTypeFlags(WSC_ID_ENCR_TYPE_FLAGS,msg);
        m2.connTypeFlags = CTlvConnTypeFlags(WSC_ID_CONN_TYPE_FLAGS,msg);
        m2.configMethods = CTlvConfigMethods(WSC_ID_CONFIG_METHODS, msg);
        m2.manufacturer  = CTlvManufacturer(WSC_ID_MANUFACTURER, 
                                            msg, SIZE_64_BYTES);
        m2.modelName     = CTlvModelName(WSC_ID_MODEL_NAME, msg, SIZE_32_BYTES);
        m2.modelNumber   = CTlvModelNumber(WSC_ID_MODEL_NUMBER, 
                                           msg, SIZE_32_BYTES);
        m2.serialNumber  = CTlvSerialNum(WSC_ID_SERIAL_NUM, msg, SIZE_32_BYTES);
        m2.primDeviceType.parse(msg);
        m2.deviceName    = CTlvDeviceName(WSC_ID_DEVICE_NAME, 
                                          msg, SIZE_32_BYTES);
        m2.rfBand        = CTlvRfBand(WSC_ID_RF_BAND, msg);
        m2.assocState    = CTlvAssocState(WSC_ID_ASSOC_STATE, msg);
        m2.configError   = CTlvConfigError(WSC_ID_CONFIG_ERROR, msg);
        m2.devPwdId      = CTlvDevicePwdId(WSC_ID_DEVICE_PWD_ID, msg);
        m2.osVersion     = CTlvOsVersion(WSC_ID_OS_VERSION, msg);

        //skip the vendor extensions and any other optional TLVs until we get to the authenticator
        while(WSC_ID_AUTHENTICATOR != msg.NextType())
        {
			// optional encrypted settings
			if(WSC_ID_ENCR_SETTINGS == msg.NextType() && m2.encrSettings.encrDataLength == 0)
			{ // only process the first encrypted settings attribute encountered
				m2.encrSettings.parse(msg);
				Pos = msg.Pos();
			} else { //advance past the TLV
				Pos = msg.Advance( sizeof(S_WSC_TLV_HEADER) + 
								WscNtohs(*(uint16 *)(msg.Pos()+sizeof(uint16))) );
			}
            
            //If Advance returned NULL, it means there's no more data in the
            //buffer. This is an error.
            if(Pos == NULL)
                throw RPROT_ERR_REQD_TLV_MISSING;
        }

        m2.authenticator = CTlvAuthenticator(
                                            WSC_ID_AUTHENTICATOR,
                                            msg,
                                            SIZE_64_BITS);

        //Now start processing the message

        //confirm the enrollee nonce
        if(memcmp(regInfo->enrolleeNonce, 
                m2.enrolleeNonce.Value(),
                m2.enrolleeNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect enrollee nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }
        TUTRACE((TUTRACE_VERBOSE, "Correct Enrollee nonce\n"));

        //to verify the hmac, we need to process the nonces, generate
        //the DH secret, the KDK and finally the auth key
        memcpy(regInfo->registrarNonce, 
            m2.registrarNonce.Value(),
            m2.registrarNonce.Length());

        //read the registrar's public key 
        //First store the raw public key (to be used for e/rhash computation)
        memcpy(regInfo->pkr, m2.publicKey.Value(), SIZE_PUB_KEY);

        if (regInfo->oobMode && !ValidatePeerPubKeyHash(
                    regInfo->pkr, regInfo->peerPubKeyHash)) {
            throw "Public Key Hash validation failed";
        }
        TUTRACE((TUTRACE_VERBOSE, "Correct peer PK hash\n"));

        //Next, allocate memory for the pub key
        regInfo->DH_PubKey_Peer = BN_new(); 
        if(!regInfo->DH_PubKey_Peer)
            throw WSC_ERR_OUTOFMEMORY;

        //Finally, import the raw key into the bignum datastructure
        if(BN_bin2bn(regInfo->pkr, 
                     SIZE_PUB_KEY, 
                     regInfo->DH_PubKey_Peer) == NULL)
        {
            throw RPROT_ERR_CRYPTO;
        }

        //****** KDK generation ******
        //1. generate the DH shared secret
        uint8 secret[SIZE_PUB_KEY];

        int secretLen = DH_compute_key(secret, 
                               regInfo->DH_PubKey_Peer, 
                               regInfo->DHSecret);
        if(secretLen == -1)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: DH_compute_key: %s", 
                    ERR_error_string(ERR_get_error(), NULL)));
            throw RPROT_ERR_CRYPTO;
        }   
		if (TUTRACELEVEL & TUVERBOSE && g_verbose)
		{
			printf("\nDH Secret (first 256 bits only): ");
			for (int i = 0; i < BUF_SIZE_256_BITS; i++) {
				printf("%2x ",secret[i]);
			}
		}

        //2. compute the DHKey based on the DH secret
        uint8 DHKey[SIZE_256_BITS];
        if(SHA256(secret, secretLen, DHKey) == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: SHA256 calculation failed\n"));
            throw RPROT_ERR_CRYPTO;
        }

		if (TUTRACELEVEL & TUVERBOSE && g_verbose)
		{
		printf("\nHash of DH Secret used to generate KDK: ");
			for (int i = 0; i < BUF_SIZE_256_BITS; i++) {
				printf("%2x ",DHKey[i]);
			}
		}

        //3. Append the enrollee nonce(N1), enrollee mac and registrar nonce(N2)
        BufferObj kdkData;
        kdkData.Append(SIZE_128_BITS, regInfo->enrolleeNonce);
        kdkData.Append(SIZE_MAC_ADDR, regInfo->p_enrolleeInfo->macAddr);
        kdkData.Append(SIZE_128_BITS, regInfo->registrarNonce);

        //4. now generate the KDK
        uint8 kdk[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), DHKey, SIZE_256_BITS, 
                kdkData.GetBuf(), kdkData.Length(), kdk, NULL) == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }
        //****** KDK generation ******

        //****** Derivation of AuthKey, KeyWrapKey and EMSK ******
        //1. declare and initialize the appropriate buffer objects
        BufferObj kdkBuf(kdk, SIZE_256_BITS);
        BufferObj pString((uint8 *)PERSONALIZATION_STRING, 
                          strlen(PERSONALIZATION_STRING));
        BufferObj keys;

        //2. call the key derivation function
        DeriveKey(kdkBuf, pString, KDF_KEY_BITS, keys);

		if (TUTRACELEVEL & TUVERBOSE && g_verbose)
		{
			printf("\nKDK: ");
				for (int i = 0; i < BUF_SIZE_256_BITS; i++) {
					printf("%2x ",kdkBuf.GetBuf()[i]);
				}
			printf("\nauthKey: ");
				for (int i = 0; i < BUF_SIZE_256_BITS; i++) {
					printf("%2x ",keys.GetBuf()[i]);
				}
			printf("\n");
		}

        //3. split the key into the component keys and store them
        keys.Rewind(keys.Length());
		regInfo->authKey.Reset();
        regInfo->authKey.Append(SIZE_256_BITS, keys.Pos());
        keys.Advance(SIZE_256_BITS);

 		regInfo->keyWrapKey.Reset();
		regInfo->keyWrapKey.Append(SIZE_128_BITS, keys.Pos());
        keys.Advance(SIZE_128_BITS);

		regInfo->emsk.Reset();
		regInfo->emsk.Append(SIZE_256_BITS, keys.Pos());
        //****** Derivation of AuthKey, KeyWrapKey and EMSK ******

        //****** HMAC validation ******
        BufferObj hmacData;
        //append the last message sent
		int enrolleeMsgLen = regInfo->outMsg.Length();
        hmacData.Append(regInfo->outMsg.Length(), regInfo->outMsg.GetBuf());
        //append the current message. Don't append the last TLV (auth)
        hmacData.Append(
            msg.Length()-(sizeof(S_WSC_TLV_HEADER)+m2.authenticator.Length()),
            msg.GetBuf());

        if(!ValidateMac(hmacData, m2.authenticator.Value(), regInfo->authKey))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC validation failed"));
            throw RPROT_ERR_INVALID_VALUE;
        }

        //****** HMAC validation ******

        //Now we can proceed with copying out and processing the other data

        //First, check if we need to allocate registrarInfo
        //Master Control might not have allocated memory if it didn't have
        //any info about the registrar
        if(!regInfo->p_registrarInfo)
            regInfo->p_registrarInfo = new S_DEVICE_INFO;

        memcpy(regInfo->p_registrarInfo->uuid, 
            m2.uuid.Value(), 
            m2.uuid.Length());

        regInfo->p_registrarInfo->authTypeFlags = m2.authTypeFlags.Value();
        regInfo->p_registrarInfo->encrTypeFlags = m2.encrTypeFlags.Value();
        regInfo->p_registrarInfo->connTypeFlags = m2.connTypeFlags.Value();
        regInfo->p_registrarInfo->configMethods = m2.configMethods.Value();
        strncpy(regInfo->p_registrarInfo->manufacturer,
                m2.manufacturer.Value(), 
                SIZE_64_BYTES);
        strncpy(regInfo->p_registrarInfo->modelName,
                m2.modelName.Value(),
                SIZE_32_BYTES);
        strncpy(regInfo->p_registrarInfo->serialNumber,
                m2.serialNumber.Value(),
                SIZE_32_BYTES);
        regInfo->p_registrarInfo->primDeviceCategory = 
                                            m2.primDeviceType.categoryId;
        regInfo->p_registrarInfo->primDeviceOui = 
                                            m2.primDeviceType.oui;
        regInfo->p_registrarInfo->primDeviceSubCategory = 
                                            m2.primDeviceType.subCategoryId;
        strncpy(regInfo->p_registrarInfo->deviceName,
                m2.deviceName.Value(),
                SIZE_32_BYTES);
        regInfo->p_registrarInfo->rfBand = m2.rfBand.Value();
        regInfo->p_registrarInfo->assocState = m2.assocState.Value();
        regInfo->p_registrarInfo->configError = m2.configError.Value();
        regInfo->p_registrarInfo->devPwdId = m2.devPwdId.Value();
        regInfo->p_registrarInfo->osVersion = m2.osVersion.Value();

        //****** extract encrypted settings ******
        if(m2.encrSettings.encrDataLength)
        {
            BufferObj cipherText(m2.encrSettings.ip_encryptedData, 
                                 m2.encrSettings.encrDataLength);
            BufferObj iv(m2.encrSettings.iv, SIZE_128_BITS);
            BufferObj plainText;
            
            DecryptData(cipherText, 
                        iv, 
                        regInfo->keyWrapKey, 
                        regInfo->authKey, 
                        plainText);
#ifdef WSC_AP_DEFINED
            if(regInfo->p_enrolleeInfo->b_ap)
            {
                CTlvEsM8Ap *esAP = new CTlvEsM8Ap();
                esAP->parse(plainText, regInfo->authKey, true);
                *encrSettings = (void *)esAP;
            }
            else
#endif
            {
                CTlvEsM8Sta *esSta = new CTlvEsM8Sta();
                esSta->parse(plainText, regInfo->authKey, true);
                *encrSettings = (void *)esSta;
            }
        }
        //****** extract encrypted settings ******

        //now set the registrar's b_ap flag. If the local enrollee is an ap,
        //the registrar shouldn't be one
        if(regInfo->p_enrolleeInfo->b_ap)
            regInfo->p_registrarInfo->b_ap = true;

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_VERBOSE, "Successfully processed M2\n"));

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM2 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM2 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM2 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageM2

uint32 CRegProtocol::ProcessMessageM2D(S_REGISTRATION_DATA *regInfo, 
                                       BufferObj &msg)
{
    S_WSC_M2 m2d;
    uint8 *Pos;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM2D: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        m2d.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m2d.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //Don't deserialize incompatible messages!
        if((version & 0xF0) != (m2d.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M2D != m2d.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m2d.enrolleeNonce = CTlvEnrolleeNonce(WSC_ID_ENROLLEE_NONCE, 
                                              msg, SIZE_128_BITS);
        m2d.registrarNonce = CTlvRegistrarNonce(WSC_ID_REGISTRAR_NONCE, 
                                                msg, SIZE_128_BITS);
        m2d.uuid          = CTlvUuid(WSC_ID_UUID_R, msg, SIZE_UUID);
		// No Public Key in M2D
        m2d.authTypeFlags = CTlvAuthTypeFlags(WSC_ID_AUTH_TYPE_FLAGS,msg);
        m2d.encrTypeFlags = CTlvEncrTypeFlags(WSC_ID_ENCR_TYPE_FLAGS,msg);
        m2d.connTypeFlags = CTlvConnTypeFlags(WSC_ID_CONN_TYPE_FLAGS,msg);
        m2d.configMethods = CTlvConfigMethods(WSC_ID_CONFIG_METHODS, msg);
        m2d.manufacturer  = CTlvManufacturer(WSC_ID_MANUFACTURER, 
                                             msg, SIZE_64_BYTES);
        m2d.modelName     = CTlvModelName(WSC_ID_MODEL_NAME, 
                                          msg, SIZE_32_BYTES);
        m2d.modelNumber   = CTlvModelNumber(WSC_ID_MODEL_NUMBER, 
                                            msg, SIZE_32_BYTES);
        m2d.serialNumber  = CTlvSerialNum(WSC_ID_SERIAL_NUM, 
                                          msg, SIZE_32_BYTES);
        m2d.primDeviceType.parse(msg);
        m2d.deviceName    = CTlvDeviceName(WSC_ID_DEVICE_NAME, 
                                           msg, SIZE_32_BYTES);
        m2d.rfBand        = CTlvRfBand(WSC_ID_RF_BAND, msg);
        m2d.assocState    = CTlvAssocState(WSC_ID_ASSOC_STATE, msg);
        m2d.configError   = CTlvConfigError(WSC_ID_CONFIG_ERROR, msg);

		// Per 1.0b version of spec, M2D no longer includes Device Password ID.
        // m2d.devPwdId      = CTlvDevicePwdId(WSC_ID_DEVICE_PWD_ID, msg);

        // ignore any other TLVs in the message

        //Now start processing the message

        //confirm the enrollee nonce
        if(memcmp(regInfo->enrolleeNonce, 
                m2d.enrolleeNonce.Value(),
                m2d.enrolleeNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect enrollee nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

        memcpy(regInfo->registrarNonce, 
            m2d.registrarNonce.Value(),
            m2d.registrarNonce.Length());

        //First, check if we need to allocate registrarInfo
        //Master Control might not have allocated memory if it didn't have
        //any info about the registrar
        if(!regInfo->p_registrarInfo)
            regInfo->p_registrarInfo = new S_DEVICE_INFO;

        //Now we can proceed with copying out and processing the other data
        memcpy(regInfo->p_registrarInfo->uuid, 
            m2d.uuid.Value(), 
            m2d.uuid.Length());

		// No public key in M2D

        if(0 == (m2d.authTypeFlags.Value() && 0x3F))
            throw RPROT_ERR_INCOMPATIBLE;

        regInfo->p_registrarInfo->authTypeFlags = m2d.authTypeFlags.Value();

        if(0 == (m2d.encrTypeFlags.Value() && 0x0F))
            throw RPROT_ERR_INCOMPATIBLE;

        regInfo->p_registrarInfo->encrTypeFlags = m2d.encrTypeFlags.Value();
        regInfo->p_registrarInfo->connTypeFlags = m2d.connTypeFlags.Value();
        regInfo->p_registrarInfo->configMethods = m2d.configMethods.Value();
        strncpy(regInfo->p_registrarInfo->manufacturer,
                m2d.manufacturer.Value(), 
                SIZE_32_BYTES);
        strncpy(regInfo->p_registrarInfo->modelName,
                m2d.modelName.Value(),
                SIZE_32_BYTES);
        strncpy(regInfo->p_registrarInfo->serialNumber,
                m2d.serialNumber.Value(),
                SIZE_32_BYTES);
        regInfo->p_registrarInfo->primDeviceCategory = 
                                            m2d.primDeviceType.categoryId;
        regInfo->p_registrarInfo->primDeviceOui = 
                                            m2d.primDeviceType.oui;
        regInfo->p_registrarInfo->primDeviceSubCategory = 
                                            m2d.primDeviceType.subCategoryId;
        strncpy(regInfo->p_registrarInfo->deviceName,
                m2d.deviceName.Value(),
                SIZE_32_BYTES);
        regInfo->p_registrarInfo->rfBand = m2d.rfBand.Value();
        regInfo->p_registrarInfo->assocState = m2d.assocState.Value();
        regInfo->p_registrarInfo->configError = m2d.configError.Value();
        // No Dev Pwd Id in this message: regInfo->p_registrarInfo->devPwdId = m2d.devPwdId.Value();

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM2D generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM2D generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM2D generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageM2D

uint32 
CRegProtocol::BuildMessageM3(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    uint8 message;
    uint8 hashBuf[SIZE_256_BITS];
    
    //First, generate or gather the required data
    try
    {
        message = WSC_ID_MESSAGE_M3;

        //****** PSK1 and PSK2 generation ******
        uint8 *pwdPtr = regInfo->password.GetBuf();
        int pwdLen = regInfo->password.Length();

        //Hash 1st half of passwd. If it is an odd length, the extra byte
        //goes along with the first half
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                 pwdPtr, (pwdLen/2)+(pwdLen%2), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for PSK1\n"));
            throw RPROT_ERR_CRYPTO;
        }
        //copy first 128 bits into psk1;
        memcpy(regInfo->psk1, hashBuf, SIZE_128_BITS);
        
        //Hash 2nd half of passwd
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                 pwdPtr+(pwdLen/2)+(pwdLen%2), pwdLen/2, hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for PSK2\n"));
            throw RPROT_ERR_CRYPTO;
        }
        //copy first 128 bits into psk2;
        memcpy(regInfo->psk2, hashBuf, SIZE_128_BITS);
        //****** PSK1 and PSK2 generation ******

        //****** EHash1 and EHash2 generation ******
        RAND_bytes(regInfo->es1, SIZE_128_BITS);
        RAND_bytes(regInfo->es2, SIZE_128_BITS);

        BufferObj ehashBuf;
        ehashBuf.Append(SIZE_128_BITS, regInfo->es1);
        ehashBuf.Append(SIZE_128_BITS, regInfo->psk1);
        ehashBuf.Append(SIZE_PUB_KEY, regInfo->pke);
        ehashBuf.Append(SIZE_PUB_KEY, regInfo->pkr);

        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
            ehashBuf.GetBuf(), ehashBuf.Length(), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for EHash1\n"));
            throw RPROT_ERR_CRYPTO;
        }

        memcpy(regInfo->eHash1, hashBuf, SIZE_256_BITS);

        ehashBuf.Reset();
        ehashBuf.Append(SIZE_128_BITS, regInfo->es2);
        ehashBuf.Append(SIZE_128_BITS, regInfo->psk2);
        ehashBuf.Append(SIZE_PUB_KEY, regInfo->pke);
        ehashBuf.Append(SIZE_PUB_KEY, regInfo->pkr);

        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
            ehashBuf.GetBuf(), ehashBuf.Length(), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for EHash2\n"));
            throw RPROT_ERR_CRYPTO;
        }

        memcpy(regInfo->eHash2, hashBuf, SIZE_256_BITS);
        //****** EHash1 and EHash2 generation ******

        //Now assemble the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvRegistrarNonce(
                        WSC_ID_REGISTRAR_NONCE,
                        msg,
                        regInfo->registrarNonce, 
                        SIZE_128_BITS);
        CTlvHash(WSC_ID_E_HASH1,
                 msg,
                 regInfo->eHash1,
                 SIZE_256_BITS);
        CTlvHash(WSC_ID_E_HASH2,
                 msg,
                 regInfo->eHash2,
                 SIZE_256_BITS);
        //No vendor extension

        //Calculate the hmac
        BufferObj hmacData;
        hmacData.Append(regInfo->inMsg.Length(), regInfo->inMsg.GetBuf());
        hmacData.Append(msg.Length(), msg.GetBuf());

        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                hmacData.GetBuf(), hmacData.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_AUTHENTICATOR,
                        msg,
                        hmac,
                        SIZE_64_BITS);

        //Store the outgoing message 
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM3 built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM3 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM3 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM3 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM3
#endif // WPS_ENROLLEE  

#ifdef WPS_REGISTRAR
uint32 
CRegProtocol::ProcessMessageM3(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    S_WSC_M3 m3;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM3: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        m3.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m3.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //Don't deserialize incompatible messages!
        if((version & 0xF0) != (m3.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M3 != m3.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m3.registrarNonce = CTlvRegistrarNonce(WSC_ID_REGISTRAR_NONCE, 
                                               msg, SIZE_128_BITS);
        m3.eHash1       = CTlvHash(WSC_ID_E_HASH1, msg, SIZE_256_BITS);
        m3.eHash2       = CTlvHash(WSC_ID_E_HASH2, msg, SIZE_256_BITS);

        //skip all optional attributes until we get to the authenticator
        while(WSC_ID_AUTHENTICATOR != msg.NextType())
        {
            //advance past the TLV
            uint8 *Pos = msg.Advance( sizeof(S_WSC_TLV_HEADER) + 
                            WscNtohs(*(uint16 *)(msg.Pos()+sizeof(uint16))) );
            
            //If Advance returned NULL, it means there's no more data in the
            //buffer. This is an error.
            if(Pos == NULL)
                throw RPROT_ERR_REQD_TLV_MISSING;
        }

        m3.authenticator = CTlvAuthenticator(
                                            WSC_ID_AUTHENTICATOR,
                                            msg,
                                            SIZE_64_BITS);

        //Now start validating the message

        //confirm the registrar nonce
        if(memcmp(regInfo->registrarNonce, 
                m3.registrarNonce.Value(),
                m3.registrarNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect registrar nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

        //****** HMAC validation ******
        BufferObj hmacData;
        //append the last message sent
        hmacData.Append(regInfo->outMsg.Length(), regInfo->outMsg.GetBuf());
        //append the current message. Don't append the last TLV (auth)
        hmacData.Append(
            msg.Length()-(sizeof(S_WSC_TLV_HEADER)+m3.authenticator.Length()),
            msg.GetBuf());

        if(!ValidateMac(hmacData, m3.authenticator.Value(), regInfo->authKey))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC validation failed"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        //****** HMAC validation ******

        //Now copy the relevant data
        memcpy(regInfo->eHash1, m3.eHash1.Value(), SIZE_256_BITS);
        memcpy(regInfo->eHash2, m3.eHash2.Value(), SIZE_256_BITS);

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM3 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM3 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM3 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageM3

uint32 
CRegProtocol::BuildMessageM4(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    uint8 message;
    uint8 hashBuf[SIZE_256_BITS];
    
    try
    {
        //First, generate or gather the required data
        message = WSC_ID_MESSAGE_M4;

        //****** PSK1 and PSK2 generation ******
        uint8 *pwdPtr = regInfo->password.GetBuf();
        int pwdLen = regInfo->password.Length();

        //Hash 1st half of passwd. If it is an odd length, the extra byte
        //goes along with the first half
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                 pwdPtr, (pwdLen/2)+(pwdLen%2), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for PSK1\n"));
            throw RPROT_ERR_CRYPTO;
        }
        //copy first 128 bits into psk1;
        memcpy(regInfo->psk1, hashBuf, SIZE_128_BITS);
        
        //Hash 2nd half of passwd
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                 pwdPtr+(pwdLen/2)+(pwdLen%2), pwdLen/2, hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for PSK2\n"));
            throw RPROT_ERR_CRYPTO;
        }
        //copy first 128 bits into psk2;
        memcpy(regInfo->psk2, hashBuf, SIZE_128_BITS);
        //****** PSK1 and PSK2 generation ******

        //****** RHash1 and RHash2 generation ******
        RAND_bytes(regInfo->rs1, SIZE_128_BITS);
        RAND_bytes(regInfo->rs2, SIZE_128_BITS);

        BufferObj rhashBuf;
        rhashBuf.Append(SIZE_128_BITS, regInfo->rs1);
        rhashBuf.Append(SIZE_128_BITS, regInfo->psk1);
        rhashBuf.Append(SIZE_PUB_KEY, regInfo->pke);
        rhashBuf.Append(SIZE_PUB_KEY, regInfo->pkr);

        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
            rhashBuf.GetBuf(), rhashBuf.Length(), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for RHash1\n"));
            throw RPROT_ERR_CRYPTO;
        }

        memcpy(regInfo->rHash1, hashBuf, SIZE_256_BITS);

        rhashBuf.Reset();
        rhashBuf.Append(SIZE_128_BITS, regInfo->rs2);
        rhashBuf.Append(SIZE_128_BITS, regInfo->psk2);
        rhashBuf.Append(SIZE_PUB_KEY, regInfo->pke);
        rhashBuf.Append(SIZE_PUB_KEY, regInfo->pkr);

        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
            rhashBuf.GetBuf(), rhashBuf.Length(), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for RHash2\n"));
            throw RPROT_ERR_CRYPTO;
        }

        memcpy(regInfo->rHash2, hashBuf, SIZE_256_BITS);
        //****** RHash1 and RHash2 generation ******

        //encrypted settings.
        BufferObj encData;
        CTlvEsNonce rsNonce;
        rsNonce.nonce.Set(WSC_ID_R_SNONCE1,
                                    regInfo->rs1,
                                    SIZE_128_BITS);
        rsNonce.write(encData, regInfo->authKey);

        BufferObj cipherText, iv;
        EncryptData(encData, 
                    regInfo->keyWrapKey, 
                    regInfo->authKey, 
                    cipherText, 
                    iv);


        //Now assemble the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvHash(WSC_ID_R_HASH1,
                 msg,
                 regInfo->rHash1,
                 SIZE_256_BITS);
        CTlvHash(WSC_ID_R_HASH2,
                 msg,
                 regInfo->rHash2,
                 SIZE_256_BITS);
        CTlvEncrSettings encrSettings;
        encrSettings.iv = iv.GetBuf();
        encrSettings.ip_encryptedData = cipherText.GetBuf();
        encrSettings.encrDataLength = cipherText.Length();
        encrSettings.write(msg);
        //No vendor extension

        //Calculate the hmac
        BufferObj hmacData;
        hmacData.Append(regInfo->inMsg.Length(), regInfo->inMsg.GetBuf());
        hmacData.Append(msg.Length(), msg.GetBuf());

        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                hmacData.GetBuf(), hmacData.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_AUTHENTICATOR,
                        msg,
                        hmac,
                        SIZE_64_BITS);

        //Store the outgoing message 
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM4 built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM4 exiting with error %d\n",
                               err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM4 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessage4 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM4
#endif // WPS_REGISTRAR 

#ifdef WPS_ENROLLEE
uint32 
CRegProtocol::ProcessMessageM4(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    S_WSC_M4 m4;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM4: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        m4.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m4.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //Don't deserialize incompatible messages!
        if((version & 0xF0) != (m4.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M4 != m4.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m4.enrolleeNonce = CTlvEnrolleeNonce(WSC_ID_ENROLLEE_NONCE, 
                                             msg, SIZE_128_BITS);


        m4.rHash1       = CTlvHash(WSC_ID_R_HASH1, msg, SIZE_256_BITS);
        m4.rHash2       = CTlvHash(WSC_ID_R_HASH2, msg, SIZE_256_BITS);
        m4.encrSettings.parse(msg);

        //skip all optional attributes until we get to the authenticator
        while(WSC_ID_AUTHENTICATOR != msg.NextType())
        {
            //advance past the TLV
            uint8 *Pos = msg.Advance( sizeof(S_WSC_TLV_HEADER) + 
                            WscNtohs(*(uint16 *)(msg.Pos()+sizeof(uint16))) );
            
            //If Advance returned NULL, it means there's no more data in the
            //buffer. This is an error.
            if(Pos == NULL)
                throw RPROT_ERR_REQD_TLV_MISSING;
        }

        m4.authenticator = CTlvAuthenticator(
                                            WSC_ID_AUTHENTICATOR,
                                            msg,
                                            SIZE_64_BITS);

        //Now start validating the message

        //confirm the enrollee nonce
        if(memcmp(regInfo->enrolleeNonce, 
                m4.enrolleeNonce.Value(),
                m4.enrolleeNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect enrollee nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }
        TUTRACE((TUTRACE_VERBOSE, "Nonce is correct\n"));

        //****** HMAC validation ******
        BufferObj hmacData;
        //append the last message sent
        hmacData.Append(regInfo->outMsg.Length(), regInfo->outMsg.GetBuf());
        //append the current message. Don't append the last TLV (auth)
        hmacData.Append(
            msg.Length()-(sizeof(S_WSC_TLV_HEADER)+m4.authenticator.Length()),
            msg.GetBuf());

        if(!ValidateMac(hmacData, m4.authenticator.Value(), regInfo->authKey))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC validation failed"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        TUTRACE((TUTRACE_VERBOSE, "HMAC matches\n"));

		//****** HMAC validation ******

        //Now copy the relevant data
        memcpy(regInfo->rHash1, m4.rHash1.Value(), SIZE_256_BITS);
        memcpy(regInfo->rHash2, m4.rHash2.Value(), SIZE_256_BITS);

        //****** extract encrypted settings ******
        BufferObj cipherText(m4.encrSettings.ip_encryptedData, 
                             m4.encrSettings.encrDataLength);
        TUTRACE((TUTRACE_VERBOSE, "got cipherText with length %d, ", m4.encrSettings.encrDataLength));

        BufferObj iv(m4.encrSettings.iv, SIZE_128_BITS);
        BufferObj plainText;
        
        DecryptData(cipherText, 
                    iv, 
                    regInfo->keyWrapKey, 
                    regInfo->authKey, 
                    plainText);

		if (TUTRACELEVEL & TUVERBOSE && g_verbose)
		{
			printf("decrypted data\nPlaintext is: ");
			for (int i = 0; i < plainText.Length(); i++) {
				printf("%02x ",plainText.GetBuf()[i]);
			}
			printf("\n");
		}

        CTlvEsNonce rNonce;
        rNonce.parse(WSC_ID_R_SNONCE1, plainText, regInfo->authKey);
        //****** extract encrypted settings ******

        //****** RHash1 validation ******
        //1. Save RS1
        memcpy(regInfo->rs1, rNonce.nonce.Value(), rNonce.nonce.Length());

		if (TUTRACELEVEL & TUVERBOSE && g_verbose)
		{
			printf("parsed R_SNONCE1, ***********\n\nhashed value of RS1 is computed from:");
			printf("\nRS1: ");
			for (int i = 0; i < SIZE_128_BITS; i++) {
				printf("%02x ",regInfo->rs1[i]);
			}
			printf("\nPSK1: ");
			for (int i = 0; i < SIZE_128_BITS; i++) {
				printf("%02x ",regInfo->psk1[i]);
			}
			printf("\nPKE: ");
			for (int i = 0; i < SIZE_PUB_KEY; i++) {
				printf("%02x ",regInfo->pke[i]);
			}
			printf("\nPKR: ");
			for (int i = 0; i < SIZE_PUB_KEY; i++) {
				printf("%02x ",regInfo->pkr[i]);
			}
			printf("\n\n");
		}

        //2. prepare the buffer
        BufferObj rhashBuf;
        rhashBuf.Append(SIZE_128_BITS, regInfo->rs1);
        rhashBuf.Append(SIZE_128_BITS, regInfo->psk1);
        rhashBuf.Append(SIZE_PUB_KEY, regInfo->pke);
        rhashBuf.Append(SIZE_PUB_KEY, regInfo->pkr);

        //3. generate the mac
        uint8 hashBuf[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
            rhashBuf.GetBuf(), rhashBuf.Length(), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for RHash1\n"));
            throw RPROT_ERR_CRYPTO;
        }
		if (TUTRACELEVEL & TUVERBOSE && g_verbose)
		{
			printf("computed RS1 hash, should match rHash1 from M4 that has this value:\n");
			for (int i = 0; i < SIZE_256_BITS; i++) {
				printf("%02x ",regInfo->rHash1[i]);
			}
			printf("\n\n");
		}

        //4. compare the mac to rhash1
        if(memcmp(regInfo->rHash1, hashBuf, SIZE_256_BITS))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: RS1 hash doesn't match RHash1\n"));
            throw RPROT_ERR_CRYPTO;
        }
        TUTRACE((TUTRACE_VERBOSE, "RHash1 matches\n"));

        //5. Instead of steps 3 & 4, we could have called ValidateMac
        //****** RHash1 validation ******

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM4 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM4 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM4 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageM4

uint32 
CRegProtocol::BuildMessageM5(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    uint8 message;

    try
    {
        //First, generate or gather the required data
        message = WSC_ID_MESSAGE_M5;

        //encrypted settings.
        BufferObj encData;
        CTlvEsNonce esNonce;
        esNonce.nonce.Set(WSC_ID_E_SNONCE1,
                            regInfo->es1,
                            SIZE_128_BITS);
        esNonce.write(encData, regInfo->authKey);

        BufferObj cipherText, iv;
        EncryptData(encData, 
                    regInfo->keyWrapKey, 
                    regInfo->authKey, 
                    cipherText, 
                    iv);


        //Now assemble the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvRegistrarNonce(
                        WSC_ID_REGISTRAR_NONCE,
                        msg,
                        regInfo->registrarNonce, 
                        SIZE_128_BITS);
        CTlvEncrSettings encrSettings;
        encrSettings.iv = iv.GetBuf();
        encrSettings.ip_encryptedData = cipherText.GetBuf();
        encrSettings.encrDataLength = cipherText.Length();
        encrSettings.write(msg);
        //No vendor extension

        //Calculate the hmac
        BufferObj hmacData;
        hmacData.Append(regInfo->inMsg.Length(), regInfo->inMsg.GetBuf());
        hmacData.Append(msg.Length(), msg.GetBuf());

        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                hmacData.GetBuf(), hmacData.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_AUTHENTICATOR,
                        msg,
                        hmac,
                        SIZE_64_BITS);

        //Store the outgoing message 
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM5 built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM5 exiting with error %d\n",
                               err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM5 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessage5 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM5
#endif // WPS_ENROLLEE  

#ifdef WPS_REGISTRAR
uint32 
CRegProtocol::ProcessMessageM5(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    S_WSC_M5 m5;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM5: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        m5.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m5.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //Don't deserialize incompatible messages!
        if((version & 0xF0) != (m5.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M5 != m5.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m5.registrarNonce = CTlvRegistrarNonce(WSC_ID_REGISTRAR_NONCE, 
                                               msg, SIZE_128_BITS);
        m5.encrSettings.parse(msg);

        //skip all optional attributes until we get to the authenticator
        while(WSC_ID_AUTHENTICATOR != msg.NextType())
        {
            //advance past the TLV
            uint8 *Pos = msg.Advance( sizeof(S_WSC_TLV_HEADER) + 
                            WscNtohs(*(uint16 *)(msg.Pos()+sizeof(uint16))) );
            
            //If Advance returned NULL, it means there's no more data in the
            //buffer. This is an error.
            if(Pos == NULL)
                throw RPROT_ERR_REQD_TLV_MISSING;
        }

        m5.authenticator = CTlvAuthenticator(
                                            WSC_ID_AUTHENTICATOR,
                                            msg,
                                            SIZE_64_BITS);

        //Now start validating the message
        if((version & 0xF0) != (m5.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M5 != m5.msgType.Value())
            throw RPROT_ERR_INCOMPATIBLE;

        //confirm the enrollee nonce
        if(memcmp(regInfo->registrarNonce, 
                m5.registrarNonce.Value(),
                m5.registrarNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect registrar nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

        //****** HMAC validation ******
        BufferObj hmacData;
        //append the last message sent
        hmacData.Append(regInfo->outMsg.Length(), regInfo->outMsg.GetBuf());
        //append the current message. Don't append the last TLV (auth)
        hmacData.Append(
            msg.Length()-(sizeof(S_WSC_TLV_HEADER)+m5.authenticator.Length()),
            msg.GetBuf());

        if(!ValidateMac(hmacData, m5.authenticator.Value(), regInfo->authKey))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC validation failed"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        //****** HMAC validation ******

        //****** extract encrypted settings ******
        BufferObj cipherText(m5.encrSettings.ip_encryptedData, 
                             m5.encrSettings.encrDataLength);
        BufferObj iv(m5.encrSettings.iv, SIZE_128_BITS);
        BufferObj plainText;
        
        DecryptData(cipherText, 
                    iv, 
                    regInfo->keyWrapKey, 
                    regInfo->authKey, 
                    plainText);
        CTlvEsNonce eNonce;
        eNonce.parse(WSC_ID_E_SNONCE1, plainText, regInfo->authKey);
        //****** extract encrypted settings ******

        //****** EHash1 validation ******
        //1. Save ES1
        memcpy(regInfo->es1, eNonce.nonce.Value(), eNonce.nonce.Length());

        //2. prepare the buffer
        BufferObj ehashBuf;
        ehashBuf.Append(SIZE_128_BITS, regInfo->es1);
        ehashBuf.Append(SIZE_128_BITS, regInfo->psk1);
        ehashBuf.Append(SIZE_PUB_KEY, regInfo->pke);
        ehashBuf.Append(SIZE_PUB_KEY, regInfo->pkr);

        //3. generate the mac
        uint8 hashBuf[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
            ehashBuf.GetBuf(), ehashBuf.Length(), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for EHash1\n"));
            throw RPROT_ERR_CRYPTO;
        }

        //4. compare the mac to ehash1
        if(memcmp(regInfo->eHash1, hashBuf, SIZE_256_BITS))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: ES1 hash doesn't match EHash1\n"));
            throw RPROT_ERR_CRYPTO;
        }
        //5. Instead of steps 3 & 4, we could have called ValidateMac
        //****** EHash1 validation ******

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM5 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM5 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM5 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageM5

uint32 
CRegProtocol::BuildMessageM6(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    uint8 message;
    
    try
    {
        //First, generate or gather the required data
        message = WSC_ID_MESSAGE_M6;

        //encrypted settings.
        BufferObj encData;
        CTlvEsNonce rsNonce;
        rsNonce.nonce.Set(WSC_ID_R_SNONCE2,
                            regInfo->rs2,
                            SIZE_128_BITS);
        rsNonce.write(encData, regInfo->authKey);

        BufferObj cipherText, iv;
        EncryptData(encData, 
                    regInfo->keyWrapKey, 
                    regInfo->authKey, 
                    cipherText, 
                    iv);


        //Now assemble the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvEncrSettings encrSettings;
        encrSettings.iv = iv.GetBuf();
        encrSettings.ip_encryptedData = cipherText.GetBuf();
        encrSettings.encrDataLength = cipherText.Length();
        encrSettings.write(msg);
        //No vendor extension

        //Calculate the hmac
        BufferObj hmacData;
        hmacData.Append(regInfo->inMsg.Length(), regInfo->inMsg.GetBuf());
        hmacData.Append(msg.Length(), msg.GetBuf());

        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                hmacData.GetBuf(), hmacData.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_AUTHENTICATOR,
                        msg,
                        hmac,
                        SIZE_64_BITS);

        //Store the outgoing message 
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM6 built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM6 exiting with error %d\n",
                               err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM6 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessage6 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM6
#endif // WPS_REGISTRAR   

#ifdef WPS_ENROLLEE
uint32 
CRegProtocol::ProcessMessageM6(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    S_WSC_M6 m6;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM6: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        m6.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m6.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //Don't deserialize incompatible messages!
        if((version & 0xF0) != (m6.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M6 != m6.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m6.enrolleeNonce = CTlvEnrolleeNonce(WSC_ID_ENROLLEE_NONCE, 
                                             msg, SIZE_128_BITS);
        m6.encrSettings.parse(msg);

        //skip all optional attributes until we get to the authenticator
        while(WSC_ID_AUTHENTICATOR != msg.NextType())
        {
            //advance past the TLV
            uint8 *Pos = msg.Advance( sizeof(S_WSC_TLV_HEADER) + 
                            WscNtohs(*(uint16 *)(msg.Pos()+sizeof(uint16))) );
            
            //If Advance returned NULL, it means there's no more data in the
            //buffer. This is an error.
            if(Pos == NULL)
                throw RPROT_ERR_REQD_TLV_MISSING;
        }

        m6.authenticator = CTlvAuthenticator(
                                            WSC_ID_AUTHENTICATOR,
                                            msg,
                                            SIZE_64_BITS);

        //Now start validating the message

        //confirm the enrollee nonce
        if(memcmp(regInfo->enrolleeNonce, 
                m6.enrolleeNonce.Value(),
                m6.enrolleeNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect enrollee nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

        //****** HMAC validation ******
        BufferObj hmacData;
        //append the last message sent
        hmacData.Append(regInfo->outMsg.Length(), regInfo->outMsg.GetBuf());
        //append the current message. Don't append the last TLV (auth)
        hmacData.Append(
            msg.Length()-(sizeof(S_WSC_TLV_HEADER)+m6.authenticator.Length()),
            msg.GetBuf());

        if(!ValidateMac(hmacData, m6.authenticator.Value(), regInfo->authKey))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC validation failed"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        //****** HMAC validation ******

        //****** extract encrypted settings ******
        BufferObj cipherText(m6.encrSettings.ip_encryptedData, 
                             m6.encrSettings.encrDataLength);
        BufferObj iv(m6.encrSettings.iv, SIZE_128_BITS);
        BufferObj plainText;
        
        DecryptData(cipherText,
                    iv,
                    regInfo->keyWrapKey,
                    regInfo->authKey,
                    plainText);        
        CTlvEsNonce rNonce;
        rNonce.parse(WSC_ID_R_SNONCE2, plainText, regInfo->authKey);
        //****** extract encrypted settings ******

        //****** RHash2 validation ******
        //1. Save RS2
        memcpy(regInfo->rs2, rNonce.nonce.Value(), rNonce.nonce.Length());

        //2. prepare the buffer
        BufferObj rhashBuf;
        rhashBuf.Append(SIZE_128_BITS, regInfo->rs2);
        rhashBuf.Append(SIZE_128_BITS, regInfo->psk2);
        rhashBuf.Append(SIZE_PUB_KEY, regInfo->pke);
        rhashBuf.Append(SIZE_PUB_KEY, regInfo->pkr);

        //3. generate the mac
        uint8 hashBuf[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
            rhashBuf.GetBuf(), rhashBuf.Length(), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for RHash2\n"));
            throw RPROT_ERR_CRYPTO;
        }

        //4. compare the mac to rhash2
        if(memcmp(regInfo->rHash2, hashBuf, SIZE_256_BITS))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: RS2 hash doesn't match RHash2\n"));
            throw RPROT_ERR_CRYPTO;
        }
        //5. Instead of steps 3 & 4, we could have called ValidateMac
        //****** RHash2 validation ******

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM6 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM6 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM6 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageM6

uint32 
CRegProtocol::BuildMessageM7(S_REGISTRATION_DATA *regInfo, 
                             BufferObj &msg,
                             void *encrSettings)
{
    uint8 message;

    try
    {
        //First, generate or gather the required data
        message = WSC_ID_MESSAGE_M7;

        //encrypted settings.
        BufferObj esBuf;
#ifdef WSC_AP_DEFINED
        if(regInfo->p_enrolleeInfo->b_ap)
        {
            if(!encrSettings)
            {
                TUTRACE((TUTRACE_ERR, "RPROTO: AP Encr settings are NULL\n"));
                throw WSC_ERR_INVALID_PARAMETERS;
            }

            CTlvEsM7Ap *apEs = (CTlvEsM7Ap *)encrSettings;
            //Set ES Nonce2
            apEs->nonce.Set(WSC_ID_E_SNONCE2,
                            regInfo->es2,
                            SIZE_128_BITS);
            apEs->write(esBuf, regInfo->authKey);
        }

        else
#endif        
		{
            CTlvEsM7Enr *staEs;
            if(!encrSettings)
            {
                TUTRACE((TUTRACE_INFO, "RPROTO: NULL STA Encrypted settings."
                                       " Allocating memory...\n"));
                staEs = new CTlvEsM7Enr ();
                regInfo->staEncrSettings = (void *)staEs;
            }
            else
            {
                staEs = (CTlvEsM7Enr *)encrSettings;
            }
            //Set ES Nonce2
            staEs->nonce.Set(WSC_ID_E_SNONCE2,
                            regInfo->es2,
                            SIZE_128_BITS);
            staEs->write(esBuf, regInfo->authKey);
			// TODO:  vbl: free the staEs if encrSettings is NULL
        }//if

        //Now encrypt the serialize Encrypted settings buffer
        BufferObj cipherText, iv;
        EncryptData(esBuf, 
                    regInfo->keyWrapKey, 
                    regInfo->authKey,
                    cipherText,
                    iv);


        //Now assemble the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvRegistrarNonce(
                        WSC_ID_REGISTRAR_NONCE,
                        msg,
                        regInfo->registrarNonce, 
                        SIZE_128_BITS);
        CTlvEncrSettings encrSettings;
        encrSettings.iv = iv.GetBuf();
        encrSettings.ip_encryptedData = cipherText.GetBuf();
        encrSettings.encrDataLength = cipherText.Length();
        encrSettings.write(msg);

        if(regInfo->x509csr.Length())
        {
            CTlvX509CertReq(
                        WSC_ID_X509_CERT_REQ,
                        msg,
                        regInfo->x509csr.GetBuf(),
                        regInfo->x509csr.Length());
        }
        //No vendor extension

        //Calculate the hmac
        BufferObj hmacData;
        hmacData.Append(regInfo->inMsg.Length(), regInfo->inMsg.GetBuf());
        hmacData.Append(msg.Length(), msg.GetBuf());

        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                hmacData.GetBuf(), hmacData.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_AUTHENTICATOR,
                        msg,
                        hmac,
                        SIZE_64_BITS);

        //Store the outgoing message 
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM7 built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM7 exiting with error %d\n",
                               err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM7 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessage7 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM7
#endif // WPS_ENROLLEE

#ifdef WPS_REGISTRAR 

uint32 
CRegProtocol::ProcessMessageM7(S_REGISTRATION_DATA *regInfo, 
                               BufferObj &msg, 
                               void **encrSettings)
{
    S_WSC_M7 m7;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM7: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        m7.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m7.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //Don't deserialize incompatible messages!
        if((version & 0xF0) != (m7.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M7 != m7.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m7.registrarNonce = CTlvRegistrarNonce(WSC_ID_REGISTRAR_NONCE, 
                                               msg, 
                                               SIZE_128_BITS);
        m7.encrSettings.parse(msg);

        if(WSC_ID_X509_CERT_REQ == msg.NextType())
            m7.x509CertReq = CTlvX509CertReq(WSC_ID_X509_CERT_REQ, msg);

        //skip all optional attributes until we get to the authenticator
        while(WSC_ID_AUTHENTICATOR != msg.NextType())
        {
            //advance past the TLV
            uint8 *Pos = msg.Advance( sizeof(S_WSC_TLV_HEADER) + 
                            WscNtohs(*(uint16 *)(msg.Pos()+sizeof(uint16))) );
            
            //If Advance returned NULL, it means there's no more data in the
            //buffer. This is an error.
            if(Pos == NULL)
                throw RPROT_ERR_REQD_TLV_MISSING;
        }

        m7.authenticator = CTlvAuthenticator(
                                            WSC_ID_AUTHENTICATOR,
                                            msg,
                                            SIZE_64_BITS);

        //Now start validating the message
        if((version & 0xF0) != (m7.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M7 != m7.msgType.Value())
            throw RPROT_ERR_INCOMPATIBLE;

        //confirm the enrollee nonce
        if(memcmp(regInfo->registrarNonce, 
                m7.registrarNonce.Value(),
                m7.registrarNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect registrar nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

        //****** HMAC validation ******
        BufferObj hmacData;
        //append the last message sent
        hmacData.Append(regInfo->outMsg.Length(), regInfo->outMsg.GetBuf());
        //append the current message. Don't append the last TLV (auth)
        hmacData.Append(
            msg.Length()-(sizeof(S_WSC_TLV_HEADER)+m7.authenticator.Length()),
            msg.GetBuf());

        if(!ValidateMac(hmacData, m7.authenticator.Value(), regInfo->authKey))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC validation failed"));
            throw RPROT_ERR_INVALID_VALUE;
        }
        //****** HMAC validation ******

        //****** extract encrypted settings ******
        BufferObj cipherText(m7.encrSettings.ip_encryptedData, 
                             m7.encrSettings.encrDataLength);
        BufferObj iv(m7.encrSettings.iv, SIZE_128_BITS);
        BufferObj plainText;
        
        DecryptData(cipherText, 
                    iv, 
                    regInfo->keyWrapKey,
                    regInfo->authKey,
                    plainText);

        CTlvNonce eNonce;
#ifdef WSC_AP_DEFINED
        if(regInfo->p_enrolleeInfo->b_ap)
        {
            CTlvEsM7Ap *esAP = new CTlvEsM7Ap();
            esAP->parse(plainText, regInfo->authKey, true);
            eNonce = esAP->nonce;
            *encrSettings = (void *)esAP;
        }
        else
#endif
        {
            CTlvEsM7Enr *esSta = new CTlvEsM7Enr();
            esSta->parse(plainText, regInfo->authKey, true);
            eNonce = esSta->nonce;
            *encrSettings = (void *)esSta;
        }
        //****** extract encrypted settings ******

        //****** EHash2 validation ******
        //1. Save ES2
        memcpy(regInfo->es2, eNonce.Value(), eNonce.Length());

        //2. prepare the buffer
        BufferObj ehashBuf;
        ehashBuf.Append(SIZE_128_BITS, regInfo->es2);
        ehashBuf.Append(SIZE_128_BITS, regInfo->psk2);
        ehashBuf.Append(SIZE_PUB_KEY, regInfo->pke);
        ehashBuf.Append(SIZE_PUB_KEY, regInfo->pkr);

        //3. generate the mac
        uint8 hashBuf[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
            ehashBuf.GetBuf(), ehashBuf.Length(), hashBuf, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed for EHash1\n"));
            throw RPROT_ERR_CRYPTO;
        }

        //4. compare the mac to ehash2
        if(memcmp(regInfo->eHash2, hashBuf, SIZE_256_BITS))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: ES2 hash doesn't match EHash2\n"));
            throw RPROT_ERR_CRYPTO;
        }
        //5. Instead of steps 3 & 4, we could have called ValidateMac
        //****** EHash1 validation ******

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM7 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM7 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM7 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageM7

uint32 
CRegProtocol::BuildMessageM8(S_REGISTRATION_DATA *regInfo, 
                             BufferObj &msg, 
                             void *encrSettings)
{
    uint8 message;

    try
    {
        //First, generate or gather the required data
        message = WSC_ID_MESSAGE_M8;

        //encrypted settings.
        BufferObj esBuf;
        if(!encrSettings)
        {
            TUTRACE((TUTRACE_ERR, "Encrypted settings settings are NULL\n"));
            throw WSC_ERR_INVALID_PARAMETERS;
        }
#ifdef WSC_AP_DEFINED
        if(regInfo->p_enrolleeInfo->b_ap)
        {
            CTlvEsM8Ap *apEs = (CTlvEsM8Ap *)encrSettings;
            apEs->write(esBuf, regInfo->authKey);
        }
        else
#endif
        {
            CTlvEsM8Sta *staEs = (CTlvEsM8Sta *)encrSettings;
            staEs->write(esBuf, regInfo->authKey);
        }

        //Now encrypt the serialize Encrypted settings buffer
        BufferObj cipherText, iv;
        EncryptData(esBuf, 
                    regInfo->keyWrapKey, 
                    regInfo->authKey,
                    cipherText,iv);

        //Now assemble the message
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvEncrSettings encrSettings;
        encrSettings.iv = iv.GetBuf();
        encrSettings.ip_encryptedData = cipherText.GetBuf();
        encrSettings.encrDataLength = cipherText.Length();
        encrSettings.write(msg);

        if(regInfo->x509Cert.Length())
        {
            CTlvX509Cert(
                        WSC_ID_X509_CERT,
                        msg,
                        regInfo->x509Cert.GetBuf(),
                        regInfo->x509Cert.Length());
        }
        //No vendor extension

        //Calculate the hmac
        BufferObj hmacData;
        hmacData.Append(regInfo->inMsg.Length(), regInfo->inMsg.GetBuf());
        hmacData.Append(msg.Length(), msg.GetBuf());

        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), regInfo->authKey.GetBuf(), SIZE_256_BITS, 
                hmacData.GetBuf(), hmacData.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_AUTHENTICATOR,
                        msg,
                        hmac,
                        SIZE_64_BITS);

        //Store the outgoing message 
        regInfo->outMsg.Reset();
        regInfo->outMsg.Append(msg.Length(), msg.GetBuf());

        TUTRACE((TUTRACE_INFO, "RPROTO: BuildMessageM8 built: %d bytes\n",
                                msg.Length()));
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM8 exiting with error %d\n",
                               err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageM8 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessage8 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageM8
#endif // WPS_REGISTRAR 

#ifdef WPS_ENROLLEE
uint32 
CRegProtocol::ProcessMessageM8(S_REGISTRATION_DATA *regInfo, 
                               BufferObj &msg, 
                               void **encrSettings)
{
    S_WSC_M8 m8;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageM8: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        m8.version       = CTlvVersion(WSC_ID_VERSION, msg);
        m8.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //Don't deserialize incompatible messages!
        if((version & 0xF0) != (m8.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M8 != m8.msgType.Value())
            throw RPROT_ERR_WRONG_MSGTYPE;

        m8.enrolleeNonce = CTlvEnrolleeNonce(WSC_ID_ENROLLEE_NONCE, 
                                             msg, SIZE_128_BITS);

		m8.encrSettings.parse(msg);

        if(WSC_ID_X509_CERT == msg.NextType())
            m8.x509Cert = CTlvX509Cert(WSC_ID_X509_CERT, msg);

        //skip all optional attributes until we get to the authenticator
        while(WSC_ID_AUTHENTICATOR != msg.NextType())
        {
            //advance past the TLV
            uint8 *Pos = msg.Advance( sizeof(S_WSC_TLV_HEADER) + 
                            WscNtohs(*(uint16 *)(msg.Pos()+sizeof(uint16))) );
            
            //If Advance returned NULL, it means there's no more data in the
            //buffer. This is an error.
            if(Pos == NULL)
                throw RPROT_ERR_REQD_TLV_MISSING;
        }

        m8.authenticator = CTlvAuthenticator(
                                            WSC_ID_AUTHENTICATOR,
                                            msg,
                                            SIZE_64_BITS);

        //Now start validating the message
        if((version & 0xF0) != (m8.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_M8 != m8.msgType.Value())
            throw RPROT_ERR_INCOMPATIBLE;

        //confirm the enrollee nonce
        if(memcmp(regInfo->enrolleeNonce, 
                m8.enrolleeNonce.Value(),
                m8.enrolleeNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect enrollee nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }
        TUTRACE((TUTRACE_VERBOSE, "enrollee nonce matches\n"));

        //****** HMAC validation ******
        BufferObj hmacData;
        //append the last message sent
        hmacData.Append(regInfo->outMsg.Length(), regInfo->outMsg.GetBuf());
        //append the current message. Don't append the last TLV (auth)
        hmacData.Append(
            msg.Length()-(sizeof(S_WSC_TLV_HEADER)+m8.authenticator.Length()),
            msg.GetBuf());

        if(!ValidateMac(hmacData, m8.authenticator.Value(), regInfo->authKey))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC validation failed"));
            throw RPROT_ERR_INVALID_VALUE;
        }

		TUTRACE((TUTRACE_VERBOSE, "HMAC matches\n"));

        //****** HMAC validation ******

        //****** extract encrypted settings ******
        BufferObj cipherText(m8.encrSettings.ip_encryptedData, 
                             m8.encrSettings.encrDataLength);
        BufferObj iv(m8.encrSettings.iv, SIZE_128_BITS);
        BufferObj plainText;
        
        DecryptData(cipherText,
                    iv,
                    regInfo->keyWrapKey,
                    regInfo->authKey,
                    plainText); 
		TUTRACE((TUTRACE_VERBOSE, "data decrypted\n"));

#ifdef WSC_AP_DEFINED
        if(regInfo->p_enrolleeInfo->b_ap)
        {
            CTlvEsM8Ap *esAP = new CTlvEsM8Ap();
            esAP->parse(plainText, regInfo->authKey, true);
            *encrSettings = (void *)esAP;
        }
        else
#endif
        {
            CTlvEsM8Sta *esSta = new CTlvEsM8Sta();
            esSta->parse(plainText, regInfo->authKey, true);
            *encrSettings = (void *)esSta;
        }
        //****** extract encrypted settings ******

        //Store the received buffer
        regInfo->inMsg.Reset();
        regInfo->inMsg.Append(msg.Length(), msg.GetBuf());

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM8 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM8 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageM8 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageM8

uint32 
CRegProtocol::BuildMessageAck(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    uint8 message;

    TUTRACE((TUTRACE_INFO, "RPROTO: In BuildMessageAck\n"));
    try
    {
        message = WSC_ID_MESSAGE_ACK;
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvRegistrarNonce(
                        WSC_ID_REGISTRAR_NONCE,
                        msg,
                        regInfo->registrarNonce, 
                        SIZE_128_BITS);
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageAck generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageAck generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageAck generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageAck
#endif // WPS_ENROLLEE  

#ifdef WPS_REGISTRAR
uint32 
CRegProtocol::ProcessMessageAck(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    S_WSC_ACK ack;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageAck: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        ack.version       = CTlvVersion(WSC_ID_VERSION, msg);
        ack.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);
        ack.enrolleeNonce = CTlvEnrolleeNonce(
                                            WSC_ID_ENROLLEE_NONCE, 
                                            msg, 
                                            SIZE_128_BITS);
        ack.registrarNonce = CTlvRegistrarNonce(
                                            WSC_ID_REGISTRAR_NONCE, 
                                            msg, 
                                            SIZE_128_BITS);

        //Now process the received message
        if((version & 0xF0) != (ack.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_ACK != ack.msgType.Value())
            throw RPROT_ERR_INCOMPATIBLE;

        //confirm the enrollee nonce
        if(memcmp(regInfo->enrolleeNonce, 
                ack.enrolleeNonce.Value(),
                ack.enrolleeNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect enrollee nonce\n"));
			return RPROT_ERR_NONCE_MISMATCH;
        }

        //confirm the registrar nonce
        if(memcmp(regInfo->registrarNonce, 
                ack.registrarNonce.Value(),
                ack.registrarNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect registrar nonce\n"));
            return RPROT_ERR_NONCE_MISMATCH;
        }

		// ignore any other TLVs

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageAck generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageAck generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageAck generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageAck
#endif // WPS_REGISTRAR  

// NACK messages can be sent by both Registrars and Enrollees
//
uint32 
CRegProtocol::BuildMessageNack(S_REGISTRATION_DATA *regInfo, 
                               BufferObj &msg, 
                               uint16 configError)
{
    uint8 message;

    TUTRACE((TUTRACE_INFO, "RPROTO: In BuildMessageNack\n"));
    try
    {
        message = WSC_ID_MESSAGE_NACK;
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvRegistrarNonce(
                        WSC_ID_REGISTRAR_NONCE,
                        msg,
                        regInfo->registrarNonce, 
                        SIZE_128_BITS);
        CTlvConfigError(WSC_ID_CONFIG_ERROR, msg, &configError);

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageNack generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageNack generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageNack generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageNack


uint32 
CRegProtocol::ProcessMessageNack(S_REGISTRATION_DATA *regInfo, BufferObj &msg, uint16 *configError)
{
    S_WSC_NACK nack;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageNack: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        nack.version       = CTlvVersion(WSC_ID_VERSION, msg);
        nack.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);
        nack.enrolleeNonce = CTlvEnrolleeNonce(
                                            WSC_ID_ENROLLEE_NONCE, 
                                            msg, 
                                            SIZE_128_BITS);
        nack.registrarNonce = CTlvRegistrarNonce(
                                            WSC_ID_REGISTRAR_NONCE, 
                                            msg, 
                                            SIZE_128_BITS);
        nack.configError    = CTlvConfigError(
                                            WSC_ID_CONFIG_ERROR,
                                            msg);

        //Now process the received message
        if((version & 0xF0) != (nack.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_NACK != nack.msgType.Value())
            throw RPROT_ERR_INCOMPATIBLE;

        //confirm the enrollee nonce
        if(memcmp(regInfo->enrolleeNonce, 
                nack.enrolleeNonce.Value(),
                nack.enrolleeNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect enrollee nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

        //confirm the registrar nonce
        if(memcmp(regInfo->registrarNonce, 
                nack.registrarNonce.Value(),
                nack.registrarNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect registrar nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

		// ignore any other TLVs

        *configError = nack.configError.Value();
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageNack generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageNack generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageNack generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageNack

uint32 
CRegProtocol::BuildMessageDone(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    uint8 message;

    TUTRACE((TUTRACE_INFO, "RPROTO: In BuildMessageDone\n"));
    try
    {
        message = WSC_ID_MESSAGE_DONE;
        CTlvVersion(WSC_ID_VERSION, msg, &version);
        CTlvMsgType(WSC_ID_MSG_TYPE, msg, &message);
        CTlvEnrolleeNonce(
                        WSC_ID_ENROLLEE_NONCE,
                        msg,
                        regInfo->enrolleeNonce, 
                        SIZE_128_BITS);
        CTlvRegistrarNonce(
                        WSC_ID_REGISTRAR_NONCE,
                        msg,
                        regInfo->registrarNonce, 
                        SIZE_128_BITS);
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageDone generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageDone generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: BuildMessageDone generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//BuildMessageDone


uint32 
CRegProtocol::ProcessMessageDone(S_REGISTRATION_DATA *regInfo, BufferObj &msg)
{
    S_WSC_ACK done;

    TUTRACE((TUTRACE_INFO, "RPROTO: In ProcessMessageDone: %d byte message\n",
                                msg.Length()));
    try
    {
        //First, deserialize (parse) the message.
        done.version       = CTlvVersion(WSC_ID_VERSION, msg);
        done.msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);
        done.enrolleeNonce = CTlvEnrolleeNonce(
                                            WSC_ID_ENROLLEE_NONCE, 
                                            msg, 
                                            SIZE_128_BITS);
        done.registrarNonce = CTlvRegistrarNonce(
                                            WSC_ID_REGISTRAR_NONCE, 
                                            msg, 
                                            SIZE_128_BITS);

        //Now process the received message
        if((version & 0xF0) != (done.version.Value()& 0xF0))
            throw RPROT_ERR_INCOMPATIBLE;

        if(WSC_ID_MESSAGE_DONE != done.msgType.Value())
            throw RPROT_ERR_INCOMPATIBLE;

        //confirm the enrollee nonce
        if(memcmp(regInfo->enrolleeNonce, 
                done.enrolleeNonce.Value(),
                done.enrolleeNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect enrollee nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

        //confirm the registrar nonce
        if(memcmp(regInfo->registrarNonce, 
                done.registrarNonce.Value(),
                done.registrarNonce.Length()))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Incorrect registrar nonce\n"));
            throw RPROT_ERR_NONCE_MISMATCH;
        }

        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageDone generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageDone generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: ProcessMessageDone generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//ProcessMessageDone
