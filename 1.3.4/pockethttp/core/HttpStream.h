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
Portions created by Simon Fell are Copyright (C) 2003-2004
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#pragma once


class ATL_NO_VTABLE HttpStream :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IReadableStream,
	public IReadableStreamVB
{
public:
	virtual ~HttpStream()
	{
		if ( m_src )
			m_src->Release() ;
	}

BEGIN_COM_MAP(HttpStream)
	COM_INTERFACE_ENTRY(IReadableStream)
	COM_INTERFACE_ENTRY(IReadableStreamVB)
END_COM_MAP()

	void Init ( CHTTPTransport * src )
	{
		m_src = src ;
		if ( m_src )
			m_src->AddRef() ;
	}

// IReadableStream
	STDMETHOD(Read)(	void *pv,			// Pointer to buffer into which the stream is read
						ULONG cb,			// Specifies the number of bytes to read
						ULONG *pcbRead )	// Pointer to location that contains actual
											// number of bytes read
	{
		return m_src->Read(pv, cb, pcbRead) ;
	}

// IReadableStreamVB
	STDMETHOD(ReadBytes)(long cb, SAFEARRAY ** ppsaBytes)
	{
		if ( ! ppsaBytes ) return E_POINTER ;
		*ppsaBytes = 0 ;
		SAFEARRAYBOUND rga ;
		rga.lLbound = 0;
		rga.cElements = cb ;
		SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
		if ( ! psa )
			return E_OUTOFMEMORY ;

		void * data = NULL ;
		SafeArrayAccessData(psa, &data) ;
		ULONG pcb = 0 ;
		HRESULT hr = Read ( data, cb, &pcb ) ;
		SafeArrayUnaccessData(psa) ;
		if ( FAILED(hr))
		{
			SafeArrayDestroy(psa) ;
			return hr ;
		
		}
		// if we got less bytes than we asked for, shrink the array down
		if ( pcb != (ULONG)cb )
		{
			rga.cElements = pcb ;
			SafeArrayRedim(psa, &rga) ;
		}
		*ppsaBytes = psa ;
		return S_OK ;
	}

private:
	CHTTPTransport *	m_src ;

};