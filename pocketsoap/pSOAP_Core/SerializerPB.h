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
Portions created by Simon Fell are Copyright (C) 2000
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#ifndef __SERIALIZERPB_H_
#define __SERIALIZERPB_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSerializerPB
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSerializerPB : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSerializerPB, &CLSID_CoSerializerPB>,
	public ISoapSerializer,
	public ISoapDeSerializer,
	public ITypesInit,
	public IPropertyBag
{
public:
	CSerializerPB() ;
	~CSerializerPB() ;

DECLARE_REGISTRY_RESOURCEID(IDR_SERIALIZERPB)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSerializerPB)
	COM_INTERFACE_ENTRY(ISoapSerializer)
	COM_INTERFACE_ENTRY(ISoapDeSerializer)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(IPropertyBag)
END_COM_MAP()

// ITypesInit
	STDMETHOD(Initialize)( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) ;

// ISoapSerializer
	STDMETHOD(Serialize)( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;

// ISoapDeSerializer
	STDMETHOD(Start)    ( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;
	STDMETHOD(Child)		( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildReady)	( long id, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildRef)		( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) ;
	STDMETHOD(Ref)			( BSTR id,	 /*[in]*/ ISOAPNode * idNode ) ;
	STDMETHOD(Characters)	( /*[in]*/ BSTR charData ) ;
	STDMETHOD(End)			() ;

// IPropertyBag
	STDMETHOD(Read) ( LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog) ;
	STDMETHOD(Write)( LPCOLESTR pszPropName, VARIANT *pVar) ;

private:
	typedef struct tag_WaitItem
	{
		CComBSTR			id ;
		CComPtr<ISOAPNode>	node ;
	} WaitItem ;
	typedef std::vector<WaitItem> WAITITEMS ;
	WAITITEMS					m_refs ;
	CComPtr<ISOAPNode>			m_node ;
	CComPtr<IUnknown>			m_obj ;
	ISerializerOutput *			m_dest ;
	CComBSTR					m_type, m_typeNS, m_progID ;
};

#endif //__SERIALIZERPB_H_
