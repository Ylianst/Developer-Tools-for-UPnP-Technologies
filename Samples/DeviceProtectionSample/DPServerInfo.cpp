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

#include "stdafx.h"

/*
#if defined(WIN32)
	#define _CRTDBG_MAP_ALLOC
#endif

// #include "DPServerInfo.h"

#if defined(WIN32) && !defined(_WIN32_WCE)
	#include <crtdbg.h>
#endif
	*/

#include "DPServerInfo.h"

#ifdef __linux__
#else
#include <winsock2.h>
#include <windows.h>
#endif

#include <algorithm>
#include <cctype>
#include <iostream>




#undef uuid_t
typedef struct {
    uint32  time_low;
    uint16  time_mid;
    uint16  time_hi_and_version;
    uint8   clock_seq_hi_and_reserved;
    uint8   clock_seq_low;
    uint8   node[6];
} uuid_t;

#define NAME_BASED_UUID_TYPE 0x5
extern bool IsNameBasedUUID(void * guid)
{
	unsigned char *uuid_bin = (unsigned char *) guid;
	if ((uuid_bin[8] & 0xc0) == 0x80 &&
	    (uuid_bin[6] & 0xF0) == (NAME_BASED_UUID_TYPE << 4) ) {
		return true;
	}
	return false;
}

extern bool format_uuid_v5(void *guid, unsigned char hash[16])
{
      if (!guid || ! hash) { 
            return false;
      }

      unsigned char *uuid_bin = (unsigned char *) guid;
      // For consistency with RFC 4122, treat the hash input parameter 
      // as a UUID in network byte order.
      memcpy(uuid_bin, hash, sizeof uuid_t);

      /* put in the variant and version bits */
      uuid_bin[6] &= 0x0F;
      uuid_bin[6] |= (NAME_BASED_UUID_TYPE << 4);
      uuid_bin[8] &= 0x3F;
      uuid_bin[8] |= 0x80;

      return true;
}

static char * NamespaceSchema = "xmlns=\"urn:schemas-upnp-org:gw:DeviceProtection\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
xsi:schemaLocation=\"urn:schemas-upnp-org:gw:DeviceProtection \
http://www.upnp.org/schemas/gw/DeviceProtection-v1.xsd\"";


extern string UUIDToString( void * guid )
{
	uuid_t id = *((uuid_t *) guid);
	char buf[40];

	// For consistency with RFC 4122, treat the hash input parameter 
	// as a UUID in network byte order.  We need to first convert this
	// to host byte order to correctly print its content.
	id.time_low = ntohl(id.time_low);
	id.time_mid = ntohs(id.time_mid);
	id.time_hi_and_version = ntohs(id.time_hi_and_version);
	
	sprintf(buf,"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		id.time_low, id.time_mid, id.time_hi_and_version,
		id.clock_seq_hi_and_reserved, id.clock_seq_low,
		id.node[0], id.node[1], id.node[2], id.node[3], id.node[4], id.node[5]);

	return buf;
}

// vendorRoles is a space-separated list of DCP-defined or vendor-defined Role names
// vendorProtocols is a list of space-separated vendor-specific introduction protocols followed by 
// a space and a comma and another space and a list of vendor-specific login protocols.  For example:
// "TLS-PSK , MyLoginProtocol".
//
DPServerInfo::DPServerInfo( const char * adminPassword, const char * vendorRoles, const char * vendorProtocols )
{
	vector<uint8> storedPassword;
	vector<uint8> salt;
	bool res = ComputeStoredPassword( "Administrator", adminPassword, salt, true, storedPassword);
    string buf("Admin"); 
	if (res) {
		AddUser("Administrator", salt, storedPassword, buf);
	}
	m_deviceRoles.push_back(buf);
	m_deviceRoles.push_back("Basic");
	m_deviceRoles.push_back("Public");
	m_introductionProtocols.push_back("WPS");
	m_loginProtocols.push_back("PKCS5");
	// vendorRoles contains space-separated role names. Parse and push these into m_deviceRoles
	if (vendorRoles) {
		stringstream ss(vendorRoles); // Insert the string into a stream
		while (ss >> buf) {
			if (buf.length() <= DP_MAX_ROLE_NAME_SIZE) { // otherwise, ignore the Role name
				m_deviceRoles.push_back(buf);
			}
		}
	}
	if (vendorProtocols) {
		stringstream ss2(vendorProtocols); 
		bool intro = true;
		while (ss2 >> buf) {
			if (buf == ",") {
				intro = false;
				continue;
			}
			if (intro) {
				m_introductionProtocols.push_back(buf);
			} else {
				m_loginProtocols.push_back(buf);
			}
		}
	}
}

DPServerInfo::~DPServerInfo( )
{
	//WscLock( s_hLock );
	list<UserIdentity *>::iterator it;
	for (it = m_userIdentities.begin(); it != m_userIdentities.end(); it++) {
		delete *it;
	}
	m_userIdentities.clear();

	list<CPIdentity *>::iterator it2;
	for (it2 = m_CPIdentities.begin(); it2 != m_CPIdentities.end(); it2++) {
		delete *it2;
	}
	m_CPIdentities.clear();

    list<Session *>::iterator it3;
	for (it3 = m_sessions.begin(); it3 != m_sessions.end(); it3++) {
		delete *it3;
	}
	m_sessions.clear();

	m_deviceRoles.clear();
	m_primaryPolicy.clear();	
	m_restrictedPolicy.clear();
	//WscUnlock( s_hLock );
}

bool DPServerInfo::GetUserLoginChallenge( GUID & sessionID, const string & protocolType, const string & name, 
										 vector<uint8> & salt, vector<uint8> & challenge)
{
	//WscLock( s_hLock );
	UserIdentity * user = FindUser(name);
	Session * sess = GetSession(sessionID);
	if (! user || !sess || protocolType != "PKCS5") {
		//WscUnlock( s_hLock );
		return false;
	}

	std::copy(user->m_salt, user->m_salt + sizeof(user->m_salt), std::back_inserter(salt) );
	uint8 nonce[SIZE_128_BITS];
    RAND_bytes(nonce, sizeof(nonce));

	std::copy(nonce, nonce + SIZE_128_BITS, std::back_inserter(challenge) );

	// remember most recent challenge value for session
	memcpy(sess->m_challenge, nonce, sizeof(sess->m_challenge)); 
	sess->m_challengeName = name; // remember the user name of the challenge as well
	//WscUnlock( s_hLock );
	return true;
}

bool DPServerInfo::UserLogin( GUID & sessionID, GUID & CP_GUID, GUID & DeviceGUID,
							 unsigned char* Challenge,int _ChallengeLength,
							 unsigned char* Authenticator,int _AuthenticatorLength)
{
	Session * sess = GetSession(sessionID);

	if (!sess || !Challenge || !Authenticator ||
		memcmp(sess->m_challenge, Challenge, sizeof(sess->m_challenge)) != 0 ||
		_ChallengeLength != sizeof(sess->m_challenge) || _AuthenticatorLength != SIZE_128_BITS) {
		return false;
	}
	UserIdentity * user = FindUser(sess->m_challengeName);
	if (! user) { // we need user to get to the STORED password and Salt
		return false;
	}
	vector<uint8> stored_password; // copy stored password into stored_password vector
	std::copy(user->m_stored_password, user->m_stored_password + SIZE_128_BITS, 
			  std::back_inserter(stored_password) );

	// copy values into vectors
	vector<uint8> challenge;		
	vector<uint8> authenticator;
	std::copy(Challenge, Challenge + sizeof(sess->m_challenge), std::back_inserter(challenge) );
	std::copy(Authenticator, Authenticator + _AuthenticatorLength, std::back_inserter(authenticator) );

	vector<uint8> computed_authenticator; // The value that the CP should be passing in
	if (Dev_ComputeAuthenticator(stored_password, (unsigned char *) &CP_GUID,(unsigned char *) &DeviceGUID, 
								 challenge, computed_authenticator)) {
		bool res = (authenticator == computed_authenticator);
		if (res) {
			sess->m_user = user;
			memset(sess->m_challenge,0,sizeof(sess->m_challenge)); // clear challenge data
			sess->m_failedLogins = 0;
			return true;
		}
	}

	if (sess->m_failedLogins++ > 5) { // TODO: add more return values to signal the caller to reset the SSL connection.
		memset(sess->m_challenge,0,sizeof(sess->m_challenge)); // at least force another GetUserLoginChallenge()
		sess->m_failedLogins = 0;
	}

	return false;
}

// ComputeAuthenticator returns the first 128 bits of 
// HMAC-SHA-256(STORED, Challenge || DeviceID || ControlPointID)
bool DPServerInfo::Dev_ComputeAuthenticator(vector<uint8> & stored_password, unsigned char* CP_GUID, 
											unsigned char* DeviceGUID, vector<uint8> & challenge, 
											vector<uint8> & authenticator)
{
	int csize = challenge.size();
	int psize = stored_password.size();

	if (csize == SIZE_128_BITS && psize == SIZE_128_BITS) {
		// set up data to hash
		unsigned char iv[SIZE_128_BITS + SIZE_128_BITS + SIZE_128_BITS];
		memcpy(iv, & challenge[0], csize);
		memcpy(iv + csize, DeviceGUID, SIZE_128_BITS);
		memcpy(iv + csize + SIZE_128_BITS, CP_GUID, SIZE_128_BITS);

		unsigned char hash[SHA256_DIGEST_LENGTH];
		unsigned int hashlen = SHA256_DIGEST_LENGTH;

		if (!HMAC(EVP_sha256(),& stored_password[0], psize, iv, sizeof(iv), hash, &hashlen)) {
			return false;
		}
		authenticator.clear();
		std::copy(hash, hash + SIZE_128_BITS, std::back_inserter(authenticator) );
		return true;
	}
	return false;
}

bool DPServerInfo::CP_GetAuthenticator(const char * name, const char * password, 
									   GUID & CP_GUID, GUID & DeviceGUID,
								unsigned char* Salt,int SaltLength,unsigned char* Challenge,int ChallengeLength,
								vector<uint8> & authenticator)
{
	vector<uint8> stored_password;
	vector<uint8> salt;
	vector<uint8> challenge;
	std::copy(Salt, Salt + SaltLength, std::back_inserter(salt) );
	std::copy(Challenge, Challenge + ChallengeLength, std::back_inserter(challenge) );
	bool res = ComputeStoredPassword(name, password, salt, false, stored_password);
	if (res) { // use the same helper function that a Device calls to get the Authenticator value
		return Dev_ComputeAuthenticator(stored_password, (unsigned char*) &CP_GUID, 
			(unsigned char*) &DeviceGUID, challenge, authenticator);
	}
	return false;
}


// If initializeSalt is true, a new random salt will be computed and returned in the salt 
// parameter. If false, the initial value passed in as salt will be used.
//
bool DPServerInfo::ComputeStoredPassword(const char * name, const char * password,
										 vector<uint8> & salt, bool initializeSalt,
										 vector<uint8> & stored_password)
{
	unsigned char digest[SHA256_DIGEST_LENGTH];
	unsigned char result[SHA256_DIGEST_LENGTH];
	unsigned char *p;
	unsigned char block_index[4];
	unsigned char saltBuf[SIZE_128_BITS];
	int namelen, passlen;
	int iter = 5000; // count value for PKCS#5 iterations

	if (!name || !password) {
		return false;
	}
	string userName = name;

	namelen = (int) userName.length();
	passlen = (int) strlen(password);
	if (initializeSalt) { // generate random salt to use
		RAND_bytes(saltBuf, sizeof(saltBuf));
		std::copy(saltBuf, saltBuf + sizeof(saltBuf), std::back_inserter(salt) );
	} else { // use the salt passed in to the function
		if (salt.size() == sizeof(saltBuf)) {
			std::copy(salt.begin(), salt.end(), saltBuf);
		} else {
			return false;
		}
	}
	// set block_index = 1 in little-endian order
	block_index[0] = block_index[1] = block_index[2] = 0;
	block_index[3] = 1;

	HMAC_CTX hctx;
	HMAC_CTX_init(&hctx);
	p = result;

	// First configure the HMAC context with the stored password.
	HMAC_Init_ex(&hctx, password, passlen, EVP_sha256(), NULL);
	// Next hash in the name, salt, and block index
	HMAC_Update(&hctx, (const unsigned char *) userName.c_str(), namelen);
	HMAC_Update(&hctx, saltBuf, sizeof(saltBuf));
	HMAC_Update(&hctx, block_index, sizeof(block_index));
	HMAC_Final(&hctx, digest, NULL);
	memcpy(p, digest, SHA256_DIGEST_LENGTH);
	// Now iterate 
	for (int j = 1; j < iter; j++) {
		HMAC(EVP_sha256(), password, passlen,
			 digest, SHA256_DIGEST_LENGTH, digest, NULL);
		for (int k = 0; k < SHA256_DIGEST_LENGTH; k++) {
			p[k] ^= digest[k]; // XOR each intermediate digest into result
		}
	}
	HMAC_CTX_cleanup(&hctx);
	// Now copy the first 128 bits of the result into STORED 
	stored_password.clear();
	std::copy ( result, result + SIZE_128_BITS, std::back_inserter ( stored_password ) );
	return true;
}

// std::copy(v.begin(), v.end(), a) ;
// std::copy ( a, a + n, std::back_inserter ( v ) );

// stored points to 128-bit STORED password value, salt points to 128-bit salt for that
// password
bool DPServerInfo::SetUserLoginPassword(const char * name, unsigned char* Stored,
										int _StoredLength,unsigned char* Salt,int _SaltLength)
{
	if (! Stored || _StoredLength != SIZE_128_BITS || ! Salt || _SaltLength != SIZE_128_BITS) {
		return false;
	}
	UserIdentity * user = FindUser(name);
	if (user) { 
		std::copy(Stored, Stored + _StoredLength, user->m_stored_password) ;
		std::copy(Salt, Salt + _SaltLength, user->m_salt) ;
		return true;
	}
	return false;
}

bool DPServerInfo::RemoveIdentity( const char * identity ) {
	IdentityBase * id = FindIdentity(identity);
	UserIdentity * user = dynamic_cast<UserIdentity*>(id);
	if (user) {
		// First clear all sessions with logins of that user
		//WscLock( s_hLock );
		Session * sess = NULL;
		list<Session *>::iterator it;				;
		for (it = m_sessions.begin(); it != m_sessions.end(); it++) {
			if (user == (*it)->m_user) {
				(*it)->m_user = NULL;
			}
		}
		//WscUnlock( s_hLock );
		// Now remove the user identity record.
		RemoveUser(user);
		return true;
	} else {
		CPIdentity * cp = dynamic_cast<CPIdentity*>(id);
		if (cp) {
			// First clear all sessions with that CP identity
			//WscLock( s_hLock );
			Session * sess = NULL;
			list<Session *>::iterator it;				;
			for (it = m_sessions.begin(); it != m_sessions.end(); it++) {
				if (cp == (*it)->m_cp) {
					(*it)->m_cp = NULL;
				}
			}
			//WscUnlock( s_hLock );
			// Now remove the CP identity record.
			RemoveCP(cp);
			return true;
		}
	}
	return false;
}

CPIdentity * DPServerInfo::AddCP( const string & certName, const string & id, const string & roles, const string & alias, bool introduced ) {
	CPIdentity * identity = new CPIdentity( certName, id, roles, alias, introduced);

	//WscLock( s_hLock );
	if (identity) {
		m_CPIdentities.push_back(identity);
	}
	//WscUnlock( s_hLock );
	return identity;
}

void DPServerInfo::RemoveCP( CPIdentity * id ) {
	if (! id)
		return;
	//WscLock( s_hLock );
	m_CPIdentities.remove(id);
	delete id;
	//WscUnlock( s_hLock );
}

// AddIdentityList processes its input XML and adds any User or CP Identities that are not already
// known and that have all of the required elements (Name, uuid, etc.).  If an invalid Identity
// is parsed, it is skipped and processing of other Identities continues.
//
bool DPServerInfo::AddIdentityList( const char * identityList )
{
	bool SomeIdAdded = false;
	char *p_Name = NULL;
	int p_NameLength = 0;
	char *p_Alias = NULL;
	int p_AliasLength = 0;
	char *p_ID = NULL;
	int p_IDLength = 0;

	struct ILibXMLNode *xnode = ILibParseXML((char *) identityList,0/*offset*/,(int) strlen(identityList));
	struct ILibXMLNode *root = xnode;
	if(ILibProcessXMLNodeList(root)!=0) 
	{
		ILibDestructXMLNodeList(root);
		return SomeIdAdded;	
	}
	while(xnode!=NULL) // outer
	{
		if(xnode->StartTag!=0 && xnode->NameLength==10 && memcmp(xnode->Name,"Identities",10)==0)
		{
			xnode = xnode->Next; // get first child
			while(xnode!=NULL)   
			{
				if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"User",4)==0)
				{
					p_Name = NULL;
					xnode = xnode->Next; // get first child
					while(xnode!=NULL)
					{
						if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"Name",4)==0)
						{
							SomeIdAdded = true;
							p_NameLength = ILibReadInnerXML(xnode,&p_Name);
							p_Name[p_NameLength]=0; // NOTE: modifies underlying string
							UserIdentity * user = FindUser(p_Name);
							if (! user) { // If not known yet, add the CP Identity record
								AddUser( p_Name, vector<uint8>(), vector<uint8>(), "Public" );
							}
							xnode = xnode->Parent; // pop up to User again
							break;
						}
						if(xnode->Peer==NULL)
						{
							xnode = xnode->Parent; // no Name, pop up to User again
							break;
						}
						else
						{
							xnode = xnode->Peer; // keep looking for Name
						}
					}
				} // if node is User
				else if(xnode->StartTag!=0 && xnode->NameLength==2 && memcmp(xnode->Name,"CP",2)==0)
				{
					p_Name = p_ID = p_Alias = NULL;
					int OK = 0;
					xnode = xnode->Next; // get first child
					while(xnode!=NULL)
					{
						if(xnode->NameLength==4 && memcmp(xnode->Name,"Name",4)==0)
						{
							p_NameLength = ILibReadInnerXML(xnode,&p_Name);
							p_Name[p_NameLength]=0;
							OK |= 1;
						}
						else if(xnode->NameLength==2 && memcmp(xnode->Name,"ID",2)==0)
						{
							p_IDLength = ILibReadInnerXML(xnode,&p_ID);
							p_ID[p_IDLength]=0;
							OK |= 2;
						}
						else if(xnode->NameLength==5 && memcmp(xnode->Name,"Alias",5)==0)
						{
							p_AliasLength = ILibReadInnerXML(xnode,&p_Alias);
							p_Alias[p_AliasLength]=0;
							OK |= 4;
						}
						if(xnode->Peer==NULL)
						{
							xnode = xnode->Parent; // no more children, pop up to CP again
							SomeIdAdded = true;
							CPIdentity * id = NULL;
							if (OK & 2) { // ID data was found
								id = FindCPbyID(p_ID);
							}
							if (! id && (OK & 3) == 3) { // If not known yet, add the CP Identity record
								if (p_Alias) {
									AddCP(p_Name, p_ID, "Public", p_Alias);
								} else {
									AddCP(p_Name, p_ID, "Public", "");
								}
							}			
							break;
						}
						else
						{
							xnode = xnode->Peer; // keep looking for child nodes
						}
					} // while processing children of CP
				} // if node is CP
				xnode = xnode->Peer; // process next child of Identities
			} // while(xnode!=NULL) // inside Identities
		} // if xnode is Identities
		if(xnode!=NULL){xnode = xnode->Peer;} 
	} 	// while(xnode!=NULL), scan for Identities

	ILibDestructXMLNodeList(root);
	return SomeIdAdded;
}


// CheckAuthorization checks if the CP is unconditionally authorized to invoke the action (i.e., it
// has a Role present in the primary access control policy of the service).
//
bool DPServerInfo::CheckAuthorization( GUID & sessionID, 
									   const string & UDN, const string & Service, const string & Action)
{
	GUID localGUID;
	memset(&localGUID, 0, sizeof(localGUID));
	if (memcmp( & sessionID, & localGUID, sizeof(localGUID)) == 0) { // unknown session
		return false;
	}

	string requiredRoles = m_primaryPolicy[UDN + Service + Action];
	if (requiredRoles.length() == 0) {
		return true; // if no policy set, assume publicly-available
	}
	string sessionRoles = GetRolesOfSession(sessionID);

	// Don't just do a substring search, because there is no assurance that role names
	// won't contain other role names as substrings.  So, build a list where each role is
	// a separate item on the list.
	list<string> requiredRolesList;
	string buf; // temp variable to extract one Role at a time
    stringstream ss1(requiredRoles); 
	while (ss1 >> buf) {
		requiredRolesList.push_back(buf); // build a list of required roles
	}

    list<string>::iterator pos;
    stringstream ss2(sessionRoles); // Check each session role
	while (ss2 >> buf) {
		pos = find (requiredRolesList.begin(), requiredRolesList.end(), buf);       
		if (pos != requiredRolesList.end()) { // session role is permitted by policy
			return true;
		}
	}
	return false;
}

void DPServerInfo::NewSession(GUID & sessionID)
{
	//WscLock( s_hLock );
	Session * sess = GetSession(sessionID); // Don't do anything if session already exists
	if (! sess) {
		Session * sess = new Session(sessionID);
		if (sess) {
			m_sessions.push_back(sess);
		}
	}
	//WscUnlock( s_hLock );
}

void DPServerInfo::RemoveSession(GUID & sessionID)
{
	//WscLock( s_hLock );
	Session * sess = GetSession(sessionID);
	if (sess) {
		m_sessions.remove(sess);
		delete sess;
	}
	//WscUnlock( s_hLock );
}

Session * DPServerInfo::GetSession(GUID & sessionID) {
	//WscLock( s_hLock );
	Session * sess = NULL;
	list<Session *>::iterator it;
	for (it = m_sessions.begin(); it != m_sessions.end(); it++) {
		if (memcmp(& sessionID, & (*it)->m_sessionId, sizeof(GUID)) == 0) {
			sess = *it;
			break;
		}
	}
	//WscUnlock( s_hLock );
	return sess;
}

string DPServerInfo::GetRolesOfSession( GUID & sessionID )
{
	string roles = "Public";
	Session * sess = GetSession(sessionID);
	if (sess) {
		if (sess->m_cp) {
			roles += " " + sess->m_cp->m_roles;
		}
		if (sess->m_user) {
			roles += " " + sess->m_user->m_roles;
		}
	}
	return roles;
}

string DPServerInfo::GetUserOfSession( GUID & sessionID )
{
	Session * sess = GetSession(sessionID);
	if (sess && sess->m_user) {
		return sess->m_user->m_name;
	}
	return "";
}

UserIdentity * DPServerInfo::FindUser(const string & user) {
	if (user.size() == 0) {
		return NULL;
	}

	list<UserIdentity *>::iterator it;
	//WscLock( s_hLock );
	for (it = m_userIdentities.begin(); it != m_userIdentities.end(); it++) {

		string identity = (*it)->m_name.c_str();

		if (user == identity) {
			//WscUnlock( s_hLock );
			return *it;
		}
	}
	//WscUnlock( s_hLock );
	return NULL;
}

CPIdentity * DPServerInfo::FindCPbyID(string ID) {
	list<CPIdentity *>::iterator it;
	//WscLock( s_hLock );
	for (it = m_CPIdentities.begin(); it != m_CPIdentities.end(); it++) {
		if (_strcmpi(ID.c_str(), (*it)->m_ID.c_str()) == 0) {
			//WscUnlock( s_hLock );
			return *it;
		}
	}
	//WscUnlock( s_hLock );
	return NULL;
}

IdentityBase * DPServerInfo::FindIdentity( const char * id )
{
	IdentityBase * ret = NULL;
	char *p_Identity = NULL;
	int p_IdentityLength = 0;

	struct ILibXMLNode *xnode = ILibParseXML((char *) id,0/*offset*/,(int) strlen(id));
	struct ILibXMLNode *root = xnode;
	if(ILibProcessXMLNodeList(root)!=0) 
	{
		goto NotFound; /* The XML is not well formed! */
	}
	while(xnode!=NULL) // outer
	{
		if(xnode->StartTag!=0 && xnode->NameLength==8 && memcmp(xnode->Name,"Identity",8)==0)
		{
			xnode = xnode->Next; // get first child
			while(xnode!=NULL)   
			{
				if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"User",4)==0)
				{
					xnode = xnode->Next; // get first child
					while(xnode!=NULL)
					{
						if(xnode->StartTag!=0 && xnode->NameLength==4 && memcmp(xnode->Name,"Name",4)==0)
						{
							p_IdentityLength = ILibReadInnerXML(xnode,&p_Identity);
							p_Identity[p_IdentityLength]=0; // NOTE: modifies underlying string
							ILibDestructXMLNodeList(root);
							return FindUser(p_Identity);
						} 
						xnode = xnode->Peer; // next child
					}
					goto NotFound;
				} 
				else if(xnode->StartTag!=0 && xnode->NameLength==2 && memcmp(xnode->Name,"CP",2)==0)
				{
					xnode = xnode->Next; // get first child
					while(xnode!=NULL)
					{
						if(xnode->StartTag!=0 && xnode->NameLength==2 && memcmp(xnode->Name,"ID",2)==0)
						{
							p_IdentityLength = ILibReadInnerXML(xnode,&p_Identity);
							p_Identity[p_IdentityLength]=0; // NOTE: modifies underlying string
							ILibDestructXMLNodeList(root);
							return FindCPbyID(p_Identity);
						} 
						xnode = xnode->Peer; // next child
					}
					goto NotFound;
				}
				xnode = xnode->Peer; // next child
			} // while(xnode!=NULL) // inside Identity
		} // if xnode is Identity
		if(xnode!=NULL){xnode = xnode->Peer;}
	} 	// while scan for Identity

NotFound:
	ILibDestructXMLNodeList(root);
	return NULL;
}

string DPServerInfo::GetIdentityList()
{
	// Note:  to reduce the number of reallocations, may want to pre-allocate space in 
	// return string based on number of identities.

	//WscLock( s_hLock );
	string ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Identities ";
	ret += NamespaceSchema;
	ret += ">";
	list<UserIdentity *>::iterator it;
	for (it = m_userIdentities.begin(); it != m_userIdentities.end(); it++) {
		ret += "<User><Name>";
		ret += (*it)->m_name;
		ret += "</Name></User>";
	}
	list<CPIdentity *>::iterator it2;
	for (it2 = m_CPIdentities.begin(); it2 != m_CPIdentities.end(); it2++) {
		ret += "<CP";
		if ((*it2)->m_introduced) {
			ret += " introduced=\"1\"><Name>";
		} else {
			ret += "><Name>";
		}
		ret += (*it2)->m_certName;
		ret += "</Name><Alias>";
		ret += (*it2)->m_alias;
		ret += "</Alias><ID>";
		ret += (*it2)->m_ID;
		ret += "</ID></CP>";
	}
	ret += "</Identities>";
	//WscUnlock( s_hLock );
	return ret;
}

string DPServerInfo::GetACL()
{
	// Note:  to reduce the number of reallocations, may want to pre-allocate space in 
	// return string based on number of identities.

	//WscLock( s_hLock );
	string ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><ACL ";
	ret += NamespaceSchema;
	ret += "><Identities>";
	list<UserIdentity *>::iterator it;
	for (it = m_userIdentities.begin(); it != m_userIdentities.end(); it++) {
		ret += "<User><Name>";
		ret += (*it)->m_name;
		ret += "</Name><RoleList>";
		ret += (*it)->m_roles;
		ret += "</RoleList></User>";
	}
	list<CPIdentity *>::iterator it2;
	for (it2 = m_CPIdentities.begin(); it2 != m_CPIdentities.end(); it2++) {
		ret += "<CP";
		if ((*it2)->m_introduced) {
			ret += " introduced=\"1\"><Name>";
		} else {
			ret += "><Name>";
		}
		ret += (*it2)->m_certName;
		ret += "</Name><Alias>";
		ret += (*it2)->m_alias;
		ret += "</Alias><ID>";
		ret += (*it2)->m_ID;
		ret += "</ID><RoleList>";
		ret += (*it2)->m_roles;
		ret += "</RoleList></CP>";
	}
	ret += "</Identities><Roles>";
	vector<string>::iterator it3;
	for (it3 = m_deviceRoles.begin(); it3 != m_deviceRoles.end(); it3++) {
		ret += "<Role><Name>";
		ret += *it3;
		ret += "</Name></Role>";
	}
	ret += "</Roles></ACL>";
	//WscUnlock( s_hLock );
	return ret;
}

string DPServerInfo::GetSupportedProtocols()
{
	//WscLock( s_hLock );
	string ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><SupportedProtocols ";
	ret += NamespaceSchema;
	ret += ">";
	vector<string>::iterator it;
	for (it = m_introductionProtocols.begin(); it != m_introductionProtocols.end(); it++) {
		ret += "<Introduction><Name>";
		ret += *it;
		ret += "</Name></Introduction>";
	}
	for (it = m_loginProtocols.begin(); it != m_loginProtocols.end(); it++) {
		ret += "<Login><Name>";
		ret += *it;
		ret += "</Name></Login>";
	}
	ret += "</SupportedProtocols>";
	//WscUnlock( s_hLock );
	return ret;
}

bool DPServerInfo::RemoveRolesForIdentity( const char * identity, const char * roles )
{
	IdentityBase * id = FindIdentity(identity);
	if (! id) {
		return false;
	}
	// now do error checking on the role list to remove
	stringstream tempRoles(roles); // roles to remove
	string buf; // temp variable to extract one Role at a time
	while (tempRoles >> buf) {
		if ( find(m_deviceRoles.begin(), m_deviceRoles.end(), buf) == m_deviceRoles.end() ) {
			// role to add is not known to the Device... return error.
			return false;
		}
	}

	list<string> currentRoleList;
	list<string>::iterator pos;
	stringstream ss1(id->m_roles); 
	while (ss1 >> buf) {
		currentRoleList.push_back(buf); // build a list of current roles
	}
	stringstream ss2(roles); // now go through list of roles to remove
	while (ss2 >> buf) {
		pos = find (currentRoleList.begin(), currentRoleList.end(), buf);       
		if (pos != currentRoleList.end()) { // in current list, so needs to be removed
			currentRoleList.remove(buf); 
		}
	}
	// Now reconstruct m_roles based on updated currentRoleList
	//WscLock( s_hLock );
	id->m_roles.clear();
	for (pos = currentRoleList.begin(); pos != currentRoleList.end(); pos++) {
		if (pos != currentRoleList.begin()) {
			id->m_roles += " "; // put space before all except first Role
		}
		id->m_roles += *pos; // add role to the Role string of the user
	}
	//WscUnlock( s_hLock );
	return true;
}

bool DPServerInfo::AddRolesForIdentity( const char * identity, const char * roles ) {
	IdentityBase * id = FindIdentity(identity);
	if (! id) {
		return false;
	}
	// now do error checking on the role list to add
	stringstream tempRoles(roles); // roles to add
	string buf; // temp variable to extract one Role at a time
	while (tempRoles >> buf) {
		if ( find(m_deviceRoles.begin(), m_deviceRoles.end(), buf) == m_deviceRoles.end() ) {
			// role to add is not known to the Device... return error.
			return false;
		}
	}

	list<string> currentRoleList;
	stringstream ss1(id->m_roles); 
	while (ss1 >> buf) {
		currentRoleList.push_back(buf); // build a list of current roles
	}
	stringstream ss2(roles); // now go through list of roles to add
	while (ss2 >> buf) {
		list<string>::iterator pos;

		pos = find (currentRoleList.begin(), currentRoleList.end(), buf);       
		if (pos == currentRoleList.end()) { // not in current list
			//WscLock( s_hLock );
			id->m_roles += " ";
			id->m_roles += buf; // add role to the Role string of the user
			//WscUnlock( s_hLock );
			currentRoleList.push_back(buf); 
		}
	}
	return true;
}

#ifdef notdef
int _tmain(int argc, _TCHAR* argv[])
#endif

extern int TestDPServerInfo(DPServerInfo & rec)
{
	// DPServerInfo rec("ExtraRole", "TLS-PSK , random", "admin password");

	string buf;

	buf = "<?xml ffsd ?><Identity><User><Name>my name</Name></User></Identity>";
	rec.RemoveIdentity(buf.c_str());
	buf = "<?xml ffsd ?><Identity><CP><ID>UU-ID</ID></CP></Identity>";
	rec.RemoveIdentity(buf.c_str());

	UserIdentity * us = rec.AddUser( "foo", vector<uint8>(), vector<uint8>(), "Admin" );

	buf = "<?xml version?><Identities xmlns= xmlns:xsi=on-v1.xsd\">\
	<User><Name>Vic</Name></User>\
	<CP><Name>Vendor X Device</Name><Alias>My phone</Alias><ID>e593d8e6-6b8b-49d9-845a-21828db570e9</ID></CP>\
	<User><Name>Mika</Name></User>\
	<CP><Name>Vendor Y Device</Name><ID>e593d8e6-6b8b-49d9-845a-21828db570e9</ID></CP>\
	</Identities>";
	rec.AddIdentityList(buf.c_str());

	CPIdentity * foo = rec.AddCP("cert name","abcd-ef-fe-ababcdcd","Basic","");
	buf = "<?xml ffsd ?><Identity><CP><ID>abcd-ef-fe-ababcdcd</ID></CP></Identity>";
	rec.AddRolesForIdentity(buf.c_str(),"Public Admin Public Public");
	buf = "<?xml ffsd ?><Identity><CP><ID>abcd-ef-fe-ababcdcd</ID></CP></Identity>";
	rec.RemoveRolesForIdentity(buf.c_str()," Public  ");
	foo->m_alias = "my alias";
	string s = rec.GetACL();
	printf("ACL: %s\n\n",s.c_str());
	s = rec.GetSupportedProtocols();
	printf("Protocols: %s\n\n",s.c_str());

	buf = "<?xml ffsd ?><Identity><CP><ID>abcd-ef-fe-ababcdcd</ID></CP></Identity>";
	bool res = rec.RemoveIdentity(buf.c_str());

	s = rec.GetACL();
	printf("ACL: %s\n\n",s.c_str());
	s = rec.GetIdentityList();
	printf("Identities: %s\n",s.c_str());

	int i = 9;
	return i;
}

/* Code to insert in DVMicroStack.c in function DVMicroStackToken DVCreateMicroStack()
	{ // vbl added this hack to extend DDD with DeviceProtection-related URLs.  Warning: this only supports one instance of DeviceProtection.
		char * DevProtService = "e xmlns:dp=\"urn:schemas-upnp-org:gw:DeviceProtection\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
xsi:schemaLocation=\"urn:schemas-upnp-org:gw:DeviceProtection http://www.upnp.org/schemas/gw/DeviceProtection-v1.xsd\">";
		char * DevProtSecureURLs = "L><dp:secureSCPDURL>https:DeviceProtection/scpd.xml</dp:secureSCPDURL> \
<dp:secureControlURL>https:DeviceProtection/control</dp:secureControlURL> \
<dp:secureEventSubURL>https:DeviceProtection/event</dp:secureEventSubURL>";
		char * pTempDDD = NULL;
		char * pDevProt = NULL;

		pDevProt = strstr(RetVal->DeviceDescription, "service:DeviceProtection:1"); // DeviceProtection service present
		if (pDevProt != NULL) { 
			// First grow the DDD to be big enough for the extra data
			pTempDDD = malloc( RetVal->DeviceDescriptionLength +
							   strlen(DevProtService) + strlen(DevProtSecureURLs));
		}
		if (pDevProt && pTempDDD) { // find the service description in the realloced DDD string
			char * pDevProtService = strstr(pDevProt - 50, "<service>"); // find the previous <service> tag
			char * pDevProtSecureURLs = strstr(pDevProt, "</eventSubURL>"); // find the event subscription URL tag
			int newLength = 0;
			if (pDevProtSecureURLs && pDevProtService) {
				memcpy(pDevProtService,"<servic%s",strlen("<service>"));
				memcpy(pDevProtSecureURLs,"</eventSubUR%s",strlen("</eventSubURL>"));
				newLength = sprintf(pTempDDD,RetVal->DeviceDescription,DevProtService,DevProtSecureURLs);
				free(RetVal->DeviceDescription);
				RetVal->DeviceDescription = pTempDDD;
				RetVal->DeviceDescriptionLength = newLength;
			}
		}
	} // vbl end of inserted code
	free(DDT);

	*/
