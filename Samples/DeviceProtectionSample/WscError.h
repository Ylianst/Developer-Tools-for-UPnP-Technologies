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

#ifndef _WSC_ERROR_
#define _WSC_ERROR_

#pragma pack(push, 1)

// generic
#define WSC_BASE                    0x1000
#define WSC_SUCCESS                 WSC_BASE+1
#define WSC_ERR_OUTOFMEMORY         WSC_BASE+2
#define WSC_ERR_SYSTEM              WSC_BASE+3
#define WSC_ERR_NOT_INITIALIZED     WSC_BASE+4
#define WSC_ERR_INVALID_PARAMETERS  WSC_BASE+5
#define WSC_ERR_BUFFER_TOO_SMALL    WSC_BASE+6
#define WSC_ERR_NOT_IMPLEMENTED     WSC_BASE+7
#define WSC_ERR_ALREADY_INITIALIZED WSC_BASE+8
#define WSC_ERR_GENERIC             WSC_BASE+9
#define WSC_ERR_FILE_OPEN           WSC_BASE+10
#define WSC_ERR_FILE_READ           WSC_BASE+11
#define WSC_ERR_FILE_WRITE          WSC_BASE+12

// CQueue
#define CQUEUE_BASE               0x2000
#define CQUEUE_ERR_INTERNAL       CQUEUE_BASE+1
#define CQUEUE_ERR_IPC            CQUEUE_BASE+2

// State machine
#define SM_BASE                   0x3000
#define SM_ERR_INVALID_PTR        SM_BASE+1
#define SM_ERR_WRONG_STATE        SM_BASE+2
#define SM_ERR_MESSAGE_DATA       SM_BASE+3

// MasterControl
#define MC_BASE							0x4000
#define MC_ERR_CFGFILE_CONTENT          MC_BASE+1
#define MC_ERR_CFGFILE_OPEN             MC_BASE+2
#define MC_ERR_STACK_ALREADY_STARTED    MC_BASE+3
#define MC_ERR_STACK_NOT_STARTED        MC_BASE+4
#define MC_ERR_VALUE_UNCHANGED			MC_BASE+5

// Transport
#define TRANS_BASE                 0x5000

#define TRUFD_BASE                 0x5100
#define TRUFD_ERR_DRIVE_REMOVED    TRUFD_BASE+1
#define TRUFD_ERR_FILEOPEN         TRUFD_BASE+2
#define TRUFD_ERR_FILEREAD         TRUFD_BASE+3
#define TRUFD_ERR_FILEWRITE        TRUFD_BASE+4
#define TRUFD_ERR_FILEDELETE       TRUFD_BASE+5

#define TRNFC_BASE                 0x5200
#define TRNFC_ERR_NO_TAG           TRNFC_BASE+1
#define TRNFC_ERR_NO_READER        TRNFC_BASE+2
#define TRNFC_ERR_INVALID_NAME     TRNFC_BASE+3
#define TRNFC_ERR_FILEREAD         TRNFC_BASE+4
#define TRNFC_ERR_FILEWRITE        TRNFC_BASE+5

#define TREAP_BASE                 0x5300
#define TREAP_ERR_SENDRECV         TREAP_BASE+1

#define TRWLAN_BASE                0x5400
#define TRWLAN_ERR_SENDRECV        TRWLAN_BASE+1

#define TRIP_BASE                       0x5500
#define TRIP_ERR_SENDRECV               TRIP_BASE+1
#define TRIP_ERR_NETWORK                TRIP_BASE+2
#define TRIP_ERR_NOT_MONITORING         TRIP_BASE+3
#define TRIP_ERR_ALREADY_MONITORING     TRIP_BASE+4
#define TRIP_ERR_INVALID_SOCKET         TRIP_BASE+5

#define TRUPNP_BASE                 0x5600
#define TRUPNP_ERR_SENDRECV         TRUPNP_BASE+1

// RegProtocol
#define RPROT_BASE                 0x6000

#define RPROT_ERR_REQD_TLV_MISSING RPROT_BASE+1
#define RPROT_ERR_CRYPTO           RPROT_BASE+2
#define RPROT_ERR_INCOMPATIBLE     RPROT_BASE+3
#define RPROT_ERR_INVALID_VALUE    RPROT_BASE+4
#define RPROT_ERR_NONCE_MISMATCH   RPROT_BASE+5
#define RPROT_ERR_WRONG_MSGTYPE    RPROT_BASE+6

#define WSC_PASSWORD_AUTH_ERROR 18
// Define a new error type for general processing errors
#define WSC_MESSAGE_PROCESSING_ERROR 19

//Portability
#define PORTAB_BASE                 0x7000
#define PORTAB_ERR_SYNCHRONIZATION  PORTAB_BASE+1
#define PORTAB_ERR_THREAD           PORTAB_BASE+2
#define PORTAB_ERR_EVENT            PORTAB_BASE+3
#define PORTAB_ERR_WAIT_ABANDONED   PORTAB_BASE+4
#define PORTAB_ERR_WAIT_TIMEOUT     PORTAB_BASE+5

#pragma pack(pop)
#endif // _WSC_ERROR_
