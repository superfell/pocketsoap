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

// AttachmentsCollection.h : Declaration of the CAttachmentsCollection

#ifndef __ATTACHMENTSCOLLECTION_H_
#define __ATTACHMENTSCOLLECTION_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAttachmentsCollection
class ATL_NO_VTABLE CAttachmentsCollection : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IsfDelegatingDispImpl<ISoapAttachments>
{
public:
	CAttachmentsCollection() : m_format(formatDime)
	{
	}

DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAttachmentsCollection)
	COM_INTERFACE_ENTRY(ISoapAttachments)
	COM_INTERFACE_ENTRY(ISoapAttachmentFormat)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
// IAttachmentsCollection
	STDMETHOD(get__NewEnum)	( /*[out, retval]*/ IUnknown **pVal);
	STDMETHOD(get_Count)	( /*[out,retval]*/ long * Count ) ;
	STDMETHOD(get_Item)		( /*[in]*/ long idx,  /*[out,retval]*/ ISoapAttachment ** attachment ) ;
	STDMETHOD(Find)         ( /*[in]*/ BSTR uri,  /*[out,retval]*/ ISoapAttachment ** attachment ) ;
	STDMETHOD(Append)		( /*[in]*/ ISoapAttachment * newNode);
	STDMETHOD(Clear)		();
	STDMETHOD(Create)		( /*[in]*/ VARIANT src,
							  /*[in]*/ TypeNameFormat tnf,	
							  /*[in]*/ BSTR contentType,
							  /*[out,retval]*/ ISoapAttachment ** attachment ) ;
  	STDMETHOD(get_Format)	( /*[out, retval]*/AttachmentFormat *pVal);
	STDMETHOD(put_Format)	( /*[in]*/		   AttachmentFormat newVal);


private:
	typedef std::vector<CAdapt<CComPtr<ISoapAttachment> > > COL ;

	COL	m_col ;
	AttachmentFormat m_format ;
};

#endif //__ATTACHMENTSCOLLECTION_H_
