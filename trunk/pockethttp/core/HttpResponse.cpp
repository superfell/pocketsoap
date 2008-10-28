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
Portions created by Simon Fell are Copyright (C) 2003-2004
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "pocketHTTP.h"
#include "httpResponse.h"
#include "headersCollection.h"

HttpResponse::HttpResponse() : 
	m_statusCode(0)
{
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::Init ( IReadableStream * src, Headers headers, short sc ) 
{
	m_stream = src ;
	m_headers = headers ;
	m_statusCode = sc ;
}

STDMETHODIMP HttpResponse::get_Headers(IHeadersCollection ** ppCol) 
{
	if ( ! ppCol ) return E_POINTER ;
	CComObject<HeadersCollection> * hc = 0 ;
	_HR ( hc->CreateInstance(&hc) ) ;
	hc->AddRef() ;
	hc->Init ( GetUnknown(), &m_headers ) ;
	HRESULT hr = hc->QueryInterface(ppCol) ;
	hc->Release() ;
	return S_OK ;
}

STDMETHODIMP HttpResponse::get_Stream( IReadableStream ** stm ) 
{
	return m_stream.CopyTo(stm) ;
}

HRESULT HttpResponse::ReadToBuffer(std::vector<BYTE> &buff)
{
	buff.clear() ;
	BYTE tmp[PH_BUFFER_SIZE] ;
	DWORD cb = 0 ;
	do
	{
		_HR ( m_stream->Read ( tmp, sizeof(tmp), &cb ) ) ;
		if ( cb > 0 )
			buff.insert ( buff.end(), tmp, tmp + cb ) ;
    } while ( cb > 0 ) ;
	return S_OK ;
}

HRESULT HttpResponse::ReadToString(CComBSTR &dest, TranscodeToUtf16 transcoder)
{
	BYTE tmp[PH_BUFFER_SIZE] ;
	DWORD cb = 0, numBytesLeftInSrc = 0, srcSize;
	std::vector<UTF16> tmpBuffer;
	do
	{
		_HR ( m_stream->Read ( tmp + numBytesLeftInSrc, sizeof(tmp) - numBytesLeftInSrc, &cb ) ) ;
		if ( cb > 0 )
		{
			srcSize = cb + numBytesLeftInSrc;
			_HR ( transcoder ( tmpBuffer, tmp, srcSize, dest, &numBytesLeftInSrc )) ;
			if(numBytesLeftInSrc>0)
			{
				memcpy(tmp, tmp + srcSize - numBytesLeftInSrc, numBytesLeftInSrc);
			}
		}
	} while ( cb > 0 ) ;
	return S_OK ;
}

static HRESULT Utf8Transcoder ( std::vector<UTF16> &tmpBuffer, const BYTE *start, DWORD len, CComBSTR &dest, DWORD *numBytesLeftInSrc)
{
	// worse case is every utf-8 byte maps to utf16 character
	tmpBuffer.resize(len);

	const UTF8 * srcStart = (const UTF8 *)start;
	const UTF8 * srcEnd   = (const UTF8 *)(start + len);
	UTF16 *destStart	  = &tmpBuffer[0];
	UTF16 *destEnd		  = destStart + tmpBuffer.size();

	ConversionResult cr = ConvertUTF8toUTF16(&srcStart, srcEnd, &destStart, destEnd, lenientConversion);
	if ( sourceIllegal == cr )
		return AtlReportError(__uuidof(CoPocketHTTP), OLESTR("The returned UTF-8 data has an invalid byte sequence for UTF-8 encoded data"), IID_NULL, E_INVALID_UTF8_BYTES);

	*numBytesLeftInSrc = (srcEnd - srcStart) ;
	return dest.Append(tmpBuffer.begin(), destStart - &tmpBuffer[0]);
}

static HRESULT Utf16Transcoder ( std::vector<UTF16> &tempBuffer, const BYTE *start, DWORD len, CComBSTR &dest, DWORD *numBytesLeftInSrc)
{
	// src & dest are utf-16, nothing much to do, other than to copy the src over to dest, making sure we don't slice a pair of bytes.
	if ((len % 2) == 1)
	{
		--len;
		*numBytesLeftInSrc = 1;
	}
	else
		*numBytesLeftInSrc = 0;

	return dest.Append((OLECHAR *)start, len/2);
}

static HRESULT CPTranscoder ( std::vector<UTF16> &tmpBuffer, const BYTE *start, DWORD len, CComBSTR &dest, DWORD *numBytesLeftInSrc)
{
	tmpBuffer.resize(len);
	
	DWORD wideLen = MultiByteToWideChar(CP_ACP, 0, (const char *)start, len, &tmpBuffer[0], len);
	if(wideLen == 0)
		return AtlReportError(__uuidof(CoPocketHTTP), OLESTR("Unable to convert source bytes to string using the local code page"), IID_NULL, HRESULT_FROM_WIN32(GetLastError()) );

	dest.Append(&tmpBuffer[0], wideLen);
	*numBytesLeftInSrc = 0;
	return S_OK;
}

STDMETHODIMP HttpResponse::get_String(BSTR * responseString) 
{
	if ( ! responseString ) return E_POINTER ;
	
	// find the content-type header, then find the charset attribute
	static const CComBSTR CONTENT_TYPE_NAME(L"Content-Type");
	static const CComBSTR CHARSET_NAME(L"charset");

	CComBSTR charset;
	CComPtr<IHeadersCollection> headers;
	_HR(get_Headers(&headers));
	CComPtr<IHeader> contentType;
	headers->Find(CONTENT_TYPE_NAME, &contentType);
	if(contentType != NULL)
		contentType->get_Attribute(CHARSET_NAME, &charset);

	CComBSTR dest;
	bool hasCharset = charset.Length() > 0;
	if (hasCharset && (( _wcsicmp ( charset, L"utf-8" ) == 0) || ( _wcsicmp ( charset, L"utf8") == 0)))
	{
		_HR (ReadToString ( dest, Utf8Transcoder ));
	}
	else if (hasCharset && ((_wcsicmp ( charset, L"utf-16" ) == 0) || ( _wcsicmp ( charset, L"utf16") == 0)))
	{
		_HR (ReadToString ( dest, Utf16Transcoder ));
	}
	else
	{
		// assume local code page for now
		_HR (ReadToString ( dest, CPTranscoder ));
	}

	*responseString = dest.Detach();
	return S_OK ;
}


STDMETHODIMP HttpResponse::get_Bytes ( SAFEARRAY ** ppsa )
{
	if ( ! ppsa ) return E_POINTER ;
	
	std::vector<BYTE> buff ;
	_HR ( ReadToBuffer(buff) ) ;

	SAFEARRAYBOUND rga ;
	rga.lLbound = 0 ;
	rga.cElements = buff.size() ;

	SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
	if ( ! psa ) return E_OUTOFMEMORY ;

	void * data = 0 ;
	SafeArrayAccessData(psa, &data ) ;
	memcpy ( data, buff.begin(), buff.size() ) ;
	SafeArrayUnaccessData(psa) ;

	*ppsa = psa ;
	return S_OK ;
}

STDMETHODIMP HttpResponse::SaveAs ( BSTR filename )
{
	USES_CONVERSION;
	CComPtr<IReadableStream> stm;
	_HR(get_Stream(&stm));

	HANDLE hfile = CreateFile(OLE2CT(filename), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, CREATE_FILE_FLAGS, NULL);
	if( INVALID_HANDLE_VALUE == hfile)
		return HRESULT_FROM_WIN32(GetLastError());

	BYTE buff[PH_BUFFER_SIZE];
	DWORD cbRead =0, cbWrite = 0;
	HRESULT hr;
	do 
	{
		hr = stm->Read(buff, sizeof(buff), &cbRead);
		if (FAILED(hr))
		{
			CloseHandle(hfile);
			return hr;
		}
		if (cbRead > 0)
		{
			if(!WriteFile(hfile, buff, cbRead, &cbWrite, NULL))
			{
				CloseHandle(hfile);
				return HRESULT_FROM_WIN32(GetLastError());
			}
		} 
	} while ( cbRead > 0);
	CloseHandle(hfile);
	return S_OK;
}

STDMETHODIMP HttpResponse::get_StatusCode( short * statusCode ) 
{
	if ( ! statusCode ) return E_POINTER ;
	*statusCode = m_statusCode ;
	return S_OK ;
}


