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
Portions created by Simon Fell are Copyright (C) 2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#pragma once

#include "HTTPtransport.h"
#include "ConvertUTF.h"

typedef HRESULT (* TranscodeToUtf16)( std::vector<UTF16> &tmpBuffer, const BYTE *start, DWORD len, CComBSTR &dest, DWORD *numBytesLeftInSrc);

class ATL_NO_VTABLE HttpResponse :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IsfDelegatingDispImpl<IHttpResponse>
{
public:
	HttpResponse();
	virtual ~HttpResponse();

BEGIN_COM_MAP(HttpResponse)
	COM_INTERFACE_ENTRY(IHttpResponse)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	void Init ( IReadableStream * src, Headers headers, short sc ) ;

// IHttpResponse
	STDMETHOD(get_String)		( BSTR * responseString  ) ;
	STDMETHOD(get_Bytes)		( SAFEARRAY ** byteArray ) ;
	STDMETHOD(get_Stream)		( IReadableStream ** stm ) ;

	STDMETHOD(get_StatusCode)	( short * statusCode ) ;
	STDMETHOD(get_Headers)		( IHeadersCollection ** ppHeaders ) ;

	STDMETHOD(SaveAs)			( BSTR filename ) ;

private:
	CComPtr<IReadableStream>	m_stream ;
	short						m_statusCode ;
	Headers						m_headers ;

	HRESULT ReadToBuffer(std::vector<BYTE> &buff);
	HRESULT ReadToString(CComBSTR &dest, TranscodeToUtf16 transcoder );
};


