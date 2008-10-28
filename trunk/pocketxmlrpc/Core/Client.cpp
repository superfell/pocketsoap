/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketXML-RPC.

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2002-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "PocketXMLRPC.h"
#include "Client.h"
#include "base64.h"
#include "deserializers.h"

static const HRESULT E_XMLRPC_PARSE_FAILED   = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x4300) ;
static const HRESULT E_XMLRPC_SRV_FAULT_BASE = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x4310) ;

/////////////////////////////////////////////////////////////////////////////
// XmlRpcParser
/////////////////////////////////////////////////////////////////////////////
class XmlRpcParser : public expatpp
{
public:
	XmlRpcParser()
	{
		m_ps = psBegin ;
		IValueDeserializer * d = new containerDeserializer() ;
		d->Init(L"root") ;
		m_des.push(d) ;
	}

	~XmlRpcParser()
	{
		while ( m_des.size() )
		{
			delete m_des.top() ;
			m_des.pop() ;
		}
	}

	int Parse (const char* buffer, int len, int isFinal) ;
	// expat callbacks
	virtual void startElement(const XML_Char* name, const XML_Char** atts);
	virtual void endElement(const XML_Char*);
	virtual void charData(const XML_Char*, int len);

	bool	Faulted() ;
	HRESULT getValue(VARIANT *val) ;

	std::wstring ParserFault() ;

private:
	enum parserState {
		psBegin,
		psGotResponse,
		psGotParams,
		psGotFault
	} ;

	typedef std::stack<IValueDeserializer *> DESER_STACK ;

	parserState		m_ps ;
	std::wstring	m_chardata ;
	DESER_STACK		m_des ;
} ;

HRESULT XmlRpcParser::getValue(VARIANT *val) 
{
	IValueDeserializer * d = m_des.top() ;
	ATLASSERT(d && "Can only call XmlRpcParser::getValue once") ;
	if ( ! d )
		return E_UNEXPECTED ;
	m_des.pop() ;
	HRESULT hr = d->Value().val.Detach(val) ;	
	delete d ;
	return hr ;
}

bool XmlRpcParser::Faulted() 
{
	return m_ps == psGotFault ;
}

int XmlRpcParser::Parse (const char* buffer, int len, int isFinal) 
{
	return XML_Parse ( buffer, len, isFinal ) ;
}

std::wstring XmlRpcParser::ParserFault()
{
	std::wstring r ( XML_ErrorString(XML_GetErrorCode()) ) ;
	WCHAR buff[200] ;
	wsprintfW(buff, L" at line %d", XML_GetCurrentLineNumber() ) ;
	r.append(buff) ;
	return r ;
}

// expat callbacks
void XmlRpcParser::startElement(const XML_Char* name, const XML_Char** atts)
{
	ATLTRACE(_T("startElement\t%ls\n"), name ) ;
	m_chardata.erase() ;
	IValueDeserializer * d = NULL ;

	if ( m_ps == psBegin && wcscmp ( name , L"methodResponse" ) == 0 )
	{
		m_ps = psGotResponse ;
		d = new containerDeserializer() ;
	}
	else if ( m_ps == psGotResponse && wcscmp ( name, L"params" ) == 0 )
	{
		m_ps = psGotParams ;
		d = new containerDeserializer() ;
	}
	else if ( m_ps == psGotResponse && wcscmp ( name, L"fault" ) == 0 )
	{
		m_ps = psGotFault ;
		d = new containerDeserializer() ;
	}
	else if ( m_ps == psGotParams || m_ps == psGotFault )
	{
		if ( ( wcscmp ( name, L"i4" ) == 0 ) || wcscmp ( name, L"int" ) == 0 )
			d = new simpleDeserializer() ;
		else if ( wcscmp ( name , L"boolean" ) == 0 )
			d = new simpleDeserializer() ;
		else if ( wcscmp ( name, L"string" ) == 0 || wcscmp ( name, L"value" ) == 0 ) 
			d = new simpleDeserializer() ;
		else if ( wcscmp ( name, L"double" ) == 0 )
			d = new simpleDeserializer() ;
		else if ( wcscmp ( name, L"param" ) == 0 )
			d = new containerDeserializer() ;
		else if ( wcscmp ( name, L"data" ) == 0 )
			d = new arrayDeserializer() ;
		else if ( wcscmp ( name, L"array" ) == 0 )
			d = new containerDeserializer() ;
		else if ( wcscmp ( name, L"name" ) == 0 )
			d = new simpleDeserializer() ;
		else if ( wcscmp ( name, L"member" ) == 0 )
			d = new memberDeserializer() ;
		else if ( wcscmp ( name, L"struct" ) == 0 )
			d = new structDeserializer() ;
		else if ( wcscmp ( name, L"dateTime.iso8601" ) == 0 )
			d = new dateDeserializer() ;
		else if ( wcscmp ( name, L"base64" ) == 0 )
			d = new base64Deserializer() ;

	}
	if ( d != NULL )
		d->Init(name) ;
	m_des.push(d) ;
}

void XmlRpcParser::endElement(const XML_Char* name)
{
	ATLTRACE(_T("endElement\t\t%ls\n"), name ) ;
	IValueDeserializer *d = m_des.top() ;
	m_des.pop() ;
	if (d)
	{	
		d->Characters(m_chardata) ;
		d->Done() ;
		IValueDeserializer *p = m_des.top() ;
		if (p)
			p->Child(d->Value()) ;
		delete d ;
	}
	m_chardata.erase() ;
}

void XmlRpcParser::charData(const XML_Char* sz, int len)
{
	m_chardata.append ( sz, len ) ;
}

/////////////////////////////////////////////////////////////////////////////
// CClient
/////////////////////////////////////////////////////////////////////////////
HRESULT CClient::Initialize ( BSTR endpointURL, BSTR methodNamePrefix, CComPtr<IHttpRequest> &t )
{
	m_url = endpointURL ;
	m_prefix = methodNamePrefix ;
	m_http = t ;
	return S_OK ;
}

STDMETHODIMP CClient::GetTypeInfoCount(UINT* pctinfo) 
{
	*pctinfo = 0 ;
	return S_OK ;
}

STDMETHODIMP CClient::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) 
{
	return E_INVALIDARG ;
}

STDMETHODIMP CClient::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
	DISPIDS::iterator i ;
	while ( cNames-- )
	{
		rgdispid[cNames] = m_nextId++ ;
		m_ids[rgdispid[cNames]] = rgszNames[cNames] ;
	}
	return S_OK ;
}

STDMETHODIMP CClient::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) 
{
	CComBSTR bstrXml ;
	HRESULT hr ;
	{
		std::wstring xml ;
		xml.reserve(1024) ;
		xml.append ( L"<methodCall>\r\n<methodName>" ) ;
		xml.append ( m_prefix ) ;
		xml.append ( m_ids[dispidMember] ) ;
		xml.append ( L"</methodName>\r\n" ) ;
	
		
		if ( pdispparams->cArgs > 0 )
		{
			xml.append ( L"<params>\r\n" ) ;
			for ( int i = pdispparams->cArgs -1 ; i >= 0 ; i-- )
			{
				xml.append (L"<param>") ;
				hr = SerializeValue ( xml, &pdispparams->rgvarg[i] ) ;
				if (FAILED(hr)) return hr ;
				xml.append (L"</param>\r\n" ) ;
			}
			xml.append ( L"</params>\r\n") ;
		}
		xml.append ( L"</methodCall>" ) ;

		bstrXml.Attach ( SysAllocStringLen ( xml.c_str(), xml.size() ) );
	}

	CComPtr<IHttpResponse> httpRes ;
	VARIANT vPayload ;
	VariantInit(&vPayload) ;
	vPayload.vt = VT_BSTR ;
	vPayload.bstrVal = bstrXml ;
	hr = m_http->GetResponse ( m_url, vPayload, &httpRes ) ;
	if (FAILED(hr)) return hr ;
	bstrXml.Empty() ;
	
	CComPtr<IReadableStream> rdr ;
	hr = httpRes->get_Stream(&rdr) ;
	if (FAILED(hr)) return hr ;

	static const DWORD buff_size = 4096;
	char buff[buff_size] ;
	DWORD cb, oldcb ;
	XmlRpcParser p ;
	do
	{
		rdr->Read ( buff, buff_size, &cb ) ;
		if ( ! p.Parse ( buff, cb, cb == 0 ? TRUE : FALSE ) )
			return raiseError ( pexcepinfo, E_XMLRPC_PARSE_FAILED, 0x4300, p.ParserFault().c_str() ) ;
		oldcb = cb ;

	} while ( cb > 0 ) ;

	if (p.Faulted())
	{
		CComVariant f ;
		p.getValue(&f) ;
		CComBSTR fault(L"Fault returned by server") ;
		WORD     fc = 0 ;

		if ( f.vt == VT_DISPATCH )
		{
			CComPtr<IXmlRpcStruct> s ;
			f.pdispVal->QueryInterface(__uuidof(s), (void **)&s) ;
			if (s)
			{
				CComVariant faultString, faultCode ;
				s->get_Value(CComBSTR(L"faultString"), &faultString ) ;
				s->get_Value(CComBSTR(L"faultCode"),   &faultCode ) ;
				if ( VT_BSTR == faultString.vt && SysStringLen(faultString.bstrVal) > 0 )
					fault = faultString.bstrVal ;
				if ( VT_I4 == faultCode.vt )
					fc = (WORD)faultCode.lVal ;
			}
		}
		if ( pexcepinfo )
		{
			pexcepinfo->scode = 0 ;
			pexcepinfo->wReserved = 0 ;
			pexcepinfo->bstrHelpFile = NULL ;
			pexcepinfo->dwHelpContext =0 ;
			pexcepinfo->pvReserved = NULL ;
			pexcepinfo->pfnDeferredFillIn = NULL ;
			pexcepinfo->bstrSource =  SysAllocString(L"PocketXMLRPC") ;
			pexcepinfo->bstrDescription = fault.Detach() ;
			pexcepinfo->wCode = 1000 + fc  ;
			return DISP_E_EXCEPTION ;
		}
		return AtlReportError ( CLSID_CoFactory, fault, IID_NULL, E_XMLRPC_SRV_FAULT_BASE + fc ) ;
	}
	if (pvarResult)
	{
		VariantInit(pvarResult) ;
		hr = p.getValue(pvarResult) ;
	}
	return hr;
}

HRESULT CClient::raiseError ( EXCEPINFO * pexcepinfo, HRESULT hr, WORD wCode, LPCOLESTR desc )
{
	if ( pexcepinfo )
	{
		pexcepinfo->scode = 0 ;
		pexcepinfo->wReserved = 0 ;
		pexcepinfo->bstrHelpFile = NULL ;
		pexcepinfo->dwHelpContext =0 ;
		pexcepinfo->pvReserved = NULL ;
		pexcepinfo->pfnDeferredFillIn = NULL ;
		pexcepinfo->bstrSource =  SysAllocString(L"PocketXMLRPC") ;
		pexcepinfo->bstrDescription = SysAllocString(desc) ;
		pexcepinfo->wCode = wCode  ;
		return DISP_E_EXCEPTION ;
	}
	return AtlReportError ( CLSID_CoFactory, desc, IID_NULL, hr ) ;
}

HRESULT CClient::SerializeValue ( std::wstring &xml, VARIANT * val )
{
	HRESULT hr = E_INVALIDARG ;
	if ( val->vt & VT_BYREF ) 
	{
		CComVariant v ;
		VariantCopyInd ( &v, val ) ;
		return SerializeValue ( xml, &v ) ;
	}

	switch ( val->vt )
	{
		case VT_I1 :
		case VT_I2 :
		case VT_I4 :
		case VT_UI1 :
		case VT_UI2 :
			hr = SerializeInt ( xml, val ) ;
			break ;

		case VT_R4:
		case VT_R8:
			hr = SerializeFloat ( xml, val ) ;
			break ;

		case VT_BSTR:
			hr = SerializeString ( xml, val ) ;
			break ;

		case VT_BOOL:
			hr = SerializeBool ( xml, val ) ;
			break ;

		case VT_DATE:
			hr = SerializeDate ( xml, val ) ;
			break ;

		case VT_UI1 | VT_ARRAY:
			hr = SerializeBase64 ( xml, val ) ;
			break ;

		case VT_VARIANT | VT_ARRAY :
			hr = SerializeArray ( xml, val ) ;
			break ;

		case VT_UNKNOWN :
		case VT_DISPATCH :
			hr = SerializeObject ( xml, val ) ;
			break ;

	}
	return hr ;
}

std::wstring XmlEnc(BSTR src)
{
	std::wstring r ;
	WCHAR * c = src ;
	DWORD l = SysStringLen(src) ;
	r.reserve(l) ;
	for ( size_t i = 0 ; i <l ; i++ )
	{
		if ( *c == '<' )
			r.append(L"&lt;") ;
		else if ( *c == '>' )
			r.append(L"&gt;") ;
		else if ( *c == '&' )
			r.append(L"&amp;" ) ;
		else
			r.append(c,1) ;
		++c ;
	}
	return r ;
}

HRESULT CClient::SerializeString( std::wstring &xml, VARIANT *val )
{
	xml.append ( L"<value>" ) ;
	xml.append ( XmlEnc(val->bstrVal) ) ;
	xml.append ( L"</value>" ) ;
	return S_OK ;
}

HRESULT CClient::SerializeInt   ( std::wstring &xml, VARIANT *val ) 
{
	CComVariant v ;
	HRESULT hr = v.ChangeType ( VT_BSTR, val ) ;
	if (FAILED(hr)) return hr ;
	xml.append ( L"<value><i4>") ;
	xml.append ( v.bstrVal ) ;
	xml.append ( L"</i4></value>") ;
	return S_OK ;
}

HRESULT CClient::SerializeFloat ( std::wstring &xml, VARIANT *val ) 
{
	CComVariant v ;
	HRESULT hr = VariantChangeTypeEx ( &v, val, serLCID, 0, VT_BSTR ) ;
	if (FAILED(hr)) return hr ;
	xml.append ( L"<value><double>") ;
	xml.append ( v.bstrVal ) ;
	xml.append ( L"</double></value>") ;
	return S_OK ;
}

HRESULT CClient::SerializeBool  ( std::wstring &xml, VARIANT *val ) 
{
	if ( VARIANT_TRUE == val->boolVal ) 
		xml.append ( L"<value><boolean>1</boolean></value>" ) ;
	else
		xml.append ( L"<value><boolean>0</boolean></value>" ) ;
	return S_OK ;
}

HRESULT CClient::SerializeDate  ( std::wstring &xml, VARIANT *val ) 
{
	SYSTEMTIME st ;
	VariantTimeToSystemTime(val->date, &st) ;
	WCHAR buff[30] ;
	swprintf ( buff, L"%04d%02d%02dT%02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond ) ;
	xml.append ( L"<value><dateTime.iso8601>" ) ;
	xml.append ( buff ) ;
	xml.append ( L"</dateTime.iso8601></value>" ) ;
	return S_OK ;
}

HRESULT CClient::SerializeBase64( std::wstring &xml, VARIANT *val ) 
{
	long ub, lb ;
	SafeArrayGetUBound(val->parray, 1, &ub ) ;
	SafeArrayGetLBound(val->parray, 1, &lb ) ;
	size_t sizeNeeded = ( ub - lb + 1 ) * 4 / 3 ;
	if ( ( sizeNeeded % 4 ) != 0 )
		sizeNeeded += ( 4 - sizeNeeded % 4 ) ;
	std::auto_ptr<WCHAR> b64(new WCHAR[sizeNeeded]) ;
	void * data ;
	HRESULT hr = SafeArrayAccessData ( val->parray, &data ) ;
	if (SUCCEEDED(hr))
	{
		base64<WCHAR>::BufferEncode64 (  b64.get(), sizeNeeded, (BYTE *)data, ub-lb+1 ) ;
		SafeArrayUnaccessData ( val->parray ) ;
		xml.append ( L"<value><base64>" ) ;
		xml.append ( b64.get(), sizeNeeded ) ;
		xml.append ( L"</base64></value>" ) ;
	}
	return hr ;
}

HRESULT CClient::SerializeArray ( std::wstring &xml, VARIANT *val ) 
{
	long ub, lb ;
	SafeArrayGetUBound(val->parray, 1, &ub ) ;
	SafeArrayGetLBound(val->parray, 1, &lb ) ;
	VARIANT * item = 0 ;
	xml.append ( L"<value><array><data>" ) ;
	HRESULT hr = SafeArrayAccessData ( val->parray, (void **)&item ) ;
	if (SUCCEEDED(hr))
	{
		for ( int i = lb ; ! (i > ub) ; i++ )
		{
			hr = SerializeValue ( xml, item ) ;
			if (FAILED(hr)) break;
			xml.append ( L"\r\n" ) ;
			item++ ;
		}
		SafeArrayUnaccessData ( val->parray ) ;
	}
	xml.append ( L"</data></array></value>" ) ;
	return hr;
}

HRESULT CClient::SerializeObject( std::wstring &xml, VARIANT *val ) 
{
	CComPtr<IXmlRpcStruct> str ;
	HRESULT hr = val->punkVal->QueryInterface(__uuidof(str), (void **)&str) ;
	if (FAILED(hr)) return hr ;

	CComDispatchDriver disp(str) ;
	CComVariant vEnum ;
	hr = disp.GetProperty ( DISPID_NEWENUM, &vEnum ) ;
	if (FAILED(hr)) return hr ;

	CComPtr<IEnumVARIANT> e ;
	hr = vEnum.punkVal->QueryInterface(IID_IEnumVARIANT, (void **)&e) ;
	if (FAILED(hr)) return hr ;

	xml.append(L"<value><struct>\r\n") ;

	CComVariant key, ItemVal ;
	hr = e->Next ( 1, &key, NULL ) ;
	while ( S_OK == hr )
	{
		disp.GetPropertyByName(key.bstrVal, &ItemVal ) ;
		// serialize key/val into struct here
		xml.append(L"<member><name>") ;
		xml.append(key.bstrVal) ;
		xml.append(L"</name>\r\n") ;
		SerializeValue ( xml, &ItemVal ) ;
		ItemVal.Clear() ;
		key.Clear() ;
		xml.append(L"</member>\r\n") ;
		hr = e->Next ( 1, &key, NULL ) ;
	}

	xml.append(L"</struct></value>") ;
	return S_OK ;
}
