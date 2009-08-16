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
Portions created by Simon Fell are Copyright (C) 2002
Simon Fell. All Rights Reserved.

Contributor(s):
*/

//////////////////////////////////////////////////////////////////////
// PartPayload.cpp: implementation of the PartPayload class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "attachments.h"
#include "PartPayload.h"

//////////////////////////////////////////////////////////////////////

void FilePayload::Init ( BSTR fileName )
{
	m_fileName = fileName ;
}

HRESULT FilePayload::OpenFile()
{
	USES_CONVERSION ;

#ifndef _WIN32_WCE
	DWORD flags = FILE_FLAG_SEQUENTIAL_SCAN ;
#else
	DWORD flags = 0 ;
#endif	
	if ( m_hf == INVALID_HANDLE_VALUE )
	{
		TCHAR * fn = OLE2T(m_fileName) ;
		m_hf = CreateFile ( fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, flags, NULL ) ;
	}
	if ( m_hf == INVALID_HANDLE_VALUE )
		return HRESULT_FROM_WIN32(GetLastError()) ;
	return S_OK ;
}

HRESULT FilePayload::CloseFile()
{
	if ( m_hf != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hf) ;
		m_hf = INVALID_HANDLE_VALUE ;
	}
	return S_OK ;
}

HRESULT FilePayload::Lock() 
{
	ObjectLock lock(this) ;
	if ( ++m_lock == 1 )
		return OpenFile() ;
	return S_OK ;
}

HRESULT FilePayload::Unlock() 
{
	ObjectLock lock(this) ;
	if ( --m_lock == 0 )
		return CloseFile() ;
	return S_OK ;
}

HRESULT FilePayload::Size(DWORD *size) 
{
	ObjectLock lock(this) ;
	HRESULT hr = OpenFile() ;
	if (FAILED(hr)) return hr ;
	*size = ::GetFileSize ( m_hf, NULL ) ;
	return S_OK ;
}

HRESULT FilePayload::Read( void * pv, DWORD cb, DWORD * pcbRead ) 
{
	if ( ! ReadFile ( m_hf, pv, cb, pcbRead, NULL ) )
		return HRESULT_FROM_WIN32(GetLastError()) ;
	return S_OK ;
}

HRESULT FilePayload::Reset()
{
	HRESULT hr = OpenFile() ;
	if (FAILED(hr)) return hr ;

	if ( INVALID_SET_FILE_POINTER == SetFilePointer ( m_hf, 0, 0, FILE_BEGIN ) )
		return HRESULT_FROM_WIN32(GetLastError()) ;

	return S_OK ;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


HRESULT MemoryPayload::Lock() 
{
	SafeArrayAccessData(m_psa, (void **)&m_data ) ;
	Size(&m_size) ;
	return S_OK ;
}

HRESULT MemoryPayload::Unlock() 
{
	SafeArrayUnaccessData(m_psa) ;
	m_data = 0 ;
	m_size = 0 ;
	return S_OK ;
}

HRESULT MemoryPayload::Size(DWORD *size) 
{
	if ( m_size != 0 )
	{
		*size = m_size ;
		return S_OK ;
	}
	long ub, lb ;
	SafeArrayGetUBound(m_psa, 1, &ub ) ;
	SafeArrayGetLBound(m_psa, 1, &lb ) ;
	*size = ub - lb + 1 ;
	return S_OK ;
}

HRESULT MemoryPayload::Read( void * pv, DWORD cb, DWORD * pcbRead ) 
{
	*pcbRead = min ( cb, m_size - m_pos ) ;
	memcpy ( pv, m_data + m_pos, *pcbRead ) ;
	m_pos += *pcbRead ;
	return S_OK ;
}

HRESULT MemoryPayload::Reset()
{
	m_pos = 0 ;
	return S_OK ;
}

void MemoryPayload::Init ( SAFEARRAY * psa ) 
{
	m_psa = psa ;
}