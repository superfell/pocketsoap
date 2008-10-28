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

/////////////////////////////////////////////////////////////////////////////
// Envelope.h : Declaration of the CEnvelope
//
// The represents a combined SOAP Envelope & Body, and is the main entry
// point for client code.
/////////////////////////////////////////////////////////////////////////////

#ifndef __ENVELOPE_H_
#define __ENVELOPE_H_

#include "resource.h"       // main symbols
#include "stringBuff.h"
#include "psParser.h"
#include "Namespaces.h"

class EnvWriter ;

/////////////////////////////////////////////////////////////////////////////
// CEnvelopeClassFactory
/////////////////////////////////////////////////////////////////////////////
class CEnvelopeClassFactory : 
	public CComClassFactory,
	public IClassFactoryVersion
{
public:

BEGIN_COM_MAP(CEnvelopeClassFactory)
	COM_INTERFACE_ENTRY(IClassFactoryVersion) 
	COM_INTERFACE_ENTRY_CHAIN(CComClassFactory)
END_COM_MAP()

	STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj) ;
	STDMETHOD(setVersion)(LPCWSTR envURI) ;
private:
	CComBSTR m_uri ;
} ;

/////////////////////////////////////////////////////////////////////////////
// CEnvelope
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CEnvelope : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnvelope, &CLSID_CoEnvelope>,
	public ISupportErrorInfo,
	public IsfDelegatingDispImpl<ISOAPEnvelope2>,
	public IProvideClassInfo2Impl<&CLSID_CoEnvelope, &IID_ISOAPEnvelope>,
	public ISOAPNamespaces,
	public IStreamReader
{
public:
	CEnvelope()  {  }
	~CEnvelope() {  }
		
DECLARE_REGISTRY_RESOURCEID(IDR_ENVELOPE)
DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_CLASSFACTORY_EX(CEnvelopeClassFactory)

BEGIN_COM_MAP(CEnvelope)
	COM_INTERFACE_ENTRY(ISOAPEnvelope2)
	COM_INTERFACE_ENTRY(ISOAPEnvelope)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IStreamReader)
	COM_INTERFACE_ENTRY(ISOAPNamespaces)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	HRESULT FinalConstruct() ;

public:
// ISOAPEnvelope
	STDMETHOD(SetMethod)		( /*[in]*/ BSTR methodName, /*[in]*/ BSTR methodNameNamespaceURI ) ;
	STDMETHOD(get_MethodName)   (/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_MethodName)   (/*[in]*/ BSTR newVal);
	STDMETHOD(get_URI)			(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_URI)			(/*[in]*/ BSTR newVal);
	STDMETHOD(get_EncodingStyle)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_EncodingStyle)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ThrowFaults)	(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_ThrowFaults)	(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(Parse)			(/*[in]*/ VARIANT Envelope, /*[in,defaultvalue(L"")]*/ BSTR characterEncoding );
	STDMETHOD(Serialize)		(/*[out,retval]*/ BSTR * soapMsg);

	STDMETHOD(get_Headers)		( /*[out,retval]*/ ISOAPNodes ** Nodes ) ;
	STDMETHOD(get_Body)			( ISOAPNodes ** Nodes ) ;
	STDMETHOD(get_Parameters)	( ISOAPNodes ** ppParams ) ;

	STDMETHOD(get_SerializerFactory)	( /*[out,retval]*/ ISerializerFactoryConfig ** sf ) ;
	STDMETHOD(putref_SerializerFactory) ( /*[in]*/			ISerializerFactoryConfig * sf ) ;

// ISOAPEnvelope2
	STDMETHOD(get_EnvelopeVersion) ( BSTR * EnvelopeURI ) ;  
	STDMETHOD(put_EnvelopeVersion) ( BSTR   EnvelopeURI ) ;

// ISOAPNamespaces
	STDMETHOD(GetPrefixForURI)( /*[in]*/ BSTR NamespaceURI, /*[out,retval]*/ BSTR * prefix ) ;
	STDMETHOD(GetURIForPrefix)( /*[in]*/ BSTR prefix,		/*[out,retval]*/ BSTR * NamespaceURI ) ;

// IStreamReader
	STDMETHOD(Read) (void *pv, ULONG cb, ULONG *pcbRead );
	STDMETHOD(Reset)() ;

// class
	void GetBodyNode ( CComPtr<ISOAPNode2> &body ) 
	{
		body = m_bodyNode ;
	}

	void GetHeaderNode ( CComPtr<ISOAPNode2> &hdr ) 
	{
		hdr = m_headersNode ;
	}

	HRESULT get_SerializerFactory(ISerializerFactory ** sf) ;	
	void setEnvelopeVersion(bool isSoap12) ;

	friend EnvWriter ;
	
private:
	CComBSTR					m_encStyle ;
	CComBSTR					m_methodName ;
	CComBSTR					m_URI ;
	bool						m_throwFaults ;
	bool						m_methodNameDirty ;
	bool						m_soap12 ;
	bool						m_encStyleDefaulted ;
	CComPtr<ISOAPNode2>			m_bodyNode , m_headersNode ;
	CComPtr<ISOAPNodes>			m_bodyParts ;
	CComPtr<ISOAPNode>			m_bodyRoot ;
	CComPtr<ISerializerFactory>	m_sf ;
	Namespaces					m_namespaces ;
	stringBuff_A				m_stream ;
	DWORD						m_stream_pos ;

	HRESULT CheckForFault		( const psParser& parser ) ;
	HRESULT ExtractNodeVal		( CComPtr<ISOAPNodes> &p, LPCOLESTR itemName, CComVariant &str ) ;
	void	updateEncodingStyle	( LPCWSTR newEncodingStyle ) ;
};

#endif //__ENVELOPE_H_
