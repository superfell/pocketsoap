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
Portions created by Simon Fell are Copyright (C) 2001-2002,2005
Simon Fell. All Rights Reserved.

Contributor(s):
*/

/////////////////////////////////////////////////////////////////////////////
// SerializerSimple.h : Declaration of the CSerializerSimple
/////////////////////////////////////////////////////////////////////////////

#ifndef __SERIALIZERSIMPLE_H_
#define __SERIALIZERSIMPLE_H_

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSerializerSimpleBase
// this defines a base class that makes it easier to implement simple serializer
/////////////////////////////////////////////////////////////////////////////
class CSerializerSimpleBase :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ISupportErrorInfoImpl<&__uuidof(ISoapSerializer)>, 
	public ISoapSerializer,
	public ISoapDeSerializer,
	public ITypesInit,
	public ISimpleSoapSerializer,
	public ISimpleSoapDeSerializer
{
public:
	CSerializerSimpleBase() ;
	~CSerializerSimpleBase();

	DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSerializerSimpleBase)
	COM_INTERFACE_ENTRY(ISoapSerializer)
	COM_INTERFACE_ENTRY(ISoapDeSerializer)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(ISimpleSoapSerializer)
	COM_INTERFACE_ENTRY(ISimpleSoapDeSerializer)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ITypesInit
	STDMETHOD(Initialize)( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) ;

// ISoapSerializer
	STDMETHOD(Serialize)( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;

// ISimpleSoapSerializer
	STDMETHOD(Serialize)( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ BSTR * dest ) ;

// ISoapDeSerializer
	STDMETHOD(Start)    ( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;
	STDMETHOD(Child)		( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildReady)	( long id, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildRef)		( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) ;
	STDMETHOD(Ref)			( BSTR id,	 /*[in]*/ ISOAPNode * idNode ) ;
	STDMETHOD(Characters)	( /*[in]*/ BSTR charData ) ;
	STDMETHOD(End)			() ;

// 	ISimpleSoapDeSerializer
	STDMETHOD(Deserialize) ( BSTR characters, ISOAPNamespaces * ns, VARIANT * dest ) ;
 
// override these if you need to
	virtual HRESULT WriteType	( VARIANT * val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;
	virtual HRESULT WriteValue	( VARIANT * val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;

	virtual const CLSID & GetCLSID() = 0 ;

protected:
	CComPtr<ISOAPNode>	m_node ;
	CComBSTR			m_type, m_typeNS ;
	CComVariant			m_comType ;
	CComPtr<ISOAPNamespaces> m_ns ;
};

/////////////////////////////////////////////////////////////////////////////
// CSerializerSimple
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSerializerSimple : 
	public CComCoClass<CSerializerSimple, &CLSID_CoSerializerSimple>,
	public CSerializerSimpleBase
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_SERIALIZERSIMPLE)

BEGIN_COM_MAP(CSerializerSimple)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_CHAIN(CSerializerSimpleBase)
END_COM_MAP()

private:
	virtual const CLSID & GetCLSID()
	{
		return GetObjectCLSID() ;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CSerializerBoolean
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSerializerBoolean : 
	public CComCoClass<CSerializerBoolean, &CLSID_CoSerializerBoolean>,
	public CSerializerSimpleBase
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_SERIALIZERBOOLEAN)

BEGIN_COM_MAP(CSerializerBoolean)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_CHAIN(CSerializerSimpleBase)
END_COM_MAP()

// ISimpleSoapSerializer
	STDMETHOD(Serialize)( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ BSTR * dest ) ;

private:
	virtual const CLSID & GetCLSID()
	{
		return GetObjectCLSID() ;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CDeserializerXsdLong
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CDeserializerXsdLong : 
	public CComCoClass<CDeserializerXsdLong, &CLSID_CoDeserializerXsdLong>,
	public CSerializerSimpleBase
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_DESERIALIZERXSDLONG)

BEGIN_COM_MAP(CDeserializerXsdLong)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_CHAIN(CSerializerSimpleBase)
END_COM_MAP()

// 	ISimpleSoapDeSerializer
	STDMETHOD(Deserialize) ( BSTR characters, ISOAPNamespaces * ns, VARIANT * dest ) ;

	virtual HRESULT WriteValue	( VARIANT * val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;

private:
	virtual const CLSID & GetCLSID()
	{
		return GetObjectCLSID() ;
	}
};
#endif //__SERIALIZERFACTORY_H_
