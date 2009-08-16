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
Portions created by Simon Fell are Copyright (C) 2002-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

// Manager.cpp : Implementation of CManager

#include "stdafx.h"
#include "Attachments.h"
#include "Manager.h"
#include "AttachmentsCollection.h"
#include "mimePackager.h"
#include "dimePackager.h"
#include "stringHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CManager
void WINAPI CManager::ObjectMain(bool bStarting)
{
	if ( bStarting )
	{
		SYSTEMTIME t ;
		GetSystemTime(&t) ;
		FILETIME ft ;
		SystemTimeToFileTime(&t, &ft) ;
		srand(*((unsigned int *)&ft)) ;
	}
}


HRESULT CManager::FinalConstruct()
{
	CComObject<CAttachmentsCollection> * c ;
	HRESULT hr = c->CreateInstance(&c) ;
	if (FAILED(hr)) return hr ;
	c->AddRef(); 
	c->QueryInterface(&m_req) ;
	c->Release() ;
	hr = c->CreateInstance(&c) ;
	if (FAILED(hr)) return hr ;
	c->AddRef(); 
	c->QueryInterface(&m_res) ;
	c->Release() ;

	put_Format(m_format) ;
	return S_OK ;
}

STDMETHODIMP CManager::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IAttachmentManger, &__uuidof(ISOAPTransport), &IID_ISoapAttachmentFormat
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CManager::get_Transport(ISOAPTransport **pVal)
{
	return m_trn.CopyTo(pVal) ;
}

STDMETHODIMP CManager::putref_Transport(ISOAPTransport *newVal)
{
	CComQIPtr<ISwATransport> swa(newVal) ;
	if ( ! swa )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Transport object does not implement required interface [ISwATransport], you cannot send attachments over this transport"), __uuidof(ISOAPTransport), E_INVALID_TRANSPORT ) ;

	m_trn = newVal ;
	return S_OK ;
}

STDMETHODIMP CManager::get_Request	( ISoapAttachments ** collection )
{
	return m_req.CopyTo(collection) ;
}

STDMETHODIMP CManager::get_Response	( ISoapAttachments ** collection ) 
{
	return m_res.CopyTo(collection) ;
}

template<class T>
class PackagerCreator
{
public:
	static void Create (CComPtr<IPackager> &res)
	{
		CComObject<T> * p ;
		p->CreateInstance(&p) ;
		p->AddRef() ;
		res = p ;
		p->Release() ;
	}
} ;

CComPtr<IPackager> CManager::CreatePackager(AttachmentFormat f)
{
	CComPtr<IPackager> p ;
	if ( f == formatMime )
		PackagerCreator<MimePackager>::Create(p) ;
	else
		PackagerCreator<DimePackager>::Create(p) ;

	p->Init(m_sizeLimit) ;
	return p ;
}

STDMETHODIMP CManager::Send(BSTR endpoint, BSTR envelope)
{
	if ( ! m_trn )
		return AtlReportError(CLSID_CoAttachmentManager, OLESTR("Must set transport object before calling send"), IID_NULL, E_UNEXPECTED ) ;

	CComPtr<ISoapAttachment> soap ;
	HRESULT hr = CreateSoapMsgPart(envelope, &soap) ;
	if (FAILED(hr)) return hr ;
	return CreatePackager(m_format)->PackageAndSend(endpoint, soap, m_req, m_trn ) ;
}


HRESULT CManager::PassThroughReceive ( IStreamReader * resStream, SAFEARRAY ** Envelope ) 
{
	stringBuff_A res ;
	static const DWORD buff_size = 8192 ;
	char buff[buff_size] ;
	DWORD cb =0 ;
	do
	{
		resStream->Read ( buff, buff_size, &cb ) ;
		res.Append ( buff, cb ) ;
	} while ( cb > 0 ) ;

	SAFEARRAYBOUND rga ;
	rga.lLbound = 0;
	rga.cElements = res.Size() ;
	*Envelope = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
	void * data = 0 ;
	SafeArrayAccessData ( *Envelope, &data ) ;
	memcpy( data, res.c_str(), rga.cElements ) ;
	SafeArrayUnaccessData ( *Envelope ) ;

	return S_OK ;
}

STDMETHODIMP CManager::Receive( /*[in,out]*/ BSTR * characterEncoding, /*[out,retval]*/ SAFEARRAY ** Envelope )
{
	if ( ! m_trn )
		return AtlReportError(CLSID_CoAttachmentManager, OLESTR("Must set transport object before calling receive"), IID_NULL, E_UNEXPECTED ) ;

	CComPtr<IPackager>		p ;
	CComPtr<IStreamReader>	response ;
	CComPtr<ISwATransport>	t ;
	CComPtr<IUnknown>		resUnk ;
	HRESULT hr = m_trn->QueryInterface(__uuidof(t), (void **)&t) ;
	if (FAILED(hr)) return hr ;

	hr = t->Receive(characterEncoding, &resUnk) ;
	if (FAILED(hr)) return hr ;
	hr = resUnk->QueryInterface(__uuidof(response), (void **)&response) ;
	if (FAILED(hr)) return hr ;

	CComBSTR contentType, lwrContentType ;
	t->get_ContentType(&contentType) ;
	lwrContentType  = contentType ;
	lwrContentType.ToLower() ;
	if ( StartsWith(lwrContentType, L"text/xml") )
		return PassThroughReceive ( response, Envelope ) ;
	else if ( StartsWith ( lwrContentType, L"application/dime") )
		p = CreatePackager(formatDime) ;
	else if ( StartsWith ( lwrContentType, L"multipart/related" ) ) 
		p = CreatePackager(formatMime) ;
	else
	{
		std::wstring er ( L"Unknown content-type of '" ) ;
		er += contentType ;
		er += L"' returned by server" ;
		return AtlReportError ( GetObjectCLSID(), er.c_str(), IID_NULL, E_INVALID_CONTENTTYPE ) ;
	}
	return p->Receive(response, contentType, m_res, characterEncoding, Envelope ) ;
}

STDMETHODIMP CManager::get_DiskThreshold(long *pVal)
{
	if ( ! pVal ) return E_POINTER ;
	*pVal = m_sizeLimit ;
	return S_OK;
}

STDMETHODIMP CManager::put_DiskThreshold(long newVal)
{
	m_sizeLimit = newVal ;
	return S_OK;
}

STDMETHODIMP CManager::get_Format(AttachmentFormat *pVal)
{
	if ( ! pVal ) return E_POINTER ;
	*pVal = m_format ;
	return S_OK;
}

STDMETHODIMP CManager::put_Format(AttachmentFormat newVal)
{
	m_format = newVal ;
	CComQIPtr<ISoapAttachmentFormat> freq(m_req) ;
	freq->put_Format(m_format) ;
	CComQIPtr<ISoapAttachmentFormat> fres(m_res) ;
	fres->put_Format(m_format) ;

	return S_OK;
}

HRESULT CManager::CreateSoapMsgPart  ( BSTR Envelope, ISoapAttachment ** part )
{
	// build a utf-8 part for the SOAP Envelope
	stringBuff_A buff ;
	Ole2Utf8 ( Envelope, buff ) ;

	CComPtr<ISoapAttachment> soap ;
	HRESULT hr = soap.CoCreateInstance(__uuidof(CoSoapAttachment)) ;
	if (FAILED(hr)) return hr ;

	SAFEARRAYBOUND rga ;
	rga.lLbound = 0 ;
	rga.cElements = buff.Size() ;
	SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
	void * data = 0 ;
	SafeArrayAccessData(psa, &data ) ;;
	memcpy ( data, buff.c_str(), buff.Size() ) ;
	SafeArrayUnaccessData(psa) ;
	buff.Clear() ;
	CComVariant vArr ;
	vArr.vt = VT_UI1 | VT_ARRAY ;
	vArr.parray = psa ;
	soap->put_Body(vArr) ;

	return soap.CopyTo(part) ;
}
