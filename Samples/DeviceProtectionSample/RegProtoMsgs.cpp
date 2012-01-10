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

//OpenSSL includes
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "slist.h"
#include "tutrace.h"
#include "WscTypes.h"
#include "WscCommon.h"
#include "WscError.h"
#include "RegProtoMsgs.h"

//Encrypted settings for M4, M5, M6
void CTlvEsNonce::parse(uint16 theType, BufferObj &theBuf, BufferObj &authKey)
{
    nonce = CTlvNonce(theType, theBuf, SIZE_128_BITS);
    
	// Skip attributes until the KeyWrapAuthenticator
	while(WSC_ID_KEY_WRAP_AUTH != theBuf.NextType())
    {
        //advance past the TLV
        uint8 *Pos = theBuf.Advance( sizeof(S_WSC_TLV_HEADER) + 
							WscNtohs(*(uint16 *)(theBuf.Pos()+sizeof(uint16))) );
        
        //If Advance returned NULL, it means there's no more data in the
        //buffer. This is an error.
        if(Pos == NULL)
            throw RPROT_ERR_REQD_TLV_MISSING;
    }

	uint8 * startOfAuthenticator = theBuf.Pos();
    keyWrapAuth = CTlvAuthenticator(WSC_ID_KEY_WRAP_AUTH, theBuf, SIZE_64_BITS);

    //validate the mac
    uint8 dataMac[BUF_SIZE_256_BITS];

    //calculate the hmac of the data (data only, not the last auth TLV)
    if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, theBuf.GetBuf(), 
            startOfAuthenticator - theBuf.GetBuf(), 
            dataMac, NULL) == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    //compare it against the received hmac
    if(memcmp(dataMac, keyWrapAuth.Value(), SIZE_64_BITS) != 0)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC results don't match\n"));
        throw RPROT_ERR_INVALID_VALUE;
    }
}

void CTlvEsNonce::write(BufferObj &theBuf, BufferObj &authKey)
{
    nonce.Write(theBuf);

    //calculate the hmac and append the TLV to the buffer
    uint8 hmac[SIZE_256_BITS];
    if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, 
            theBuf.GetBuf(), theBuf.Length(), hmac, NULL) 
        == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
        throw RPROT_ERR_CRYPTO;
    }

    CTlvAuthenticator(
                    WSC_ID_KEY_WRAP_AUTH,
                    theBuf,
                    hmac,
                    SIZE_64_BITS); //Only the first 64 bits are sent
}

//Encrypted settings for M7
//ES when M7 is from an enrollee
void CTlvEsM7Enr::parse(BufferObj &theBuf, BufferObj &authKey, bool allocate)
{
    nonce = CTlvNonce(WSC_ID_E_SNONCE2, theBuf, SIZE_128_BITS);

    if(WSC_ID_IDENTITY_PROOF == theBuf.NextType())
        idProof = CTlvIdentityProof(WSC_ID_IDENTITY_PROOF, theBuf, 0, allocate);

	// Skip attributes until the KeyWrapAuthenticator
	while(WSC_ID_KEY_WRAP_AUTH != theBuf.NextType())
    {
        //advance past the TLV
        uint8 *Pos = theBuf.Advance( sizeof(S_WSC_TLV_HEADER) + 
							WscNtohs(*(uint16 *)(theBuf.Pos()+sizeof(uint16))) );
        
        //If Advance returned NULL, it means there's no more data in the
        //buffer. This is an error.
        if(Pos == NULL)
            throw RPROT_ERR_REQD_TLV_MISSING;
    }

	uint8 * startOfAuthenticator = theBuf.Pos();
    keyWrapAuth = CTlvAuthenticator(WSC_ID_KEY_WRAP_AUTH, theBuf, SIZE_64_BITS);

    //validate the mac
    uint8 dataMac[BUF_SIZE_256_BITS];

    //calculate the hmac of the data (data only, not the last auth TLV)
    if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, theBuf.GetBuf(),
            startOfAuthenticator - theBuf.GetBuf(), 
            dataMac, NULL) == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    //compare it against the received hmac
    if(memcmp(dataMac, keyWrapAuth.Value(), SIZE_64_BITS) != 0)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC results don't match\n"));
        throw RPROT_ERR_INVALID_VALUE;
    }
}

void CTlvEsM7Enr::write(BufferObj &theBuf, BufferObj &authKey)
{
    nonce.Write(theBuf);
    if(idProof.Length())
        idProof.Write(theBuf);

    //calculate the hmac and append the TLV to the buffer
    uint8 hmac[SIZE_256_BITS];
    if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, 
            theBuf.GetBuf(), theBuf.Length(), hmac, NULL) 
        == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
        throw RPROT_ERR_CRYPTO;
    }

    CTlvAuthenticator(
                    WSC_ID_KEY_WRAP_AUTH,
                    theBuf,
                    hmac,
                    SIZE_64_BITS);
}

//ES when M7 is from an AP
CTlvEsM7Ap::CTlvEsM7Ap()
{
    nwKeyIndex = ListCreate();
    if(!nwKeyIndex)
        throw WSC_ERR_OUTOFMEMORY;

    nwKey = ListCreate();
    if(!nwKey)
        throw WSC_ERR_OUTOFMEMORY;
}

CTlvEsM7Ap::~CTlvEsM7Ap()
{
    LPLISTITR itr;
    CTlvNwKeyIndex  *keyIndex;
    CTlvNwKey        *key;

    itr = ListItrCreate(nwKeyIndex);
    if(!itr)
        throw WSC_ERR_OUTOFMEMORY;

    while((keyIndex = (CTlvNwKeyIndex *)ListItrGetNext(itr)))
    {
        delete keyIndex;
    }

    ListItrDelete(itr);
    ListDelete(nwKeyIndex);

    itr = ListItrCreate(nwKey);
    if(!itr)
        throw WSC_ERR_OUTOFMEMORY;

    while((key = (CTlvNwKey *)ListItrGetNext(itr)))
    {
        delete key;
    }

    ListItrDelete(itr);
    ListDelete(nwKey);
}

#ifdef WSC_AP_DEFINED
void CTlvEsM7Ap::parse(BufferObj &theBuf, BufferObj &authKey, bool allocate)
{
    CTlvNwKeyIndex  *keyIndex;
    CTlvNwKey        *key;

    nonce = CTlvNonce(WSC_ID_E_SNONCE2, theBuf, SIZE_128_BITS);
    ssid = CTlvSsid(WSC_ID_SSID, theBuf, SIZE_32_BYTES, allocate);
    macAddr = CTlvMacAddr(WSC_ID_MAC_ADDR, theBuf, SIZE_6_BYTES, allocate);
    authType = CTlvAuthType(WSC_ID_AUTH_TYPE, theBuf);
    encrType = CTlvEncrType(WSC_ID_ENCR_TYPE, theBuf);

    //The next field is network key index. There are two possibilities:
    //1. The TLV is omitted, in which case, there is only 1 network key
    //2. The TLV is present, in which case, there may be 1 or more network keys
    
    //condition 1. If the next field is a network Key, the index TLV was omitted
    if(WSC_ID_NW_KEY == theBuf.NextType())
    {
        key = new CTlvNwKey(WSC_ID_NW_KEY, theBuf, SIZE_64_BYTES, allocate);
        if(!key)
            throw WSC_ERR_OUTOFMEMORY;
        ListAddItem(nwKey, key);
    }
    else
    {
        //condition 2. all other possibities are illegal & will be caught later
        while(WSC_ID_NW_KEY_INDEX == theBuf.NextType())
        {
            keyIndex = new CTlvNwKeyIndex(WSC_ID_NW_KEY_INDEX, theBuf);
            if(!keyIndex)
                throw WSC_ERR_OUTOFMEMORY;
            ListAddItem(nwKeyIndex, keyIndex);

            key = new CTlvNwKey(WSC_ID_NW_KEY, theBuf, SIZE_64_BYTES, allocate);
            if(!key)
                throw WSC_ERR_OUTOFMEMORY;
            ListAddItem(nwKey, key);
        }//while
    }//else

	// Skip attributes until the KeyWrapAuthenticator
	while(WSC_ID_KEY_WRAP_AUTH != theBuf.NextType())
    {
        //advance past the TLV
        uint8 *Pos = theBuf.Advance( sizeof(S_WSC_TLV_HEADER) + 
							WscNtohs(*(uint16 *)(theBuf.Pos()+sizeof(uint16))) );
        
        //If Advance returned NULL, it means there's no more data in the
        //buffer. This is an error.
        if(Pos == NULL)
            throw RPROT_ERR_REQD_TLV_MISSING;
    }

	uint8 * startOfAuthenticator = theBuf.Pos();
    keyWrapAuth = CTlvAuthenticator(WSC_ID_KEY_WRAP_AUTH, theBuf, SIZE_64_BITS);

    //validate the mac
    uint8 dataMac[BUF_SIZE_256_BITS];

    //calculate the hmac of the data (data only, not the last auth TLV)
    if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, theBuf.GetBuf(),
            startOfAuthenticator - theBuf.GetBuf(), 
            dataMac, NULL) == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    //compare it against the received hmac
    if(memcmp(dataMac, keyWrapAuth.Value(), SIZE_64_BITS) != 0)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC results don't match\n"));
        throw RPROT_ERR_INVALID_VALUE;
    }
}

void CTlvEsM7Ap::write(BufferObj &theBuf, BufferObj &authKey)
{
    LPLISTITR indexItr, keyItr;
    CTlvNwKeyIndex  *keyIndex;
    CTlvNwKey        *key;

    try
    {
        indexItr = ListItrCreate(nwKeyIndex);
        if(!indexItr)
            throw WSC_ERR_OUTOFMEMORY;

        keyItr = ListItrCreate(nwKey);
        if(!keyItr)
            throw WSC_ERR_OUTOFMEMORY;

        nonce.Write(theBuf);
        ssid.Write(theBuf);
        macAddr.Write(theBuf);
        authType.Write(theBuf);
        encrType.Write(theBuf);

        //write the network index and network key to the buffer
        if(ListGetCount(nwKeyIndex) == 0)
        {
            //Condition1. There is no key index, so there can only be 1 nw key
            if(!(key = (CTlvNwKey *) ListItrGetNext(keyItr)))
                throw WSC_ERR_OUTOFMEMORY;
            key->Write(theBuf);
        }
        else
        {
            //Condition2. There are multiple network keys.
            while((keyIndex= (CTlvNwKeyIndex *) ListItrGetNext(indexItr)))
            {
                if(!(key = (CTlvNwKey *) ListItrGetNext(keyItr)))
                    throw WSC_ERR_OUTOFMEMORY;
                keyIndex->Write(theBuf);
                key->Write(theBuf);
            }//while
        }//else

        //calculate the hmac and append the TLV to the buffer
        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, 
                theBuf.GetBuf(), theBuf.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_KEY_WRAP_AUTH,
                        theBuf,
                        hmac,
                        SIZE_64_BITS);

        ListItrDelete(indexItr);
        ListItrDelete(keyItr);
    }
    catch(...)
    {
        if(indexItr)
            ListItrDelete(indexItr);
        if(keyItr)
            ListItrDelete(keyItr);

        throw;
    }
}

//Encrypted settings for M8
//ES when M8 is from an AP
CTlvEsM8Ap::CTlvEsM8Ap()
{
    if(!(nwKeyIndex = ListCreate()))
        throw WSC_ERR_OUTOFMEMORY;

    if(!(nwKey = ListCreate()))
        throw WSC_ERR_OUTOFMEMORY;

}

CTlvEsM8Ap::~CTlvEsM8Ap()
{
    LPLISTITR itr;
    CTlvNwKeyIndex  *keyIndex;
    CTlvNwKey        *key;

    if(!(itr = ListItrCreate(nwKeyIndex)))
        throw WSC_ERR_OUTOFMEMORY;

    while((keyIndex = (CTlvNwKeyIndex *)ListItrGetNext(itr)))
    {
        delete keyIndex;
    }

    ListItrDelete(itr);

    if(!(itr = ListItrCreate(nwKey)))
        throw WSC_ERR_OUTOFMEMORY;

    while((key = (CTlvNwKey *)ListItrGetNext(itr)))
    {
        delete key;
    }

    ListItrDelete(itr);
}

void CTlvEsM8Ap::parse(BufferObj &theBuf, BufferObj &authKey, bool allocate)
{
    CTlvNwKeyIndex  *keyIndex;
    CTlvNwKey        *key;

    //NW Index is optional
    if(WSC_ID_NW_INDEX == theBuf.NextType())
        nwIndex = CTlvNwIndex(WSC_ID_NW_INDEX, theBuf);

    ssid  = CTlvSsid(WSC_ID_SSID, theBuf, SIZE_32_BYTES, allocate);
    authType = CTlvAuthType(WSC_ID_AUTH_TYPE, theBuf);
    encrType = CTlvEncrType(WSC_ID_ENCR_TYPE, theBuf);
   
    //The next field is network key index. There are two possibilities:
    //1. The TLV is omitted, in which case, there is only 1 network key
    //2. The TLV is present, in which case, there may be 1 or more network keys
    
    //condition 1. If the next field is a network Key, the index TLV was omitted
    if(WSC_ID_NW_KEY == theBuf.NextType())
    {
        key = new CTlvNwKey(WSC_ID_NW_KEY, theBuf, SIZE_64_BYTES, allocate);
        if(!key)
            throw WSC_ERR_OUTOFMEMORY;
        ListAddItem(nwKey, key);
    }
    else
    {
        //condition 2. any other possibities are illegal & will be caught later
        while(WSC_ID_NW_KEY_INDEX == theBuf.NextType())
        {
            keyIndex = new CTlvNwKeyIndex(WSC_ID_NW_KEY_INDEX, theBuf);
            if(!keyIndex)
                throw WSC_ERR_OUTOFMEMORY;
            ListAddItem(nwKeyIndex, keyIndex);

            key = new CTlvNwKey(WSC_ID_NW_KEY, theBuf, SIZE_64_BYTES, allocate);
            if(!key)
                throw WSC_ERR_OUTOFMEMORY;
            ListAddItem(nwKey, key);
        }//while
    }//else

    macAddr = CTlvMacAddr(WSC_ID_MAC_ADDR, theBuf, SIZE_6_BYTES, allocate);

    if(WSC_ID_NEW_PWD == theBuf.NextType())
    {
       //If the New Password TLV is included, the Device password ID is required
        new_pwd = CTlvNewPwd(WSC_ID_NEW_PWD, theBuf, SIZE_64_BYTES, allocate);
        pwdId = CTlvDevicePwdId(WSC_ID_DEVICE_PWD_ID, theBuf);
    }

    //skip Permitted Config Methods field.
    if(WSC_ID_PERM_CFG_METHODS == theBuf.NextType())
    {
        //advance past the TLV
        theBuf.Advance( sizeof(S_WSC_TLV_HEADER) + 
                        WscNtohs(*(uint16 *)(theBuf.Pos()+sizeof(uint16))) );
    }

	// Skip attributes until the KeyWrapAuthenticator
	while(WSC_ID_KEY_WRAP_AUTH != theBuf.NextType())
    {
        //advance past the TLV
        uint8 *Pos = theBuf.Advance( sizeof(S_WSC_TLV_HEADER) + 
							WscNtohs(*(uint16 *)(theBuf.Pos()+sizeof(uint16))) );
        
        //If Advance returned NULL, it means there's no more data in the
        //buffer. This is an error.
        if(Pos == NULL)
            throw RPROT_ERR_REQD_TLV_MISSING;
    }

	uint8 * startOfAuthenticator = theBuf.Pos();
    keyWrapAuth = CTlvAuthenticator(WSC_ID_KEY_WRAP_AUTH, theBuf, SIZE_64_BITS);

    //validate the mac
    uint8 dataMac[BUF_SIZE_256_BITS];

    //calculate the hmac of the data (data only, not the last auth TLV)
    if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, theBuf.GetBuf(),
            startOfAuthenticator - theBuf.GetBuf(), 
            dataMac, NULL) == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    //compare it against the received hmac
    if(memcmp(dataMac, keyWrapAuth.Value(), SIZE_64_BITS) != 0)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC results don't match\n"));
        throw RPROT_ERR_INVALID_VALUE;
    }
}

void CTlvEsM8Ap::write(BufferObj &theBuf, BufferObj &authKey)
{
    LPLISTITR indexItr, keyItr;
    CTlvNwKeyIndex  *keyIndex;
    CTlvNwKey        *key;

    try
    {
        if(!(indexItr = ListItrCreate(nwKeyIndex)))
            throw WSC_ERR_OUTOFMEMORY;

        if(!(keyItr = ListItrCreate(nwKey)))
            throw WSC_ERR_OUTOFMEMORY;

        //nwIndex is an optional field
        if(nwIndex.Length())
            nwIndex.Write(theBuf);
        
        ssid.Write(theBuf);
        authType.Write(theBuf);
        encrType.Write(theBuf);

        //write the network index and network key to the buffer
        if(ListGetCount(nwKeyIndex) == 0)
        {
            //Condition1. There is no key index, so there can only be 1 nw key
            if(!(key = (CTlvNwKey *) ListItrGetNext(keyItr)))
                throw WSC_ERR_OUTOFMEMORY;
            key->Write(theBuf);
        }
        else
        {
            //Condition2. There are multiple network keys.
            while((keyIndex= (CTlvNwKeyIndex *) ListItrGetNext(indexItr)))
            {
                if(!(key = (CTlvNwKey *) ListItrGetNext(keyItr)))
                    throw WSC_ERR_OUTOFMEMORY;
                keyIndex->Write(theBuf);
                key->Write(theBuf);
            }//while
        }//else

        //write the mac address
        macAddr.Write(theBuf);

        //write the optional new password and device password ID
        if(new_pwd.Length())
        {
            new_pwd.Write(theBuf);
            pwdId.Write(theBuf);
        }

        //calculate the hmac and append the TLV to the buffer
        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, 
                theBuf.GetBuf(), theBuf.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_KEY_WRAP_AUTH,
                        theBuf,
                        hmac,
                        SIZE_64_BITS);

        ListItrDelete(indexItr);
        ListItrDelete(keyItr);
    }
    catch(...)
    {
        if(indexItr)
            ListItrDelete(indexItr);
        if(keyItr)
            ListItrDelete(keyItr);

        throw;
    }
}
#endif

//ES when M8 is from a STA
CTlvEsM8Sta::CTlvEsM8Sta()
{
    if(!(credential = ListCreate()))
        throw WSC_ERR_OUTOFMEMORY;
}

CTlvEsM8Sta::~CTlvEsM8Sta()
{
    LPLISTITR itr;
    CTlvCredential *pCredential;

    if(!(itr = ListItrCreate(credential)))
        throw WSC_ERR_OUTOFMEMORY;

    while((pCredential = (CTlvCredential *)ListItrGetNext(itr)))
        delete pCredential;

    ListItrDelete(itr);
    ListDelete(credential);
}

void CTlvEsM8Sta::parse(BufferObj &theBuf, BufferObj &authKey, bool allocate, bool isWirelessWPS)
{
    //There should be at least 1 credential TLV
    CTlvCredential *pCredential;

	if (isWirelessWPS) { // Credential(s) are processed only when doing wireless WPS
		pCredential = new CTlvCredential();
		pCredential->parse(theBuf, allocate);
		ListAddItem(credential, pCredential);

		//now parse any additional credential TLVs
		while(WSC_ID_CREDENTIAL == theBuf.NextType())
		{
			pCredential = new CTlvCredential();
			pCredential->parse(theBuf, allocate);
			ListAddItem(credential, pCredential);
		}
	} 
	// Note that all Credentials are ignored if not doing wireless WPS.  Also, if a Credential
	// is present in this case, then the new password attribute will be ignored as well, because 
	// the Credential attribute will be next to be parsed in the message, which will cause the code 
	// to drop down to just skipping all attributes until the key wrap authenticator.

    if(WSC_ID_NEW_PWD == theBuf.NextType())
    {
       //If the New Password TLV is included, the Device password ID is required
        new_pwd = CTlvNewPwd(WSC_ID_NEW_PWD, theBuf, SIZE_64_BYTES, allocate);
        pwdId = CTlvDevicePwdId(WSC_ID_DEVICE_PWD_ID, theBuf);
    }

	// Skip attributes until the KeyWrapAuthenticator
	while(WSC_ID_KEY_WRAP_AUTH != theBuf.NextType())
    {
        //advance past the TLV
        uint8 *Pos = theBuf.Advance( sizeof(S_WSC_TLV_HEADER) + 
							WscNtohs(*(uint16 *)(theBuf.Pos()+sizeof(uint16))) );
        
        //If Advance returned NULL, it means there's no more data in the
        //buffer. This is an error.
        if(Pos == NULL)
            throw RPROT_ERR_REQD_TLV_MISSING;
    }

	uint8 * startOfAuthenticator = theBuf.Pos();
    keyWrapAuth = CTlvAuthenticator(WSC_ID_KEY_WRAP_AUTH, theBuf, SIZE_64_BITS);

    //validate the mac
    uint8 dataMac[BUF_SIZE_256_BITS];

    //calculate the hmac of the data (data only, not the last auth TLV)
    if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, theBuf.GetBuf(),
            startOfAuthenticator - theBuf.GetBuf(), 
            dataMac, NULL) == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    //compare it against the received hmac
    if(memcmp(dataMac, keyWrapAuth.Value(), SIZE_64_BITS) != 0)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC results don't match\n"));
        throw RPROT_ERR_INVALID_VALUE;
    }
}

void CTlvEsM8Sta::write(BufferObj &theBuf)
{
    LPLISTITR itr;
    CTlvCredential *pCredential;

    //there should be at least one credential TLV
    if(0 == ListGetCount(credential))
        throw RPROT_ERR_REQD_TLV_MISSING;

    try
    {
        if(!(itr = ListItrCreate(credential)))
            throw WSC_ERR_OUTOFMEMORY;

        while((pCredential = (CTlvCredential *)ListItrGetNext(itr)))
        {
            pCredential->write(theBuf);
        }

        ListItrDelete(itr);  
    }
    catch(...)
    {
        if(itr)
            ListItrDelete(itr);
    }
}

void CTlvEsM8Sta::writeGenericCredentials(BufferObj &theBuf)
{
    LPLISTITR itr;
    CTlvCredential *pCredential;

    //there should be at least one credential TLV
    if(0 == ListGetCount(credential))
        throw RPROT_ERR_REQD_TLV_MISSING;

    try
    {
        if(!(itr = ListItrCreate(credential)))
            throw WSC_ERR_OUTOFMEMORY;

		uint8   macAddr[SIZE_6_BYTES];
		memset(macAddr,0,sizeof(macAddr));

        while((pCredential = (CTlvCredential *)ListItrGetNext(itr)))
        {
			uint8 * pos = theBuf.Pos(); // pos will point prior to Credential 
            pCredential->write(theBuf); // write out the Credential data
			theBuf.Set(pos);			// rewind prior to Credential
			CTlvCredential cr;			// local Credential to override MAC address pointer
			cr.parse(theBuf);			// create copy of Credential from buffer
			cr.macAddr.Set( WSC_ID_MAC_ADDR, macAddr, SIZE_6_BYTES );
			theBuf.Set(pos);
			cr.write(theBuf);			// overwrite the data in the buffer
		}

        ListItrDelete(itr);  
    }
    catch(...)
    {
        if(itr)
            ListItrDelete(itr);
    }
}




void CTlvEsM8Sta::write(BufferObj &theBuf, BufferObj &authKey, bool isWirelessWPS)
{
    LPLISTITR itr;
    CTlvCredential *pCredential;

    //there should be at least one credential TLV
    if(isWirelessWPS && 0 == ListGetCount(credential))
        throw RPROT_ERR_REQD_TLV_MISSING;

    try
    {
		if (isWirelessWPS) { // skip including Credential if not in wireless WPS mode
			if(!(itr = ListItrCreate(credential)))
				throw WSC_ERR_OUTOFMEMORY;

			while((pCredential = (CTlvCredential *)ListItrGetNext(itr)))
			{
				pCredential->write(theBuf);
			}
		}

        //write the optional new password and device password ID
        if(new_pwd.Length())
        {
            new_pwd.Write(theBuf);
            pwdId.Write(theBuf);
        }


        //calculate the hmac and append the TLV to the buffer
        uint8 hmac[SIZE_256_BITS];
        if(HMAC(EVP_sha256(), authKey.GetBuf(), SIZE_256_BITS, 
                theBuf.GetBuf(), theBuf.Length(), hmac, NULL) 
            == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Error generating HMAC\n"));
            throw RPROT_ERR_CRYPTO;
        }

        CTlvAuthenticator(
                        WSC_ID_KEY_WRAP_AUTH,
                        theBuf,
                        hmac,
                        SIZE_64_BITS);
		if (isWirelessWPS) { // itr is only valid if in wireless WPS mode
			ListItrDelete(itr);  
		}
    }
    catch(...)
    {
        if(itr)
            ListItrDelete(itr);
    }
}
