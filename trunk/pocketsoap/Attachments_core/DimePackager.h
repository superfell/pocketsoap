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
// DimePackager.h: interface for the DimePackager class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIMEPACKAGER_H__1E12E9E3_5CE3_4B26_978C_3DB27F970940__INCLUDED_)
#define AFX_DIMEPACKAGER_H__1E12E9E3_5CE3_4B26_978C_3DB27F970940__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dime.h"
#include "IPackager.h"
#include "PackagerStreamerBase.h"

class ATL_NO_VTABLE DimePackager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IPackager,
	public PackagerStreamerImpl<DimePackager>
{
public:
	DimePackager();
	~DimePackager();

BEGIN_COM_MAP(DimePackager)
	COM_INTERFACE_ENTRY(IStreamReader)
END_COM_MAP()

	virtual HRESULT Init( long sizeLimit ) ;

	virtual HRESULT PackageAndSend(	BSTR				endpoint, 
									ISoapAttachment	*	envelope, 
									ISoapAttachments *	parts, 
									ISOAPTransport *	transport ) ;

	virtual HRESULT Receive  (	/*[in]*/			IStreamReader *		responseStream, 
								/*[in]*/			BSTR				contentType, 
								/*[in]*/			ISoapAttachments *	parts,
								/*[in,out]*/		BSTR *				characterEncoding, 
								/*[out,retval]*/	SAFEARRAY **		Envelope  ) ;

	// PackagerStreamerImpl callbacks
	BUFFER CreateCurrentHeader() ;
	bool AddNextChunk     ( BYTE * &pv, DWORD &size )  ; 

private:
	DWORD m_sizeLimit ;
};

#endif // !defined(AFX_DIMEPACKAGER_H__1E12E9E3_5CE3_4B26_978C_3DB27F970940__INCLUDED_)
