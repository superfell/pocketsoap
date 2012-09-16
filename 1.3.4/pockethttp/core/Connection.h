/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketSOAP.

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2002,2007
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#pragma once

#include "httpBase.h"		// this is a platform dependent mix-in class
#include <hash_set>

// fwd decls
class ConnectionKey ;
class Connection ;
class ConnectionPool ;

/////////////////////////////////////////////////////////////////////////////
// ConnectionKey
//
// this is the key used to compare connections in the pool
// for direct connections addr/port/ssl is used to compare
// for proxied HTTP  px_addr/px_port is used
// for proxied HTTPS px_addr/px_port/targetUrlHash is used
/////////////////////////////////////////////////////////////////////////////
class ConnectionKey
{
public:
	ConnectionKey() : 
		addr(0), port(0), ssl(false), px_addr(0), px_port(0), authInfoHash(0), targetUrlHash(0)
	{
	}

	void setAuthInfo ( CComBSTR &userName, CComBSTR &password, CComBSTR &pxUsername, CComBSTR &pxPassword )
	{
		static const std::wstring sep(L"!") ;
		std::wstring ai ;
		ai.reserve(userName.Length() + password.Length() + pxUsername.Length() + pxPassword.Length() + (sep.size() * 3) ) ; // remember to include separaters
		ai.append ( userName, userName.Length() ) ;
		ai.append ( sep ) ;
		ai.append ( password, password.Length() ) ;
		ai.append ( sep ) ;
		ai.append ( pxUsername, pxUsername.Length() ) ;
		ai.append ( sep ) ;
		ai.append ( pxPassword, pxPassword.Length() ) ;
		authInfoHash = std::hash<std::wstring>()(ai) ;
	}

	void setTargetUrlHash ( const char *targetHostName)
	{
		std::string host(targetHostName);
		targetUrlHash = std::hash<std::string>()(host);
	}

	u_long	addr ;
	u_long	px_addr ;
	u_short	port ;
	u_short	px_port ;
	size_t	authInfoHash ;
	size_t	targetUrlHash;
	bool	ssl ;
};

bool operator == ( const ConnectionKey &a, const ConnectionKey &b ) ;

/////////////////////////////////////////////////////////////////////////////
// Connection
/////////////////////////////////////////////////////////////////////////////
class Connection : 
	public httpTransportBase<Connection>,
	public ConnectionKey
{
public:
	Connection() : 
		inUse(false), 
		socket(INVALID_SOCKET),
		used(0)
	{
		ATLTRACE("Connection:: empty constructor\n") ;
	}
	Connection(const ConnectionKey &key) : 
		ConnectionKey(key),
		inUse(false), 
		socket(INVALID_SOCKET),
		used(0)
	{
		ATLTRACE("Connection:: ConnectionKey constructor\n") ;
	}
	Connection(const Connection &rhs) :
		ConnectionKey(rhs),
		inUse(rhs.inUse),
		socket(rhs.socket),
		used(rhs.used)
	{
		ATLTRACE("Connection:: Connection copy constructor\n") ;
	}
	~Connection() ;

	void populateSockAddr(sockaddr_in &s)
	{
		memset(&s, 0, sizeof(s)) ;
		s.sin_port = px_port ? px_port : port ;
		s.sin_family = AF_INET ;
		s.sin_addr.S_un.S_addr = px_addr ? px_addr : addr ;
	}

	bool			inUse ;
	SOCKET			socket ;
	DWORD			used ;

protected:
	friend class	ConnectionPool ;
	bool			ConnectionOk() ;
};

/////////////////////////////////////////////////////////////////////////////
// ConnectionPool
/////////////////////////////////////////////////////////////////////////////
class ConnectionPool
{
public:
	ConnectionPool() ;
	~ConnectionPool();

	HRESULT findConnection    ( ConnectionKey &key, Connection ** ppConnection ) ;
	HRESULT returnToPool      ( Connection * c );
	HRESULT destroyConnection ( Connection * c );

private:
	void scavangeConnections() ;

	typedef std::list<Connection> LIST_CON ;

	LIST_CON					m_pool ;
	CComAutoCriticalSection		m_lock ;
	DWORD						m_lastScavange ;
	bool						m_cleaning ;
};
