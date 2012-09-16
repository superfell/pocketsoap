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

#if !defined(AFX_CHUNKEDTE_H__0661D188_19F1_4644_8134_905107F6620B__INCLUDED_)
#define AFX_CHUNKEDTE_H__0661D188_19F1_4644_8134_905107F6620B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CHTTPTransport ;

// the body is transfered through one or more transferDecoder implementations.
// at the end of the content, this should issue a zero length read against
// the InternalRead method, and return a 0 bytes read indication to the
// caller, both the underlying stream, and the calling code use this as
// an indication the the response stream is ended.
//
// we have 3 impl's at the moment, 
//		1) Content-Length header transfers
//		2) chunked transfer encoding
//		3) http/1.0 transfers that use the connection closed state to signal
//		   the end of the content
//		4) a deflate handler that can be chained up with one of the above
//		   to handle compressed payloads
//

class transferDecoder
{
public:
	virtual ~transferDecoder() {} ;
	virtual HRESULT Read ( BYTE* pvx, DWORD cb, DWORD * pcbRead ) = 0 ;
};

//////////////////////////////////////////////////////////////////////
// contentLengthTE - The response uses a content-length header
//////////////////////////////////////////////////////////////////////
class contentLengthTE : public transferDecoder
{
public:
	contentLengthTE(CHTTPTransport *src, DWORD contentLength);
	virtual ~contentLengthTE();

	virtual HRESULT Read ( BYTE* pvx, DWORD cb, DWORD * pcbRead ) ;

private:
	CHTTPTransport	*m_src ;
	DWORD			m_length ;
};

//////////////////////////////////////////////////////////////////////
// ChunkedTE - The response uses a chunked encoded stream
//////////////////////////////////////////////////////////////////////
class ChunkedTE : public transferDecoder
{
public:
	ChunkedTE(CHTTPTransport * src);
	virtual ~ChunkedTE();

	virtual HRESULT Read ( BYTE* pvx, DWORD cb, DWORD * pcbRead ) ;

private:
	CHTTPTransport * m_src ;

	enum cteState
	{
		expectingChunkSize,
		expectingChunkData,
		expectingChunkDataTerm,
		expectingTrailers,
		alldone
	};

	cteState	m_state ;
	DWORD		m_remainingInChunk ;

	HRESULT getChunkSize(BYTE* &pvx, DWORD &cb, DWORD * pcbRead) ;
};

//////////////////////////////////////////////////////////////////////
// connectionWillClose - HTTP/1.0 support, the response is complete
// when the server closes the connection
//////////////////////////////////////////////////////////////////////
class connectionWillClose : public transferDecoder
{
public:
	connectionWillClose(CHTTPTransport *src);
	virtual ~connectionWillClose() ;

	virtual HRESULT Read ( BYTE* pvx, DWORD cb, DWORD * pcbRead ) ;

private:
	CHTTPTransport * m_src ;
};
#endif // !defined(AFX_CHUNKEDTE_H__0661D188_19F1_4644_8134_905107F6620B__INCLUDED_)
