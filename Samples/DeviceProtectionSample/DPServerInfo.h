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

#ifndef __DPServerInfo__
#define __DPServerInfo__

/*
#if defined(WIN32)
	#define _CRTDBG_MAP_ALLOC
#endif

// #include "DPServerInfo.h"

#if defined(WIN32) && !defined(_WIN32_WCE)
	#include <crtdbg.h>
#endif
	*/


#include <stdio.h>
#include <assert.h>
#include "DataTypes.h"


#ifdef __linux__
#else
#include <winsock2.h>
#include <windows.h>
#endif


extern "C" {
#include "ILibParsers.h"
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
}

#include <list>
#include <iterator>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>

using namespace std;

struct IdentityBase {
	IdentityBase( const string & roles ) : m_roles(roles) { }
	virtual ~IdentityBase() { }
	string m_roles;
};

struct CPIdentity : public IdentityBase {
	CPIdentity( const string & certName, const string & ID, const string & roles, const string & alias, bool introduced = false) : 
	  m_certName(certName), m_ID(ID), IdentityBase(roles), m_introduced(introduced), m_alias(alias) { }
	virtual ~CPIdentity() { }
	void SetIntroduced( bool introduced ) { m_introduced = introduced; }
    bool m_introduced;
	string m_certName;  // needs to be consistent with cert CommonName field
	string m_alias;		// note:  need unicode support
	string m_ID;
};

struct UserIdentity : public IdentityBase {
	UserIdentity( const string & name, const vector<uint8> & salt, 
		const vector<uint8> & stored_password, const string & roles) : 
	  m_name(name), IdentityBase(roles) 
	{ 
		memset(m_salt, 0, sizeof(m_salt)); 
		memset(m_stored_password, 0, sizeof(m_stored_password)); 
		if (salt.size() == sizeof(m_salt) && stored_password.size() == sizeof(m_stored_password)) {
			std::copy(salt.begin(), salt.end(), m_salt);
			std::copy(stored_password.begin(), stored_password.end(), m_stored_password);
		}
	}
	virtual ~UserIdentity() { }

	string m_name;
	uint8 m_stored_password[SIZE_128_BITS];
	uint8 m_salt[SIZE_128_BITS];
};

struct Session {
	Session(GUID & sessionId) : m_cp(NULL), m_user(NULL), m_failedLogins(0) 
		{ memcpy(& m_sessionId,& sessionId, sizeof(GUID)); memset(m_challenge,0,sizeof(m_challenge)); }
	GUID m_sessionId;
	uint8 m_challenge[SIZE_128_BITS];
	string m_challengeName;
	CPIdentity * m_cp;
	UserIdentity * m_user;
	int m_failedLogins;
};

#define DP_MAX_ROLE_NAME_SIZE 64

// The DPServerInfo class contains the data used for DeviceProtection by the root device that contains the 
// DeviceProtection service.
//
class DPServerInfo
{
    list<CPIdentity *>			m_CPIdentities;
    list<UserIdentity *>		m_userIdentities;
    list<Session *>				m_sessions;
	vector<string>				m_deviceRoles;
	vector<string>				m_introductionProtocols;
	vector<string>				m_loginProtocols;
	map<string,string>			m_primaryPolicy;
	map<string,string>			m_restrictedPolicy;

	static uint32 *				s_hLock; // one lock per class (coarse-grained)

public:

	DPServerInfo( const char * adminPassword, const char * vendorRoles = 0, const char * vendorProtocols = 0 );

    virtual ~DPServerInfo();

	//*************************************************************************
	// Methods associated with sessions
	//*************************************************************************
	void NewSession(GUID & sessionID);	
	void RemoveSession(GUID & sessionID);

	Session * GetSession(GUID & sessionID);
	string GetRolesOfSession(GUID & sessionID );
	string GetUserOfSession(GUID & sessionID );
	void UserLogout(GUID & sessionID) {
		Session * sess = GetSession(sessionID);
		//WscLock( s_hLock );
		if (sess) {
			sess->m_user = NULL;
		}
		//WscUnlock( s_hLock );
	}
	bool GetUserLoginChallenge( GUID & sessionID, const string & protocolType, const string & name, 
								vector<uint8> & salt, vector<uint8> & challenge );
	bool UserLogin( GUID & sessionID, GUID & CP_GUID, GUID & DeviceGUID,
					unsigned char* Challenge,int _ChallengeLength,
					unsigned char* Authenticator,int _AuthenticatorLength);

	bool ComputeStoredPassword(const char * name, const char * password,
								vector<uint8> & salt, bool initializeSalt,
								vector<uint8> & stored_password);

	// Control points call CP_GetAuthenticator to obtain the Authenticator value to use with UserLogin
	bool CP_GetAuthenticator(const char * name, const char * password, GUID & CP_GUID, GUID & DeviceGUID,
								unsigned char* Salt,int SaltLength,unsigned char* Challenge,int ChallengeLength,
								vector<uint8> & authenticator);

	// Devices call Dev_ComputeAuthenticator to derive the authenticator value that a Control Point
	// should be providing in response to a prior challenge.
	bool Dev_ComputeAuthenticator(vector<uint8> & stored_password, unsigned char* CP_GUID, 
											unsigned char* DeviceGUID, vector<uint8> & challenge, 
											vector<uint8> & authenticator);

	//*************************************************************************
	// Methods associated with Identities
	//*************************************************************************
	IdentityBase * FindIdentity( const char * id);
	UserIdentity * FindUser(const string & user);
	CPIdentity * FindCPbyID(string ID);

	UserIdentity * AddUser( const string & name, const vector<uint8> & salt, 
							const vector<uint8> & storedPassword, const string & roles ) {
		UserIdentity * user = new UserIdentity(name, salt, storedPassword, roles);
		//WscLock( s_hLock );
		if (user) {
			m_userIdentities.push_back(user);
		}
		//WscUnlock( s_hLock );
		return user;
	}
	void RemoveUser( UserIdentity * user ) {
		if (! user)
			return;
		//WscLock( s_hLock );
		m_userIdentities.remove(user);
		delete user;
		//WscUnlock( s_hLock );
	}
	bool SetUserLoginPassword(const char * name, unsigned char* Stored,
							  int _StoredLength,unsigned char* Salt,int _SaltLength);

	CPIdentity * AddCP( const string & certName, const string & id, const string & roles, 
						const string & alias, bool introduced = false );
	void RemoveCP( CPIdentity * id );

	string GetIdentityList();
	bool AddIdentityList( const char * identityList );
	bool RemoveIdentity( const char * identity );

	//*************************************************************************
	// Methods associated with access control policies
	//*************************************************************************
	bool AddRolesForIdentity( const char * identity, const char * roles );
	bool RemoveRolesForIdentity( const char * identity, const char * roles );

	// SetPrimaryPolicy allows a Device to store its mapping of actions to required roles in the 
	// DPServerInfo object and then call CheckAuthorization() at runtime to verify that the
	// caller has the appropriate permissions to invoke the action.  Note that SetPrimaryPolicy()
	// is only used for the (unconditional) RoleList policy.  Actions with finer-grained 
	// policy represented by a RestrictedRoleList will need to put the extra logic examining
	// the SOAP arguments in the action implementation code itself.  The recommended way to
	// do this is to first check if the caller has unconditional authorization by calling
	// CheckAuthorization().  If this check fails, then the action code should do the finer-
	// grained check itself after calling GetRolesOfSession().
	//
	void SetPolicy(const string & UDN, const string & Service, const string & Action, const string & PrimaryRoles,
		const string & RestrictedRoles) {
		m_primaryPolicy[UDN + Service + Action] = PrimaryRoles;
		m_restrictedPolicy[UDN + Service + Action] = RestrictedRoles;
	}
	string GetPrimaryPolicy(const string & UDN, const string & Service, const string & Action) {
		if (strnicmp("uuid:",UDN.c_str(),5) != 0) {
			return m_primaryPolicy["uuid:" + UDN + Service + Action];
		}
		return m_primaryPolicy[UDN + Service + Action];
	}
	string GetRestrictedPolicy(const string & UDN, const string & Service, const string & Action) {
		if (strnicmp("uuid:",UDN.c_str(),5) != 0) {
			return m_restrictedPolicy["uuid:" + UDN + Service + Action];
		}
		return m_restrictedPolicy[UDN + Service + Action];
	}
	bool CheckAuthorization( GUID & sessionID, const string & UDN,
							 const string & Service, const string & Action);
	string GetACL();

	string GetSupportedProtocols();
}; // DPServerInfo

#endif
