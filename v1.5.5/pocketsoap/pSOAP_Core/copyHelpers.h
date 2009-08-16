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

#if !defined(AFX_OPYHELERS_H__FCB63C47_D33E_4234_9988_54484AD5A8F2__INCLUDED_)
#define AFX_OPYHELERS_H__FCB63C47_D33E_4234_9988_54484AD5A8F2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


HRESULT TypedCopyHelper ( byte * pDest, const VARTYPE vt, const VARIANT * src  ) ; 
HRESULT TypedCopyHelper ( VARIANT * pDest, const VARTYPE vt, const byte * psrc ) ;






#endif // !defined(AFX_OPYHELERS_H__FCB63C47_D33E_4234_9988_54484AD5A8F2__INCLUDED_)
