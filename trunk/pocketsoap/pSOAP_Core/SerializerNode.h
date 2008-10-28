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
Portions created by Simon Fell are Copyright (C) 2000-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#ifndef __NODESERIALIZER_H_
#define __NODESERIALIZER_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSerializerNode
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSerializerNode : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSerializerNode, &CLSID_CoSerializerNode>,
	public ISupportErrorInfo,
	public ISoapSerializer,
	public ISoapDeSerializer,
	public ITypesInit,
	public ISoapDeSerializerDefaultHandler
{
public:
	CSerializerNode() ;
	~CSerializerNode() ;

DECLARE_REGISTRY_RESOURCEID(IDR_NODESERIALIZER)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSerializerNode)
	COM_INTERFACE_ENTRY(ISoapSerializer)
	COM_INTERFACE_ENTRY(ISoapDeSerializer)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(ISoapDeSerializerDefaultHandler)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// ITypesInit
	STDMETHOD(Initialize)( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) ;

// ISoapSerializer
	STDMETHOD(Serialize)( /*[in]*/ VARIANT * val, ISerializerContext * ctx, ISerializerOutput * dest ) ;

// ISoapDeSerializer
	STDMETHOD(Start)		( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;
	STDMETHOD(Child)		( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildReady)	( long id, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildRef)		( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) ;
	STDMETHOD(Ref)			( BSTR id,	 /*[in]*/ ISOAPNode * idNode ) ;
	STDMETHOD(Characters)	( /*[in]*/ BSTR charData ) ;
	STDMETHOD(End)			() ;

private:
	typedef struct tag_WaitItem
	{
		CComBSTR			id ;
		CComPtr<ISOAPNode>	node ;
	} WaitItem ;
	typedef std::vector<WaitItem> WAITITEMS ;
	WAITITEMS					m_refs ;
	CComPtr<ISOAPNode>			m_node ;
	CComBSTR					m_name ;
};

#endif //__NODESERIALIZER_H_
