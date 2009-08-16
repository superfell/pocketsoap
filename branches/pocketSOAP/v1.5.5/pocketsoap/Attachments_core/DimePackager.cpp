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

// DimePackager.cpp: implementation of the DimePackager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "attachments.h"
#include "DimePackager.h"
#include "contentId.h"
#include "AttachmentBuilder.h"


//////////////////////////////////////////////////////////////////////
// StreamToBuffer
//////////////////////////////////////////////////////////////////////
HRESULT StreamToBuffer ( IStreamReader * s, BYTE * buff, DWORD cbToRead )
{
	DWORD cb = 0 ;
	BYTE * pos = buff ;
	HRESULT hr = S_OK ;
	while ( cbToRead )
	{
		hr = s->Read ( pos, cbToRead, &cb ) ;
		if (FAILED(hr)) return hr ;
		if (cb == 0) return AtlReportError( CLSID_CoAttachmentManager, OLESTR("The transport said there was no more data, but we were expecting more."), IID_NULL, HRESULT_FROM_WIN32(ERROR_HANDLE_EOF));
		pos += cb ;
		cbToRead -= cb ;
	}
	return hr ;
}

//////////////////////////////////////////////////////////////////////
// PartToArray
//////////////////////////////////////////////////////////////////////
HRESULT PartToArray ( ISoapAttachment * part, SAFEARRAY ** ppsa )
{
	HRESULT hr ;
	AttachmentLocation loc ;
	part->get_Located(&loc) ;
	if ( attOnDisk == loc )
	{
		CComQIPtr<IPartPayloadProvider> ppp(part) ;
		CComPtr<IPartPayload> pp ;
		ppp->PartPayload(&pp) ;

		pp->Lock() ;
		DWORD size = 0 ;
		pp->Size(&size) ;
		SAFEARRAYBOUND rga ;
		rga.cElements = size ;
		rga.lLbound = 0 ;
		SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
		BYTE * data = 0 ;
		SafeArrayAccessData ( psa, (void **)&data ) ;
		hr = StreamToBuffer ( pp, data, size ) ;
		SafeArrayUnaccessData ( psa ) ;
		pp->Unlock() ;
		*ppsa = psa ;
	}
	else
	{
		VARIANT vdata;
		hr = part->get_Body(&vdata) ;
		if ( SUCCEEDED(hr))
			*ppsa = vdata.parray ;
	}
	return hr ;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DimePackager::DimePackager() 
{
}

DimePackager::~DimePackager()
{
}

HRESULT DimePackager::Init ( long sizeLimit ) 
{
	m_sizeLimit = sizeLimit ;
	return S_OK ;
}

HRESULT DimePackager::PackageAndSend ( BSTR endpoint, ISoapAttachment * envelope, ISoapAttachments * parts, ISOAPTransport * transport ) 
{
	USES_CONVERSION ;

	static CComBSTR typeName(L"http://schemas.xmlsoap.org/soap/envelope/") ;
	envelope->put_TypeName(typeName) ;
	envelope->put_TypeNameFormat(tnfAbsoluteURI) ;

	long numParts = 0 ;
	parts->get_Count(&numParts) ;
	m_payload.reserve(numParts+1) ;

	PART part ;
	CreatePart(envelope, part) ;
	if ( numParts == 0 )
		part.last = true ;

	m_payload.push_back(part) ;

	// add the rest of the attachments
	CComPtr<ISoapAttachment> thePart ;
	for ( long i = 0 ; i < numParts ; i++ ) 
	{
		parts->get_Item(i, &thePart) ;
		CreatePart(thePart, part) ;
		if ( i+1 == numParts )
			part.last = true ;
		m_payload.push_back(part) ;
		thePart.Release() ;
	}
	
	CComVariant vStm (GetUnknown()) ;
	HRESULT hr = Reset() ;
	if (FAILED(hr)) return hr;

	CComQIPtr<ISwATransport> swa(transport) ;
	static CComBSTR DIME_CONTENT_TYPE(OLESTR("application/dime")) ;
	swa->put_ContentType(DIME_CONTENT_TYPE) ;

	hr = swa->Send ( endpoint, vStm ) ;

	for ( PARTS::iterator it = m_payload.begin(); it != m_payload.end() ; it++ )
		it->payload->Unlock() ;

	return hr ;
}

BUFFER DimePackager::CreateCurrentHeader() 
{
	CComBSTR ct, id ;
	TypeNameFormat tnf ;
	m_currentRecord->part->get_TypeName(&ct) ;
	m_currentRecord->part->get_Uri(&id) ;
	m_currentRecord->part->get_TypeNameFormat(&tnf) ;

	DWORD payloadsize ;
	m_currentRecord->payload->Size(&payloadsize) ;
	return Dime::HeaderBuilder::BuildHeader( m_currentRecord == m_payload.begin(), false, m_currentRecord->last, tnf, ct, id, payloadsize ) ;
}

// return true when this is the last chunk for the current record
bool DimePackager::AddNextChunk ( BYTE * &pv, DWORD &size ) 
{
	unsigned char pad[4] = {0};

	DWORD cb ;
	m_currentRecord->payload->Read ( pv, size, &cb ) ;

	pv += cb ;
	size -= cb ;
	m_pos += cb ;

	// handle padding
	if ( ( cb == 0 ) && ( m_pos % 4 > 0 ) )
	{
		u_int padSize = 4 - ( m_pos % 4 ) ;
		int ck = min ( padSize, size ) ;
		memcpy ( pv, pad, ck ) ;
		pv += ck ;
		size -= ck ;
		padSize -= ck ;
		if ( padSize > 0 )
		{
			// catch it next time around
			m_readBuffer.insert ( m_readBuffer.end(), pad, pad + padSize ) ;
		}	
	}
	return ( cb == 0 ) ;
} 

HRESULT DimePackager::Receive ( IStreamReader * response, BSTR ContentType, ISoapAttachments * parts, /*[in,out]*/ BSTR * characterEncoding, /*[out,retval]*/ SAFEARRAY ** Envelope ) 
{
	HRESULT hr = E_FAIL ;
	static const DWORD buff_size = 8192 ;
	BYTE buff[buff_size] ;

	AttachmentBuilder builder(m_sizeLimit) ;
	Dime::DimeHeader hdr, chunkedHdr ;
	CComPtr<ISoapAttachment> newPart ;
	DWORD chunk, padding ;
	bool chunking = false ;
	// we can't just read whatever's available, as we might overrun the end
	// of this message, and get the start of the next
	// so we read the header, then work out how much more to read, then if its
	// not the last record in the set, we read the next RECORD
	do
	{
		// read and parse the first part of the RECORD header
		hr = StreamToBuffer ( response, buff, Dime::HEADER_SIZE ) ;
		if (FAILED(hr)) return hr;
		Dime::HeaderReader::ParseFixedHeader ( buff, hdr ) ;

		// check that the RECORD is of a suported version number.
		if ( ! hdr.VersionIsOk() )
			return AtlReportError ( CLSID_CoAttachmentManager, OLESTR("Received a DIME record with an unsupported version #"), IID_NULL, E_UNEXPECTED ) ;

		// read and parse the rest of the RECORD header
		hr = StreamToBuffer ( response, buff + Dime::HEADER_SIZE, hdr.HeaderSize(false) ) ;
		if (FAILED(hr)) return hr ;

		Dime::HeaderReader::Parse ( buff, hdr ) ;
		if (( ! chunking ) && ( hdr.chunked ))
		{
			// starting chunking, keep a copy of the first RECORD header in the chunked set
			chunkedHdr = hdr ;
			chunking = true ;
		}
		
		// how much trailing padding is there going to be ?
		padding = Dime::GetAlignedSize(hdr.length) - hdr.length ;

		// stream in the RECORD body, and let the attachment builder accumulate it
		while ( hdr.length )
		{
			chunk = min ( buff_size, hdr.length ) ;
			hr = StreamToBuffer ( response, buff, chunk ) ;
			if (FAILED(hr)) return hr ;
			builder.AddToPayload ( buff, chunk ) ;
			hdr.length -= chunk ;
		}
		// eat record trailing padding
		hr = StreamToBuffer ( response, buff, padding ) ;

		// have hdr & payload at this point
		if ( ! hdr.chunked )
		{
			Dime::DimeHeader &h = chunking ? chunkedHdr : hdr ;
			builder.Generate ( h.id, true, h.typeName, h.typeNameFormat, formatDime, &newPart ) ;
			parts->Append ( newPart ) ;
			newPart.Release() ;
			chunking = false ;
		}

	} while ( ! hdr.messageEnd ) ;

	long count =0 ;
	parts->get_Count(&count) ;
	if ( count > 0 )
	{
		// its possible that the SOAP part was large enough that it got
		// streamed to disk, so we go through the partProvider interface
		// to get the contents of that part
		CComPtr<ISoapAttachment> primary ;
		parts->get_Item(0, &primary) ;
		hr = PartToArray ( primary, Envelope ) ;
	}
	SysFreeString(*characterEncoding) ;
	*characterEncoding = 0 ;
	return hr ;
}

