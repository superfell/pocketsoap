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


/////////////////////////////////////////////////////////////////////////////
// psParser
//
// the pocketSOAP parser class, this is used to parse SOAP messages
// and uses expat & expatpp for the parser engine.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stringBuff.h"
#include "expatpp.h"
#include "namespaces.h"
#include "fixupMgr.h"
#include "reportErrorImpl.h"
#include "tags.h"

class CEnvelope ;

/////////////////////////////////////////////////////////////////////////////
// COM Wrapper around a set of attributes
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CAttributes : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public ISoapDeSerializerAttributes2
{
public:
		
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAttributes)
	COM_INTERFACE_ENTRY(ISoapDeSerializerAttributes)
	COM_INTERFACE_ENTRY(ISoapDeSerializerAttributes2)
END_COM_MAP()

// ISoapDeSerializerAttributes
	STDMETHOD(Exists) ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[out,retval]*/ VARIANT_BOOL * Exists ) ;
	STDMETHOD(Value)  ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[out,retval]*/ BSTR * val ) ;

// ISoapDeSerializerAttributes2
	STDMETHOD(ValueAs)( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, BSTR XmlType, BSTR TypeNamespace, /*[out,retval]*/ VARIANT * val ) ;

	void Init(ISOAPNamespaces * ns, ISerializerFactory2 * f) ;
	void SetAtts(const XML_Char ** atts) ;

private:
	const XML_Char ** m_atts ;
	CComPtr<ISerializerFactory2> m_sf ;
	CComPtr<ISOAPNamespaces>	 m_ns ;
};

/////////////////////////////////////////////////////////////////////////////
// the actual parser class
/////////////////////////////////////////////////////////////////////////////
class psParser : 
	public expatpp,
	public reportErrorImpl<psParser>
{
public:
	psParser(CEnvelope& env) ;
	~psParser() ;

	HRESULT parse(BSTR src) ;
	HRESULT parse(VARIANT src, BSTR characterEncoding) ;

	const WCHAR * methodName( void ) const { return m_methodName.c_str() ; }
	
	const CLSID & GetObjectCLSID() ;	// needed for reportErrorImpl

	// callbacks for the fixupMgr
	bool	elemNumInStack(unsigned long elem) ;

private:
	virtual void startElement   (const XML_Char* name, const XML_Char** atts);
	virtual void endElement     (const XML_Char*);
	virtual void charData       (const XML_Char*, int len);
	virtual void startNamespace	(const XML_Char* prefix , const XML_Char* uri ) ;
	virtual void endNamespace	(const XML_Char* prefix) ;

	void createParser	(BSTR characterEncoding) ;
	char * ExtractSrc	(VARIANT src, size_t &cb) ;
	void RegisterID		(BSTR id, ISOAPNode * node ) ;
	void ClearStack		() ;

	HRESULT CreateAndInitNode ( const XML_Char	*name, 
								const XML_Char	**atts, 
								ISOAPNode2		**n, 
								CComBSTR		&Name, 
								CComBSTR		&NameNS, 
								VARIANT_BOOL	&IsArray, 
								CComBSTR		&xmlType, 
								CComBSTR		&xmlTypeNS ) ;

	HRESULT ExpandQName ( const XML_Char *qName, CComBSTR &ns, CComBSTR &localName ) ;
	
	stringBuff_W				m_methodName, m_val ;
	bool						m_gotBody , m_gotEnv, m_inHeaders , m_soap12 ;
	unsigned long				m_nodenum ;
	
	CComObject<ISOAPNamespacesImpl> * m_namespaces ;

	CEnvelope							&m_theEnv ;
	CComPtr<ISerializerFactory2>		m_sf ;
	CComPtr<ISerializerFactoryHeaders>	m_headerChecker ;
	CComObject<CAttributes>				*m_attributes ;

	class StackItem
	{
	public:
		StackItem() : canComplete(true) { }
		StackItem( const unsigned long n, const CComPtr<ISOAPNode2> &nd, const CComPtr<ISoapDeSerializer> &s, LPCOLESTR enc ) :
			num(n), node(nd), ser(s), canComplete(true), encStyle(enc)
		{
		}
		StackItem( const CComBSTR& h, const unsigned long n, const CComPtr<ISOAPNode2> &nd, const CComPtr<ISoapDeSerializer> &s, LPCOLESTR enc ) :
			href(h), num(n), node(nd), ser(s), canComplete(true), encStyle(enc)
		{
		}
		StackItem( const StackItem &rhs ) : href(rhs.href), num(rhs.num), node(rhs.node), ser(rhs.ser), canComplete(rhs.canComplete), encStyle(rhs.encStyle)
		{
		}
		~StackItem()
		{
			node.Release() ;
			ser.Release() ;
		}
		
		bool isSoap11Encoded()
		{
			return ( wcscmp ( encStyle, SOAP_ENCODING_11_URI ) == 0 ) ;
		}
		bool isSoap12Encoded()
		{
			return ( wcscmp ( encStyle, SOAP_ENCODING_12_URI ) == 0 ) ;
		}

		CComBSTR					href ;
		unsigned long				num ;
		CComPtr<ISOAPNode2>			node ;
		CComPtr<ISoapDeSerializer>	ser ;
		bool						canComplete ;
		CComBSTR					encStyle ;
	} ;

	typedef std::map<std::wstring, CComPtr<ISoapDeSerializer> > STR2DESER ;

	STR2DESER					m_deferedTypes ; // maps id to a deserializer 

	std::vector<StackItem>		m_Stack ;	// stack of elements we're current processing
	FixupMgr					m_fixups;	// nodes waiting for fixups
	HRESULT						m_hr ;
};

