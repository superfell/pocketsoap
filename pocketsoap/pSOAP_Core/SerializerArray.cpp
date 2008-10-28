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

#include "stdafx.h"
#include "PSOAP.h"
#include "SerializerArray.h"
#include "serializerFactory.h"
#include "tags.h"
#include "copyHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// Position : This looks after our current position within the array
/////////////////////////////////////////////////////////////////////////////
Position::Position() : m_pos(0), m_valid(false), m_growbounds(false)
{
}

Position::~Position()
{
	if ( m_pos )
		delete [] m_pos ;
}

HRESULT	Position::MoveNext()
{
	return IncBound ( m_bounds.size()-1 ) ;
}

long * Position::Pos()
{
	return m_pos ;
}

size_t Position::Dimensions()
{
	return m_bounds.size() ;
}

std::vector<SAFEARRAYBOUND> & Position::Bounds()
{
	return m_bounds ;
}

HRESULT	Position::InitFromArrayType(LPCOLESTR arrayType)
{
	SAFEARRAYBOUND b ;
	b.lLbound = 0 ;
	std::vector<unsigned long> bs ;
	HRESULT hr = ExtractArrayDims ( arrayType, bs ) ;
	m_growbounds = S_FALSE == hr ;
	m_bounds.clear() ;
	for ( unsigned long j = 0 ; j < bs.size() ; ++j )
	{
		b.cElements = bs[j] ;
		m_bounds.push_back(b) ;
	}
	if ( m_pos )
	{
		delete [] m_pos ;
		m_pos = 0 ;
	}
	m_pos = new long [m_bounds.size()] ;
	memset ( m_pos, 0, sizeof(*m_pos) * m_bounds.size() ) ;
	m_valid = true ;
	return hr ;
}

HRESULT	Position::InitFromArraySize(LPCOLESTR arrayType)
{
	SAFEARRAYBOUND b ;
	b.lLbound = 0 ;
	std::vector<unsigned long> bs ;
	HRESULT hr = ExtractArrayDimsSoap12 ( arrayType, bs ) ;
	m_growbounds = S_FALSE == hr ;
	m_bounds.clear() ;
	for ( unsigned long j = 0 ; j < bs.size() ; ++j )
	{
		b.cElements = bs[j] ;
		m_bounds.push_back(b) ;
	}
	if ( m_pos )
	{
		delete [] m_pos ;
		m_pos = 0 ;
	}
	m_pos = new long [m_bounds.size()] ;
	memset ( m_pos, 0, sizeof(*m_pos) * m_bounds.size() ) ;
	m_valid = true ;
	return hr ;
}

HRESULT	Position::Init(SAFEARRAY * psa)
{
	if ( m_pos )
	{
		delete [] m_pos ;
		m_pos = 0 ;
	}
	m_growbounds = false ;
	m_bounds.clear() ;
	long dim = SafeArrayGetDim(psa) ;
	m_pos = new long [dim] ;

	// note that the bounds appear stored in reverse order
	for ( long j = dim-1 ; j >= 0 ; --j )
	{
		m_bounds.push_back(psa->rgsabound[j]) ;
		m_pos[dim-j-1] = psa->rgsabound[j].lLbound ;
	}
	m_valid = true ;
	return S_OK ;
}

HRESULT	Position::SetPosition(LPCOLESTR position)
{
	ARR_LONG v ;
	HRESULT hr = ExtractArrayDims( position, v ) ;
	if (FAILED(hr)) return hr ;
	memcpy ( m_pos, &v[0], sizeof(*m_pos) * m_bounds.size() ) ;
	if ( m_growbounds )
	{
		int b = m_bounds.size()-1 ;
		if (m_pos[b] >= (long)m_bounds[b].cElements + m_bounds[b].lLbound )
			m_bounds[b].cElements = m_pos[b] - m_bounds[b].lLbound + 1;
	}
	return S_OK ;
}

HRESULT Position::IsValidPos()
{
	for ( unsigned long i = 0 ; i < m_bounds.size() ; ++i ) 
	{
		if ( m_pos[i] >= (long)m_bounds[i].cElements + m_bounds[i].lLbound )
		{
			m_valid = false ;
			return AtlReportError ( __uuidof(CoEnvelope), OLESTR("Attempt to set position outside of valid range for array"), __uuidof(ISOAPEnvelope), E_INVALID_ARRAY_POS ) ;
		}
	}
	return S_OK ;
}

HRESULT Position::IncBound(unsigned long bound)
{
	if ( ! m_valid )
		return E_FAIL ;

	++m_pos[bound] ;
	if (m_pos[bound] >= (long)m_bounds[bound].cElements + m_bounds[bound].lLbound )
	{
		if ( m_growbounds )
		{
			m_bounds[bound].cElements = m_pos[bound] + m_bounds[bound].lLbound + 1;
			return S_OK ;
		}
		if ( bound > 0 )
		{
			m_pos[bound] = m_bounds[bound].lLbound ;
			return IncBound(bound-1) ;
		}
		else 
		{
			m_valid = false ;
			return S_FALSE ; // last valid location
		}
	}
	return S_OK ;
}

HRESULT Position::ExtractArrayDims ( LPCOLESTR src, ARR_LONG &v )
{
	v.clear() ;
	LPOLESTR next, s = wcschr ( src, '[' ) ;
	if ( ! s )
		return AtlReportError ( __uuidof(CoEnvelope), OLESTR("Array Co-ordinates missing opening ["), __uuidof(ISOAPEnvelope), E_ARRAY_COORD_FORMAT ) ;
	if ( *(s+1) == ']' )
	{
		v.push_back(0) ;
		return S_FALSE ;	// unbound mode
	}
	while ( s )
	{
		++s ;
		v.push_back( wcstol ( s, &next, 10 ) ) ;
		
		s = next ;
		if ( *s != ',' )
			break ;
	}
	if ( *s != ']' )
		return AtlReportError ( __uuidof(CoEnvelope), OLESTR("Array Co-ordinates missing closing ]"), __uuidof(ISOAPEnvelope), E_ARRAY_COORD_FORMAT ) ;

	return S_OK ;
}

HRESULT Position::ExtractArrayDimsSoap12 ( LPCOLESTR src, ARR_LONG &v)
{
	v.clear() ;
	LPOLESTR next, s = (LPOLESTR)src ;
	if ( *s == '*' )
	{
		v.push_back(0) ;
		return S_FALSE ; // unbound mode
	}
	while(*s)
	{
		v.push_back ( wcstol(s, &next, 10 ) ) ;
		s = next ;
		while(*s == ' ')
			++s ;
	}
	return S_OK ;
}

/////////////////////////////////////////////////////////////////////////////
// CSerializerArrayBase
/////////////////////////////////////////////////////////////////////////////
HRESULT CSerializerArrayBase::SerializeSafeArray ( VARTYPE vt, SAFEARRAY * psa, Position &p, ISerializerOutput * dest, const CComBSTR &arrayItemName, const CComBSTR &arrayItemNS )
{
	SafeArrayLock(psa) ;
	if ( VT_VARIANT == vt )
		SerializeVariantSafeArray ( psa, p, dest, arrayItemName, arrayItemNS ) ;
	else
	{
		BYTE * d = 0 ;
		VARIANT v ;
		HRESULT hr = S_OK ;
		while ( S_OK == hr )
		{
			hr = SafeArrayPtrOfIndex ( psa, p.Pos(), (void **)&d ) ;
			TypedCopyHelper ( &v, vt, d ) ;
			dest->SerializeValue ( &v , arrayItemName, arrayItemNS ) ;
			hr = p.MoveNext() ;
		}
	}
	SafeArrayUnlock(psa) ;
	return S_OK ;
}

HRESULT CSerializerArrayBase::SerializeVariantSafeArray ( SAFEARRAY * psa, Position &p, ISerializerOutput * dest, const CComBSTR &arrayItemName, const CComBSTR &arrayItemNS )
{
	VARIANT * d = 0 ;
	HRESULT hr = S_OK ;
	while ( S_OK == hr )
	{
		hr = SafeArrayPtrOfIndex ( psa, p.Pos(), (void **)&d ) ;
		dest->SerializeValue ( d , arrayItemName, arrayItemNS ) ;
		hr = p.MoveNext() ;
	}
	return S_OK ;
}


// returns true if offset contains something non-default
bool CSerializerArrayBase::GetBounds(SAFEARRAY * psa, stringBuff_W &bounds, stringBuff_W &offset, bool applyOffset, const wchar_t * boundsSeparator )
{
	bounds.Clear() ;
	offset.Clear() ;
	WCHAR buff[10] ;
	long ub, lb, dim = SafeArrayGetDim(psa) ;
	long arrSize ;
	bool needsOffset = false ;
	for ( long i = 1; i <= dim ; ++i )
	{
		SafeArrayGetUBound(psa, i, &ub) ;
		SafeArrayGetLBound(psa, i, &lb) ;
		// if applyOffset is true, then we can handle arrays with offsets, so the # of array items is ub+1
		// otherwise, we can't handle offsets and the # of array items is ub - lb + 1 ;
		if ( applyOffset )
			arrSize = ub + 1 ;
		else
			arrSize = ub - lb + 1 ;

		_itow( arrSize, buff, 10 ) ;
		bounds.Append ( buff ) ;

		_itow( lb, buff, 10 ) ;
		offset.Append ( buff ) ;
		if ( lb > 0 )
			needsOffset = true ;
		if ( i != dim )
		{
			bounds.Append ( boundsSeparator ) ;
			offset.Append ( boundsSeparator ) ;
		}
	}
	return needsOffset ;
}

/////////////////////////////////////////////////////////////////////////////
// CSerializerArray
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CSerializerArray::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType )
{
	m_ns = xmlTypeNamespace ;
	m_type = xmlType ;
	return S_OK ;
}

STDMETHODIMP CSerializerArray::Serialize( /*[in]*/ VARIANT * n, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	SAFEARRAY * psa = n->parray ;

	stringBuff_W bounds , offset ;
	bool needsOffset = GetBounds ( psa, bounds, offset, true, L"," ) ;

	static CComBSTR arrayType(OLESTR("arrayType")) ;
	static CComBSTR soapEnc(SOAP_ENCODING_11_URI) ;
	static CComBSTR xsiType(OLESTR("type")) ;
	static CComBSTR Array(OLESTR("Array")) ;

	// add the xsi:type='soapenc:Array' attribute
	// whilst nothing really should need these, after all soapenc;arrayType is all the hint you should need
	// apache soap still needs this
	CComPtr<ISerializerFactory> sf ;
	ctx->get_SerializerFactory(&sf) ;
	CComBSTR xsi ;
	sf->XsiForPrimaryNS(&xsi) ;
	dest->QNameAttribute ( xsiType, xsi, Array, soapEnc ) ;

	// the soapenc;arrayType="foo:bar[2]" attribute
	stringBuff_W arrType ;
	arrType.Append(m_type, m_type.Length());
	arrType.Append(L"[",1 ) ;
	arrType.Append( bounds ) ;
	arrType.Append(L"]", 1 ) ;
	dest->QNameAttribute ( arrayType, soapEnc, CComBSTR(arrType.c_str()), m_ns ) ;

	// the offset attribute if we need it
	if ( needsOffset ) 
	{
		stringBuff_W offsetVal ( L"[" ) ;
		offsetVal.Append(offset) ;
		offsetVal.Append(L"]", 1) ;
		static const CComBSTR offset_attr(OLESTR("offset")) ;
		dest->Attribute ( offset_attr, soapEnc, CComBSTR(offsetVal.c_str()) ) ; 
	}

	long ub, lb ;
	SafeArrayGetUBound(psa, 1, &ub ) ;
	SafeArrayGetLBound(psa, 1, &lb ) ;

	if ( ( ub + lb ) >= 0 )
	{
		CComPtr<IUnknown> punk ;
		CComBSTR itemVal ;
		VARIANT * d = 0 ;

		static const CComBSTR arrayItemName("i") ;
		static const CComBSTR arrayItemNS ;

		Position p ;
		HRESULT hr = p.Init(psa) ;
		VARTYPE vt = n->vt & ~ ( VT_ARRAY | VT_BYREF ) ;
		SerializeSafeArray ( vt, psa, p, dest, arrayItemName, arrayItemNS ) ;
	}

	return S_OK ;
}

/////////////////////////////////////////////////////////////////////////////
// CSerializerArray12
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CSerializerArray12::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType )
{
	m_ns = xmlTypeNamespace ;
	m_type = xmlType ;
	return S_OK ;
}

STDMETHODIMP CSerializerArray12::Serialize( /*[in]*/ VARIANT * n, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	SAFEARRAY * psa = n->parray ;

	stringBuff_W bounds , offset ;
	bool needsOffset = GetBounds ( psa, bounds, offset, false, L" " ) ;

	static CComBSTR arraySize(OLESTR("arraySize")) ;
	static CComBSTR itemType (OLESTR("itemType")) ;
	static CComBSTR soapEnc(SOAP_ENCODING_12_URI) ;

	// the soapenc;arraySize="2" attribute
	dest->Attribute ( arraySize, soapEnc, CComBSTR(bounds.c_str()) ) ;

	// the soapenc:itemType attribute
	dest->QNameAttribute ( itemType, soapEnc, m_type, m_ns ) ;

	long ub, lb ;
	SafeArrayGetUBound(psa, 1, &ub ) ;
	SafeArrayGetLBound(psa, 1, &lb ) ;

	if ( ( ub + lb ) >= 0 )
	{
		CComPtr<IUnknown> punk ;
		CComBSTR itemVal ;
		VARIANT * d = 0 ;

		static const CComBSTR arrayItemName("i") ;
		static const CComBSTR arrayItemNS ;

		Position p ;
		HRESULT hr = p.Init(psa) ;
		VARTYPE vt = n->vt & ~ ( VT_ARRAY | VT_BYREF ) ;
		SerializeSafeArray ( vt, psa, p, dest, arrayItemName, arrayItemNS ) ;
	}

	return S_OK ;
}

/////////////////////////////////////////////////////////////////////////////
// CDeserializerArrayImpl
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeserializerArrayImpl::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType )
{
	m_ns = xmlTypeNamespace ;
	m_type = xmlType ;
	if ( VT_I4 != comType.vt )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Unexpected array type"), __uuidof(ISoapDeSerializer), E_UNEXPECTED ) ;
	m_comType = (VARTYPE)comType.lVal ;
	return S_OK ;
}


// *** De-Serializer
STDMETHODIMP CDeserializerArrayImpl::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	m_node = node ;
	if ( m_helper )
	{
		delete m_helper ;
		m_helper = 0 ;
	}

	_HR(Start(Attributes, ns)) ;

	if ( ! m_helper )
		m_helper = new CArrayDeserializer(*this) ;

	return m_helper->Start() ;
}

STDMETHODIMP CDeserializerArrayImpl::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) 
{
	ATLTRACE(_T("@@@ CSA::Child %d, %ls\n"), id, ready == VARIANT_TRUE ? L"ready" : L"not ready" ) ;

	HRESULT hr ;
	CComBSTR position ;
	childNode->get_position(&position) ;
	if ( position.Length() )
	{
		hr= m_pos.SetPosition(position) ;
		if (FAILED(hr)) return hr ;
		hr = m_pos.IsValidPos() ;
		if (FAILED(hr)) return hr ;
	}

	hr = m_helper->Child(id, ready, childNode) ;
	if (SUCCEEDED(hr))
		m_pos.MoveNext() ;

	return hr ;
}

STDMETHODIMP CDeserializerArrayImpl::ChildReady ( long id, /*[in]*/ ISOAPNode * childNode ) 
{
	ATLTRACE(_T("@@@ CSA::ChildReady %d\n"), id ) ;
	return m_helper->ChildReady ( id, childNode ) ;
}

STDMETHODIMP CDeserializerArrayImpl::ChildRef( BSTR href, /*[in]*/ ISOAPNode * childNode ) 
{
	ATLTRACE(_T("@@@ CSA::ChildRef %ls\n"), href ) ;
	HRESULT hr ;
	CComBSTR position ;
	childNode->get_position(&position) ;
	if ( position.Length() )
	{
		hr = m_pos.SetPosition(position) ;
		if (FAILED(hr)) return hr ;
		hr = m_pos.IsValidPos() ;
		if (FAILED(hr)) return hr ;
	}

	hr = m_helper->ChildRef(href, childNode ) ;
	if (SUCCEEDED(hr))
		m_pos.MoveNext() ;

	return hr;
}

STDMETHODIMP CDeserializerArrayImpl::Ref( BSTR id, /*[in]*/ ISOAPNode * childNode )
{
	ATLTRACE(_T("@@@ CSA::Ref %ls\n"), id ) ;
	return m_helper->Ref(id, childNode) ;
}


STDMETHODIMP CDeserializerArrayImpl::Characters( /*[in]*/ BSTR charData ) 
{
	return S_OK ;
}

STDMETHODIMP CDeserializerArrayImpl::End()
{
	HRESULT hr = m_helper->End() ;
	delete m_helper ;
	m_helper = 0 ;
	return hr ;
}

/////////////////////////////////////////////////////////////////////////////
// CDeserializerArray
/////////////////////////////////////////////////////////////////////////////
HRESULT CDeserializerArray::Start ( ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	CComBSTR val, offset ;
	static CComBSTR bstrArrayType(L"arrayType") ;
	static CComBSTR bstrOffset(L"offset") ;
	static CComBSTR SOAP_ENC(SOAP_ENCODING_11_URI) ;

	HRESULT hr = Attributes->Value(bstrArrayType, SOAP_ENC, &val ) ;
	if ( S_OK == hr )
	{
		hr = m_pos.InitFromArrayType(val) ;
		if (FAILED(hr)) return hr ;
		if ( S_FALSE == hr )
			m_helper = new CUnboundArrayDeserializer(*this);
	}

	hr = Attributes->Value(bstrOffset, SOAP_ENC, &offset ) ;
	if ( offset.Length() ) 
	{
		hr = m_pos.SetPosition(offset) ;
		if (FAILED(hr)) return hr ;
	}
	return S_OK ;
}

/////////////////////////////////////////////////////////////////////////////
// CDeserializerArray12
/////////////////////////////////////////////////////////////////////////////
HRESULT CDeserializerArray12::Start ( ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	static CComBSTR arraySize(L"arraySize") ;
	static CComBSTR SOAP_ENC(SOAP_ENCODING_12_URI) ;

	CComBSTR size ;
	HRESULT hr = Attributes->Value(arraySize, SOAP_ENC, &size) ;
	// apply default, if attribute can't be found
	if ( E_INVALIDARG == hr )
		size = L"*" ;

	hr = m_pos.InitFromArraySize(size) ;
	if ( FAILED(hr)) return hr ;
	if ( S_FALSE == hr )
		m_helper = new CUnboundArrayDeserializer(*this);

	return S_OK ;
}

// Array De-serializer Helpers
//	because the processing for an unbounded array =(i.e. arrayType='xsd:int[]')
//  is so much difference, a bunch of the processing is pushed down into helper classes
//  the deserializer picks the right helper based on the arrayType

CArrayDeserializer::CArrayDeserializer(CDeserializerArrayImpl &parent) : CArrayDeserializerBase(parent), m_psa(0), m_base(0)
{
}

CArrayDeserializer::~CArrayDeserializer()
{
	if ( m_psa )
		SafeArrayDestroy(m_psa) ;
}

HRESULT CArrayDeserializer::Start(void)
{
	m_psa = SafeArrayCreate ( m_p.m_comType, m_p.m_pos.Dimensions(), &m_p.m_pos.Bounds()[0] ) ;
	return SafeArrayAccessData(m_psa, (void **)&m_base) ;
}

HRESULT CArrayDeserializer::PutNodeValInArray ( BYTE * dest, ISOAPNode * node ) 
{
	VARIANT val ;
	VariantInit(&val) ;
	HRESULT hr = node->get_ValueAs(m_p.m_type, m_p.m_ns, &val) ;
	if (FAILED(hr)) return hr ;
	if ( m_p.m_comType != VT_VARIANT )
	{
		hr = VariantChangeType ( &val, &val, 0, m_p.m_comType ) ;
		if (FAILED(hr)) return hr ;
	}
	return TypedCopyHelper ( dest, m_p.m_comType, &val ) ;
}

HRESULT CArrayDeserializer::Child(long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) 
{
	VARIANT * pos = 0 ;
	HRESULT hr = SafeArrayPtrOfIndex ( m_psa, m_p.m_pos.Pos(), (void **)&pos ) ;
	if (FAILED(hr)) return hr ;

	if ( ready == VARIANT_TRUE ) 
	{
		hr = PutNodeValInArray ( (BYTE *)pos, childNode ) ;
	}
	else
	{
		RefItem r ;
		r.num = id ;
		r.pos = pos ;
		m_refs.push_back(r) ;
	}
	return hr ;
}

HRESULT CArrayDeserializer::ChildReady ( long id, /*[in]*/ ISOAPNode * childNode ) 
{
	unsigned int j = 0 ;
	while ( j < m_refs.size() )
	{
		if ( m_refs[j].num == id )
		{
			PutNodeValInArray ( (BYTE *)m_refs[j].pos, childNode ) ;
			m_refs.erase(m_refs.begin() + j) ;
		}
		else
			++j ;
	}
	return S_OK ;
}

HRESULT CArrayDeserializer::ChildRef( BSTR href, /*[in]*/ ISOAPNode * childNode )
{
	VARIANT * pos = 0 ;
	HRESULT hr = SafeArrayPtrOfIndex ( m_psa, m_p.m_pos.Pos(), (void **)&pos ) ;
	if (FAILED(hr)) return hr ;

	RefItem r ;
	r.href.Attach(SysAllocStringLen ( href, SysStringLen(href))) ;
	r.pos  = pos ;
	r.num  = 0 ;
	m_refs.push_back(r) ;
	return S_OK ;
}

HRESULT CArrayDeserializer::Ref( BSTR id,   /*[in]*/ ISOAPNode * childNode )
{
	unsigned int j = 0 ;
	while ( j < m_refs.size() )
	{
		if ( bstrEqual(m_refs[j].href, id) )
		{
			PutNodeValInArray ( (BYTE *)m_refs[j].pos, childNode ) ;
			m_refs.erase(m_refs.begin() + j) ;
		}
		else
			++j ;
	}
	return S_OK ;
}

HRESULT CArrayDeserializer::End( void )
{
	SafeArrayUnaccessData( m_psa ) ;
	// transfer ownership to the Variant
	CComVariant v ;
	v.vt = VT_ARRAY | m_p.m_comType ;
	v.parray = m_psa ;
	m_psa = 0 ;
	return m_p.m_node->put_Value(v) ;
}

CUnboundArrayDeserializer::CUnboundArrayDeserializer(CDeserializerArrayImpl &parent) : CArrayDeserializerBase(parent)
{

}

CUnboundArrayDeserializer::~CUnboundArrayDeserializer()
{
	ClearNodes() ;
}

HRESULT CUnboundArrayDeserializer::Start(void)
{
	m_refs.clear();
	ClearNodes() ;
	return S_OK ;
}

HRESULT CUnboundArrayDeserializer::Child(long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) 
{
	ISOAPNode * dummy = 0 ;
	unsigned long p = *m_p.m_pos.Pos() ;
	ATLTRACE(_T("CUnboundArrayDeserializer::Child -> filling pos %d\n"), p ) ;
	while ( m_nodes.size() <= p )
		m_nodes.push_back(dummy) ;

	childNode->AddRef() ;
	m_nodes[p] = childNode ;
	return S_OK ;
}

HRESULT CUnboundArrayDeserializer::ChildReady( long id,   /*[in]*/ ISOAPNode * childNode ) 
{
	// we don't have to worry about this, as we don't pull the values out until the end
	return S_OK ;
}

HRESULT CUnboundArrayDeserializer::ChildRef( BSTR href, /*[in]*/ ISOAPNode * childNode ) 
{
	long p = *m_p.m_pos.Pos() ;

	RefItem r ;
	r.href.Attach(SysAllocStringLen ( href, SysStringLen(href))) ;
	r.pos  = p ;
	m_refs.push_back(r) ;
	return S_OK ;
}

HRESULT CUnboundArrayDeserializer::Ref( BSTR id,   /*[in]*/ ISOAPNode * childNode ) 
{
	ISOAPNode * dummy = 0  ;
	unsigned int j = 0 ;
	while ( j < m_refs.size() )
	{
		if ( bstrEqual(m_refs[j].href, id) )
		{
			while ( m_nodes.size() <= m_refs[j].pos )
				m_nodes.push_back(dummy) ;
			childNode->AddRef() ;
			m_nodes[m_refs[j].pos] = childNode ;
			m_refs.erase(m_refs.begin() + j) ;
		}
		else
			++j ;
	}
	return S_OK ;
}

HRESULT CUnboundArrayDeserializer::End( void )  
{
	ATLTRACE(_T("CUnboundArrayDeserializer::End() Array Size = %d\n"), m_nodes.size() ) ;
	SAFEARRAYBOUND rga ;
	rga.lLbound = 0 ;
	rga.cElements = m_nodes.size() ;
	SAFEARRAY * psa = SafeArrayCreate ( m_p.m_comType, 1, &rga ) ;
	BYTE * v = 0 ;
	VARIANT val ;
	size_t cbItem = SafeArrayGetElemsize(psa) ;
	VariantInit(&val) ;
	SafeArrayAccessData(psa, (void **)&v) ;
	for ( unsigned int i = 0 ; i < m_nodes.size() ; ++i )
	{
		if ( m_nodes[i] )
		{
			m_nodes[i]->get_ValueAs(m_p.m_type, m_p.m_ns, &val) ;
			TypedCopyHelper ( v, m_p.m_comType, &val ) ;
			// TypedCopyHelper transfers ownership, so we zap the ownership
			// in val, otherwise if you call VariantCopy or similar with val
			// as the destination, it will call VariantClear, and zap the
			// thing we just transfered ownership on. 
			val.vt = VT_EMPTY ;
		}
		v += cbItem ;
	}
	SafeArrayUnaccessData(psa) ;
	VariantInit(&val) ;
	val.vt = VT_ARRAY | m_p.m_comType ;
	val.parray = psa ;
	m_p.m_node->put_Value(val) ;
	SafeArrayDestroy(psa) ;
	return S_OK ;
}

void CUnboundArrayDeserializer::ClearNodes(void)
{
	for ( unsigned int i = 0 ; i < m_nodes.size() ; ++i )
	{
		if ( m_nodes[i] )
			m_nodes[i]->Release() ;
	}
	m_nodes.clear() ;
}
