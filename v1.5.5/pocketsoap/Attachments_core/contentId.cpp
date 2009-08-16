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

#include "stdafx.h"
#include "Attachments.h"
#include "contentId.h"

void CreateContentId(AttachmentFormat f, CComBSTR &cid) 
{
	GUID g ;
	CoCreateGuid(&g) ;
	WCHAR b[50] ;
	StringFromGUID2(g, b, 50 ) ;
	cid.Empty() ;
	cid.Attach ( SysAllocStringLen(b+1, wcslen(b)-2));

	if ( f == formatMime )
		cid.Append ( L"@swa.pocketsoap.com" ) ;
}
