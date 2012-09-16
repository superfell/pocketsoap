/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketHTTP.

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2005
Simon Fell. All Rights Reserved.

Contributor(s):
*/

//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PocketHTTP.h"
#include "TracingStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
TracingMgr::TracingMgr() : m_trace(INVALID_HANDLE_VALUE), m_enabled(false), m_refcount(0), m_closeOnZeroBytes(false)
{
	ATLTRACE(_T("TracingMgr::TracingMgr()\n"));
}

TracingMgr::~TracingMgr()
{
	ATLTRACE(_T("TracingMgr::~TracingMgr()\n"));
	Close();
}

void TracingMgr::AddRef()
{
	ATLTRACE(_T("TracingMgr::AddRef()\n"));
	InterlockedIncrement(&m_refcount);
}

void TracingMgr::Release()
{
	ATLTRACE(_T("TracingMgr::Release()\n"));
	long rc = InterlockedDecrement(&m_refcount);
	if (0 == rc)
		delete this;
}

HRESULT TracingMgr::Init(CComBSTR &file)
{
	USES_CONVERSION;
	Close();
	m_trace = ::CreateFile(OLE2T(file), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, CREATE_FILE_FLAGS, NULL);	
	if (INVALID_HANDLE_VALUE == m_trace)
		return HRESULT_FROM_WIN32(GetLastError());
	SetFilePointer(m_trace, 0, NULL, FILE_END);
	return S_OK;
}

void TracingMgr::Close()
{
	if(INVALID_HANDLE_VALUE != m_trace)
	{
		CloseHandle(m_trace);
		m_trace = INVALID_HANDLE_VALUE;
		m_enabled = false;
	}
}

void TracingMgr::setTracing(bool tracingOn)
{
	m_enabled = tracingOn && (m_trace != INVALID_HANDLE_VALUE);
}

void TracingMgr::WriteRequestHeader()
{
	USES_CONVERSION;
	char status[100];
	SYSTEMTIME stNow;
	CComVariant vNow;
	GetLocalTime(&stNow);
	::SystemTimeToVariantTime(&stNow, &vNow.date);
	vNow.vt = VT_DATE;
	vNow.ChangeType(VT_BSTR);
	int sLen = sprintf(status, "\r\n\r\nHTTP Request sent at %s\r\n", OLE2A(vNow.bstrVal));
	Write(status, sLen);
}

void TracingMgr::WriteResponseHeader()
{
	Write("\r\n", 2);
	m_closeOnZeroBytes = true;
}

void TracingMgr::Write(const void *pv, DWORD cb) 
{
	if (!m_enabled) return;
	if (cb > 0)
	{
		DWORD written;
		WriteFile(m_trace, pv, cb, &written, NULL);
	}
	else
	{
		if (m_closeOnZeroBytes) Close();
	}
}

IReadableStream * TracingMgr::WrapStream(IReadableStream *s)
{
	CComObject<TracingStream> * ts = 0;
	ts->CreateInstance(&ts);
	ts->AddRef();
	ts->Init(s, this);
	return ts;
}

void TracingStream::Init(IReadableStream *s, TracingMgr *mgr)
{
	m_src = s;
	m_mgr = mgr;
	m_mgr->AddRef();
}

STDMETHODIMP TracingStream::Read( void *pv,			// Pointer to buffer into which the stream is read
								 ULONG cb,			// Specifies the number of bytes to read
								 ULONG *pcbRead )	// Pointer to location that contains actual
{													// number of bytes read
	HRESULT hr = m_src->Read(pv, cb, pcbRead);
	if (SUCCEEDED(hr))
		m_mgr->Write(pv, *pcbRead);
	return hr;
}

STDMETHODIMP TracingStream::Reset()
{
	CComQIPtr<IResetableStream> rs(m_src);
	if(rs)
		return rs->Reset();
	else
		return E_NOINTERFACE;
}
