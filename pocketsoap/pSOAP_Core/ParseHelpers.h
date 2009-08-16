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
Portions created by Simon Fell are Copyright (C) 2009
Simon Fell. All Rights Reserved.

Contributor(s):
*/
#if !defined(AFX_PARSEHELPERS_H__70B7FB43_E951_4A0C_B938_AB79346C323D__INCLUDED_)
#define AFX_PARSEHELPERS_H__70B7FB43_E951_4A0C_B938_AB79346C323D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// pass it 50.5 get back 500, pass it 0.123 get back 123
int parseMilliseconds(const WCHAR *p);


#endif // !defined(AFX_PARSEHELPERS_H__70B7FB43_E951_4A0C_B938_AB79346C323D__INCLUDED_)
