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

// reportErrorImpl.h
// this is a mix-in class, for returning detailed COM errors
//

#pragma once

template<class T>
class reportErrorImpl
{
public:
	HRESULT _cdecl ReportError ( HRESULT hr, LPCWSTR errorString, ...)
	{
		va_list args;
		va_start(args, errorString);

		int nBuf;
		WCHAR szBuffer[512];

		nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer) / sizeof(WCHAR), errorString, args);
		ATLASSERT(nBuf < sizeof(szBuffer));//Output truncated as it was > sizeof(szBuffer)
		va_end(args);

		return AtlReportError ( static_cast<T *>(this)->GetObjectCLSID(), szBuffer, GUID_NULL, hr ) ;
	}
};

