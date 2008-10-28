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

/////////////////////////////////////////////////////////////////////////////
// Manager.h : Declaration of the CManager
/////////////////////////////////////////////////////////////////////////////

#ifndef __MANAGER_H_
#define __MANAGER_H_

#include "resource.h"       // main symbols
#include "IPackager.h"

/////////////////////////////////////////////////////////////////////////////
// CManager
/////////////////////////////////////////////////////////////////////////////

class ATL_NO_VTABLE CManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CManager, &CLSID_CoAttachmentManager>,
	public ISupportErrorInfo,
	public IsfDelegatingDispImpl<IAttachmentMangerDisp>,
	public ISOAPTransport
{
public:
	CManager() : m_sizeLimit ( 1024 * 1024 ), m_format (formatDime)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MANAGER)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CManager)
	COM_INTERFACE_ENTRY(IAttachmentManger)
	COM_INTERFACE_ENTRY(ISOAPTransport)
	COM_INTERFACE_ENTRY(ISoapAttachmentFormat)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

	HRESULT FinalConstruct() ;
	static void WINAPI ObjectMain(bool /* bStarting */) ;

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:
	STDMETHOD(get_Format)		( /*[out, retval]*/AttachmentFormat *pVal);
	STDMETHOD(put_Format)		( /*[in]*/		   AttachmentFormat newVal);
	STDMETHOD(get_DiskThreshold)( /*[out, retval]*/long *pVal);
	STDMETHOD(put_DiskThreshold)( /*[in]*/		   long newVal);
	STDMETHOD(get_Transport)	( /*[out,retval]*/ ISOAPTransport ** theTransport ) ;
	STDMETHOD(putref_Transport)	( /*[in]*/		   ISOAPTransport *  theTransport ) ;
	STDMETHOD(get_Request)		( /*[out,retval]*/ ISoapAttachments ** collection ) ;
	STDMETHOD(get_Response)		( /*[out,retval]*/ ISoapAttachments ** collection ) ;

// ISOAPTransport
	STDMETHOD(Send)    ( /*[in]*/ BSTR endpoint, /*[in]*/ BSTR Envelope );
	STDMETHOD(Receive) ( /*[in,out]*/ BSTR * characterEncoding, /*[out,retval]*/ SAFEARRAY ** Envelope );

private:
	CComPtr<ISoapAttachments> m_req, m_res;
	CComPtr<ISOAPTransport>   m_trn ;
	long					  m_sizeLimit ;
	AttachmentFormat		  m_format ;

	CComPtr<IPackager> CreatePackager(AttachmentFormat f);
	HRESULT CreateSoapMsgPart  ( BSTR Envelope, ISoapAttachment ** part ) ;

	HRESULT PassThroughReceive ( IStreamReader * resStream, SAFEARRAY ** Envelope ) ;
};

#endif //__MANAGER_H_
