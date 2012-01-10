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

#ifndef _SM_INFO_H
#define _SM_INFO_H

#pragma pack(push, 1)

/* Moved to inbeap.h
//WSC Message types
#define WSC_Start 0x01
#define WSC_ACK   0x02
#define WSC_NACK  0x03
#define WSC_MSG   0x04
#define WSC_Done  0x05

//WSC packet header
typedef struct {
    uint8 opCode;
    uint8 flags;
}S_WSC_HEADER;

typedef struct {
    S_WSC_HEADER header;
    uint16       messageLength;
}S_WSC_FRAGMENT_HEADER;

*/
// data structures for each instance of registration protocol
typedef enum {
    START = 0,
    CONTINUE,
    RESTART,
    SUCCESS, 
    FAILURE
} ESMState;

typedef enum {
    MSTART = 0,
    M1,
    M2,
    M2D,
    M3,
    M4,
    M5,
    M6,
    M7,
    M8,
    DONE,
    MNONE = 99
} EMsg;

// data structure to store info about a particular instance
// of the Registration protocol
typedef struct {
    ESMState    e_smState;
    EMsg        e_lastMsgRecd;
    EMsg        e_lastMsgSent;

    // TODO: must store previous message as well to compute hash
    
    // enrollee endpoint - filled in by the Registrar, NULL for Enrollee
    S_DEVICE_INFO    *p_enrolleeInfo;        
    // Registrar endpoint - filled in by the Enrollee, NULL for Registrar
    S_DEVICE_INFO    *p_registrarInfo;    

    //Diffie Hellman parameters
    BIGNUM      *DH_PubKey_Peer; //peer's pub key stored in bignum format
    DH          *DHSecret;       //local key pair in bignum format
    uint8       pke[SIZE_PUB_KEY]; //enrollee's raw pub key
    uint8       pkr[SIZE_PUB_KEY]; //registrar's raw pub key

    uint8       peerPubKeyHash[SIZE_256_BITS];

    BufferObj   password;
	int			passwordUseCounter;
    void        *staEncrSettings; // to be sent in M2/M8 by reg & M7 by enrollee
    void        *apEncrSettings;

    uint8       enrolleeNonce[SIZE_128_BITS];//N1
    uint8       registrarNonce[SIZE_128_BITS];//N2
    

    uint8       psk1[SIZE_128_BITS];
    uint8       psk2[SIZE_128_BITS];

    uint8       eHash1[SIZE_256_BITS];
    uint8       eHash2[SIZE_256_BITS];
    uint8       es1[SIZE_128_BITS];
    uint8       es2[SIZE_128_BITS];

    uint8       rHash1[SIZE_256_BITS];
    uint8       rHash2[SIZE_256_BITS];
    uint8       rs1[SIZE_128_BITS];
    uint8       rs2[SIZE_128_BITS];

    BufferObj   authKey;
    BufferObj   keyWrapKey;
    BufferObj   emsk;
    //BufferObj   iv;

    BufferObj   UPnPPSauthKey;
    BufferObj   UPnPPSkeyWrapKey;

    BufferObj   x509csr;
    BufferObj   x509Cert;

    BufferObj   inMsg;        // A recd msg will be stored here
    BufferObj   outMsg;     // Contains msg to be transmitted

    bool        oobMode;
} S_REGISTRATION_DATA;

#pragma pack(pop)
#endif //_SM_INFO_H
