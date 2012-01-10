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

#ifndef _WSC_HEADERS_
#define _WSC_HEADERS_

#include "WscTypes.h"
#include "slist.h"
#include "WscTlvBase.h"

//Include the following until we figure out where to put the beacons
#include "RegProtoTlv.h"

#pragma pack(push, 1)

#define WSC_VERSION                0x10

// Beacon Info
typedef struct
{
    CTlvVersion			version;
    CTlvScState			scState;
    CTlvAPSetupLocked	apSetupLocked;
    CTlvSelRegistrar    selRegistrar;
    CTlvDevicePwdId     pwdId;
	CTlvSelRegCfgMethods  selRegConfigMethods;
} WSC_BEACON_IE;

// Probe Request Info
typedef struct {
    CTlvVersion           version;
    CTlvReqType           reqType;
    CTlvConfigMethods     confMethods;
    CTlvUuid              uuid;
    CTlvPrimDeviceType    primDevType;
    CTlvRfBand            rfBand;
    CTlvAssocState        assocState;
    CTlvConfigError       confErr;
    CTlvDevicePwdId       pwdId;
	CTlvPortableDevice	  portableDevice;
    CTlvVendorExt         vendExt;
} WSC_PROBE_REQUEST_IE;

// Probe Response Info
typedef struct {
    CTlvVersion           version;
	CTlvScState			  scState;
    CTlvAPSetupLocked	  apSetupLocked;
    CTlvSelRegistrar      selRegistrar;
    CTlvDevicePwdId       pwdId;
	CTlvSelRegCfgMethods  selRegConfigMethods;
    CTlvRespType          respType;
    CTlvUuid              uuid;
    CTlvManufacturer      manuf;
    CTlvModelName         modelName;
    CTlvModelNumber       modelNumber;
    CTlvSerialNum         serialNumber;
    CTlvPrimDeviceType    primDevType;
    CTlvDeviceName        devName;
	CTlvConfigMethods     confMethods;
    CTlvRfBand            rfBand;
    CTlvVendorExt         vendExt;
} WSC_PROBE_RESPONSE_IE;

#pragma pack(pop)

#endif // _WSC_HEADERS_
