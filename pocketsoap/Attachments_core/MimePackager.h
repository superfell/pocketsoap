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

// MimePackager.h: interface for the MimePackager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MIMEPACKAGER_H__885B1EDE_18C7_441E_9849_1FB53E52E1A7__INCLUDED_)
#define AFX_MIMEPACKAGER_H__885B1EDE_18C7_441E_9849_1FB53E52E1A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PackagerStreamerBase.h"
#include "IPackager.h"

class NoCaseCompare
{
public:
	inline bool operator() (const std::string &a, const std::string &b) const 
	{
		return  _stricmp ( a.c_str(), b.c_str() ) < 0  ;
	}
} ;

class ATL_NO_VTABLE MimePackager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IPackager,
	public PackagerStreamerImpl<MimePackager>
{
public:
	MimePackager();
	virtual ~MimePackager();

BEGIN_COM_MAP(MimePackager)
	COM_INTERFACE_ENTRY(IStreamReader)
END_COM_MAP()

// IPackager
	virtual HRESULT Init ( long sizeLimit ) ;
	virtual HRESULT PackageAndSend ( BSTR endpoint, ISoapAttachment * envelope, ISoapAttachments * parts, ISOAPTransport * transport ) ;
	virtual HRESULT Receive        ( IStreamReader * transport, BSTR ContentType, ISoapAttachments * parts, /*[in,out]*/ BSTR * characterEncoding, /*[out,retval]*/ SAFEARRAY ** Envelope  ) ;

// PackagerStreamerImpl callbacks
	BUFFER CreateCurrentHeader() ;
	bool AddNextChunk     ( BYTE * &pv, DWORD &size )  ; 

private:
	long					m_sizeLimit ;
	stringBuff_A			m_boundary ;

	stringBuff_A CreateBoundary() ;
	bool validBoundary  ( stringBuff_A &boundary, ISoapAttachment * envelope, ISoapAttachments * parts ) ;
	bool boundaryInPart ( stringBuff_A &boundary, ISoapAttachment * evelope ) ;

	typedef std::map<std::string, std::string, NoCaseCompare> STR_MAP ;

	STR_MAP UnpackContentType	 ( BSTR contentType ) ;
	HRESULT ParseMimePartHeader  ( char * hdr, std::string &cid, std::string &ct ) ;
	HRESULT ReadToBuffer		 ( char * buff_start, DWORD buff_size, char *&start, char *&end, DWORD &buffspace, IStreamReader * stream ) ;

};

#endif // !defined(AFX_MIMEPACKAGER_H__885B1EDE_18C7_441E_9849_1FB53E52E1A7__INCLUDED_)
