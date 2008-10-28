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

// Proxy.h : Declaration of the CProxy

#ifndef __PROXY_H_
#define __PROXY_H_

#include "resource.h"       // main symbols
#include "..\psoap_w32\psoap.h"

/////////////////////////////////////////////////////////////////////////////
// CProxy
class ATL_NO_VTABLE CProxy : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatch,
	public ISupportErrorInfoImpl<&IID_IDispatch>
{
public:
	CProxy()
	{
	}

DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CProxy)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()


// IDispatch
    STDMETHODIMP GetTypeInfoCount ( UINT *pctinfo ) ;
    STDMETHODIMP GetTypeInfo( 
        /* [in] */ UINT iTInfo,
        /* [in] */ LCID lcid,
        /* [out] */ ITypeInfo ** ppTInfo);
    
    STDMETHODIMP GetIDsOfNames( 
        /* [in] */ REFIID riid,
        /* [size_is][in] */ LPOLESTR *rgszNames,
        /* [in] */ UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ DISPID *rgDispId) ;
    
    STDMETHODIMP Invoke( 
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr);

public:
	void Init ( BSTR endpointURL, BSTR methodnameURI, VARIANT transport, BSTR SOAPActionPattern );

private:
	typedef CSimpleArray<CComBSTR> BSTR_ARRAY ;

	CComBSTR	m_url, m_methodURI, m_saPattern ;
	BSTR_ARRAY	m_names ;
	CComVariant	m_transport ;

	HRESULT ResolveTransport ( CComPtr<ISOAPTransport>& t ) ;
	HRESULT ExpandSoapAction ( CComBSTR &soapAction ) ;
	HRESULT DoSoap ( DISPPARAMS * pDispParams, VARIANT *pVarResult ) ;
};

#endif //__PROXY_H_
