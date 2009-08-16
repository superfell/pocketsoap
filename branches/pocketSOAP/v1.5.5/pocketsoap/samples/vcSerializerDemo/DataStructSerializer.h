// DataStructSerializer.h : Declaration of the CDataStructSerializer

#ifndef __DATASTRUCTSERIALIZER_H_
#define __DATASTRUCTSERIALIZER_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CDataStructSerializer
class ATL_NO_VTABLE CDataStructSerializer : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDataStructSerializer, &CLSID_DataStructSerializer>,
	public ITypesInit,
	public ISoapSerializer,
	public ISoapDeSerializer
{
public:
	CDataStructSerializer()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_DATASTRUCTSERIALIZER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDataStructSerializer)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(ISoapSerializer)
	COM_INTERFACE_ENTRY(ISoapDeSerializer)
END_COM_MAP()

public:

// ITypesInit
	STDMETHOD(Initialize)( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) ;

// ISoapSerializer
	STDMETHOD(Serialize)( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;

// ISoapDeSerializer
	STDMETHOD(Start)		( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;
	STDMETHOD(Child)		( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildReady)	( long id, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildRef)		( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) ;
	STDMETHOD(Ref)			( BSTR id,	 /*[in]*/ ISOAPNode * idNode ) ;
	STDMETHOD(Characters)	( /*[in]*/ BSTR charData ) ;
	STDMETHOD(End)			() ;

private:
	CComBSTR					m_ComType ;
	CComPtr<ISOAPNode>			m_node ;
	CComPtr<IDataStruct>		m_val;
};

#endif //__DATASTRUCTSERIALIZER_H_
