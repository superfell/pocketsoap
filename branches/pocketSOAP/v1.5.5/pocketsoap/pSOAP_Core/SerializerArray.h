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
Portions created by Simon Fell are Copyright (C) 2000,2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#ifndef __SERIALIZERARRAY_H_
#define __SERIALIZERARRAY_H_

#include "resource.h"       // main symbols
#include "stringBuff.h"

class CArrayDeserializerBase  ;
class CArrayDeserializer ;
class CUnboundArrayDeserializer ;

/////////////////////////////////////////////////////////////////////////////
// Position, this keeps track of what position we're at in an x dimensional array
/////////////////////////////////////////////////////////////////////////////
typedef std::vector<unsigned long> ARR_LONG ;

class Position
{
public:
	Position()  ;
	~Position() ;

	HRESULT								InitFromArrayType(LPCOLESTR arrayType) ;
	HRESULT								InitFromArraySize(LPCOLESTR arraySize) ;
	HRESULT								Init(SAFEARRAY * psa) ;
	HRESULT								SetPosition(LPCOLESTR position) ;
	HRESULT								MoveNext() ;
	HRESULT								IsValidPos() ;
	long *								Pos() ;
	size_t								Dimensions() ;
	std::vector<SAFEARRAYBOUND> &		Bounds() ;

private:
	long 							*m_pos ;
	bool							m_valid, m_growbounds ;
	std::vector<SAFEARRAYBOUND>		m_bounds ;

	HRESULT		IncBound(unsigned long bound) ;
	HRESULT		ExtractArrayDims	   ( LPCOLESTR src, ARR_LONG &vals ) ;
	HRESULT		ExtractArrayDimsSoap12 ( LPCOLESTR src, ARR_LONG &vals ) ;

} ;

/////////////////////////////////////////////////////////////////////////////
// CSerializerArrayBase
/////////////////////////////////////////////////////////////////////////////
class CSerializerArrayBase
{
public:
	HRESULT SerializeSafeArray		  ( VARTYPE vt, SAFEARRAY * psa, Position &p, ISerializerOutput * dest, const CComBSTR &arrayItemName, const CComBSTR &arrayItemNS ) ;
	HRESULT SerializeVariantSafeArray ( SAFEARRAY * psa, Position &p, ISerializerOutput * dest, const CComBSTR &arrayItemName, const CComBSTR &arrayItemNS ) ;
	bool	GetBounds				  ( SAFEARRAY * psa, stringBuff_W &bounds, stringBuff_W &offset, bool applyOffset, const wchar_t * boundsSeparator ) ;
} ;

/////////////////////////////////////////////////////////////////////////////
// CSerializerArray
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSerializerArray : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSerializerArray, &CLSID_CoSerializerArray>,
	public ISupportErrorInfoImpl<&__uuidof(ISoapSerializer)>,
	public ISoapSerializer,
	public ITypesInit,
	public CSerializerArrayBase
{
public:
DECLARE_REGISTRY_RESOURCEID(IDR_SERIALIZERARRAY)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSerializerArray)
	COM_INTERFACE_ENTRY(ISoapSerializer)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ITypesInit
	STDMETHOD(Initialize)( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) ;

// ISoapSerializer
	STDMETHOD(Serialize) ( /*[in]*/ VARIANT * n, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;

private:
	CComBSTR m_type, m_ns ;
};

/////////////////////////////////////////////////////////////////////////////
// CSerializerArray12
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSerializerArray12 : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSerializerArray12, &CLSID_CoSerializerArray12>,
	public ISupportErrorInfoImpl<&__uuidof(ISoapSerializer)>,
	public ISoapSerializer,
	public ITypesInit,
	public CSerializerArrayBase
{
public:
DECLARE_REGISTRY_RESOURCEID(IDR_SERIALIZERARRAY12)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSerializerArray)
	COM_INTERFACE_ENTRY(ISoapSerializer)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ITypesInit
	STDMETHOD(Initialize)( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) ;

// ISoapSerializer
	STDMETHOD(Serialize) ( /*[in]*/ VARIANT * n, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) ;

private:
	CComBSTR m_type, m_ns ;
};


/////////////////////////////////////////////////////////////////////////////
// CDeserializerArrayImpl
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CDeserializerArrayImpl : 
	public ISoapDeSerializer,
	public ITypesInit
{
public:
	CDeserializerArrayImpl() : m_helper(0)
	{
		ATLTRACE(_T("CDeserializerArrayImpl::CDeserializerArrayImpl()\n")) ;
	} 

	~CDeserializerArrayImpl() 
	{
		if ( m_helper )
			End() ;
		ATLTRACE(_T("CDeserializerArrayImpl::~CDeserializerArrayImpl()\n")) ;
	}

// ITypesInit
	STDMETHOD(Initialize)( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) ;

// ISoapDeSerializer
	STDMETHOD(Start)	  ( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;
	STDMETHOD(Child)	  ( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildReady) ( long id, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(ChildRef)	  ( BSTR href, /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(Ref)		  ( BSTR id,   /*[in]*/ ISOAPNode * childNode ) ;
	STDMETHOD(Characters) ( /*[in]*/ BSTR charData ) ;
	STDMETHOD(End)		  ( ) ;

	// derived class specific initialization
	virtual HRESULT Start ( ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;

	virtual const CLSID& GetObjectCLSID() = 0 ;

protected:
	CComPtr<ISOAPNode>				m_node ;
	Position						m_pos ;
	CComBSTR						m_type, m_ns ;
	VARTYPE							m_comType ;
	CArrayDeserializerBase *		m_helper ;

	friend CArrayDeserializer ;
	friend CUnboundArrayDeserializer ;
};

/////////////////////////////////////////////////////////////////////////////
// CDeserializerArray
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CDeserializerArray : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDeserializerArray, &CLSID_CoDeserializerArray>,
	public ISupportErrorInfoImpl<&__uuidof(ISoapDeSerializer)>,
	public CDeserializerArrayImpl
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_DESERIALIZERARRAY)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDeserializerArray)
	COM_INTERFACE_ENTRY(ISoapDeSerializer)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

	virtual const CLSID& GetObjectCLSID()
	{
		return CComCoClass<CDeserializerArray, &CLSID_CoDeserializerArray>::GetObjectCLSID() ;
	}

	virtual HRESULT Start ( ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;
};

/////////////////////////////////////////////////////////////////////////////
// CDeserializerArray12
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CDeserializerArray12 : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDeserializerArray12, &CLSID_CoDeserializerArray12>,
	public ISupportErrorInfoImpl<&__uuidof(ISoapDeSerializer)>,
	public CDeserializerArrayImpl
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_DESERIALIZERARRAY12)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDeserializerArray12)
	COM_INTERFACE_ENTRY(ISoapDeSerializer)
	COM_INTERFACE_ENTRY(ITypesInit)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

	virtual const CLSID& GetObjectCLSID()
	{
		return CComCoClass<CDeserializerArray12, &CLSID_CoDeserializerArray12>::GetObjectCLSID() ;
	}

	virtual HRESULT Start ( ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) ;
};

/////////////////////////////////////////////////////////////////////////////
// CArrayDeserializerBase
/////////////////////////////////////////////////////////////////////////////
class CArrayDeserializerBase 
{
public:
	CArrayDeserializerBase(CDeserializerArrayImpl &parent) : m_p(parent) { }
	virtual ~CArrayDeserializerBase() { }
	CDeserializerArrayImpl & m_p ;

	virtual HRESULT Start	   (void) = 0 ;
	virtual HRESULT Child(long id, VARIANT_BOOL ready, ISOAPNode *childNode) = 0 ;
	virtual HRESULT ChildReady ( long id,   /*[in]*/ ISOAPNode * childNode ) = 0 ;
	virtual HRESULT ChildRef   ( BSTR href, /*[in]*/ ISOAPNode * childNode ) = 0 ;
	virtual HRESULT Ref		   ( BSTR id,   /*[in]*/ ISOAPNode * childNode ) = 0 ;
	virtual HRESULT End		   ( void ) = 0 ;
} ;

/////////////////////////////////////////////////////////////////////////////
// CArrayDeserializer
/////////////////////////////////////////////////////////////////////////////
class CArrayDeserializer : 
	public CArrayDeserializerBase 
{
public:
	CArrayDeserializer(CDeserializerArrayImpl &parent) ;
	virtual ~CArrayDeserializer() ;

	virtual HRESULT Start(void) ;
	virtual HRESULT Child(long id, VARIANT_BOOL ready, ISOAPNode *childNode) ;
	virtual HRESULT ChildReady ( long id,   /*[in]*/ ISOAPNode * childNode ) ;
	virtual HRESULT ChildRef   ( BSTR href, /*[in]*/ ISOAPNode * childNode ) ;
	virtual HRESULT Ref		   ( BSTR id,   /*[in]*/ ISOAPNode * childNode ) ;
	virtual HRESULT End		   ( void ) ;

private:
	typedef struct RefItem
	{
		VARIANT *	pos ;
		CComBSTR	href ;
		long		num ;
	} RefItem ;

	std::vector<RefItem>			m_refs ;
	SAFEARRAY *						m_psa  ;
	VARIANT *						m_base ;

	HRESULT PutNodeValInArray ( BYTE * dest, ISOAPNode * node ) ;
} ;

/////////////////////////////////////////////////////////////////////////////
// CUnboundArrayDeserializer
/////////////////////////////////////////////////////////////////////////////
class CUnboundArrayDeserializer :
	public CArrayDeserializerBase 
{
public:
	CUnboundArrayDeserializer(CDeserializerArrayImpl &parent) ;
	virtual ~CUnboundArrayDeserializer() ;

	virtual HRESULT Start(void) ;
	virtual HRESULT Child(long id, VARIANT_BOOL ready, ISOAPNode *childNode) ;
	virtual HRESULT ChildReady ( long id,   /*[in]*/ ISOAPNode * childNode ) ;
	virtual HRESULT ChildRef   ( BSTR href, /*[in]*/ ISOAPNode * childNode ) ;
	virtual HRESULT Ref		   ( BSTR id,   /*[in]*/ ISOAPNode * childNode ) ;
	virtual HRESULT End		   ( void )  ;

private:
	void ClearNodes(void) ;

	typedef struct RefItem
	{
		unsigned long	pos ;
		CComBSTR		href ;
	} RefItem ;

	std::vector<ISOAPNode *> m_nodes;
	std::vector<RefItem>	 m_refs ;
} ;

#endif //__SERIALIZERARRAY_H_
