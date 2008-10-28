/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketXML-RPC

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2002-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#ifndef __CLIENT_H_
#define __CLIENT_H_

#include "resource.h"       // main symbols
#include "expatpp.h"

/////////////////////////////////////////////////////////////////////////////
// CClient
/////////////////////////////////////////////////////////////////////////////

class ATL_NO_VTABLE CClient : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatch,
	public ISupportErrorInfoImpl<&IID_IDispatch>
	
{
public:
	CClient() : m_nextId(42) 
	{
	}

	~CClient()
	{
	}

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CClient)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) ;
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) ;
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) ;

// class methods used by the factory class
	HRESULT Initialize ( BSTR endpointURL, BSTR methodNamePrefix, CComPtr<IHttpRequest> &t );

private:
	typedef std::map<DISPID, std::wstring> DISPIDS ;

	DISPIDS		m_ids ;
	DISPID		m_nextId ;

	CComBSTR				m_url, m_prefix ;
	CComPtr<IHttpRequest>	m_http ;

	HRESULT raiseError ( EXCEPINFO * pexcepinfo, HRESULT hr, WORD wCode, LPCOLESTR desc ) ;
	HRESULT SerializeValue ( std::wstring &xml, VARIANT *val ) ;
	HRESULT SerializeString( std::wstring &xml, VARIANT *val ) ;
	HRESULT SerializeInt   ( std::wstring &xml, VARIANT *val ) ;
	HRESULT SerializeFloat ( std::wstring &xml, VARIANT *val ) ;
	HRESULT SerializeBool  ( std::wstring &xml, VARIANT *val ) ;
	HRESULT SerializeDate  ( std::wstring &xml, VARIANT *val ) ;
	HRESULT SerializeBase64( std::wstring &xml, VARIANT *val ) ;
	HRESULT SerializeArray ( std::wstring &xml, VARIANT *val ) ;
	HRESULT SerializeObject( std::wstring &xml, VARIANT *val ) ;
};

#endif //__CLIENT_H_
