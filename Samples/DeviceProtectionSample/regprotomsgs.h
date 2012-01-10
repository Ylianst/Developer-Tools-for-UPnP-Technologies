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

#ifndef _WSC_MSG_H
#define _WSC_MSG_H

#include "RegProtoTlv.h"

//Message Structures

//Message M1
typedef struct{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvUuid            uuid;
    CTlvMacAddr         macAddr;
    CTlvEnrolleeNonce   enrolleeNonce;
    CTlvPublicKey       publicKey;
    CTlvAuthTypeFlags   authTypeFlags;
    CTlvEncrTypeFlags   encrTypeFlags;
    CTlvConnTypeFlags   connTypeFlags;
    CTlvConfigMethods   configMethods;    
    CTlvScState         scState;
    CTlvManufacturer    manufacturer;
    CTlvModelName       modelName;
    CTlvModelNumber     modelNumber;
    CTlvSerialNum       serialNumber;
    CTlvPrimDeviceType  primDeviceType;
    CTlvDeviceName      deviceName;
    CTlvRfBand          rfBand;
    CTlvAssocState      assocState;
    CTlvDevicePwdId     devPwdId;
    CTlvConfigError     configError;
    CTlvOsVersion       osVersion;
    //CTlvFeatureId     featureId;
    //CTlvSecDeviceType secDevType;
    //CTlvAppList       appList;
    //CTlvIdentity      identity;
    //CTlvPortableDevice portableDev;
    //CTlvVendorExt     vendorExt; Not supported
} S_WSC_M1;

//Message M2
typedef struct{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvEnrolleeNonce   enrolleeNonce;
    CTlvRegistrarNonce  registrarNonce;
    CTlvUuid            uuid;
    CTlvPublicKey       publicKey;
    CTlvAuthTypeFlags   authTypeFlags;
    CTlvEncrTypeFlags   encrTypeFlags;
    CTlvConnTypeFlags   connTypeFlags;
    CTlvConfigMethods   configMethods;    
    CTlvManufacturer    manufacturer;
    CTlvModelName       modelName;
    CTlvModelNumber     modelNumber;
    CTlvSerialNum       serialNumber;
    CTlvPrimDeviceType  primDeviceType;
    CTlvDeviceName      deviceName;
    CTlvRfBand          rfBand;
    CTlvAssocState      assocState;
    CTlvConfigError     configError;
    CTlvDevicePwdId     devPwdId;
    CTlvOsVersion       osVersion;
    //CTlvFeatureId     featureId;
    //CTlvReqType       reqType;
    //CTlvSecDeviceType secDevType;
    //CTlvAppList       appList;
    //CTlvPortableDevice portableDev;
    CTlvEncrSettings    encrSettings;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   authenticator;
} S_WSC_M2;
   
//Message M2D
typedef struct{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvEnrolleeNonce   enrolleeNonce;
    CTlvRegistrarNonce  registrarNonce;
    CTlvUuid            uuid;
    CTlvAuthTypeFlags   authTypeFlags;
    CTlvEncrTypeFlags   encrTypeFlags;
    CTlvConnTypeFlags   connTypeFlags;
    CTlvConfigMethods   configMethods;    
    CTlvManufacturer    manufacturer;
    CTlvModelName       modelName;
    CTlvModelNumber     modelNumber;
    CTlvSerialNum       serialNumber;
    CTlvPrimDeviceType  primDeviceType;
    CTlvDeviceName      deviceName;
    CTlvRfBand          rfBand;
    CTlvAssocState      assocState;
    CTlvConfigError     configError;
    CTlvDevicePwdId     devPwdId;
    //CTlvFeatureId     featureId; Not supported
    //CTlvReqType       reqType;
    //CTlvSecDeviceType secDevType;
    //CTlvAppList       appList;
    //CTlvPortableDevice portableDev;
    //CTlvVendorExt     vendorExt; 
} S_WSC_M2D;

//Message M3
typedef struct{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvRegistrarNonce  registrarNonce;
    CTlvHash            eHash1;
    CTlvHash            eHash2;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   authenticator;
}S_WSC_M3;

//Message M4
typedef struct
{
    CTlvVersion          version;
    CTlvMsgType          msgType;
    CTlvEnrolleeNonce    enrolleeNonce;
    CTlvHash             rHash1;
    CTlvHash             rHash2;
    CTlvEncrSettings     encrSettings;
    //CTlvVendorExt      vendorExt; Not supported
    CTlvAuthenticator    authenticator;
}S_WSC_M4;

//Message M5
typedef struct
{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvRegistrarNonce  registrarNonce;
    CTlvEncrSettings    encrSettings;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   authenticator;
}S_WSC_M5;

//Message M6
typedef struct
{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvEnrolleeNonce   enrolleeNonce;
    CTlvEncrSettings    encrSettings;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator  authenticator;
}S_WSC_M6;

//Message M7
typedef struct
{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvRegistrarNonce  registrarNonce;
    CTlvEncrSettings    encrSettings;
    CTlvX509CertReq     x509CertReq;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   authenticator;
}S_WSC_M7;

//Message M8
typedef struct
{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvEnrolleeNonce   enrolleeNonce;
    CTlvEncrSettings    encrSettings;
    CTlvX509Cert        x509Cert;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   authenticator;
}S_WSC_M8;

//ACK and DONE Messages
typedef struct
{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvEnrolleeNonce   enrolleeNonce;
    CTlvRegistrarNonce  registrarNonce;
    //CTlvVendorExt     vendorExt; Not supported
}S_WSC_ACK, S_WSC_DONE;

//NACK Message
typedef struct
{
    CTlvVersion         version;
    CTlvMsgType         msgType;
    CTlvEnrolleeNonce   enrolleeNonce;
    CTlvRegistrarNonce  registrarNonce;
    CTlvConfigError     configError;
    //CTlvVendorExt     vendorExt; Not supported
}S_WSC_NACK;

//Encrypted settings for various messages

//M4, M5, M6 - contain only Nonce and vendor extension
class CTlvEsNonce
{
public:
    CTlvNonce           nonce; //could be RS1, ES1 or RS2
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   keyWrapAuth; //reuse Authenticator data struct

    void parse(uint16 theType, BufferObj &theBuf, BufferObj &authKey);
    void write(BufferObj &theBuf, BufferObj &authKey);
};

//M7
class CTlvEsM7Enr
{
public:
    CTlvNonce           nonce; //ES2
    CTlvIdentityProof   idProof;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   keyWrapAuth; //reuse Authenticator data struct

    void parse(BufferObj &theBuf, BufferObj &authKey, bool allocate = false);
    void write(BufferObj &theBuf, BufferObj &authKey);
};

class CTlvEsM7Ap
{
public:
    CTlvNonce           nonce; //ES2
    CTlvSsid            ssid;
    CTlvMacAddr         macAddr;
    CTlvAuthType        authType;
    CTlvEncrType        encrType;
    LPLIST              nwKeyIndex;
    LPLIST              nwKey;
    //CTlvPermittedCfgMethods permCfgMethods;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   keyWrapAuth; //reuse Authenticator data struct

    CTlvEsM7Ap();
    ~CTlvEsM7Ap();
    void parse(BufferObj &theBuf, BufferObj &authKey, bool allocate = false);
    void write(BufferObj &theBuf, BufferObj &authKey);
};

//M8
class CTlvEsM8Ap
{
public:
    CTlvNwIndex        nwIndex;
    CTlvSsid           ssid;
    CTlvAuthType       authType;
    CTlvEncrType       encrType;
    LPLIST             nwKeyIndex;
    LPLIST             nwKey;
    CTlvMacAddr        macAddr;
    CTlvNewPwd         new_pwd;
    CTlvDevicePwdId    pwdId;
    //CTlvPermittedCfgMethods permCfgMethods;
    //CTlvVendorExt    vendorExt; Not supported
    CTlvAuthenticator  keyWrapAuth; //reuse Authenticator data struct

    CTlvEsM8Ap();
    ~CTlvEsM8Ap();
    void parse(BufferObj &theBuf, BufferObj &authKey, bool allocate = false);
    void write(BufferObj &theBuf, BufferObj &authKey);
};

class CTlvEsM8Sta
{
public:
    LPLIST              credential;
    CTlvNewPwd          new_pwd;
    CTlvDevicePwdId     pwdId;
    //CTlvVendorExt     vendorExt; Not supported
    CTlvAuthenticator   keyWrapAuth; //reuse Authenticator data struct

    CTlvEsM8Sta();
    ~CTlvEsM8Sta();
    void parse(BufferObj &theBuf, BufferObj &authKey, bool allocate = false, bool isWirelessWPS = false);
    void write(BufferObj &theBuf, BufferObj &authKey, bool isWirelessWPS = false);
    void write(BufferObj &theBuf);
	void writeGenericCredentials(BufferObj &theBuf);
};

#endif //_WSC_MSG_H
