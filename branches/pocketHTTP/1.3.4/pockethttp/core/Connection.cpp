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

#include "stdafx.h"
#include "connection.h"

/////////////////////////////////////////////////////////////////////////////
// statics
/////////////////////////////////////////////////////////////////////////////
ConnectionPool thePool ;

/////////////////////////////////////////////////////////////////////////////
// Connection
/////////////////////////////////////////////////////////////////////////////
Connection::~Connection()
{
	if ( socket != INVALID_SOCKET )
	{
		Disconnect() ;
	}
}

bool operator == ( const ConnectionKey &a, const ConnectionKey &b ) 
{
	// matching auth info ?
	if ( a.authInfoHash != b.authInfoHash )
		return false;

	// direct connections
	if ( a.px_addr == 0 && b.px_addr == 0 )
		return ( a.addr == b.addr ) && ( a.port == b.port) && ( a.ssl == b.ssl );

	// proxied
	if ( ( a.px_addr != 0 ) && ( b.px_addr != 0 ))
	{
		if ((a.px_addr == b.px_addr) && (a.px_port == b.px_port)) {
			// proxied HTTP
			if ( ( !a.ssl ) && ( !b.ssl ) )
				return true;
			// proxied HTTPS
			if ( a.ssl && b.ssl )
				return ( a.targetUrlHash == b.targetUrlHash ) && ( a.port == b.port );
		}
	}
	return false ;	
}

// this impl only makes sense if you call it after the connection has been in the pool
// and so, we're not expecting to be able to read any data on the socket
// as there should be no inbound data after the connection has been returned to the pool
// if there is something to read, its probably the connection close packet from the server
bool Connection::ConnectionOk()
{
	// check the status of the connection
	TIMEVAL tv = {0} ;
	fd_set fd_read = {0}, fd_write= {0}, fd_err = {0};
	FD_SET(socket, &fd_read ) ;
	FD_SET(socket, &fd_write ) ;
	FD_SET(socket, &fd_err ) ;
	int selErr = select ( 0, &fd_read, &fd_write, &fd_err, &tv ) ;
	return ! FD_ISSET(socket, &fd_read) ;
}

/////////////////////////////////////////////////////////////////////////////
// ConnectionPool
/////////////////////////////////////////////////////////////////////////////
static const DWORD SCAVANGE_INTERVAL  =  60000 ;
static const DWORD MAX_CONNECTION_AGE = 600000 ;

ConnectionPool::ConnectionPool() : m_cleaning(false)
{
	ATLTRACE(_T("ConnectionPool::ConnectionPool()\n")) ;
	m_lastScavange = GetTickCount() ;
}

ConnectionPool::~ConnectionPool()
{
	ATLTRACE(_T("ConnectionPool::~ConnectionPool()\n")) ;
	m_lock.Lock() ;
	m_cleaning = true ;
	m_pool.clear() ;
	m_lock.Unlock() ;
}

HRESULT ConnectionPool::findConnection ( ConnectionKey &key, Connection ** ppConnection )
{
	*ppConnection = NULL ;
	m_lock.Lock() ;
	for ( LIST_CON::iterator i = m_pool.begin() ; i != m_pool.end() ; i++ )
	{
		if (( !i->inUse ) && (key == (*i)) )
		{
			// we can leave the not ok connections in the pool, they'll get scavanged away eventually
			if ( i->ConnectionOk() )
			{
				i->inUse = true ;
				*ppConnection = &*i ;
				break ;
			}
		}
	}
	if ( *ppConnection == NULL )
	{
		Connection c(key) ;
		c.inUse = true ;
		m_pool.push_back(c) ;
		*ppConnection = &m_pool.back() ;
	}
	m_lock.Unlock() ;
	return S_OK ;
}

HRESULT ConnectionPool::returnToPool ( Connection * c )
{
	m_lock.Lock() ;
	c->inUse = false ;
	c->used = GetTickCount() ;
	if ( c->used > m_lastScavange + SCAVANGE_INTERVAL )
		scavangeConnections() ;
	m_lock.Unlock() ;
	return S_OK ;
}

void ConnectionPool::scavangeConnections()
{
	m_lastScavange = GetTickCount() ;
	DWORD age = m_lastScavange - MAX_CONNECTION_AGE  ;
	for ( LIST_CON::iterator i = m_pool.begin(); i != m_pool.end();  )
	{
		if (( ! i->inUse ) && ( i->used < age ))
			m_pool.erase(i++) ;
		else
			i++ ;
	}
}

HRESULT ConnectionPool::destroyConnection ( Connection * c )
{
	m_lock.Lock() ;
	// this stops double deletes when the ConnectionPool object is destroyed
	// and it has open connections still in it.
	if ( m_cleaning ) return S_OK ;

	for ( LIST_CON::iterator i = m_pool.begin() ; i != m_pool.end() ; i++ )
	{
		if ( c == &*i )
		{
			m_pool.erase(i) ;
			m_lock.Unlock() ;
			return S_OK ;
		}
	}
	m_lock.Unlock() ;
	return S_FALSE ;
}