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
Portions created by Simon Fell are Copyright (C) 2002-2006
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "PocketHTTP.h"
#include "compressionHandlers.h"

#pragma comment(lib, "zlib_static.lib") 

// gzip magic numbers see RFC1952
// http://www.faqs.org/rfcs/rfc1952.html
static const BYTE gzip_id1			= 0x1f ;
static const BYTE gzip_id2			= 0x8b ;
static const BYTE gzip_cm_deflate	= 0x08 ;

// bit masks for the flags byte
static const BYTE gzip_FTEXT		= 0x01 ;
static const BYTE gzip_FHCRC		= 0x02 ;
static const BYTE gzip_FEXTRA		= 0x04 ;
static const BYTE gzip_FNAME		= 0x08 ;
static const BYTE gzip_FCOMMENT		= 0x10 ;

// the gzip header is at least this size.
static const BYTE gzip_MIN_HDR_SIZE	= 0x0A ;

//////////////////////////////////////////////////////////////////////
// inflateHandler
//////////////////////////////////////////////////////////////////////
inflateHandler::inflateHandler(std::auto_ptr<transferDecoder> &src) :	
	m_src(src), 
	m_initDone(false), 
	m_eof(false)
{
	memset(&m_stream , 0, sizeof(m_stream));
}

inflateHandler::~inflateHandler()
{
	if ( m_initDone )
		inflateEnd(&m_stream) ;
}

// make sure the buffer contains at least this many bytes
HRESULT inflateHandler::EnsureBufferContains(DWORD cb)
{
	while(m_buff.size() < cb)
		_HR ( ReadInput(cb) ) ;
	return S_OK ;
}

HRESULT inflateHandler::ReadInput(DWORD cb)
{
	HRESULT hr = S_OK ;
	// read something from the source, if its still active
	if ( m_src.get() )
	{
		// if the caller is using large buffers, we will too[ish]
		DWORD cbToRead = max ( 4096, cb / 2 ) ;
		DWORD cbSrc ;
		DWORD oldSize = m_buff.size() ;
		m_buff.resize(oldSize + cbToRead) ;
		hr = m_src->Read ( &*m_buff.begin() + oldSize, cbToRead, &cbSrc ) ;
		m_buff.resize(oldSize + cbSrc) ;
		if ( ! cbSrc )
			m_src.reset(0) ;
	}
	return hr;
}

HRESULT inflateHandler::Read ( BYTE *pvx, DWORD cb, DWORD * pcbRead )
{
	_HR ( ReadInput(cb) ) ;

	// if we're wrapped up
	if ( m_eof )
	{
		*pcbRead = 0 ;
		return S_OK ;
	}

	// this is complicated by the fact that its not clear in the http spec
	// if the deflate scheme should include the 2 byte zlib stream header or
	// not, there seems to be a split as to which servers generate it and
	// which don't, so we end up having to try both ways, arrrgghhhh
	int zlibErr ;
	bool tryWithHeaderNext = false ;

	if ( ! m_initDone )
	{
		_HR ( PreInit() ) ;
		// make sure we have some input if the PreInit call used up the
		// whole buffer contents
		if ( m_buff.size() == 0 )
			_HR ( ReadInput(cb) ) ;
	}
	
	do
	{
		// make sure the stream buffer states are correct.
		m_stream.next_in   = &m_buff[0] ;
		m_stream.avail_in  = m_buff.size() ;
		m_stream.next_out  = pvx ;
		m_stream.avail_out = cb ;

		// init the stream first time around
		if ( ! m_initDone )
		{
			m_stream.zalloc = (alloc_func)0;
			m_stream.zfree  = (free_func)0;
			if(!tryWithHeaderNext)
			{
				zlibErr = inflateInit2(&m_stream, -MAX_WBITS);
				tryWithHeaderNext = true ;
			}
			else
			{
				zlibErr = inflateInit(&m_stream);
				tryWithHeaderNext = false ;
			}
			if ( Z_OK != zlibErr )
				return AtlReportError(CLSID_CoPocketHTTP, OLESTR("There was an error trying to uncompress the response from the server"), IID_NULL, E_INFLATE_FAILURE ) ;
			m_initDone = true ;
		}

		// inflate whatever we've got
		zlibErr = inflate ( &m_stream, Z_NO_FLUSH ) ;
		// err ?
		if (( Z_OK != zlibErr ) && ( Z_STREAM_END != zlibErr ))
		{
			if(!tryWithHeaderNext)
			{
				// if we're doing inflate without the zlib header (i.e. gzip and some
				// deflate responses), we can get a Z_BUF_ERROR when we try and do
				// an avail_in == 0, this is not a problem, see zlib/contrib/minizip/unzip.c
				// and http://gcc.gnu.org/ml/java-patches/2000-q4/msg00263.html
				if  ( Z_BUF_ERROR == zlibErr && m_stream.avail_in == 0)
				{
					BYTE dummy = 0 ;
					m_stream.next_in = &dummy ;
					m_stream.avail_in = 1 ;
					int rc = inflate ( &m_stream, Z_FINISH ) ;
					if ( rc == Z_STREAM_END )
						zlibErr = rc ;
				}
				if( Z_STREAM_END != zlibErr )
				return AtlReportError(CLSID_CoPocketHTTP, OLESTR("There was an error trying to uncompress the response from the server"), IID_NULL, E_INFLATE_FAILURE ) ;
			}
			inflateEnd(&m_stream) ;
			m_initDone = false ;
		}
		else
			break ;
	} while(true) ;

	// zap the input that was process
	m_buff.erase(m_buff.begin(), m_stream.next_in) ;

	// all done ?
	if ( Z_STREAM_END == zlibErr )
		m_eof = true ;
	
	// update the output count
	*pcbRead = cb - m_stream.avail_out ;
	return S_OK ;
}

HRESULT inflateHandler::PreInit()
{
	return S_OK ;
}

//////////////////////////////////////////////////////////////////////
// gzipInflateHandler  
//
// gzip is basically the same as deflate except it has a header
// structure before the compressed data which we need to decode
// and read over first, once we're over that we can treat it as
// a normal deflate stream. Once we get to the end of the compressed
// data there's an 8 byte trailer that contains a crc and length
// info so we can double check the deflated data.
//////////////////////////////////////////////////////////////////////
gzipInflateHandler::gzipInflateHandler(std::auto_ptr<transferDecoder> &src) 
	: inflateHandler(src)
{
}

gzipInflateHandler::~gzipInflateHandler()
{
}

HRESULT gzipInflateHandler::PreInit()
{
	// walk over the gzip file header to get to the compressed data
	_HR (EnsureBufferContains(gzip_MIN_HDR_SIZE)) ;

	if ( m_buff[0] != gzip_id1 || m_buff[1] != gzip_id2 || m_buff[2] != gzip_cm_deflate )
		return AtlReportError (__uuidof(CoPocketHTTP), OLESTR("GZIP File header is invalid"), IID_IHttpRequest, E_GZIP_HEADER_INVALID ) ;

	BYTE flags = m_buff[3] ;
	bool fCrc		= (flags & gzip_FHCRC) > 0;
	bool fExtra		= (flags & gzip_FEXTRA) > 0;
	bool fName		= (flags & gzip_FNAME) > 0;
	bool fComment	= (flags & gzip_FCOMMENT) > 0;

	// move over the fixed part of the gzip header
	int offset = gzip_MIN_HDR_SIZE ;

	if ( fExtra )
	{
		// skip over the extra stuff
		_HR(EnsureBufferContains(offset+2)) ;
		unsigned short cb = *((unsigned short *)&m_buff[offset]) ;
		offset += 2 ;	// the length of the extra data
		offset += cb ;	// the extra data
		_HR(EnsureBufferContains(offset)) ;
	}
	if ( fName )
	{
		// skip over the name field
		while(m_buff[offset++] != 0)
			_HR(EnsureBufferContains(offset)) ;
	}
	if ( fComment )
	{
		// skip over the comment
		while(m_buff[offset++] != 0)
			_HR(EnsureBufferContains(offset)) ;
	}
	if ( fCrc )
	{
		// skip over the crc
		offset += 2 ;
		_HR(EnsureBufferContains(offset)) ;
	} 

	// remove everything we've read
	m_buff.erase(m_buff.begin(), m_buff.begin() + offset) ;

	// reset the length and crc rolling values
	m_crc32     = crc32(0,NULL,0);
	m_totalSize = 0 ;

	return S_OK ;
}

HRESULT gzipInflateHandler::Read(BYTE * pvx, DWORD cb, DWORD *pcbRead)
{
	bool eofAtStart = m_eof;
	_HR ( inflateHandler::Read ( pvx, cb, pcbRead )) ;
	if ( *pcbRead )
	{
		m_crc32 = crc32 ( m_crc32, pvx, *pcbRead ) ;
		m_totalSize += *pcbRead ;
	}
	if ((!eofAtStart) && m_eof)
		return EndOfInflation();
	return S_OK ;
}

HRESULT gzipInflateHandler::EndOfInflation()
{
	// need to read over the CRC-32 (4 bytes) and size data (4 bytes)
	_HR(EnsureBufferContains(8)) ;
	DWORD len = 0, crc = 0 ;
	crc |= m_buff[0] ;	
	crc |= m_buff[1] << 8;	
	crc |= m_buff[2] << 16;	
	crc |= m_buff[3] << 24;
	len |= m_buff[4] ;	
	len |= m_buff[5] << 8;	
	len |= m_buff[6] << 16;	
	len |= m_buff[7] << 24;
	ATLTRACE(_T("calced crc %08lx expected %08lx \t\tuncompressed length %08lx expected %08lx"), m_crc32, crc, m_totalSize, len);
	m_buff.erase(m_buff.begin(), m_buff.begin() + 8);

	// check crc here
	if ( crc != m_crc32 )
	{
		wchar_t buff[100] ;
		swprintf(buff, L"Invalid gzip crc value, expecting 0x%08lx, found 0x%08lx", crc, m_crc32 ) ;
		return AtlReportError(CLSID_CoPocketHTTP, buff, IID_IHttpRequest, E_GZIP_CRC_ERROR ) ;
	}
	// check length here
	if ( len != m_totalSize )
	{
		wchar_t buff[100] ;
		swprintf(buff, L"Invalid gzip length value, expecting 0x%08lx, found 0x%08lx", len, m_totalSize ) ;
		return AtlReportError(CLSID_CoPocketHTTP, buff, IID_IHttpRequest, E_GZIP_SIZE_ERROR ) ;
	}
	return S_OK ;
}

//////////////////////////////////////////////////////////////////////
// deflateHandler
//////////////////////////////////////////////////////////////////////
deflateHandler::deflateHandler() 
{
}

deflateHandler::~deflateHandler()
{
	if ( m_initDone ) 
		deflateEnd(&m_stream) ;
}

void deflateHandler::Init ( IResetableStream * src, short compLvl )
{
	m_src		= src ;
	m_initDone  = false ;
	m_eof	    = false ;
	m_srcDone   = false ;
	m_compLevel = compLvl ;
}

STDMETHODIMP deflateHandler::Read(void *pv,	ULONG cb, ULONG *pcbRead )
{
	// read something from the source, if its still active
	if ( !m_srcDone )
	{
		DWORD cbToRead = PH_BUFFER_SIZE / 2 ;
		DWORD cbSrc ;
		DWORD oldSize = m_buff.size() ;
		m_buff.resize(oldSize + cbToRead) ;
		HRESULT hr = m_src->Read ( &*m_buff.begin() + oldSize, cbToRead, &cbSrc ) ;
		m_buff.resize(oldSize + cbSrc) ;
		if(cbSrc)
			processSource(&*m_buff.begin() + oldSize, cbSrc) ;
		if ( ! cbSrc )
			m_srcDone = true;
	}

	// if we're wrapped up
	if ( m_eof )
	{
		*pcbRead = 0 ;
		return S_OK ;
	}

	// make sure the stream buffer states are correct.
	m_stream.next_in   = &*m_buff.begin() ;
	m_stream.avail_in  = m_buff.size() ;
	m_stream.next_out  = (BYTE *)pv ;
	m_stream.avail_out = cb ;

	// init the stream first time around
	if ( ! m_initDone )
	{
	    m_stream.zalloc = (alloc_func)0;
	    m_stream.zfree  = (free_func)0;
	    m_stream.opaque = (voidpf)0;

		int zlibErr = DeflateInit() ;
		if ( Z_OK != zlibErr )
			return AtlReportError(CLSID_CoPocketHTTP, OLESTR("There was a problem trying to compress the request data"), IID_NULL, E_DEFLATE_FAILURE ) ;
		m_initDone = true ;
	}

	// deflate whatever we've got
	int zlibErr = deflate ( &m_stream, m_srcDone ? Z_FINISH : Z_NO_FLUSH ) ;
	// err ?
	if (( Z_OK != zlibErr ) && ( Z_STREAM_END != zlibErr ))
			return AtlReportError(CLSID_CoPocketHTTP, OLESTR("There was a problem trying to compress the request data"), IID_NULL, E_DEFLATE_FAILURE ) ;

	// all done ?
	if ( m_srcDone && (Z_STREAM_END == zlibErr ))
	{
		generateTrailer() ;
		m_eof = true ;
	}

	// zap the input that was process
	m_buff.erase(m_buff.begin(), m_stream.next_in) ;
	// update the output count
	*pcbRead = cb - m_stream.avail_out ;

	// if there's more togo, and we've got a reasonable amount of space
	// left in the dest buffer, have another loop around and top it up
	if ( (!m_eof) && ((*pcbRead == 0) || (cb - *pcbRead) >= 512 ))
	{
		DWORD pcbNested = 0 ;
		HRESULT hr =Read ( (BYTE *)pv + *pcbRead, cb - *pcbRead, &pcbNested ) ;
		(*pcbRead) += pcbNested ;
		return hr ;
	}

	return S_OK ;
}

void deflateHandler::processSource(BYTE * pv, DWORD cb)
{
	// nothing todo
}

int deflateHandler::DeflateInit()
{
	return deflateInit(&m_stream, m_compLevel) ;
}

STDMETHODIMP deflateHandler::Reset()
{
	m_buff.clear() ;
	if ( m_initDone ) 
		deflateEnd(&m_stream) ;
	m_initDone = false ;
	m_eof = false ;
	m_srcDone = false ;
	return m_src->Reset();
}

// deflate has no trailer
HRESULT deflateHandler::generateTrailer()
{
	return S_OK ;
}

//////////////////////////////////////////////////////////////////////
// gzipDeflateHandler  
//////////////////////////////////////////////////////////////////////
void gzipDeflateHandler::Init(IResetableStream *src, short compressionLevel)
{
	deflateHandler::Init(src, compressionLevel) ;
	Init() ;
}

STDMETHODIMP gzipDeflateHandler::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	if(!m_initDone)
		_HR ( generateHeader() ) ;

	if(m_cOut>0)
	{
		ULONG c = min ( m_cOut, cb ) ;
		memcpy ( pv, m_outBuff, c ) ;
		m_cOut -= (BYTE)c ;
		*pcbRead = c ;
		BYTE * pvx = (BYTE *)pv ;
		pvx += c ;
		pv = pvx;
	}
	else
		*pcbRead = 0 ;
	ULONG inCB = 0 ;
	_HR ( deflateHandler::Read(pv, cb, &inCB) ) ;
	*pcbRead += inCB ;
	return S_OK ;
}

STDMETHODIMP gzipDeflateHandler::Reset()
{
	_HR ( deflateHandler::Reset() );
	Init();
	return S_OK ;
}

void gzipDeflateHandler::Init()
{
	m_cOut		= 0 ;
	m_crc32		= 0 ;
	m_totalSize = 0 ;
}

HRESULT gzipDeflateHandler::generateHeader() 
{
	m_outBuff[0] = gzip_id1 ;
	m_outBuff[1] = gzip_id2 ;
	m_outBuff[2] = gzip_cm_deflate ;
	m_outBuff[3] = 0 ;		// flags
	m_outBuff[4] = 0 ;		// mtime
	m_outBuff[5] = 0 ;		// mtime
	m_outBuff[6] = 0 ;		// mtime
	m_outBuff[7] = 0 ;		// mtime
	m_outBuff[8] = 0 ;		// xfl
	m_outBuff[9] = 255 ;	// os
	m_cOut		 = 0x0A ;
	return S_OK ;
}

void gzipDeflateHandler::processSource(BYTE * pv, DWORD cb)
{
	m_crc32 = crc32 ( m_crc32, pv, cb ) ;
	m_totalSize += cb ;
}

int gzipDeflateHandler::DeflateInit()
{
	static const int DEF_MEM_LEVEL = 8 ;
	return deflateInit2(&m_stream, m_compLevel, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY) ;
}

HRESULT gzipDeflateHandler::generateTrailer()
{
	// write out the crc & size trailer
	m_outBuff[0] = (BYTE)(m_crc32       & 0xFF) ;
	m_outBuff[1] = (BYTE)(m_crc32 >> 8  & 0xFF) ;
	m_outBuff[2] = (BYTE)(m_crc32 >> 16 & 0xFF) ;
	m_outBuff[3] = (BYTE)(m_crc32 >> 24 & 0xFF) ;

	m_outBuff[4] = (BYTE)(m_totalSize       & 0xFF) ;
	m_outBuff[5] = (BYTE)(m_totalSize >> 8  & 0xFF) ;
	m_outBuff[6] = (BYTE)(m_totalSize >> 16 & 0xFF) ;
	m_outBuff[7] = (BYTE)(m_totalSize >> 24 & 0xFF) ;

	m_cOut = 8 ;
	return S_OK ;
}
