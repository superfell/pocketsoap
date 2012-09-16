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

#if !defined(AFX_TRACINGSTREAM_H__CCD4E693_767D_4D8E_BD6F_225ADC9F1C9F__INCLUDED_)
#define AFX_TRACINGSTREAM_H__CCD4E693_767D_4D8E_BD6F_225ADC9F1C9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TracingMgr
{
public:
	TracingMgr();
	~TracingMgr();

	void AddRef();
	void Release();
	HRESULT Init(CComBSTR &file);
	IReadableStream * WrapStream(IReadableStream *s);
	void setTracing(bool tracingOn);
	void WriteRequestHeader();
	void WriteResponseHeader();
	void Write(const void *pv, DWORD cb);

private:
	long		m_refcount;
	HANDLE		m_trace;
	bool		m_enabled;
	bool		m_closeOnZeroBytes;
	void Close();
};

class ATL_NO_VTABLE TracingStream : 
	public IResetableStream, 
	public CComObjectRootEx<CComMultiThreadModel>  
{
public:
	TracingStream() : m_mgr(0)
	{
		ATLTRACE(_T("TracingStream::TracingStream()\n"));
	}
	virtual ~TracingStream()
	{
		ATLTRACE(_T("TracingStream::~TracingStream()\n"));
		if(m_mgr)
			m_mgr->Release();
	}

BEGIN_COM_MAP(TracingStream)
	COM_INTERFACE_ENTRY(IResetableStream)
	COM_INTERFACE_ENTRY(IReadableStream)
END_COM_MAP()

	STDMETHODIMP Read( void  *pv,			// Pointer to buffer into which the stream is read
					   ULONG cb,			// Specifies the number of bytes to read
					   ULONG *pcbRead );	// Pointer to location that contains actual
											// number of bytes read
	STDMETHODIMP Reset();

	void Init(IReadableStream * s, TracingMgr * mgr);
private:
	CComPtr<IReadableStream> m_src;
	TracingMgr				*m_mgr;
};

#endif // !defined(AFX_TRACINGSTREAM_H__CCD4E693_767D_4D8E_BD6F_225ADC9F1C9F__INCLUDED_)
