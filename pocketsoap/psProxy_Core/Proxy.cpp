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
// Proxy.cpp : Implementation of CProxy
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PsProxy.h"
#include "Proxy.h"

/////////////////////////////////////////////////////////////////////////////
// CProxy
/////////////////////////////////////////////////////////////////////////////
#define MAGIC_OFFSET 42 

void CProxy::Init(BSTR endpointURL, BSTR methodnameURI, VARIANT transport, BSTR SOAPActionPattern )
{
	m_methodURI = methodnameURI ;
	m_url = endpointURL ;
	m_saPattern = SOAPActionPattern ;
	m_transport = transport ;
}

STDMETHODIMP CProxy::GetTypeInfoCount ( UINT *pctinfo )
{
	*pctinfo = 0 ;
	return S_OK ;
}

STDMETHODIMP CProxy::GetTypeInfo( 
    /* [in] */ UINT iTInfo,
    /* [in] */ LCID lcid,
    /* [out] */ ITypeInfo ** ppTInfo)
{
	return E_NOTIMPL ;
}

STDMETHODIMP CProxy::GetIDsOfNames( 
    /* [in] */ REFIID riid,
    /* [size_is][in] */ LPOLESTR *rgszNames,
    /* [in] */ UINT cNames,
    /* [in] */ LCID lcid,
    /* [size_is][out] */ DISPID *rgDispId)
{
	m_names.RemoveAll() ;
	for ( UINT idx = 0 ; idx < cNames ; ++idx )
	{
		m_names.Add ( CComBSTR(rgszNames[idx]) ) ;
		ATLTRACE(_T("Name %d = %ls\n"), idx, rgszNames[idx] ) ;
		rgDispId[idx] = idx + MAGIC_OFFSET;
	}
	return S_OK ;
}

STDMETHODIMP CProxy::Invoke( 
    /* [in] */ DISPID dispIdMember,
    /* [in] */ REFIID riid,
    /* [in] */ LCID lcid,
    /* [in] */ WORD wFlags,
    /* [out][in] */ DISPPARAMS *pDispParams,
    /* [out] */ VARIANT *pVarResult,
    /* [out] */ EXCEPINFO *pExcepInfo,
    /* [out] */ UINT *puArgErr)
{
	if ( dispIdMember != MAGIC_OFFSET )
		return DISP_E_MEMBERNOTFOUND ;

	ATLTRACE(_T("%d , %d\n"), pDispParams->cArgs ,  pDispParams->cNamedArgs  ) ;
	HRESULT hr = DoSoap ( pDispParams, pVarResult ) ;
	if (FAILED(hr) && pExcepInfo )
	{
		// check to see if they're some ErrorInfo
		CComPtr<IErrorInfo> ei ;
		if ( S_OK == GetErrorInfo(0, &ei ))
		{
			ZeroMemory ( pExcepInfo, sizeof(*pExcepInfo) ) ;
			pExcepInfo->scode = hr ;
			ei->GetDescription(&pExcepInfo->bstrDescription) ;
			ei->GetSource(&pExcepInfo->bstrSource) ;
			return DISP_E_EXCEPTION ;
		}
	}
	return hr ;
}

HRESULT CProxy::DoSoap ( DISPPARAMS * pDispParams, VARIANT *pVarResult ) 
{
	CComPtr<ISOAPEnvelope> env ;
	HRESULT hr = env.CoCreateInstance( __uuidof(CoEnvelope) ) ;
	if (FAILED(hr)) return hr ;

	env->put_MethodName ( m_names[0] ) ;
	env->put_URI(m_methodURI) ;

	CComPtr<ISOAPNodes> nodes ;
	env->get_Parameters(&nodes) ;
	for ( UINT idx = pDispParams->cNamedArgs  ; idx >0  ; --idx )
		nodes->Create ( m_names[idx], pDispParams->rgvarg[idx-1], NULL, NULL, NULL, NULL ) ;

	CComBSTR ser ;
	env->Serialize(&ser) ;

	CComPtr<ISOAPTransport> tran ;
	hr = ResolveTransport ( tran ) ;
	if (FAILED(hr)) return hr ;

	CComPtr<IHTTPTransport> httpTran ;
	hr = tran.QueryInterface(&httpTran) ;
	if (SUCCEEDED(hr))
	{
		// is a HTTP Transport, set the soapAction header
		CComBSTR soapAction ;
		hr = ExpandSoapAction(soapAction) ;
		hr = httpTran->put_SOAPAction(soapAction) ;
		if (FAILED(hr)) return hr ;
	}

	hr = tran->Send ( m_url, ser ) ;
	if (FAILED(hr)) return hr ;

	CComVariant vT(tran) ;
	CComBSTR enc ;

	hr = env->Parse(vT, enc) ;
	if (FAILED(hr)) return hr ;

	if ( pVarResult )
	{
		CComPtr<ISOAPNodes> prms ;
		env->get_Parameters(&prms) ;

		long numItems = 0 ;
		prms->get_Count(&numItems) ;

		if ( numItems > 0 )
		{
			CComPtr<ISOAPNode> p ;
			prms->get_Item(0, &p ) ;
			p->get_Value(pVarResult) ;
		}
		else
			VariantInit(pVarResult) ;
	}

	return S_OK ;
}

HRESULT CProxy::ExpandSoapAction ( CComBSTR &soapAction )
{
	// substituions are (not case sensitive)

	// $url		the endpoint url
	// $uri		the methodname uri
	// $method	the name of the method being called

	static const LPOLESTR find[] = { L"$url", L"$uri", L"$method" } ;
	const		 LPOLESTR rep[]  = { m_url, m_methodURI, m_names[0] } ;

	soapAction = m_saPattern ;
	WCHAR * p = 0 ;
	CComBSTR t ;
	for ( int i = 0 ; i < sizeof(find) / sizeof(*find) ; ++i )
	{
		p = wcsstr( soapAction, find[i] ) ;
		if ( p )
		{
			t.Append( soapAction, p - soapAction ) ;
			t.Append(rep[i]) ;
			t.Append( p + lstrlenW(find[i]) ) ;

			soapAction.Empty() ;
			soapAction.Attach ( t.Detach() ) ;
		}
	}
	return S_OK ;
}

HRESULT CProxy::ResolveTransport ( CComPtr<ISOAPTransport>& t )
{
	// options are
	//	1) transport is an empty string (the default) use the standard HTTP transport
	//	2) transport is a string, use it as a progid to create a transport
	//	3) transport is an object, us it as the transport

	CComVariant vTrans ;
	VariantCopyInd ( &vTrans, &m_transport ) ;
	if ( VT_BSTR == vTrans.vt )
	{
		if ( 0 == SysStringLen(vTrans.bstrVal) )
			return t.CoCreateInstance(__uuidof(HTTPTransport)) ;
		else
			return t.CoCreateInstance(vTrans.bstrVal) ;
	}
	if ( VT_UNKNOWN == vTrans.vt || VT_DISPATCH == vTrans.vt )
	{
		return vTrans.punkVal->QueryInterface(__uuidof(t), (void **)&t ) ;
	}
	return AtlReportError ( CLSID_NULL, L"transport object of unexpected type", IID_IProxyFactory2, E_UNEXPECTED ) ;
}