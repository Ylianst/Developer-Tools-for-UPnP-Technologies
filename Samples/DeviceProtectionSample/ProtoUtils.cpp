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
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "slist.h"
#include "tutrace.h"
#include "WscTypes.h"
#include "WscCommon.h"
#include "WscError.h"
#include "RegProtoMsgs.h"
#include "StateMachineInfo.h"
#include "RegProtocol.h"

extern "C" bool g_verbose;

#pragma pack(push, 1)

static uint8 DH_P_VALUE[BUF_SIZE_1536_BITS] = 
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
    0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
    0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
    0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
    0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
    0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
    0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
    0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
    0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
    0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
    0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
    0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
    0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
    0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
    0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
    0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
    0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
    0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
    0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
    0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
    0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
    0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static uint32 DH_G_VALUE = 2;

#pragma pack(pop)

/*
#define ENTRY_COUNT 5

static int entries[ENTRY_COUNT] = {
  {"countryName", "US"},
  {"stateOrProvinceName", "OR"},
  {"localityName", "Hillsboro"},
  {"organizationName", "Intel"},
  {"organizationalUnitName", "CTG"}
  //Remember to add the description and common name later,
};
*/

bool 
CRegProtocol::ValidateChecksum( IN unsigned long int PIN )
{
    unsigned long int accum = 0;
	accum += 3 * ((PIN / 10000000) % 10); 
	accum += 1 * ((PIN / 1000000) % 10); 
	accum += 3 * ((PIN / 100000) % 10); 
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10); 
	accum += 1 * ((PIN / 1) % 10); 
	
    return (0 == (accum % 10));
} // ValidateChecksum

uint32 
CRegProtocol::ComputeChecksum( IN unsigned long int PIN )
{
    unsigned long int accum = 0;

	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 
	accum += 1 * ((PIN / 1000000) % 10); 
	accum += 3 * ((PIN / 100000) % 10); 
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10); 

	int digit = (accum % 10);
	return (10 - digit) % 10;
} // ComputeChecksum

uint32 CRegProtocol::GenerateDHKeyPair(DH **DHKeyPair, BufferObj &pubKey)
{
    uint8 temp[SIZE_PUB_KEY];
    try
    {
        //1. Initialize the DH structure
        *DHKeyPair = DH_new();
        if(*DHKeyPair == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: DH_new failed\n"));
            throw RPROT_ERR_CRYPTO;
        }

        (*DHKeyPair)->p = BN_new();
        (*DHKeyPair)->g = BN_new();
       
        //2. load the value of P
        if(BN_bin2bn(DH_P_VALUE, 
                     BUF_SIZE_1536_BITS, 
                     (*DHKeyPair)->p)==NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: BN_bin2bn P: %s", 
                    ERR_error_string(ERR_get_error(), NULL)));
            throw RPROT_ERR_CRYPTO;
        }

        //3. load the value of G
        uint32 g = WscHtonl(DH_G_VALUE);
        if(BN_bin2bn((uint8 *)&g, 
                     4, 
                     (*DHKeyPair)->g)==NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: BN_bin2bn G: %s", 
                    ERR_error_string(ERR_get_error(), NULL)));
            throw RPROT_ERR_CRYPTO;
        }

        //4. generate the DH key
        if(DH_generate_key(*DHKeyPair) == 0)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: DH_generate_key: %s", 
                    ERR_error_string(ERR_get_error(), NULL)));
            throw RPROT_ERR_CRYPTO;
        }

        //5. extract the DH public key
        int len = BN_bn2bin((*DHKeyPair)->pub_key, temp);
        if(0 == len)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: BN_bn2bin: %s", 
                    ERR_error_string(ERR_get_error(), NULL)));
            throw RPROT_ERR_CRYPTO;
        }
        pubKey.Append(SIZE_PUB_KEY, temp);
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        return err;
    }
    catch(...)
    {
        return WSC_ERR_SYSTEM;
    }
}//GenerateDHKeyPair

void CRegProtocol::GenerateSHA256Hash(BufferObj &inBuf, BufferObj &outBuf)
{
    uint8 Hash[SIZE_256_BITS];
    if(SHA256(inBuf.GetBuf(), inBuf.Length(), Hash) == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: SHA256 calculation failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    outBuf.Append(SIZE_256_BITS, Hash);
}//GenerateHash

void CRegProtocol::DeriveKey(BufferObj &KDK, 
                             BufferObj &prsnlString, 
                             uint32 keyBits, 
                             BufferObj &key)
{
    uint32 i = 0, iterations = 0;
    BufferObj input, output;    
    uint8 hmac[SIZE_256_BITS];
    uint32 hmacLen = 0;
    uint8 *inPtr;
    uint32 temp;

    TUTRACE((TUTRACE_INFO, "RPROTO: Deriving a key of %d bits\n", keyBits));

    iterations = ((keyBits/8) + PRF_DIGEST_SIZE - 1)/PRF_DIGEST_SIZE;

    //Prepare the input buffer. During the iterations, we need only replace the 
    //value of i at the start of the buffer.
    temp = WscHtonl(i);
    input.Append(SIZE_4_BYTES, (uint8 *)&temp);
    input.Append(prsnlString.Length(), prsnlString.GetBuf());
    temp = WscHtonl(keyBits);
    input.Append(SIZE_4_BYTES, (uint8 *)&temp);
    inPtr = input.GetBuf();

    for(i = 0; i < iterations; i++)
    {
        //Set the current value of i at the start of the input buffer
        *(uint32 *)inPtr = WscHtonl(i+1); //i should start at 1
        if(HMAC(EVP_sha256(), KDK.GetBuf(), SIZE_256_BITS, input.GetBuf(), 
                input.Length(), hmac, &hmacLen) == NULL)
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed\n"));
            throw RPROT_ERR_CRYPTO;
        }
        output.Append(hmacLen, hmac);
    }

    //Sanity check
    if(keyBits/8 > output.Length())
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: Key derivation generated less bits "
                              "than asked\n"));
        throw RPROT_ERR_CRYPTO;
    }

    //We now have at least the number of key bits requested.
    //Return only the number of bits asked for. Discard the excess.
    key.Append(keyBits/8, output.GetBuf());
}

bool CRegProtocol::ValidateMac(BufferObj &data, uint8 *hmac, BufferObj &key)
{
    uint8 dataMac[BUF_SIZE_256_BITS];

    //First calculate the hmac of the data
    if(HMAC(EVP_sha256(), key.GetBuf(), SIZE_256_BITS, data.GetBuf(), 
            data.Length(), dataMac, NULL) == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed\n"));
		if (TUTRACELEVEL & TUVERBOSE && g_verbose)
		{
		// for debugging purposes
			printf("Computed HMAC (note: only first 64 bits will be compared)\n");
			for (int i = 0; i < BUF_SIZE_256_BITS; i++) {
				printf("%2x ",dataMac[i]);
			}
			printf("\nHMAC from Authenticator(should match first 64 bits of computed HMAC)\n");
			for (int i = 0; i < 8; i++) {
				printf("%2x ",hmac[i]);
			}
			printf("\nKey is:\n");
			for (int i = 0; i < 8; i++) {
				printf("%2x ",(key.GetBuf())[i]);
			}
			printf("\n");
		}
        throw RPROT_ERR_CRYPTO;
	} else { 
		if (TUTRACELEVEL & TUVERBOSE && g_verbose)
		{
		// for debugging purposes
			printf("Computed HMAC (note: only first 64 bits will be compared)\n");
			for (int i = 0; i < BUF_SIZE_256_BITS; i++) {
				printf("%2x ",dataMac[i]);
			}
			printf("\nHMAC from Authenticator(should match first 64 bits of computed HMAC)\n");
			for (int i = 0; i < 8; i++) {
				printf("%2x ",hmac[i]);
			}
			printf("\nKey is:\n");
			for (int i = 0; i < BUF_SIZE_256_BITS; i++) {
				printf("%2x ",(key.GetBuf())[i]);
			}
			printf("\n");
		}
	}

    //next, compare it against the received hmac
    TUTRACE((TUTRACE_INFO, "RPROTO: Verifying the first 64 bits of the generated HMAC\n"));

    if(memcmp(dataMac, hmac, SIZE_64_BITS) != 0)
    {
        printf("RPROTO: HMAC results don't match\n");
        return false;
    }
    TUTRACE((TUTRACE_VERBOSE, "RPROTO: HMAC results match\n"));
    
    return true;
}

bool CRegProtocol::ValidateKeyWrapAuth(BufferObj &data, 
                                       uint8 *hmac, 
                                       BufferObj &key)
{
    //Same as ValidateMac, except only the first 64 bits are validated
    uint8 dataMac[BUF_SIZE_256_BITS];

    //First calculate the hmac of the data
    if(HMAC(EVP_sha256(), key.GetBuf(), SIZE_256_BITS, data.GetBuf(), 
            data.Length(), dataMac, NULL) == NULL)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    //next, compare it against the received hmac
    if(memcmp(dataMac, hmac, SIZE_64_BITS) != 0)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: HMAC results don't match\n"));
        return false;
    }
    
    return true;
}

bool 
CRegProtocol::ValidatePeerPubKeyHash(uint8* pubKeyMsg, uint8 *pubKeyHash)
{
    BufferObj bo_pubKey, bo_sha256Hash;

    bo_pubKey.Append(SIZE_PUB_KEY, pubKeyMsg);
    GenerateSHA256Hash( bo_pubKey, bo_sha256Hash );

    return (!memcmp(bo_sha256Hash.GetBuf(), pubKeyHash, SIZE_160_BITS));
}

void 
CRegProtocol::EncryptData(BufferObj &plainText, 
                          BufferObj &encrKey, 
                          BufferObj &authKey, 
                          BufferObj &cipherText, 
                          BufferObj &iv)
{
    BufferObj buf;
    uint8 ivBuf[SIZE_128_BITS];
    
    if(0 == plainText.Length())
        throw WSC_ERR_INVALID_PARAMETERS;

    //Generate a random iv
    RAND_bytes(ivBuf, SIZE_128_BITS);

    iv.Reset();
    iv.Append(SIZE_128_BITS, ivBuf);

    //Now encrypt the plaintext and mac using the encryption key and IV.
    buf.Append(plainText.Length(), plainText.GetBuf());

    EVP_CIPHER_CTX ctx;
    if(0 == EVP_EncryptInit(&ctx, EVP_aes_128_cbc(), encrKey.GetBuf(), ivBuf))
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: EncryptInit failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    int bufLen = 1024;
    uint8 outBuf[1024];
    int outLen, currentLength;
    //block size = 1024 bytes - 128 bits, 
    //leave 128 bits at the end to accommodate any possible padding 
    //and avoid a buffer overflow
    int blockSize = bufLen - SIZE_128_BITS; 
    int length = buf.Length();

    uint8 *bufPtr = buf.GetBuf();
    while(length)
    {
        if(length > blockSize)
            currentLength = blockSize;
        else
            currentLength = length;

        if(0 == EVP_EncryptUpdate(&ctx, outBuf, &outLen, bufPtr, currentLength))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: EncryptUpdate failed\n"));
            throw RPROT_ERR_CRYPTO;
        }

        cipherText.Append(outLen, outBuf);
        bufPtr += currentLength;
        length -= currentLength;
    }

    if(0 == EVP_EncryptFinal(&ctx, outBuf, &outLen))
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: EncryptFinal failed\n"));
        throw RPROT_ERR_CRYPTO;
    }
    
    cipherText.Append(outLen, outBuf);
}

void 
CRegProtocol::DecryptData(BufferObj &cipherText, 
                          BufferObj &iv,
                          BufferObj &encrKey, 
                          BufferObj &authKey, 
                          BufferObj &plainText)
{
    EVP_CIPHER_CTX ctx;
    if(0 == EVP_DecryptInit(&ctx, EVP_aes_128_cbc(), encrKey.GetBuf(), iv.GetBuf()))
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: DecryptInit failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    BufferObj buf;

    int bufLen = 1024;
    uint8 outBuf[1024];
    int outLen, currentLength;
    //block size = 1024 bytes - 128 bits, 
    //leave 128 bits at the end to accommodate any possible padding 
    //and avoid a buffer overflow
    int blockSize = bufLen - SIZE_128_BITS; 
    int length = cipherText.Length();

    uint8 *bufPtr = cipherText.GetBuf();
 
    while(length)
    {
        if(length > blockSize)
            currentLength = blockSize;
        else
            currentLength = length;

        if(0 == EVP_DecryptUpdate(&ctx, outBuf, &outLen, bufPtr, currentLength))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: DecryptUpdate failed\n"));
            throw RPROT_ERR_CRYPTO;
        }

        buf.Append(outLen, outBuf);
        bufPtr += currentLength;
        length -= currentLength;
    }

    if(0 == EVP_DecryptFinal(&ctx, outBuf, &outLen))
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: DecryptFinal failed\n"));
        throw RPROT_ERR_CRYPTO;
    }

    buf.Append(outLen, outBuf);

    //Validate the mac at the end of the decrypted buffer
    //uint8 *mac = buf.GetBuf()+(buf.Length()-SIZE_256_BITS);//get the last 256 bits
    //if(0 == ValidateMac(BufferObj(buf.GetBuf(), buf.Length()-SIZE_256_BITS), mac, authKey))
    //{
    //    TUTRACE((TUTRACE_ERR, "RPROTO: Mac validation failed\n"));
    //    throw RPROT_ERR_INVALID_VALUE;
    //}
    //plainText.Append(buf.Length()-SIZE_256_BITS, buf.GetBuf());
    plainText.Append(buf.Length(), buf.GetBuf());
    plainText.Rewind(plainText.Length());
}

uint32 CRegProtocol::NewPIN()
{
	uint32 pwd;
	char devPwd[32];

	RAND_bytes((unsigned char *) & pwd, 4);
    sprintf( devPwd, "%08u", pwd );

	// Compute the checksum
	devPwd[7] = '\0'; // null out the 8th byte
	uint32 val = strtoul( devPwd, NULL, 10 );
	uint32 checksum = ComputeChecksum( val );
	val = val*10 + checksum;
	return val;
}

uint32 CRegProtocol::GeneratePSK(IN uint32 length, OUT BufferObj &PSK)
{
    uint8 temp[1024];
    if((0 == length)|| (length > 1024))
        return WSC_ERR_INVALID_PARAMETERS;

    RAND_bytes(temp, length);
    PSK.Append(length, temp);

    return WSC_SUCCESS;
}

#if 0
uint32 CRegProtocol::CreatePrivateKey(char *name, 
                                      EVP_PKEY **key)
{
    TU_RET err = TU_ERROR_CRYPTO_FAILED;
    RSA *rsaKey;
    EVP_PKEY *pkey;
    FILE *fp;

    rsaKey = RSA_generate_key(1024, 65537, NULL, NULL);
    if(rsaKey == NULL)
    {
        TUTRACE((TUTRACE_ERR, "Couldn't generate RSA key\n"));
        goto EXIT;
    }

    //Now store it in a PKEY
    pkey = EVP_PKEY_new();
    if(!pkey)
    {
        TUTRACE((TUTRACE_ERR, "Couldn't generate new EVP key\n"));
        goto EXIT;
    }

    if(!EVP_PKEY_assign_RSA(pkey, rsaKey))
    {
        TUTRACE((TUTRACE_ERR, "Couldn't assign RSA key to EVP key\n"));
        RSA_free(rsaKey);
        goto EXIT;
    }

    fp = fopen(name, "w");
    
    if(!PEM_write_PKCS8PrivateKey(fp, pkey, NULL, NULL, 0, NULL, NULL))
    {
        TUTRACE((TUTRACE_ERR, "Error writing Signing key to file\n"));
        fclose(fp);
        goto ERR_EVP;
    }
    
    fclose(fp);

    if(key)
        *key = pkey;
    else
        EVP_PKEY_free(pkey);

    return TU_SUCCESS;

ERR_EVP:
    EVP_PKEY_free(pkey);
EXIT:
    return err;
}

uint32 CRegProtocol::GetPublicKey(char *Name, 
                                EVP_PKEY **key)
{
    TU_RET err;
    FILE *fp;

    err = TU_ERROR_CRYPTO_FAILED;

    fp = fopen(NAME_BUF, "r");
    if(!fp)
    {
        err = TU_ERROR_FILEOPEN;
        goto EXIT;
    }

    if(!(*key = PEM_read_PUBKEY(fp, NULL, NULL, NULL)))
    {
        ERR_print_errors_fp(stdout);
        //If we can't read the Public key, try reading the pvt key. 
        //The above function might give an error if we use it to 
        //read a private key file
        rewind(fp);
        if(!(*key = PEM_read_PrivateKey(fp, NULL, NULL, NULL)))
        {
            ERR_print_errors_fp(stdout);
            goto EXIT_FILE;
        }
   }

    err = TU_SUCCESS;
    
EXIT_FILE:
    if(fp)
        fclose(fp);
EXIT:
    return err;
}

uint32 CRegProtocol::GetPrivateKey(char        *Name,
                                   EVP_PKEY    **key)
{
    TU_RET err = TU_ERROR_CRYPTO_FAILED;
    FILE *fp;

    fp = fopen(NAME_BUF, "r");
    if(!fp)
    {
        err = TU_ERROR_FILEOPEN;
        goto EXIT;
    }

    if(!(*key = PEM_read_PrivateKey(fp, NULL, NULL, NULL)))
    {
        goto EXIT_FILE;
    }

    err = TU_SUCCESS;

EXIT_FILE:
    if(fp)
        fclose(fp);
EXIT:
    return err;
}

uint32 CRegProtocol::GenerateCertRequest(char *SubjName,
                                         uchar **Cert,
                                         uint32 *CertLength)
{
    uint32 err; //= TU_ERROR_CRYPTO_FAILED;
    X509_REQ *req;
    X509_NAME *subj;
    EVP_PKEY *pkey;
    int nid;
    X509_NAME_ENTRY *ent;
    FILE *fp;
    int fsize;

    //First, get the private key
    err = GetPrivateKey(SubjName, TU_KEY_ENC, &pkey);
    if(TU_SUCCESS != err)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error getting private key\n"));
        goto EXIT;
    }

    //Now create a new request object
    if(!(req = X509_REQ_new()))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error creating new X509 Request\n"));
        goto ERR_PKEY;
    }

    //assign the public key to the request
    X509_REQ_set_pubkey (req, pkey);

    //Subject name processing. 
    if(!(subj = X509_NAME_new()))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error creating new X509 Subject\n"));
        goto ERR_REQ;
    }

    //First set the predefined subject fields
    for (int i = 0; i < ENTRY_COUNT; i++)
    {
        if((nid = OBJ_txt2nid (entries[i].key)) == NID_undef)
        {
            TUTRACE((TUTRACE_ERR, "PROTO: Error getting NID from text\n"));
            X509_NAME_free(subj);
            goto ERR_REQ;
        }
      
        if(!(ent = X509_NAME_ENTRY_create_by_NID(NULL, nid, MBSTRING_ASC,
                                                 (uchar *)entries[i].value, -1)))
        {
            TUTRACE((TUTRACE_ERR, "PROTO: Error creating name entry\n"));
            X509_NAME_free(subj);
            goto ERR_REQ;
        }

        if(X509_NAME_add_entry(subj, ent, -1, 0) != 1)
        {
            TUTRACE((TUTRACE_ERR, "PROTO: Error adding name entry to subject\n"));
            X509_NAME_ENTRY_free(ent);
            X509_NAME_free(subj);
            goto ERR_REQ;
        }
    }//for

    //Next set the common name and description
    if((nid = OBJ_txt2nid("commonName")) == NID_undef)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error getting NID from text\n"));
        X509_NAME_free(subj);
        goto ERR_REQ;
    }

    if(!(ent = X509_NAME_ENTRY_create_by_NID(NULL, nid, MBSTRING_ASC,
                                                 (uchar *)SubjName, -1)))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error creating name entry\n"));
        X509_NAME_free(subj);
        goto ERR_REQ;
    }

    if(X509_NAME_add_entry(subj, ent, -1, 0) != 1)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error adding name entry to subject\n"));
        X509_NAME_ENTRY_free(ent);
        X509_NAME_free(subj);
        goto ERR_REQ;
    }

    //Finally add the subject to the request
    if(X509_REQ_set_subject_name (req, subj) != 1)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error setting subject in request\n"));
        X509_NAME_free(subj);
        goto ERR_REQ;
   }

    //Sign the request
    if(!(X509_REQ_sign(req, pkey, EVP_sha1())))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error signing request\n"));
        goto ERR_REQ;
    }

    //Now we need to serialize the request. So write it to a file and read it out
    if(!(fp = fopen("protofile", "w")))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error opening file for writing\n"));
        err = TU_ERROR_FILEOPEN;
        goto ERR_REQ;
    }

    if(PEM_write_X509_REQ(fp, req) != 1)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error writing request to file\n"));
        err = TU_ERROR_FILEWRITE;
        fclose(fp);
        goto ERR_REQ;
    }

    fclose(fp);

    //now open it for reading in binary format
    if(!(fp = fopen("protofile", "rb")))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error opening file for reading\n"));
        err = TU_ERROR_FILEOPEN;
        goto ERR_FILE;
    }

    //get the filesize
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    if(fsize == -1)
    {
        TUTRACE((TUTRACE_ERR, "Couldn't determine file size\n"));
        err = TU_ERROR_FILEREAD;
        goto ERR_FILE;
    }

    //Allocate memory
    *Cert = (uchar *)malloc(fsize);
    if(!*Cert)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error allocating memory for cert buffer\n"));
        err = TU_ERROR_OUT_OF_MEMORY;
        goto ERR_FILE;
   }

    *CertLength = fsize;

    rewind(fp);
    fread(*Cert, 1, fsize, fp);

    err = TU_SUCCESS;

ERR_FILE:
    if(fp)
        fclose(fp);
    remove("protofile");
ERR_REQ:
    X509_REQ_free(req);
ERR_PKEY:
    EVP_PKEY_free(pkey);
EXIT:
    return err;
}//GenerateCertRequest

TU_RET TuProtocol::HandleCertRequest(
                            PTU_MEMBERINFO  pMemberInfo, 
                            uchar           *dataBuffer, 
                            UINT32          dataLen,
                            uchar           **message, 
                            UINT32          *msgLen)
{
    TU_RET err = TU_ERROR_CRYPTO_FAILED;
    tu_member *member;
    UINT32  hours;    
    char role[MAX_NAME_SIZE];
    char memberName[MAX_NAME_SIZE];
    UINT32 serialLen;
    uchar *serialCert;
    UINT32 CACertLength ;
    uchar *CACert;
    uchar *msgPtr;

    EVP_PKEY *pkey;
    X509_REQ *req;
    FILE *fp;

    pMemberInfo->state = State_App;

    //Store the data buffer into a temporary file to extract the request
    if(!(fp = fopen("protofile", "wb")))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error opening file for writing Cert Request.\n"));
        err = TU_ERROR_FILEOPEN;
        goto EXIT;
    }

    fwrite(dataBuffer, 1, dataLen, fp);
    fclose(fp);

    if(!(fp = fopen("protofile", "r")))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error opening file for reading Cert Request.\n"));
        err = TU_ERROR_FILEOPEN;
        goto EXIT;
    }

    if(!(req = PEM_read_X509_REQ(fp, NULL, NULL, NULL)))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error reading Cert Request.\n"));
        fclose(fp);
        err = TU_ERROR_FILEREAD;
        goto EXIT;
    }

    fclose(fp);

//Amol
#if 0
    //verify the signature with the key we have stored
    DEVICE_NAME(pMemberInfo->domainName, pMemberInfo->Name);
    err = GetPublicKey(DEVICE_NAME_BUF, TU_KEY_SIGN, &pkey);
    if(TU_SUCCESS != err)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error getting enrollee's public key.\n"));
        err = TU_ERROR_FILEOPEN;
        goto ERR_REQ;
    }
#endif

    if(!(pkey = X509_REQ_get_pubkey(req)))
    {
        ERR_print_errors_fp(stdout);
        TUTRACE((TUTRACE_ERR, "PROTO: Error extracting public key\n"));
        err = TU_ERROR_SIGN_VERIFY_FAILURE;
        goto ERR_PKEY;
    }

    if(X509_REQ_verify(req, pkey) != 1)
    {
        ERR_print_errors_fp(stdout);
        TUTRACE((TUTRACE_ERR, "PROTO: Verification failed for Cert Request.\n"));
        err = TU_ERROR_SIGN_VERIFY_FAILURE;
        goto ERR_PKEY;
    }

    if(pMemberInfo->role == guest)
    {
        hours = 24;
    }
    else
    {
        hours = 365*24;
    }

    //Finally, create a certificate
    err = m_pDomainMgr->SignAndStoreCertRequest(req, 
                                                pMemberInfo->pDomain, 
                                                hours,
                                                &serialCert,
                                                (unsigned long *)&serialLen);
    if(TU_SUCCESS != err)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error signing Cert Request.\n"));
        goto ERR_PKEY;
    }

    //We now have the cert. Now serialize the CA cert
    //Open the CA cert file. The name can be derived from the device name 
    //stored in the oobData name
    FILE_CERT(pMemberInfo->oobData.name);
    if(!(fp = fopen(NAME_BUF, "rb")))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error opening CA Cert file.\n"));
        err = TU_ERROR_FILEOPEN;
        goto ERR_CERT;
    }

    //Now check the size of the file
    fseek(fp, 0, SEEK_END);
    CACertLength = (UINT32)ftell(fp);
    if(CACertLength == (UINT32) -1)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error getting CA Cert length.\n"));
        fclose(fp);
        err = TU_ERROR_FILEREAD;
        goto ERR_CERT;
    }

    CACert = (uchar *)calloc(CACertLength , 1);
    if(!CACert)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error allocating memory for CA Cert.\n"));
        fclose(fp);
        err = TU_ERROR_OUT_OF_MEMORY;
        goto ERR_CERT;
    }

    rewind(fp);
    if(CACertLength != fread(CACert, 1, CACertLength, fp))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error reading CA Cert.\n"));
        fclose(fp);
        err = TU_ERROR_FILEREAD;
        goto ERR_CA;
    }

    fclose(fp);

    //Now construct the certificate chain - we simply lump all certs together

    //copy the current certificate and its length in the message buffer
    //Also, allocate space for a message length at the start of the buffer
    *message = (uchar *)malloc(sizeof(UINT32) 
                               + sizeof(serialLen) 
                               + serialLen 
                               + sizeof(CACertLength) 
                               + CACertLength);
    if(!*message)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error allocating memory.\n"));
        err = TU_ERROR_OUT_OF_MEMORY;
        goto ERR_CA;
    }

    //The structure of the message is: MessageLength|CertLen|Cert|CertLen|Cert...
    //The reason for having an extra message length is to help reassemble fragmented packets
    *msgLen = sizeof(UINT32) + sizeof(serialLen) + serialLen + sizeof(CACertLength) + CACertLength;
    
    msgPtr = *message;
    memcpy(msgPtr, msgLen, sizeof(UINT32));
    msgPtr += sizeof(UINT32);
    memcpy(msgPtr, &serialLen, sizeof(serialLen));
    msgPtr += sizeof(serialLen);
    memcpy(msgPtr, serialCert, serialLen);
    msgPtr += serialLen;
    memcpy(msgPtr, &CACertLength, sizeof(CACertLength));
    msgPtr += sizeof(CACertLength);
    memcpy(msgPtr, CACert, CACertLength);
    
    //TBD: Add a serial number...get the context, then extract the serial #
    err = m_pDomainMgr->AddMember(pMemberInfo->Name, 
                                  pMemberInfo->pDomain,
                                  (uchar *)&pMemberInfo->enrolleeAddr, 
                                  NULL, 
                                  &member);
    if(TU_SUCCESS != err)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Error Adding member.\n"));
        free(*message);
    }
    else
    {
        //Finally, notify the UI of the new member
        pUpdateCB(pMemberInfo->Name, pMemberInfo->pDomain->Name);
        err = TU_SUCCESS;
    }
  
ERR_CA:
    if(CACert)
        free(CACert);
ERR_CERT:
    if(serialCert)
        free(serialCert);
ERR_PKEY:
    EVP_PKEY_free(pkey);
ERR_REQ:
    X509_REQ_free(req);
EXIT:
    return err;
}//HandleCertRequest

TU_RET TuProtocol::HandleCertificate(
                            PTU_MEMBERINFO  pMemberInfo, 
                            uchar           *dataBuffer, 
                            UINT32          dataLen)
{
    TU_RET err;
    UINT32 totalLen, curLen;
    uchar *bufPtr = NULL;
    tu_domain *pDomain = NULL;
    tu_member *pMember = NULL;
    uchar Serial[8] = {0,0,0,0,0,0,0,0};
    FILE *fp;

    TUTRACE((TUTRACE_INFO, "PROTO: Entering HandleCertificate.\n"));

    pMemberInfo->state = State_App;

    //First, create a new domain 
    err = m_pDomainMgr->AddDomain(pMemberInfo->domainName, 
                                  pMemberInfo->oobData.name, 
                                  pMemberInfo->oobData.address.ssid.id,
                                  pMemberInfo->oobData.address.cryptoType,
                                  pMemberInfo->oobData.address.key,
                                  pMemberInfo->oobData.address.apMacAddr,
                                  &pDomain);
    if(TU_SUCCESS != err)
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Domain Creation failed.\n"));
        goto EXIT;
    }

    totalLen = dataLen;

    //*********************************************************
    //FIRST CERTIFICATE

    //get the first certificate's length
    bufPtr = dataBuffer+sizeof(UINT32);
    curLen = *(UINT32 *)bufPtr;
    bufPtr += sizeof(curLen);

    //Store the certificate in a file
    DEVICE_NAME(pMemberInfo->domainName, m_ownerName);
    FILE_CERT(DEVICE_NAME_BUF);

    if(!(fp = fopen(NAME_BUF, "wb")))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Failed to open file for member cert.\n"));
        err = TU_ERROR_FILEOPEN;
        goto EXIT;
   }

    fwrite(bufPtr, 1, curLen, fp);
    fclose(fp);

    //Now store the next certificate
    bufPtr += curLen;
    curLen = *(UINT32 *)bufPtr;
    bufPtr += sizeof(curLen);

    FILE_CERT(pMemberInfo->oobData.name);
    if(!(fp = fopen(NAME_BUF, "wb")))
    {
        TUTRACE((TUTRACE_ERR, "PROTO: Failed to open file for CA cert.\n"));
        err = TU_ERROR_FILEOPEN;
        goto EXIT;
    }

    fwrite(bufPtr, 1, curLen, fp);
    fclose(fp);

    //Now add a member to the domain
    char memberName[MAX_NAME_SIZE];
    strcpy(memberName, pDomain->Name);
    strcat(memberName, DELIMITER);
    strcat(memberName, m_ownerName);

    err = m_pDomainMgr->AddMember(memberName, 
                                  pDomain, 
                                  (uchar *)&pMemberInfo->enrolleeAddr,
                                  Serial,
                                  &pMember);
    if(TU_SUCCESS != err)
    {
        TUTRACE((TUTRACE_ERR,"PROTO: Add member failed.\n"));
        err = TU_ERROR_CRYPTO_FAILED;
        goto EXIT;
    }

     //Now add the owner to the domain
    err = m_pDomainMgr->AddMember(pMemberInfo->oobData.name, 
                                  pDomain, 
                                  (uchar *)&pMemberInfo->oobData.address.ipAddr,
                                  Serial,
                                  &pMember);
    if(TU_SUCCESS != err)
    {
        TUTRACE((TUTRACE_ERR,"PROTO: Add member failed.\n"));
        err = TU_ERROR_CRYPTO_FAILED;
        goto EXIT;
    }


    //Notify the UI of the domain update
    //since we are the enrollee, the memberName parameter of the callback should be NULL
    pUpdateCB(NULL, pDomain->Name);
    err = TU_SUCCESS;

EXIT:
    return err;
}//HandleCertificate

#endif

uint32 CRegProtocol::CheckNonce(IN uint8 *nonce, 
                                IN BufferObj &msg,
                                IN int nonceType)
{
    uint16 type, tempType;

    try
    {
        if((nonceType != WSC_ID_REGISTRAR_NONCE) &&
           (nonceType !=  WSC_ID_ENROLLEE_NONCE))
        {
            TUTRACE((TUTRACE_ERR, "RPROTO: Invalid attribute ID passed to"
                                  " CheckNonce\n"));
            throw WSC_ERR_INVALID_PARAMETERS;
        }

        while(1)
        {
            type = msg.NextType();
            if(!type)
                break;

            if(nonceType == type)
            {

                if(!(memcmp(nonce, 
                     msg.Pos()+sizeof(S_WSC_TLV_HEADER), 
                     SIZE_128_BITS)))
                {
                    msg.Rewind();
                    return WSC_SUCCESS;
                }
                else
                {
                    TUTRACE((TUTRACE_ERR, "RPROTO: Nonce mismatch\n"));
                    msg.Rewind();
                    return RPROT_ERR_NONCE_MISMATCH;
                }
            }
            
            //advance past the TLV - the total number of bytes to advance is
            //the size of the TLV header + the length indicated in the header
            if ( !(msg.Advance( sizeof(S_WSC_TLV_HEADER) + 
                            WscNtohs(*(uint16 *)(msg.Pos()+sizeof(uint16))))) )
            {
                TUTRACE((TUTRACE_ERR, "RPROTO: Didn't find nonce\n"));
                break;
            }
        }//while

        msg.Rewind();
        return RPROT_ERR_REQD_TLV_MISSING;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: CheckNonce generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(char *str)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: CheckNonce generated an "
                 "exception: %s\n", str));
        return WSC_ERR_SYSTEM;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: CheckNonce generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//CheckNonceAckNackDone


uint32 CRegProtocol::GetMsgType(uint32 &msgType, BufferObj &msg)
{
    try
    {
        CTlvVersion bufVersion(WSC_ID_VERSION, msg);
        CTlvMsgType bufMsgType(WSC_ID_MSG_TYPE, msg);
        msgType = bufMsgType.Value();
        msg.Rewind();
        return WSC_SUCCESS;
    }
    catch(uint32 err)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: GetMsgType generated an "
                 "error: %d\n", err));
        return err;
    }
    catch(...)
    {
        TUTRACE((TUTRACE_ERR, "RPROTO: GetMsgType generated an "
                 "unknown exception\n"));
        return WSC_ERR_SYSTEM;
    }
}//GetMsgType
