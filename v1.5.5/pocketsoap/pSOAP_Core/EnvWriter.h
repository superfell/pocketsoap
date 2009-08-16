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

#if !defined(AFX_ENVWRITER_H__75326055_662C_4C58_99F7_30AA5F4049F5__INCLUDED_)
#define AFX_ENVWRITER_H__75326055_662C_4C58_99F7_30AA5F4049F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stringBuff.h"
#include "idMgr.h"

class CEnvelope ;

class EnvWriter :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ISerializerContext2,
	public ISerializerOutput2
{
public:
	EnvWriter();
	virtual ~EnvWriter();

BEGIN_COM_MAP(EnvWriter)
	COM_INTERFACE_ENTRY(ISerializerContext)
	COM_INTERFACE_ENTRY(ISerializerContext2)
	COM_INTERFACE_ENTRY(ISerializerOutput)
	COM_INTERFACE_ENTRY(ISerializerOutput2)
END_COM_MAP()

// class entrypoint
	HRESULT Serialize ( CEnvelope &env, stringBuff_W &soap ) ;

// ISerializerContext
	STDMETHOD(get_Namespaces)			( /*[out,retval]*/ ISOAPNamespaces ** ns ) ;
	STDMETHOD(get_SerializerFactory)	( /*[out,retval]*/ ISerializerFactory ** sf ) ;

// ISerializerContext2
	STDMETHOD(get_EnvelopeVersion)	( BSTR * EnvelopeVersionURI ) ;

// ISerializerOutput
	STDMETHOD(StartElement)   ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace ) ;
	STDMETHOD(EndElement)     ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace ) ;

	STDMETHOD(Attribute)      ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[in]*/ BSTR value ) ;
	STDMETHOD(QNameAttribute) ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[in]*/ BSTR value, /*[in]*/ BSTR valueNamespace ) ;

	STDMETHOD(SerializeNode)  ( /*[in]*/ ISOAPNode * n ) ;
	STDMETHOD(SerializeValue) ( /*[in]*/ VARIANT * v, /*[in]*/ BSTR Name, /*[in]*/ BSTR NameNS ) ;

	STDMETHOD(WriteText)			( /*[in]*/ BSTR text ) ; // this gets escaped automatically
	STDMETHOD(WriteTextNoEncoding)	( /*[in]*/ BSTR text ) ; // just blindly appends it, make sure its safe

	// ISerializerOutput2
	STDMETHOD(SerializeAttribute)   ( /*[in]*/ VARIANT * value, /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace ) ;

private:
	HRESULT SerializeCollection ( ISOAPNodes * prms ) ;

	HRESULT CheckIfDone		( ISOAPNode * n, bool &done ) ;
	HRESULT CheckIfDone		( VARIANT * v, BSTR name, BSTR ns, bool &done ) ;
	bool NeedsId			( const VARIANT *v ) ;
	void * ExtractIdentity  ( VARIANT * v ) ;

	HRESULT QName				( BSTR name, BSTR ns ) ;
	HRESULT StartTag			( BSTR name, BSTR ns ) ;
	HRESULT ClosingTag			( BSTR Name, BSTR ns ) ;
	HRESULT CloseTag			( bool HasContent ) ;
	HRESULT WriteHrefElem		( BSTR name, BSTR ns, const stringBuff_W &href ) ;
	HRESULT StartHrefElem		( BSTR name, BSTR ns, const stringBuff_W &href ) ;
	HRESULT WriteHrefAttribute ( const wchar_t * href, int len_href, bool incHash) ;
	HRESULT FinishHrefElem		() ;

	HRESULT TidyStream			() ;

	stringBuff_W *	NewIndependentRoot() ;
	HRESULT			AddAndRegisterId ( VARIANT * v, stringBuff_W &id ) ;

	HRESULT WriteEncodedText	( /*[in]*/ BSTR text ) ;
	HRESULT SerializeNodeHelper	( ISOAPNode * n, ISoapSerializer * s );

	void ResolveType ( CComBSTR &type, CComBSTR &ns, const CComBSTR &ElemName, const CComBSTR &ElemNS ) ;
	void PushType ( CComBSTR &type, CComBSTR &ns, const CComBSTR &ElemName, const CComBSTR &ElemNS ) ;
	void PushType ( CComBSTR &type, CComBSTR &ns, ISOAPNode * node ) ;
	void PushType ( const CComBSTR &type, const CComBSTR &ns ) ;
	void PopType  () ;
	HRESULT SerializerForNode  ( ISOAPNode * n, BSTR * type, BSTR * typeNS, ISoapSerializer ** s ) ;
	HRESULT SerializerForValue ( VARIANT * value, BSTR Name, BSTR Namespace, BSTR * type, BSTR * typeNS, ISoapSerializer ** s ) ;

	typedef struct 
	{
		void *			v ;
		stringBuff_W	id ;
	} item ;
	typedef std::vector<item> ITEMS ;
	typedef struct Qname 
	{
		Qname(const CComBSTR &name, const CComBSTR &ns ) : localname(name), Namespace(ns) { }
		CComBSTR	localname ;
		CComBSTR	Namespace ;
	} Qname ;
	typedef std::stack<Qname> QNAMES ;

	QNAMES							m_typeStack ;
	ITEMS							m_done ;
	CComPtr<ISerializerFactoryEx>	m_sf ;
	CComPtr<ISOAPNamespaces>		m_ns ;

	arrayOfString<WCHAR>		m_ind ;		// all independently serialized items (everything with an id)
	stringBuff_W				m_body ;	// the root serialization for the body 
	stringBuff_W				m_hdr ;		// everything in the header
	stringBuff_W				*m_pcur ;	// what we're currently working on

	CComBSTR					m_lastEncStyle , m_envUri ;
	std::vector<bool>			m_closed ;	// stack of open elements, the bool indicates, if we've already closed the opening start tag
	bool						m_soap12 ;
	CEnvelope					*m_env ;
};

#endif // !defined(AFX_ENVWRITER_H__75326055_662C_4C58_99F7_30AA5F4049F5__INCLUDED_)
