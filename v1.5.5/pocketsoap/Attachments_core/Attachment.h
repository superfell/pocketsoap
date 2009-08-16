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

// Attachment.h : Declaration of the CAttachment

#ifndef __ATTACHMENT_H_
#define __ATTACHMENT_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAttachment
class ATL_NO_VTABLE CAttachment : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAttachment, &CLSID_CoSoapAttachment>,
	public IsfDelegatingDispImpl<ISoapAttachment>,
	public IPartPayloadProvider
{
public:
	CAttachment() : m_tnf(tnfNone), m_format(formatDime)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ATTACHMENT)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAttachment)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISoapAttachment)
	COM_INTERFACE_ENTRY(ISoapAttachmentFormat)
	COM_INTERFACE_ENTRY(IPartPayloadProvider)
END_COM_MAP()

public:
// ISoapAttachment
	STDMETHOD(Initialize)			( /*[in]*/		   VARIANT body, /*[in]*/ TypeNameFormat typeNameFormat, /*[in]*/ BSTR ContentType ) ;
	STDMETHOD(get_Body)				( /*[out,retval]*/ VARIANT * pVal ) ;
	STDMETHOD(put_Body)				( /*[in]*/		   VARIANT body ) ;
	STDMETHOD(get_TypeNameFormat)	( /*[out,retval]*/ TypeNameFormat * tnf ) ;
	STDMETHOD(put_TypeNameFormat)	( /*[in]*/		   TypeNameFormat tnf ) ;
	STDMETHOD(get_TypeName)			( /*[out,retval]*/ BSTR * contentType ) ;
	STDMETHOD(put_TypeName)			( /*[in]*/		   BSTR contentType ) ;
	STDMETHOD(get_ContentId)		( /*[out,retval]*/ BSTR * contentId ) ;
	STDMETHOD(put_ContentId)		( /*[in]*/		   BSTR contentId ) ;
	STDMETHOD(get_Located)			( /*[out,retval]*/ AttachmentLocation * loc ) ;
	STDMETHOD(get_Uri)				( /*[out,retval]*/ BSTR * Uri ) ;
	STDMETHOD(put_Uri)				( /*[in]*/		   BSTR   Uri ) ;
	STDMETHOD(Save)					( /*[out,retval]*/ BSTR fileName ) ;
  	STDMETHOD(get_Format)			( /*[out,retval]*/ AttachmentFormat *pVal);
	STDMETHOD(put_Format)			( /*[in]*/		   AttachmentFormat newVal);

// IPartPayloadProvider
	STDMETHOD(PartPayload)			( /*[out,retval]*/ IPartPayload ** payload ) ;

private:
	CComVariant			m_body ;
	CComBSTR			m_cid, m_type, m_uri ;
	TypeNameFormat		m_tnf ;
	AttachmentFormat	m_format ;

	bool LocatedOnDisk() ;
};

#endif //__ATTACHMENT_H_
