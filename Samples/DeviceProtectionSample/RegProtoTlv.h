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

#ifndef _WSC_REG_PROTO_TLV_
#define _WSC_REG_PROTO_TLV_

#include "WscTypes.h"
#include "slist.h"
#include "WscTlvBase.h"

#pragma pack(push, 1)

// Data Element Definitions
#define WSC_ID_AP_CHANNEL         0x1001
#define WSC_ID_ASSOC_STATE        0x1002
#define WSC_ID_AUTH_TYPE          0x1003
#define WSC_ID_AUTH_TYPE_FLAGS    0x1004
#define WSC_ID_AUTHENTICATOR      0x1005
#define WSC_ID_CONFIG_METHODS     0x1008
#define WSC_ID_CONFIG_ERROR       0x1009
#define WSC_ID_CONF_URL4          0x100A
#define WSC_ID_CONF_URL6          0x100B
#define WSC_ID_CONN_TYPE          0x100C
#define WSC_ID_CONN_TYPE_FLAGS    0x100D
#define WSC_ID_CREDENTIAL         0x100E
#define WSC_ID_DEVICE_NAME        0x1011
#define WSC_ID_DEVICE_PWD_ID      0x1012
#define WSC_ID_E_HASH1            0x1014
#define WSC_ID_E_HASH2            0x1015
#define WSC_ID_E_SNONCE1          0x1016
#define WSC_ID_E_SNONCE2          0x1017
#define WSC_ID_ENCR_SETTINGS      0x1018
#define WSC_ID_ENCR_TYPE          0x100F
#define WSC_ID_ENCR_TYPE_FLAGS    0x1010
#define WSC_ID_ENROLLEE_NONCE     0x101A
#define WSC_ID_FEATURE_ID         0x101B
#define WSC_ID_IDENTITY           0x101C
#define WSC_ID_IDENTITY_PROOF     0x101D
#define WSC_ID_KEY_WRAP_AUTH      0x101E 
#define WSC_ID_KEY_IDENTIFIER     0x101F
#define WSC_ID_MAC_ADDR           0x1020
#define WSC_ID_MANUFACTURER       0x1021
#define WSC_ID_MSG_TYPE           0x1022
#define WSC_ID_MODEL_NAME         0x1023
#define WSC_ID_MODEL_NUMBER       0x1024
#define WSC_ID_NW_INDEX           0x1026
#define WSC_ID_NW_KEY             0x1027
#define WSC_ID_NW_KEY_INDEX       0x1028
#define WSC_ID_NEW_DEVICE_NAME    0x1029
#define WSC_ID_NEW_PWD            0x102A        
#define WSC_ID_OOB_DEV_PWD        0x102C
#define WSC_ID_OS_VERSION         0x102D
#define WSC_ID_POWER_LEVEL        0x102F
#define WSC_ID_PSK_CURRENT        0x1030
#define WSC_ID_PSK_MAX            0x1031
#define WSC_ID_PUBLIC_KEY         0x1032
#define WSC_ID_RADIO_ENABLED      0x1033
#define WSC_ID_REBOOT             0x1034
#define WSC_ID_REGISTRAR_CURRENT  0x1035
#define WSC_ID_REGISTRAR_ESTBLSHD 0x1036
#define WSC_ID_REGISTRAR_LIST     0x1037
#define WSC_ID_REGISTRAR_MAX      0x1038
#define WSC_ID_REGISTRAR_NONCE    0x1039
#define WSC_ID_REQ_TYPE           0x103A
#define WSC_ID_RESP_TYPE          0x103B
#define WSC_ID_RF_BAND            0x103C
#define WSC_ID_R_HASH1            0x103D
#define WSC_ID_R_HASH2            0x103E
#define WSC_ID_R_SNONCE1          0x103F
#define WSC_ID_R_SNONCE2          0x1040
#define WSC_ID_SEL_REGISTRAR      0x1041
#define WSC_ID_SERIAL_NUM         0x1042
#define WSC_ID_SC_STATE           0x1044
#define WSC_ID_SSID               0x1045
#define WSC_ID_TOT_NETWORKS       0x1046
#define WSC_ID_UUID_E             0x1047
#define WSC_ID_UUID_R             0x1048
#define WSC_ID_VENDOR_EXT         0x1049
#define WSC_ID_VERSION            0x104A
#define WSC_ID_X509_CERT_REQ      0x104B
#define WSC_ID_X509_CERT          0x104C
#define WSC_ID_EAP_IDENTITY       0x104D
#define WSC_ID_MSG_COUNTER        0x104E
#define WSC_ID_PUBKEY_HASH        0x104F
#define WSC_ID_REKEY_KEY          0x1050
#define WSC_ID_KEY_LIFETIME       0x1051
#define WSC_ID_PERM_CFG_METHODS   0x1052
#define WSC_ID_SEL_REG_CFG_METHODS 0x1053
#define WSC_ID_PRIM_DEV_TYPE      0x1054
#define WSC_ID_SEC_DEV_TYPE_LIST  0x1055
#define WSC_ID_PORTABLE_DEVICE    0x1056
#define WSC_ID_AP_SETUP_LOCKED    0x1057
#define WSC_ID_APP_LIST           0x1058
#define WSC_ID_EAP_TYPE           0x1059
#define WSC_ID_INIT_VECTOR        0x1060
#define WSC_ID_KEY_PROVIDED_AUTO  0x1061
#define WSC_ID_8021X_ENABLED      0x1062

// Association states
#define WSC_ASSOC_NOT_ASSOCIATED  0
#define WSC_ASSOC_CONN_SUCCESS    1
#define WSC_ASSOC_CONFIG_FAIL     2
#define WSC_ASSOC_ASSOC_FAIL      3
#define WSC_ASSOC_IP_FAIL         4

// Authentication types
#define WSC_AUTHTYPE_OPEN        0x0001
#define WSC_AUTHTYPE_WPAPSK      0x0002
#define WSC_AUTHTYPE_SHARED      0x0004
#define WSC_AUTHTYPE_WPA         0x0008
#define WSC_AUTHTYPE_WPA2        0x0010
#define WSC_AUTHTYPE_WPA2PSK     0x0020

// Config methods
#define WSC_CONFMET_USBA            0x0001
#define WSC_CONFMET_ETHERNET        0x0002
#define WSC_CONFMET_LABEL           0x0004
#define WSC_CONFMET_DISPLAY         0x0008
#define WSC_CONFMET_EXT_NFC_TOK     0x0010
#define WSC_CONFMET_INT_NFC_TOK     0x0020
#define WSC_CONFMET_NFC_INTF        0x0040
#define WSC_CONFMET_PBC             0x0080
#define WSC_CONFMET_KEYPAD          0x0100

// WSC error messages
#define WSC_ERROR_NO_ERROR                0
#define WSC_ERROR_OOB_INT_READ_ERR        1
#define WSC_ERROR_DECRYPT_CRC_FAIL        2
#define WSC_ERROR_CHAN24_NOT_SUPP         3
#define WSC_ERROR_CHAN50_NOT_SUPP         4
#define WSC_ERROR_SIGNAL_WEAK             5
#define WSC_ERROR_NW_AUTH_FAIL            6
#define WSC_ERROR_NW_ASSOC_FAIL           7
#define WSC_ERROR_NO_DHCP_RESP            8
#define WSC_ERROR_FAILED_DHCP_CONF        9
#define WSC_ERROR_IP_ADDR_CONFLICT        10
#define WSC_ERROR_FAIL_CONN_REGISTRAR     11
#define WSC_ERROR_MULTI_PBC_DETECTED      12
#define WSC_ERROR_ROGUE_SUSPECTED         13
#define WSC_ERROR_DEVICE_BUSY             14
#define WSC_ERROR_SETUP_LOCKED            15
#define WSC_ERROR_MSG_TIMEOUT             16
#define WSC_ERROR_REG_SESSION_TIMEOUT     17
#define WSC_ERROR_DEV_PWD_AUTH_FAIL       18

// Connection types
#define WSC_CONNTYPE_ESS    0x01
#define WSC_CONNTYPE_IBSS   0x02

// Device password ID
#define WSC_DEVICEPWDID_DEFAULT          0x0000
#define WSC_DEVICEPWDID_USER_SPEC        0x0001
#define WSC_DEVICEPWDID_MACHINE_SPEC     0x0002
#define WSC_DEVICEPWDID_REKEY            0x0003
#define WSC_DEVICEPWDID_PUSH_BTN         0x0004
#define WSC_DEVICEPWDID_REG_SPEC         0x0005

/*
// Device type
#define WSC_DEVICETYPE_COMPUTER            "Computer"
#define WSC_DEVICETYPE_AP                  "Access_Point"
#define WSC_DEVICETYPE_ROUTER_AP           "Router_AP"
#define WSC_DEVICETYPE_PRINTER             "Printer"
#define WSC_DEVICETYPE_PRINTER_BRIDGE      "Printer_Brigde"
#define WSC_DEVICETYPE_ELECT_PIC_FRAME     "Electronic_Picture_Frame"
#define WSC_DEVICETYPE_DIG_AUDIO_RECV      "Digital_Audio_Receiver"
#define WSC_DEVICETYPE_WIN_MCE             "Windows_Media_Center_Extender"
#define WSC_DEVICETYPE_WIN_MOBILE          "Windows_Mobile"
#define WSC_DEVICETYPE_PVR                 "Personal_Video_Recorder"
#define WSC_DEVICETYPE_VIDEO_STB           "Video_STB"
#define WSC_DEVICETYPE_PROJECTOR           "Projector"
#define WSC_DEVICETYPE_IP_TV               "IP_TV"
#define WSC_DEVICETYPE_DIG_STILL_CAM       "Digital_Still_Camera"
#define WSC_DEVICETYPE_PHONE               "Phone"
#define WSC_DEVICETYPE_VOID_PHONE          "VoIP_Phone"
#define WSC_DEVICETYPE_GAME_CONSOLE        "Game_console"
#define WSC_DEVICETYPE_OTHER               "Other"
*/

// Encryption type
#define WSC_ENCRTYPE_NONE    0x0001
#define WSC_ENCRTYPE_WEP     0x0002
#define WSC_ENCRTYPE_TKIP    0x0004
#define WSC_ENCRTYPE_AES     0x0008


// WSC Message Types
#define WSC_ID_BEACON            0x01
#define WSC_ID_PROBE_REQ         0x02
#define WSC_ID_PROBE_RESP        0x03
#define WSC_ID_MESSAGE_M1        0x04
#define WSC_ID_MESSAGE_M2        0x05
#define WSC_ID_MESSAGE_M2D       0x06
#define WSC_ID_MESSAGE_M3        0x07
#define WSC_ID_MESSAGE_M4        0x08
#define WSC_ID_MESSAGE_M5        0x09
#define WSC_ID_MESSAGE_M6        0x0A
#define WSC_ID_MESSAGE_M7        0x0B
#define WSC_ID_MESSAGE_M8        0x0C
#define WSC_ID_MESSAGE_ACK       0x0D
#define WSC_ID_MESSAGE_NACK      0x0E
#define WSC_ID_MESSAGE_DONE      0x0F

//Device Type categories for primary and secondary device types
#define WSC_DEVICE_TYPE_CAT_COMPUTER        1
#define WSC_DEVICE_TYPE_CAT_INPUT_DEVICE    2
#define WSC_DEVICE_TYPE_CAT_PRINTER         3
#define WSC_DEVICE_TYPE_CAT_CAMERA          4
#define WSC_DEVICE_TYPE_CAT_STORAGE         5
#define WSC_DEVICE_TYPE_CAT_NW_INFRA        6
#define WSC_DEVICE_TYPE_CAT_DISPLAYS        7
#define WSC_DEVICE_TYPE_CAT_MM_DEVICES      8
#define WSC_DEVICE_TYPE_CAT_GAME_DEVICES    9
#define WSC_DEVICE_TYPE_CAT_TELEPHONE       10

//Device Type sub categories for primary and secondary device types
#define WSC_DEVICE_TYPE_SUB_CAT_COMP_PC         1
#define WSC_DEVICE_TYPE_SUB_CAT_COMP_SERVER     2
#define WSC_DEVICE_TYPE_SUB_CAT_COMP_MEDIA_CTR  3
#define WSC_DEVICE_TYPE_SUB_CAT_PRTR_PRINTER    1
#define WSC_DEVICE_TYPE_SUB_CAT_PRTR_SCANNER    2
#define WSC_DEVICE_TYPE_SUB_CAT_CAM_DGTL_STILL  1
#define WSC_DEVICE_TYPE_SUB_CAT_STOR_NAS        1
#define WSC_DEVICE_TYPE_SUB_CAT_NW_AP           1
#define WSC_DEVICE_TYPE_SUB_CAT_NW_ROUTER       2
#define WSC_DEVICE_TYPE_SUB_CAT_NW_SWITCH       3
#define WSC_DEVICE_TYPE_SUB_CAT_DISP_TV         1
#define WSC_DEVICE_TYPE_SUB_CAT_DISP_PIC_FRAME  2
#define WSC_DEVICE_TYPE_SUB_CAT_DISP_PROJECTOR  3
#define WSC_DEVICE_TYPE_SUB_CAT_MM_DAR          1
#define WSC_DEVICE_TYPE_SUB_CAT_MM_PVR          2
#define WSC_DEVICE_TYPE_SUB_CAT_MM_MCX          3
#define WSC_DEVICE_TYPE_SUB_CAT_GAM_XBOX        1
#define WSC_DEVICE_TYPE_SUB_CAT_GAM_XBOX_360    2
#define WSC_DEVICE_TYPE_SUB_CAT_GAM_PS          3
#define WSC_DEVICE_TYPE_SUB_CAT_PHONE_WM        1

// Device request/response type
#define WSC_MSGTYPE_ENROLLEE_INFO_ONLY    0x00
#define WSC_MSGTYPE_ENROLLEE_OPEN_8021X   0x01
#define WSC_MSGTYPE_REGISTRAR             0x02
#define WSC_MSGTYPE_AP_WLAN_MGR           0x03

// RF Band
#define WSC_RFBAND_24GHZ    0x01
#define WSC_RFBAND_50GHZ    0x02

// Simple Config state
#define WSC_SCSTATE_UNCONFIGURED    0x01
#define WSC_SCSTATE_CONFIGURED      0x02

//WSC OUI for primary and secondary device type sub-category
#define WSC_OUI 0x0050f204

/*********************************************************
*   WSC Message Header is declared in wsc_tlv_base.h as *
*   typedef struct {                                     *
*       uint16    attributeType;                         *
*       uint16    dataLength;                            *
*   } CTlvHEADER;                                  *
**********************************************************/

// WSC_TLVs
typedef TlvObj<bool>
    CTlvRadioEnabled,
    CTlvReboot,
    CTlvRegistrarEstablished,
    CTlvSelRegistrar,
    CTlvPortableDevice,
    CTlvAPSetupLocked,
    CTlvKeyProvidedAuto,
    CTlv8021XEnabled
	;

typedef TlvObj<uint8>
    CTlvConnType, 
    CTlvConnTypeFlags,
    CTlvMsgType,
    CTlvNwIndex,
    CTlvNwKeyIndex,
    CTlvPowerLevel, 
    CTlvPskCurrent,
    CTlvPskMax,
    CTlvRegistrarCurrent,
    CTlvRegistrarMax,
    CTlvReqType,
    CTlvRespType,
    CTlvRfBand,
    CTlvScState,
    CTlvTotNetworks,
    CTlvVersion;

typedef TlvObj<uint16>
    CTlvApChannel, 
    CTlvAssocState,
    CTlvAuthType,
    CTlvAuthTypeFlags,
    CTlvConfigMethods,
    CTlvConfigError,
    CTlvDevicePwdId,
    CTlvEncrType,
    CTlvEncrTypeFlags,
    CTlvPermittedCfgMethods,
    CTlvSelRegCfgMethods;

typedef TlvObj<uint32>
    CTlvFeatureId,
    CTlvOsVersion,
    CTlvKeyLifetime;

typedef TlvPtrObj<char *>
    CTlvConfUrl4,        //<= 64B
    CTlvConfUrl6,        //<= 76B
    CTlvDeviceName,      //<= 32B
    CTlvIdentity,        //<= 80B
    CTlvIdentityProof,
    CTlvManufacturer,     //<= 64B
    CTlvModelName,        //<= 32B
    CTlvModelNumber,      //<= 32B
    CTlvNwKey,            //<= 64B
    CTlvNewDeviceName,    //<= 32B
    CTlvNewPwd,           //<= 64B
    CTlvSerialNum,        //<= 32B
    CTlvAppList;          //<= 512B

typedef TlvPtrObj<uint8 *>
    CTlvMacAddr,         //=6B
    CTlvAuthenticator,    //=8B
    CTlvKeyWrapAuth,    //=8B
    CTlvNonce,            //=16B
    CTlvEnrolleeNonce,   //=16B
    CTlvKeyIdentifier,   //=16B
    CTlvRegistrarNonce,  //=16B
    CTlvUuid,             //=16B
    CTlvPubKeyHash,      //=16B
    CTlvHash,             //=32B
    CTlvInitVector,      //=32B
    CTlvSsid,             //=32B
    CTlvRekeyKey,        //=32B
    CTlvPublicKey,       //=192B
    CTlvEapId,            //TBD
    CTlvX509CertReq,
    CTlvX509Cert,
    CTlvEapType;         //<=8B


class CTlvEncrSettings : public cplxtlvbase
{
public:
    uint8 *             iv; //=16B
    uint8 *             ip_encryptedData; 

    //Internal variables, not part of actual TLV
    uint16              encrDataLength; 

    CTlvEncrSettings():encrDataLength(0){}
    void parse(BufferObj &theBuf);
    void write(BufferObj &theBuf);

};

class CTlvOobDevPwd : public cplxtlvbase
{
public:
    uint8 *             publicKeyHash; //=20B
    uint16              pwdId;
    uint8 *             ip_devPwd; // <= 32B

    //Internal variables, not part of TLV
    uint16              devPwdLength;

    CTlvOobDevPwd():devPwdLength(0){}
    void parse(BufferObj &theBuf);
    void write(BufferObj &theBuf);
};

class CTlvRegistrarList : public cplxtlvbase
{
public:
    uint8 *   registrarUUID; //=16B
    char *   cp_deviceName;

    void parse(BufferObj &theBuf);
    void write(BufferObj &theBuf);
};//MAX LEN 512B ?    

class CTlvVendorExt : public cplxtlvbase
{
public:
    uint8 *              vendorId;  //3
    uint8 *              vendorData;//<=1021B, <=246B for 802.11 Mgmt frames

    //Internal variables, not part of TLV
    uint16               dataLength;

    CTlvVendorExt():dataLength(0){}
    void parse(BufferObj &theBuf);
    void write(BufferObj &theBuf);
};    

class CTlvCredential : public cplxtlvbase
{
public:
    //Required attributes
    CTlvNwIndex         nwIndex;
    CTlvSsid            ssid;
    CTlvAuthType        authType;
    CTlvEncrType        encrType;
    CTlvNwKeyIndex      nwKeyIndex;
    CTlvNwKey           nwKey;
    CTlvMacAddr         macAddr;

    //Optional attributes
    CTlvEapType         eapType;    
    CTlvEapId           eapIdentity; //TBD: Define this
    CTlvKeyLifetime     keyLifetime; //TBD: Define this
    CTlvVendorExt       vendorExt;   //TBD: Ignore for now
    CTlvRekeyKey        rekeyKey;
    CTlvX509Cert        x509Cert;
	CTlvKeyProvidedAuto keyProvidedAuto;
    CTlv8021XEnabled    oneXEnabled;

    void parseAKey(BufferObj &theBuf, bool allocate = false);
    void parse(BufferObj &theBuf, bool allocate = false);
    void write(BufferObj &theBuf);
} ;

//Primary device type
class CTlvPrimDeviceType: public cplxtlvbase
{
public:
    uint16      categoryId;
    uint32      oui;
    uint16      subCategoryId;

    void parse(BufferObj &theBuf);
    void write(BufferObj &theBuf);
};

class CTlvSecDeviceType:public cplxtlvbase
{
public:
    LPLIST devList;

    void parse(BufferObj &theBuf);
    void write(BufferObj &theBuf);
};

extern uint32 GetMacAddrIfM1(CTlvMacAddr & macAddr, BufferObj &msg);

#pragma pack(pop)

#endif // _WSC_REG_PROTO_TLV_
