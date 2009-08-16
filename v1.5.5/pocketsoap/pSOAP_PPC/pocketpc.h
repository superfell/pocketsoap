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
Portions created by Simon Fell are Copyright (C) 2000-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/


// $Header: c:/cvs/pocketsoap/pocketsoap/pSOAP_PPC/pocketpc.h,v 1.3 2005/08/18 05:12:47 simon Exp $
//
// This contains some implementations that's for pocketPC that are pre-canned in Win32

#pragma once

HRESULT CoCreateGuid ( GUID * pguid ) ;

wchar_t * _i64tow ( __int64 i64, wchar_t * buff, int radix );