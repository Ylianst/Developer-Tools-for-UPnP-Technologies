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

#ifndef _WSC_COMMON_
#define _WSC_COMMON_

#include "WscTypes.h"

#pragma pack(push, 1)

#define REGISTRAR_ID_STRING        "WFA-SimpleConfig-Registrar-1-0"
#define ENROLLEE_ID_STRING        "WFA-SimpleConfig-Enrollee-1-0"

#define BUF_SIZE_64_BITS    8
#define BUF_SIZE_128_BITS   16
#define BUF_SIZE_160_BITS   20
#define BUF_SIZE_256_BITS   32
#define BUF_SIZE_512_BITS   64
#define BUF_SIZE_1024_BITS  128
#define BUF_SIZE_1536_BITS  192

#define PERSONALIZATION_STRING  "Wi-Fi Easy and Secure Key Derivation"
#define UPNP_PERSONALIZATION_STRING  "UPnP Protected Setup Key Derivation"
#define PRF_DIGEST_SIZE         BUF_SIZE_256_BITS
#define KDF_KEY_BITS            640

// data structure to hold Enrollee and Registrar information
typedef struct {
    uint8   version;
    uint8   uuid[SIZE_16_BYTES];
    uint8   macAddr[SIZE_6_BYTES];
    char    deviceName[SIZE_32_BYTES];
    uint16  primDeviceCategory;
    uint32  primDeviceOui;
    uint16  primDeviceSubCategory; 
    uint16  authTypeFlags;
    uint16  encrTypeFlags;
    uint8   connTypeFlags;
    uint16  configMethods;
    uint8   scState;
    char    manufacturer[SIZE_64_BYTES];
    char    modelName[SIZE_32_BYTES];
    char    modelNumber[SIZE_32_BYTES];
    char    serialNumber[SIZE_32_BYTES];
    uint8   rfBand;
    uint32  osVersion;
    uint32  featureId;
    uint16  assocState;
    uint16  devPwdId;
    uint16  configError;
    bool    b_ap;
    char    ssid[SIZE_32_BYTES + 1];
    char    keyMgmt[SIZE_20_BYTES];
} S_DEVICE_INFO;

typedef enum {
	EModeUnknown = 0,
    EModeUnconfAp = 1,
    EModeClient = 2,
    EModeRegistrar = 3,
    EModeApProxy = 4,
	EModeApProxyRegistrar = 5
} EMode;

typedef enum {
	OOBD_TYPE_UNENCRYPTED,
	OOBD_TYPE_ENCRYPTED,
	OOBD_TYPE_ENROLLEE_PIN,
	OOBD_TYPE_REGISTRAR_PIN
} EOobDataType;

// callback function prototype
typedef void (*CALLBACK_FN)( void *p_callbackMsg, void *p_callbackObj );

typedef struct {
    CALLBACK_FN  pf_callback;
    void *       p_cookie;
} S_CALLBACK_INFO;

typedef enum 
{
    CB_QUIT = 0,
    CB_TRUFD = 1,
    CB_TRNFC = 2,
    CB_TREAP = 3,
    CB_TRWLAN_BEACON = 4,
    CB_TRWLAN_PR_REQ = 5,
    CB_TRWLAN_PR_RESP = 6,
    CB_TRANS = 7,
    CB_MAIN_PUSH_MSG = 8,
    CB_MAIN_START_AP = 9,
    CB_MAIN_STOP_AP = 10,
    CB_MAIN_START_WPASUPP = 11,
    CB_MAIN_RESET_WPASUPP = 12,
    CB_MAIN_STOP_WPASUPP = 13,
    CB_MAIN_PUSH_MODE = 14,
    CB_MAIN_NEW_STA = 15,
    CB_MAIN_NFC_DATA = 16,
	CB_MAIN_PUSH_REG_RESULT = 17,
	CB_MAIN_REQUEST_PWD = 18,
    CB_TRIP = 19,
    CB_SM = 20,
    CB_SM_RESET = 21,
    CB_TRUFD_INSERTED = 22,
    CB_TRUFD_REMOVED = 23,
    CB_TRUPNP_CP = 24,
    CB_TRUPNP_DEV = 25,
    CB_TRUPNP_DEV_SSR = 26,
	CB_SSR_TIMEOUT = 27,
    CB_TRNFC_LOST = 28
} ECBType;

typedef struct 
{
    ECBType	eType;
    uint32  dataLength;
} S_CB_HEADER, S_CB_MAIN_START_WPASUPP, S_CB_MAIN_NEW_STA;

// For those callbacks that do not have extra data 
typedef struct 
{
    S_CB_HEADER     cbHeader;
    // data follows right here
} S_CB_COMMON;

// Extra structures for those that have extra data. This data
// follows the CB_HEADER and CB_HEADER.dataLength includes the
// length of the structure and any data that follows

// CB_TRWLAN used for Beacons, Probe Reqs, and Probe Responses
typedef struct 
{
    S_CB_HEADER cbHeader;
	char        ssid[SIZE_32_BYTES];
    uint8       macAddr[SIZE_MAC_ADDR];
    char        data[1];
} S_CB_TRWLAN;

typedef struct
{
    S_CB_HEADER cbHeader;
    // contains NULL-terminated string
    char        c_msg[1];
} S_CB_MAIN_PUSH_MSG, S_CB_MAIN_NFC_DATA, S_CB_DATA_TEMPLATE;

typedef struct
{
    S_CB_HEADER cbHeader;
    char        ssid[SIZE_32_BYTES];
    char        keyMgmt[SIZE_20_BYTES];
    char        nwKey[SIZE_64_BYTES];
	uint32		nwKeyLen;
    bool        b_restart;
    bool        b_configured;
} S_CB_MAIN_START_AP;

typedef struct
{
    S_CB_HEADER cbHeader;
    char        ssid[SIZE_32_BYTES];
    char        keyMgmt[SIZE_20_BYTES];
    char        nwKey[SIZE_64_BYTES];
	uint32		nwKeyLen;
    char        identity[SIZE_32_BYTES];
    bool        b_startWsc;
} S_CB_MAIN_RESET_WPASUPP;

typedef struct 
{
    S_CB_HEADER	cbHeader;
    EMode		e_mode;
	bool		b_useUsbKey;
	bool		b_useNfc;
	bool		b_useUpnp;
} S_CB_MAIN_PUSH_MODE;

typedef struct
{
	S_CB_HEADER	cbHeader;
	bool		b_result;
} S_CB_MAIN_PUSH_REG_RESULT;

typedef struct
{
	S_CB_HEADER	cbHeader;
	char		deviceName[SIZE_32_BYTES];
    char		modelNumber[SIZE_32_BYTES];
    char		serialNumber[SIZE_32_BYTES];
	uint8		uuid[SIZE_16_BYTES];
} S_CB_MAIN_REQUEST_PWD;

typedef struct 
{
    S_CB_HEADER		cbHeader;
    uint32			result;
    void			*encrSettings;
    S_DEVICE_INFO	*peerInfo;
} S_CB_SM;

#define SM_FAILURE    0
#define SM_SUCCESS    1
#define SM_SET_PASSWD 2

#define WSC_WLAN_EVENT_TYPE_EAP_FRAME 2

#pragma pack(pop)

#define GET_UUID_PRINT_STR(inp, outp)  \
    sprintf(outp, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",\
     inp[0], inp[1], inp[2], inp[3], inp[4], inp[5], inp[6], inp[7], \
     inp[8], inp[9], inp[10], inp[11], inp[12], inp[13], inp[14], inp[15] );
    
#endif // _WSC_COMMON_
