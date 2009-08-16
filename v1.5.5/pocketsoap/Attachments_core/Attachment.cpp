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
Portions created by Simon Fell are Copyright (C) 2002
Simon Fell. All Rights Reserved.

Contributor(s):
*/

// Attachment.cpp : Implementation of CAttachment

#include "stdafx.h"
#include "Attachments.h"
#include "Attachment.h"
#include "partPayload.h"
#include "contentId.h"

static const OLECHAR * DIME_DEF_URI_SCHEME = OLESTR("uuid:") ;
static const OLECHAR * MIME_DEF_URI_SCHEME = OLESTR("cid:") ;

/////////////////////////////////////////////////////////////////////////////
// CAttachment

STDMETHODIMP CAttachment::get_Body		  ( /*[out,retval]*/ VARIANT * pVal ) 
{
	if ( ! pVal ) return E_POINTER ;
	VariantInit(pVal) ;
	return VariantCopy ( pVal, &m_body ) ;
}

STDMETHODIMP CAttachment::put_Body		  ( /*[in]*/ VARIANT body ) 
{
	m_body = body ;
	return S_OK ;
}

STDMETHODIMP CAttachment::get_TypeNameFormat ( /*[out,retval]*/ TypeNameFormat * tnf ) 
{
	if ( ! tnf ) return E_POINTER ;
	*tnf = m_tnf ;
	return S_OK ;
}

STDMETHODIMP CAttachment::put_TypeNameFormat ( /*[in]*/ TypeNameFormat tnf ) 
{
	m_tnf = tnf ;
	return S_OK ;
}

STDMETHODIMP CAttachment::get_TypeName ( /*[out,retval]*/ BSTR * contentType ) 
{
	return m_type.CopyTo(contentType) ;
}

STDMETHODIMP CAttachment::put_TypeName ( /*[in]*/ BSTR contentType ) 
{
	m_type = contentType ;
	return S_OK ;
}

STDMETHODIMP CAttachment::get_ContentId   ( /*[out,retval]*/ BSTR * contentId ) 
{
	if ( ! m_cid.Length() )
		CreateContentId(m_format, m_cid) ;

	return m_cid.CopyTo(contentId) ;
}

STDMETHODIMP CAttachment::put_ContentId   ( /*[in]*/ BSTR contentId ) 
{
	m_cid = contentId ;
	return S_OK ;
}

STDMETHODIMP CAttachment::get_Uri( /*[out,retval]*/ BSTR * Uri ) 
{
	if ( ! m_uri.Length() )
	{
		//todo: relative URI's nightmare
		CComBSTR cid ;
		get_ContentId(&cid) ;
		m_uri = (m_format == formatMime ? MIME_DEF_URI_SCHEME : DIME_DEF_URI_SCHEME) ;
		m_uri.AppendBSTR(cid) ;
	}
	return m_uri.CopyTo(Uri) ;
}

STDMETHODIMP CAttachment::put_Uri( BSTR Uri )
{
	m_uri = Uri ;
	// extract contentId ;
	m_cid.Empty() ;
	if ( Uri )
	{
		WCHAR * colon = wcschr ( Uri, ':' ) ;
		if ( colon )
			m_cid = (colon+1) ;
		else
			m_cid = Uri ;
	}
	return S_OK ;
}

STDMETHODIMP CAttachment::Initialize( /*[in]*/ VARIANT body, TypeNameFormat typeNameFormat, /*[in]*/ BSTR ContentType )
{
	put_TypeName(ContentType) ;
	m_body.Clear() ;
	HRESULT hr = VariantCopyInd ( &m_body, &body ) ;
	m_tnf = typeNameFormat ;
	return hr;
}

STDMETHODIMP CAttachment::get_Located( /*[out,retval]*/ AttachmentLocation * loc )
{
	if ( ! loc ) return E_POINTER ;
	*loc =  LocatedOnDisk() ? attOnDisk : attInMemory ;
	return S_OK ;
}

bool CAttachment::LocatedOnDisk() 
{
	return m_body.vt == VT_BSTR ;
}

STDMETHODIMP CAttachment::Save( /*[out,retval]*/ BSTR fileName )
{
	USES_CONVERSION ;
	if ( LocatedOnDisk() )
	{
		// delete the dest file, this gives us the same semantics as CreateFile(CREATE_ALWAYS)
		DeleteFile( W2T(fileName) ) ;
		if(!MoveFile ( W2T(m_body.bstrVal), W2T(fileName) ))
			return AtlReportError ( GetObjectCLSID(), OLESTR("Unable to Save to requested file"), IID_NULL, HRESULT_FROM_WIN32(GetLastError()) ) ;
		CComVariant fn(fileName) ;
		m_body = fn ;
		return S_OK ;
	}

	if ( m_body.vt != ( VT_ARRAY | VT_UI1 ))
		return AtlReportError ( GetObjectCLSID(), OLESTR("Nothing to save"), IID_NULL, E_UNEXPECTED ) ;

	HANDLE hFile = CreateFile ( OLE2T(fileName), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL ) ;
	if ( hFile == INVALID_HANDLE_VALUE )
		return HRESULT_FROM_WIN32(GetLastError()) ;

	void * data = 0 ;
	long ub, lb ;
	SafeArrayGetUBound ( m_body.parray, 1, &ub ) ;
	SafeArrayGetLBound ( m_body.parray, 1, &lb ) ;
	DWORD cbW ;
	HRESULT hr = S_OK ;
	SafeArrayAccessData(m_body.parray, &data ) ;
	if ( ! WriteFile ( hFile, data, ub -lb + 1, &cbW, NULL ) )
		hr = HRESULT_FROM_WIN32(GetLastError()) ;

	CloseHandle(hFile) ;
	SafeArrayUnaccessData(m_body.parray) ;
	if ( SUCCEEDED(hr))
	{
		// now put the filename in the body
		CComVariant fn (fileName) ;
		m_body = fn ;
	}
	return hr;
}

template<class PayloadType, class InitType>
HRESULT PayloadCreator(InitType &body, IPartPayload ** payload)
{
	CComObject<PayloadType> * p ;
	HRESULT hr = p->CreateInstance(&p) ;
	if (FAILED(hr)) return hr ;
	p->AddRef() ;
	p->Init ( body ) ;
	p->QueryInterface ( payload ) ;
	p->Release() ;
	return S_OK ;
}

STDMETHODIMP CAttachment::PartPayload( /*[out,retval]*/ IPartPayload ** payload )
{
	if ( ! payload ) return E_POINTER ;

	if ( LocatedOnDisk() )
		return PayloadCreator<FilePayload>(m_body.bstrVal, payload) ;
	return PayloadCreator<MemoryPayload>(m_body.parray, payload) ;
}

STDMETHODIMP CAttachment::get_Format(AttachmentFormat *pVal)
{
	if ( ! pVal ) return E_POINTER ;
	*pVal = m_format ;
	return S_OK;
}

STDMETHODIMP CAttachment::put_Format(AttachmentFormat newVal)
{
	m_format = newVal ;
	return S_OK ;
}
