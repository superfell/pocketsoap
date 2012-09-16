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

#include "stdafx.h"
#include "pocketHTTP.h"
#include "ChunkedTE.h"
#include "HTTPTransport.h"

//////////////////////////////////////////////////////////////////////
// contentLengthTE
//////////////////////////////////////////////////////////////////////
contentLengthTE::contentLengthTE(CHTTPTransport *src, DWORD contentLength) : m_src(src), m_length(contentLength)
{
}

contentLengthTE::~contentLengthTE()
{
}

HRESULT contentLengthTE::Read(BYTE *pvx, DWORD cb, DWORD *pcbRead)
{
	DWORD cbr ;
	_HR(m_src->InternalRead(pvx, min(m_length, cb), &cbr) ) ;
	m_length -= cbr ;
	*pcbRead = cbr ;
	// issue a 0 length read to tidy up the underlying stream
	if ( m_length == 0 )
		m_src->InternalRead(NULL, 0, &cbr) ;

	return S_OK ;
}

//////////////////////////////////////////////////////////////////////
// ChunkedTE
//////////////////////////////////////////////////////////////////////
ChunkedTE::ChunkedTE(CHTTPTransport * src) : m_src(src), m_state(expectingChunkSize), m_remainingInChunk(0)
{
}

ChunkedTE::~ChunkedTE()
{
}

HRESULT ChunkedTE::Read ( BYTE* pvx, DWORD cb, DWORD * pcbRead )
{
	*pcbRead =0 ;
	if ( expectingChunkSize == m_state )
		_HR(getChunkSize(pvx, cb, pcbRead)) ;

	DWORD cbr = 0 ;
	if ( expectingChunkData == m_state )
	{
		_HR(m_src->InternalRead ( pvx, min(m_remainingInChunk, cb), &cbr )) ;
		*pcbRead += cbr ;
		m_remainingInChunk -= cbr ;
		if ( m_remainingInChunk == 0 )
			m_state = expectingChunkDataTerm ;
	}
	if ( expectingChunkDataTerm == m_state )
	{
		// after the data, there's a CRLF before the next chunk header
		// remember to read this. a smarter version would read it as
		// part of the expectingChunkData step, then throw it away
		char dataTermbuff[2] ;
		DWORD done =0 ;
		while ( done != 2 )
		{
			_HR(m_src->InternalRead ( dataTermbuff, 2 -done, &cbr )) ;
			done += cbr ;
		}
		m_state = expectingChunkSize ;
	}
	if ( expectingTrailers == m_state )
	{
		// for the last chunk there are optional trailers, we need to eat those as well
		static const size_t buff_size = 128 ;
		char buff[buff_size] ;

		static const char * term = "\r\n\r\n" ;
		static const size_t sizeofTerm = 4 ;
		static const char * termEnd = term + sizeofTerm ;
		char * bstart = buff ;
		char * bend = 0 ;
		char * t = 0 ;
		do
		{
			_HR ( m_src->InternalRead ( bstart, buff_size - (bstart-buff), &cbr ) ) ;
			bend = bstart + cbr ;
			t = std::search ( buff, bend, term, termEnd ) ;
			if ( t == bend )
			{
				if ( bend-buff > buff_size/2 )
				{
					memmove ( buff, bend - sizeofTerm, sizeofTerm ) ;
					bstart = buff + sizeofTerm ;
				}
				else
					bstart = bend ;
			}
			else
				break ;
		} while ( true ) ;
		// hoist anything extra back
		if ( bend > t + sizeofTerm )
			m_src->BufferForNextRead ( (BYTE *)(t + sizeofTerm), bend - ( t + sizeofTerm ) ) ;

		// issue zero length read on underlying stream, so that its tidied up
		m_src->InternalRead ( NULL, 0, &cbr ) ;
		m_state = alldone ;
	}
	return S_OK ;
}

HRESULT ChunkedTE::getChunkSize(BYTE* &pvx, DWORD &cb, DWORD * pcbRead)
{
	static const size_t buff_size = 80 ;
	BYTE buff[buff_size] ;

	BYTE * CRLF = (BYTE*)"\r\n" ;
	BYTE * CRLF_end = CRLF + 2 ;

	BYTE * pos = buff ;
	DWORD cbr = 0 ;
	DWORD used = 0 ;
	do
	{
		_HR(m_src->InternalRead ( pos, buff_size - used, &cbr )) ;
		used += cbr ;
		pos += cbr ;
	} while ( (std::search ( buff, buff + used, CRLF, CRLF_end ) == buff + used) && ( used < buff_size ) ) ;
	
	m_remainingInChunk = strtoul((char *)buff, (char **)&pos, 16) ;
	
	// skip anything after the chunk size, upto the CRLF
	pos = std::search ( pos, buff + used, CRLF, CRLF_end ) ;
	if ( pos == ( buff + used ))
		return AtlReportError ( CLSID_CoPocketHTTP, L"chunked transfer header, didn't get expected CRLF", IID_NULL, E_UNEXPECTED ) ;

	if ( 0 == m_remainingInChunk )
		m_state = expectingTrailers ;
	else
	{
		pos += 2 ;	// skip the CRLF
		m_state = expectingChunkData ;	
	}

	// stuff anything extra we got, back in the read queue
	// this could probably be optimized somewhat, we'll see what
	// truetime makes of it later.
	size_t extra = used - ( pos - buff ) ;
	if ( extra > 0 )
		m_src->BufferForNextRead ( (BYTE *)pos, extra ) ;

	return S_OK ;
}

//////////////////////////////////////////////////////////////////////
// connectionWillClose
//////////////////////////////////////////////////////////////////////
connectionWillClose::connectionWillClose(CHTTPTransport *src) : m_src(src)
{
}

connectionWillClose::~connectionWillClose()
{
}

HRESULT connectionWillClose::Read(BYTE *pvx, DWORD cb, DWORD *pcbRead)
{
	HRESULT hr = m_src->InternalRead ( pvx, cb, pcbRead ) ;
	return hr ;
}

