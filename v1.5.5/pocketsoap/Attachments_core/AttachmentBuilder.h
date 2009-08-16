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

// $Header: c:/cvs/pocketsoap/pocketsoap/Attachments_core/AttachmentBuilder.h,v 1.2 2005/01/03 19:44:24 simon Exp $

#pragma once

// this is a class that acculates payload data, and buffer it to memory or disk depending on its size
// once you've finished accumulating data, you call generate to build an AttachmentObject.

class AttachmentBuilder
{
public:
	AttachmentBuilder(DWORD sizeLimit) : m_sizeLimit(sizeLimit), toDisk(false), hFile(INVALID_HANDLE_VALUE)
	{
	}

	~AttachmentBuilder()
	{
		if ( INVALID_HANDLE_VALUE != hFile )
			CloseHandle(hFile) ;
	}

	HRESULT Generate ( std::string &id, bool IdIsUri, std::string &typeName, TypeNameFormat tnf, AttachmentFormat fmt, ISoapAttachment ** newAttachment ) 
	{
		USES_CONVERSION ;
		CComPtr<ISoapAttachment> att ;
		att.CoCreateInstance(CLSID_CoSoapAttachment) ;
		att->put_Format(fmt) ;

		CComBSTR bstrId ;
		bstrId.Attach ( A2BSTR(id.c_str()) ) ;
		if ( IdIsUri )
			att->put_Uri(bstrId) ;
		else
			att->put_ContentId(bstrId) ;

		CComBSTR tn ;
		tn.Attach ( A2BSTR(typeName.c_str()) ) ;
		att->put_TypeName(tn) ;

		att->put_TypeNameFormat(tnf) ;

		AddPayloadToAttachment(att) ;
		return att->QueryInterface(__uuidof(*newAttachment), (void **)newAttachment) ;
	}

	HRESULT AddPayloadToAttachment(CComPtr<ISoapAttachment> &att )
	{
		USES_CONVERSION ;
		if ( toDisk )
		{
			CloseHandle ( hFile ) ;
			hFile = INVALID_HANDLE_VALUE ;
			CComVariant v ;
			v.vt = VT_BSTR ;
			v.bstrVal = A2BSTR(fileName.c_str()) ;
			att->put_Body(v) ;			
		}
		else
		{
			SAFEARRAYBOUND rga ;
			rga.lLbound = 0 ;
			rga.cElements = payload.Size() ;
			SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
			BYTE * data = 0 ;
			SafeArrayAccessData ( psa, (void **)&data ) ;
			memcpy ( data, payload.c_str(), payload.Size() ) ;
			SafeArrayUnaccessData ( psa ) ;

			payload.Clear() ;

			CComVariant vArr ;
			vArr.vt = VT_ARRAY | VT_UI1 ;
			vArr.parray = psa ;
			att->put_Body(vArr) ;
		}
		toDisk = false ;
		return S_OK ;
	}

	void AddToPayload ( const BYTE * buff, DWORD len ) 
	{
		ATLTRACE("AddToPayload buff=0x%08x, len=0x%x\n", buff, len);
		if (len == 0) return;
		USES_CONVERSION ;
		DWORD cbWritten ;
		if ( (! toDisk) && ( len + payload.Size() > m_sizeLimit ) )
		{
			// switch from in memory to on disk
			toDisk = true ;
			static const DWORD cbPath = MAX_PATH  ;
			TCHAR path[cbPath] , file[cbPath] ;
			GetTempPath ( cbPath, path ) ;
			GetTempFileName ( path, _T("ps"), 0, file ) ;
			hFile = CreateFile ( file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL ) ;
			// write out current buffer
			WriteFile ( hFile, payload.c_str(), payload.Size(), &cbWritten, NULL ) ;
			payload.Clear() ;
			fileName.Clear() ;
			fileName.Append(T2A(file)) ;
			toDisk = true ;
		}
		if ( toDisk )
			WriteFile ( hFile, buff, len, &cbWritten, NULL ) ;
		else
			payload.Append ( (const char *)buff, len ) ;
	}

private:
	stringBuff_A	payload ;
	stringBuff_A	fileName ;
	DWORD			m_sizeLimit ;
	bool			toDisk ;
	HANDLE			hFile ;
};
