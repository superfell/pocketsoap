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

// MimePackager.cpp: implementation of the MimePackager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Attachments.h"
#include "MimePackager.h"
#include "stringHelpers.h"
#include "..\psoap_core\headerHelpers.h"
#include "attachmentBuilder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
MimePackager::MimePackager()
{
}

MimePackager::~MimePackager()
{
}

HRESULT MimePackager::Init ( long sizeLimit )
{
	m_sizeLimit = sizeLimit ;
	return S_OK ;
}

// look at the payload, to make sure the generated boundary doesn't exist (otherwise all hell will break loose)
bool MimePackager::validBoundary ( stringBuff_A &boundary, ISoapAttachment * envelope, ISoapAttachments * parts )
{
	if ( boundaryInPart ( boundary, envelope ) )
		return false ;

	CComPtr<ISoapAttachment> part ;
	long count = 0 ;
	parts->get_Count(&count) ;
	for ( long idx =0 ; idx < count ; idx++ )
	{
		parts->get_Item(idx, &part ) ;
		if ( boundaryInPart ( boundary, part ) )
			return false ;
		part.Release() ;
	}
	return true ;
}

// look to see if the boundary exists in this part
bool MimePackager::boundaryInPart ( stringBuff_A &boundary, ISoapAttachment * part )
{
	CComQIPtr<IPartPayloadProvider> ppp(part) ;
	if ( ! ppp ) return false ;

	CComPtr<IPartPayload> p ;
	ppp->PartPayload(&p) ;

	DWORD cb = 0 ;
	bool match = false ;
	static const DWORD buff_size = 8192 ;
	BYTE buff[buff_size] ;
	BYTE *endChunk, *pos = buff ;
	DWORD cbEnd, space = buff_size -1 ;
	p->Lock() ;
	const char * boundaryStart = boundary.c_str() ;
	const char * boundaryEnd   = boundaryStart + boundary.Size() ;
	do
	{
		p->Read ( pos, space, &cb ) ;
		if ( std::search ( buff, pos + cb, boundaryStart, boundaryEnd ) != ( pos + cb ) )
		{
			match = true ;
			break ;
		}
		// now, the boundary could be split across two reads, so we shuffle
		// the end of the buffer down to the start, to catch this
		cbEnd = min ( boundary.Size(), cb ) ;
		endChunk = pos + cb - cbEnd ;
		memmove ( buff, endChunk, cbEnd ) ;
		pos = buff + cbEnd ;
		space = buff_size - ( pos - buff ) ;
	} while ( cb > 0 ) ;
	p->Unlock() ;
	return match ;
}

HRESULT MimePackager::PackageAndSend ( BSTR endpoint, ISoapAttachment * envelope, ISoapAttachments * parts, ISOAPTransport * transport )
{
	USES_CONVERSION ;

	CComQIPtr<ISwATransport> swa(transport) ;

	CComBSTR startId ;
	
	static const CComBSTR envCT(OLESTR("text/xml; charset=UTF-8")) ;
	envelope->put_TypeName(envCT) ;
	envelope->put_TypeNameFormat(tnfMediaType);
	envelope->put_Format(formatMime) ;
	envelope->get_ContentId(&startId) ;

	do
	{
		m_boundary = CreateBoundary() ;
	} while ( ! validBoundary ( m_boundary, envelope, parts ) ) ;

	const char * szFirstCid = OLE2A(startId) ;
	stringBuff_A ct ;
	ct << "Multipart/Related; boundary=\"" << m_boundary << "\"; type=\"text/xml\"; start=\"<" << szFirstCid << ">\"" ;
	CComBSTR bstrCT ;
	bstrCT.Attach ( A2BSTR(ct.c_str())) ;
	swa->put_ContentType(bstrCT) ;

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
	if (FAILED(hr)) return hr ;

	hr = swa->Send ( endpoint, vStm ) ;

	for ( PARTS::iterator it = m_payload.begin(); it != m_payload.end() ; it++ )
		it->payload->Unlock() ;

	return hr ;
}

BUFFER MimePackager::CreateCurrentHeader() 
{
	USES_CONVERSION ;
	CComBSTR id, ct ;
	m_currentRecord->part->get_ContentId(&id) ;
	m_currentRecord->part->get_TypeName(&ct) ;

	stringBuff_A b ;
	b << "\r\n--" << m_boundary << "\r\n" ;
	b << "Content-Type: " << OLE2A(ct) << "\r\n" ;
	b << "Content-Transfer-Encoding: binary\r\n" ;
	b << "Content-ID: <" << OLE2A(id) << ">\r\n\r\n" ;

	BUFFER rv ;
	rv.insert ( rv.end(), b.c_str(), b.c_str() + b.Size() ) ;
	return rv ;
}

bool MimePackager::AddNextChunk ( BYTE * &pv, DWORD &size )  
{
	DWORD cb ;
	m_currentRecord->payload->Read ( pv, size, &cb ) ;

	pv += cb ;
	size -= cb ;
	m_pos += cb ;

	// handle trailer on last part
	if ( ( cb == 0 ) && ( m_currentRecord->last ))
	{
		stringBuff_A trailer ;
		trailer << "\r\n--" << m_boundary << "--\r\n" ;
		u_int cbTrailer = trailer.Size() ;
		const char * sz = trailer.c_str() ;

		int ck = min ( cbTrailer, size ) ;
		memcpy ( pv, sz, ck ) ;
		pv += ck ;
		size -= ck ;
		cbTrailer -= ck ;
		sz += ck ;
		if ( cbTrailer > 0 )
		{
			// catch it next time around
			m_readBuffer.insert ( m_readBuffer.end(), sz, sz + cbTrailer ) ;
		}	
	}
	return ( cb == 0 ) ;
}

// this will move the part of the buffer we're still interested in, down to the start
// and do a read for some more data
HRESULT MimePackager::ReadToBuffer ( char * buff_start, DWORD buff_size, char *&start, char *&end, DWORD &buffSpace, IStreamReader * stream )
{
	// shuffle down, and read some more
	ATLASSERT ( end >= start );
	DWORD cb = end - start ;
	if (cb > 0)
		memmove ( buff_start, start, cb ) ;
	start = buff_start ;
	end = buff_start + cb ;
	buffSpace = buff_size - cb ;
	HRESULT hr = stream->Read ( end, buffSpace, &cb ) ;
	if (FAILED(hr)) return hr ;
	buffSpace -= cb ;
	end += cb ;
	return S_OK ;
}

HRESULT MimePackager::Receive ( IStreamReader * response, BSTR contentType, ISoapAttachments * parts, /*[in,out]*/ BSTR * characterEncoding, /*[out,retval]*/ SAFEARRAY ** Envelope  )
{
	SysFreeString(*characterEncoding) ;
	*characterEncoding = 0 ;

	STR_MAP ctTokens = UnpackContentType ( contentType ) ;

	STR_MAP::iterator boundary = ctTokens.find("boundary") ;
	if ( boundary == ctTokens.end() )
		return AtlReportError ( CLSID_CoAttachmentManager, OLESTR("Response missing required boundary attribute in content-type header"), IID_NULL, E_BOUNDARY_MISSING ) ;

	AttachmentBuilder builder(m_sizeLimit) ;
	static const DWORD buff_size = 8192 ;
	char buff[buff_size] ;
	DWORD cb = 0 ;
	HRESULT hr ; 
	bool atFirstBoundary = false ;
	char * endPos = buff ;
	char * bdy = 0, *eoh = 0 ;
	DWORD cbSpace = buff_size ;

	std::string theBoundary ("\r\n--") ;
	theBoundary += boundary->second ;
	int boundarySize = theBoundary.size() ;
	std::string termBoundary(theBoundary) ;
	termBoundary.append("--") ;
	int termBoundarySize = termBoundary.size() ;

	std::string cid, ct ;
	CComPtr<ISoapAttachment> newPart ;

	// find the first boundary
	do 
	{
		hr = response->Read ( endPos, cbSpace, &cb );
		if (FAILED(hr)) return hr ;
		cbSpace -= cb ;
		endPos += cb ;
		// for the first boundary, we don't care about the leading \r\n
		// as some servers don't send the very first \r\n (e.g. XmlBus)
		bdy = std::search ( buff, endPos, theBoundary.c_str() +2, theBoundary.c_str() + theBoundary.size() ) ;
	} while ( (bdy == endPos) && (cbSpace>0) ) ;

	if ( bdy == endPos )
		return AtlReportError ( CLSID_CoAttachmentManager, OLESTR("Response doesn't contain starting boundary within reasonable limits"), IID_NULL, E_BOUNDARY_NOTFOUND ) ;

	static const char * eohStart = "\r\n\r\n" ;
	static const char * eohEnd   = eohStart + 4 ;
	char * searchPoint = NULL;

	do
	{
		// find the end of the headers
		// bdy point to the start of the boundary, we can skip over it.
		bdy += boundarySize ;
		do
		{
			eoh = std::search ( bdy, endPos, eohStart, eohEnd ) ;
			if ( eoh == endPos )
			{
				hr = ReadToBuffer ( buff, buff_size, bdy, endPos, cbSpace, response ) ;
				if (FAILED(hr)) return hr ;
				if ( eoh == endPos ) break;
			}
			else 
			{
				break;
			}
		} while (cbSpace>0) ;
		if (eoh == endPos)
			return AtlReportError ( CLSID_CoAttachmentManager, OLESTR("Mime part headers are too big"), IID_NULL, E_UNEXPECTED ) ;

		*eoh = 0 ;
		hr = ParseMimePartHeader ( bdy, cid, ct ) ;
		if (FAILED(hr)) return hr ;

		searchPoint = eoh + 4 ;
		ATLASSERT ( searchPoint <= endPos && "searchPoint is bigger than endPos after parsing headers");

		do
		{
			// look for next boundary, which marks the end of this part 
			bdy = std::search ( searchPoint, endPos, theBoundary.c_str(), theBoundary.c_str() + boundarySize) ;
			if ( bdy != endPos)
				break ;
			else
				bdy = NULL ;

			// accumulate this bit
			// note that there may be the begining of the boundary at the end of the chunk, we can't tell for sure
			// until we've read the next part, so we accumulate everything except for the last x bytes in the buffer
			// where x == boundary size
			// see the notes in boundaryInPart for why we do this
			// if the buffer size is smaller than the size of a boundary, we shouldn't do anything
			if ( endPos - searchPoint > boundarySize  )
			{
				long lengthToAccumulate = endPos - searchPoint - boundarySize;
				ATLASSERT ( lengthToAccumulate > 0 );
				builder.AddToPayload( (BYTE *)searchPoint, lengthToAccumulate ) ;
				searchPoint = endPos - boundarySize ;
			}

			hr = ReadToBuffer ( buff, buff_size, searchPoint, endPos, cbSpace, response ) ;
			if (FAILED(hr)) return hr ;
			
		} while ( ! bdy ) ;

		// bdy points to the start of the terminating boundary for this part
		// remember to accumulate the last of the part
		if ( bdy > searchPoint )
			builder.AddToPayload ( (BYTE *)searchPoint, bdy - searchPoint ) ;

		// create the attachment object, and add it to the collection
		builder.Generate( cid, false, ct, tnfMediaType, formatMime, &newPart ) ;
		parts->Append ( newPart ) ;
		newPart.Release() ;

		// its possible that we're at the terminating boundary, but haven't read the trailing -- into the buffer yet.
		while ( (endPos - bdy) < termBoundarySize )
		{
			ATLTRACE(_T("extra read for term boundary"));
			hr = ReadToBuffer ( buff, buff_size, bdy, endPos, cbSpace, response ) ;
			if (FAILED(hr)) return hr ;
		}
	} while ( ! StartsWith ( bdy, termBoundary.c_str() ) ) ;

	// we've gotten a terminator boundary, but we may not of read the entire http response 
	// particularly when the server is using a chunked encoding.
	// so we read some more to ensure we've read the entire response, this ensures the 
	// connection can get re-used if the server supports persistent connections
	searchPoint = buff;
	endPos = buff;
	_HR (ReadToBuffer ( buff, buff_size, searchPoint, endPos, cbSpace, response ));


	// find the start part, this has the soap envelope in it.
	// if there's a start attribute in the HTTP content-type, we use that
	// otherwise, we assume the first part is the envelope
	CComPtr<ISoapAttachment> startPart ;
	STR_MAP::iterator start = ctTokens.find("start") ;
	if ( start != ctTokens.end() )
	{
		CComBSTR bstrStart ;
		bstrStart.Attach ( A2BSTR(start->second.c_str())) ;
		hr = parts->Find(bstrStart, &startPart) ;
		if (FAILED(hr))
			return AtlReportError ( CLSID_CoAttachmentManager, OLESTR("Unable to find specified start content-id"), IID_NULL, E_UNEXPECTED ) ;
	}
	else
	{
		hr = parts->get_Item(0, &startPart) ;
		if (FAILED(hr))
			return AtlReportError ( CLSID_CoAttachmentManager, OLESTR("Unable to find a part containing the soap envelope"), IID_NULL, E_START_MISSING ) ;
	}

	CComBSTR partCT ;
	startPart->get_TypeName(&partCT) ;
	STR_MAP partTokens = UnpackContentType ( partCT ) ;
	STR_MAP::iterator partCS = partTokens.find("charset") ;
	if ( partCS != partTokens.end() )
		*characterEncoding = A2BSTR ( partCS->second.c_str() ) ;

	hr = PartToArray ( startPart, Envelope ) ;
	return hr ;
}

HRESULT MimePackager::ParseMimePartHeader ( char * hdr, std::string &cid, std::string &ct )
{
	// todo: probably need to retreive a Content-Location if its set.
	USES_CONVERSION ;
	stringBuff_A contentType = headerHelpers::ExtractHeaderVal ( hdr, "Content-Type") ;
	stringBuff_A contentID   = headerHelpers::ExtractHeaderVal ( hdr, "Content-ID" ) ;
	stringBuff_A contentTE   = headerHelpers::ExtractHeaderVal ( hdr, "Content-Transfer-Encoding" ) ;
	contentTE.ToLower() ;
	if ( contentTE.Size() > 0 &&
		 strcmp ( contentTE.c_str(), "binary" ) != 0 && 
		 strcmp ( contentTE.c_str(), "8bit" )   != 0 &&
		 strcmp ( contentTE.c_str(), "7bit" )   != 0 )
	{
		stringBuff_W err(L"Sorry, only 7bit, 8bit & binary transfer encodings are supported (got '") ;
		err.Append ( A2W(contentTE.c_str())) ;
		err.Append ( L"')" ) ;
		if ( contentTE.Size() == 0 )
		{
			err.Append ( L" (part headers='") ;
			err.Append ( A2W(hdr) ) ;
			err.Append ( L"')" ) ;
		}
		return AtlReportError ( CLSID_CoAttachmentManager, err.c_str(), IID_NULL, E_ENCODING_NOT_SUP ) ;
	}

	ct = contentType.c_str() ;
	cid.erase() ;

	if ( contentID.Size() )
	{
		// check/strip wrapper < >
		if ( *contentID.c_str() == '<' )
		{
			stringBuff_A t ;
			t.Append ( contentID.c_str() + 1, contentID.Size() -2 );
			cid = t.c_str() ;
		}
		else
			cid = contentID.c_str() ;

	}
	return S_OK ;
}

stringBuff_A MimePackager::CreateBoundary()
{
	stringBuff_A b ;
	b.Allocate(40) ;
	b.Append ( "-Mime+-" ) ;
	char t[2] ;
	t[1] = 0 ;
    while (b.Size() < 35)
	{
        int ch = rand() % 62;
		if (ch < 10)
			t[0] = ch + '0' ;
        else if (ch < 36)
            t[0] = ch + 'a' - 10;
        else
            t[0] = ch + 'A' - 36;
		b.Append ( t, 1 ) ;
    }
	return b;
}

MimePackager::STR_MAP MimePackager::UnpackContentType ( BSTR contentType )
{
	// header format is Content-Type: foo/bar; attr1=value1; [attrib2=value2[;]]
	// we get the string starting at  foo/bar

	USES_CONVERSION ;
	STR_MAP tokens ;
	char * ct = OLE2A(contentType) ;
	char * start = ct ;
	char * comma = strchr( ct, ';' ) ;

	std::string tknName("Content-Type") ;
	std::string tknVal ;

	// first one is a special case
	if ( ! comma )
	{
		tokens[tknName] = start ;
		return tokens ;
	}
	tknVal.append ( start, comma - start ) ;
	trimWhitespace(tknVal) ;
	trimQuotes(tknVal) ;
	tokens[tknName] = tknVal ;

	while ( *comma )
	{
		start = comma + 1 ;
		eatIgnorableChars ( start ) ;
		comma = strchr ( start, ';' ) ;
		if ( comma == NULL )
			comma = start + strlen(start) ;
		char * equals = strchr ( start, '=' ) ;
		tknName.erase() ;
		tknVal.erase() ;
		if ( equals < comma )
		{
			tknName.append ( start, equals - start ) ;	
			tknVal.append ( equals +1, comma - equals -1 ) ;	
		}
		else
			tknName.append ( start, comma - start ) ;

		trimWhitespace(tknName) ;
		trimWhitespace(tknVal) ;
		trimQuotes(tknVal) ;

		tokens[tknName] = tknVal ;
	}

	return tokens ;
}
