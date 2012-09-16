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

#if !defined(AFX_COMPRESSIONHANDLERS_H__22825948_A358_42CA_AF5C_F6096DDCF744__INCLUDED_)
#define AFX_COMPRESSIONHANDLERS_H__22825948_A358_42CA_AF5C_F6096DDCF744__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "chunkedTE.h"
#include "zlib.h"

//////////////////////////////////////////////////////////////////////
// inflate Handlers, these uncompress a stream as its being read
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// inflateHandler
//////////////////////////////////////////////////////////////////////
class inflateHandler : public transferDecoder
{
public:
	inflateHandler(std::auto_ptr<transferDecoder> &src) ;
	virtual ~inflateHandler() ;

	virtual HRESULT Read ( BYTE *pvx, DWORD cb, DWORD * pcbRead ) ;

protected:
	virtual HRESULT PreInit() ;

	HRESULT ReadInput(DWORD cb) ;
	HRESULT EnsureBufferContains(DWORD cb) ;

	std::auto_ptr<transferDecoder>	m_src ;
	std::vector<BYTE>				m_buff ;
	bool							m_initDone ;
	bool							m_eof ;
	z_stream						m_stream ;
} ;

//////////////////////////////////////////////////////////////////////
// gzipInflateHandler  
//////////////////////////////////////////////////////////////////////
class gzipInflateHandler : public inflateHandler
{
public:
	gzipInflateHandler(std::auto_ptr<transferDecoder> &src) ;
	virtual ~gzipInflateHandler() ;

	virtual HRESULT Read ( BYTE *pvx, DWORD cb, DWORD * pcbRead ) ;

protected:
	virtual HRESULT PreInit() ;

private:
	HRESULT EndOfInflation();

	ULONG m_crc32, m_totalSize ;
};

//////////////////////////////////////////////////////////////////////
// deflate handlers, these compress a stream as its being read
//////////////////////////////////////////////////////////////////////
class deflateHandlerInit : public IUnknown 
{
public:
	virtual void Init ( IResetableStream * src, short compressionLevel ) = 0 ;
} ;

//////////////////////////////////////////////////////////////////////
// deflateHandler  
//////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE deflateHandler :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IResetableStream,
	public deflateHandlerInit
{
public:
	deflateHandler();
	virtual ~deflateHandler();

BEGIN_COM_MAP(deflateHandler)
	COM_INTERFACE_ENTRY(IResetableStream)
END_COM_MAP()

	virtual void Init ( IResetableStream * src, short compressionLevel ) ;

// IStreamReader
	STDMETHOD(Read)(void *pv,			// Pointer to buffer into which the stream is read
					ULONG cb,			// Specifies the number of bytes to read
					ULONG *pcbRead );	// Pointer to location that contains actual
										// number of bytes read
	STDMETHOD(Reset)() ;

protected:
	virtual int		DeflateInit() ;
	virtual void    processSource(BYTE * pv, DWORD cb) ;
	virtual HRESULT generateTrailer() ;

	CComPtr<IResetableStream>	m_src ;
	std::vector<BYTE>			m_buff ;
	bool						m_initDone ;
	bool						m_srcDone ;
	bool						m_eof ;
	short						m_compLevel ;
	z_stream					m_stream ;
	
};

//////////////////////////////////////////////////////////////////////
// gzipDeflateHandler  
//////////////////////////////////////////////////////////////////////
class gzipDeflateHandler: public deflateHandler
{
public:
	virtual void Init ( IResetableStream * src, short compressionLevel ) ;

	STDMETHOD(Read)(void *pv,			// Pointer to buffer into which the stream is read
					ULONG cb,			// Specifies the number of bytes to read
					ULONG *pcbRead );	// Pointer to location that contains actual
										// number of bytes read
	STDMETHOD(Reset)() ;

protected:
	virtual int		DeflateInit() ;
	virtual void	processSource(BYTE * pv, DWORD cb) ;
	virtual HRESULT generateTrailer() ;

	void    Init();
	HRESULT generateHeader() ;

	BYTE	m_outBuff[16] ;
	BYTE	m_cOut ;

	ULONG	m_crc32, m_totalSize ;
};

#endif // !defined(AFX_COMPRESSIONHANDLERS_H__22825948_A358_42CA_AF5C_F6096DDCF744__INCLUDED_)
