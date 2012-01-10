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

#include "WscTypes.h"
#include "WscCommon.h"
#include "RegProtoTlv.h"
#include "tutrace.h"
#include "WscHeaders.h"

uint32 GetMacAddrIfM1(CTlvMacAddr & macAddr, BufferObj &msg) 
{
    TUTRACE((TUTRACE_INFO, "RPROTOTLV: In GetMacAddrIfM1: %d byte message\n",
                                msg.Length()));
    try
    {
		CTlvVersion         versionTLV;
		CTlvMsgType         msgType;
		CTlvUuid            uuid;

        //First, deserialize (parse) the message.
        versionTLV    = CTlvVersion(WSC_ID_VERSION, msg);
        msgType       = CTlvMsgType(WSC_ID_MSG_TYPE, msg);

        //First and foremost, check the version and message number.
        //If the major version number matches, assume we can parse it successfully.
		if((WSC_VERSION & 0xF0) == (versionTLV.Value()& 0xF0)) 
		{
			if(WSC_ID_MESSAGE_M1 == msgType.Value())
			{
				uuid          = CTlvUuid(WSC_ID_UUID_E, msg, SIZE_UUID);
				macAddr       = CTlvMacAddr(WSC_ID_MAC_ADDR, msg, SIZE_MAC_ADDR);
				return WSC_SUCCESS;
			}
		}
		return WSC_ERR_INVALID_PARAMETERS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTOTLV: GetMacAddrIfM1 generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTOTLV: GetMacAddrIfM1 generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTOTLV: GetMacAddrIfM1 generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}

//Encrypted settings
void CTlvEncrSettings::parse(BufferObj &theBuf)
{
    //parse the header first. Min data size of the IV + 1 block of data
    parseHdr(WSC_ID_ENCR_SETTINGS, theBuf, SIZE_ENCR_IV + ENCR_DATA_BLOCK_SIZE);

    iv = m_pos;
    ip_encryptedData = theBuf.Advance(SIZE_ENCR_IV);
    encrDataLength = m_len - SIZE_ENCR_IV;
    theBuf.Advance(encrDataLength);
}

void CTlvEncrSettings::write(BufferObj &theBuf)
{
    writeHdr(WSC_ID_ENCR_SETTINGS, SIZE_ENCR_IV+encrDataLength, theBuf);

    //Append the data. Leave the pointer positioned at the start of data.
    m_pos = theBuf.Append(SIZE_ENCR_IV, iv);
    theBuf.Append(encrDataLength, ip_encryptedData);
}

void CTlvOobDevPwd::parse(BufferObj &theBuf)
{
    //For min size, use the size of the hash, 
    //size of the password ID and one byte of password data
    parseHdr(WSC_ID_OOB_DEV_PWD, theBuf, SIZE_DATA_HASH + sizeof(uint16) + sizeof(uint8));

    publicKeyHash = theBuf.Pos();
    pwdId = WscNtohs(*(uint16 *)theBuf.Advance(SIZE_DATA_HASH));
    ip_devPwd = theBuf.Advance(sizeof(uint16));

    devPwdLength = m_len - SIZE_DATA_HASH - sizeof(uint16);
    theBuf.Advance(devPwdLength);
}

void CTlvOobDevPwd::write(BufferObj &theBuf)
{
    writeHdr(WSC_ID_OOB_DEV_PWD, SIZE_DATA_HASH + sizeof(uint16) + devPwdLength, theBuf);

    m_pos = theBuf.Append(SIZE_DATA_HASH, publicKeyHash);
	uint16 netPwdId = WscHtons(pwdId);
    theBuf.Append(sizeof(uint16), (uint8 *)&netPwdId);
    theBuf.Append(devPwdLength, ip_devPwd);
}

void CTlvCredential::parseAKey(BufferObj &theBuf, bool allocate)
{
	nwKey         = CTlvNwKey(WSC_ID_NW_KEY, theBuf, SIZE_64_BYTES, allocate);
	macAddr       = CTlvMacAddr(WSC_ID_MAC_ADDR, theBuf, SIZE_MAC_ADDR, allocate);

	//Parse optional attributes
	if(WSC_ID_EAP_TYPE == theBuf.NextType())
		eapType = CTlvEapType(WSC_ID_EAP_TYPE, theBuf, SIZE_8_BYTES, allocate); 

	if(WSC_ID_EAP_IDENTITY == theBuf.NextType())
		eapIdentity   = CTlvEapId(WSC_ID_EAP_IDENTITY, theBuf, 0, allocate);

	if(WSC_ID_KEY_PROVIDED_AUTO == theBuf.NextType())
		keyProvidedAuto = CTlvKeyProvidedAuto(WSC_ID_KEY_PROVIDED_AUTO, theBuf);

	if(WSC_ID_8021X_ENABLED == theBuf.NextType())
		oneXEnabled = CTlv8021XEnabled(WSC_ID_8021X_ENABLED, theBuf);

	// Now handle additional optional attributes that may
	// appear in arbitrary order.  
	while (theBuf.NextType() != 0) { // more attributes
		uint16 nextType = theBuf.NextType();
		switch (nextType) {
			case WSC_ID_KEY_LIFETIME:
				keyLifetime   = CTlvKeyLifetime(WSC_ID_KEY_LIFETIME, theBuf);
				break;
			case WSC_ID_REKEY_KEY:
				rekeyKey      = CTlvRekeyKey(WSC_ID_REKEY_KEY, theBuf, 0, allocate);
				break;
			case WSC_ID_X509_CERT:
				x509Cert      = CTlvX509Cert(WSC_ID_X509_CERT, theBuf, 0, allocate);
				break;
			case WSC_ID_NW_KEY_INDEX:
				return; // we've reached the end of this network key
			default: // skip over other unknown attributes
				theBuf.Advance( sizeof(S_WSC_TLV_HEADER) + 
							WscNtohs(*(uint16 *)(theBuf.Pos()+sizeof(uint16))) );
				if (theBuf.Pos() - m_pos >= m_len) {
					// Actually, an == check would be more precise, but the >=
					// is slightly safer in preventing buffer overruns.
					return; // we've reached the end of the Credential
				}
				break;
		}
	}
}

void CTlvCredential::parse(BufferObj &theBuf, bool allocate)
{
    //don't bother with min data size. 
    //the respective TLV will handle their sizes
    parseHdr(WSC_ID_CREDENTIAL, theBuf, 0);
    nwIndex       = CTlvNwIndex(WSC_ID_NW_INDEX, theBuf);
    ssid          = CTlvSsid(WSC_ID_SSID, theBuf, SIZE_32_BYTES, allocate);
    authType      = CTlvAuthType(WSC_ID_AUTH_TYPE, theBuf);
    encrType      = CTlvEncrType(WSC_ID_ENCR_TYPE, theBuf);

	// This code only supports a single network key, but it is structured to allow
	// easy extension to parsing multiple keys.

	// If Network Key Index is present, we may have multiple credentials
	while (WSC_ID_NW_KEY_INDEX == theBuf.NextType()) {
		nwKeyIndex = CTlvNwKeyIndex(WSC_ID_NW_KEY_INDEX, theBuf);
		parseAKey(theBuf, allocate);
		break; // only support a single key for now
	}
	// make sure we haven't reached the end of the Credential attribute yet.
	if ((theBuf.Pos() - m_pos < m_len) && WSC_ID_NW_KEY == theBuf.NextType()) {
		nwKeyIndex.Set(WSC_ID_NW_KEY_INDEX,1); // default to key index 1
		parseAKey(theBuf, allocate);
	}
	// Ignore any additional TLVs that might still be unparsed.  To be safe, 
	// explicitly set the buffer position at the end of the Credential TLV length.
	theBuf.Set(m_pos + m_len);
}

void CTlvCredential::write(BufferObj &theBuf)
{
    //Note: we don't support any Vendor Extensions at this time
    uint16 len =  nwIndex.Length()
                + ssid.Length()
                + authType.Length()
                + encrType.Length()
                + nwKeyIndex.Length()
                + nwKey.Length()
                + macAddr.Length()
                + eapType.Length()
                + eapIdentity.Length()
                + keyLifetime.Length()
                + rekeyKey.Length()
                + x509Cert.Length();

    //now include the size of the TLV headers of all the TLVs
    //We have 7 required TLVs, and 6 optional ones
    uint16 tlvCount = 7; //required TLVs
    if(eapType.Length())
        tlvCount++;
    if(eapIdentity.Length())
        tlvCount++;
    if(keyLifetime.Length())
        tlvCount++;
    if(rekeyKey.Length())
        tlvCount++;
    if(x509Cert.Length())
        tlvCount++;

    //now update the length
    len += (sizeof(S_WSC_TLV_HEADER) * tlvCount);

    //First write out the header
    writeHdr(WSC_ID_CREDENTIAL, len, theBuf);

    //Next, write out the required TLVs
    nwIndex.Write(theBuf);
    ssid.Write(theBuf);
    authType.Write(theBuf);
    encrType.Write(theBuf);
    nwKeyIndex.Write(theBuf);
    nwKey.Write(theBuf);
    macAddr.Write(theBuf);

    //Finally, write the optional TLVs. 
    if(eapType.Length())
        eapType.Write(theBuf);

    if(eapIdentity.Length())
        eapIdentity.Write(theBuf);

    if(keyLifetime.Length())
        keyLifetime.Write(theBuf);

    if(rekeyKey.Length())
        rekeyKey.Write(theBuf);

    if(x509Cert.Length())
        x509Cert.Write(theBuf);
}

void CTlvPrimDeviceType::parse(BufferObj &theBuf)
{
    //parse the header first. Data size is a fixed 8 bytes
    parseHdr(WSC_ID_PRIM_DEV_TYPE, theBuf, SIZE_8_BYTES);

    categoryId = WscNtohs(*((__unaligned uint16 *) m_pos));
    oui = WscHtonl(*(__unaligned uint32 *)theBuf.Advance(SIZE_2_BYTES));
    subCategoryId = WscNtohs(*((__unaligned uint16 *)theBuf.Advance(SIZE_4_BYTES)));
    theBuf.Advance(SIZE_2_BYTES);
}

void CTlvPrimDeviceType::write(BufferObj &theBuf)
{
    uint8 temp[sizeof(uint32)];

    writeHdr(WSC_ID_PRIM_DEV_TYPE, SIZE_8_BYTES, theBuf);

    *(uint16 *)temp = WscHtons(categoryId);
    m_pos = theBuf.Append(SIZE_2_BYTES, temp);

    *(uint32 *)temp = WscHtonl(oui);
    theBuf.Append(SIZE_4_BYTES, temp);

    *(uint16 *)temp = WscHtons(subCategoryId);
    theBuf.Append(SIZE_2_BYTES, temp);
}
