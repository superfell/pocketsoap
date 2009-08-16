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

#ifndef __SERIALIZERHEXBIN_H_
#define __SERIALIZERHEXBIN_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSerializerHexBin a serializer class for xsd:hexBinary
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSerializerHexBin : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSerializerHexBin, &CLSID_CoSerializerHexBin>,
	public ISoapSerializer,
	public ISoapDeSerializer,
	public ITypesInit,
	public ISimpleSoapSerializer,
	public ISimpleSoapDeSerializer
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_SERIALIZERHEXBIN)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSerializerHexBin)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(ISoapDeSerializer)
	COM_INTERFACE_ENTRY(ISoapSerializer)
	COM_INTERFACE_ENTRY(ISimpleSoapSerializer)
	COM_INTERFACE_ENTRY(ISimpleSoapDeSerializer)
END_COM_MAP()

// ITypesInit
	STDMETHOD(Initialize)( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) ;

// ISoapSerializer
	STDMETHOD(Serialize)( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;

// ISimpleSoapSerializer
	STDMETHOD(Serialize)( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ BSTR * dest ) ;

// ISimpleSoapDeSerializer
	STDMETHOD(Deserialize) ( BSTR characters, ISOAPNamespaces * ns, VARIANT * dest ) ;

// ISoapDeSerializer
	STDMETHOD(Start)    ( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;
	STDMETHOD(Child)		( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildReady)	( long id, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildRef)		( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) ;
	STDMETHOD(Ref)			( BSTR id,	 /*[in]*/ ISOAPNode * idNode ) ;
	STDMETHOD(Characters)	( /*[in]*/ BSTR charData ) ;
	STDMETHOD(End)			() ;

private:
	CComPtr<ISOAPNode>	m_node ;
	CComBSTR			m_type, m_typeNS ;

	inline BYTE hexToNum(OLECHAR *hex) ;
};

#endif //__SERIALIZERHEXBIN_H_
