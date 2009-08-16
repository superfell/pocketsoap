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

#include "stdafx.h"
#include "ParseHelpers.h"

int parseMilliseconds(const WCHAR *p) {
	if (p == NULL) return 0;
	WCHAR ms[4] = L"000";	// we need to pad it out with zero's to handle someone passing 60.5, need to get 500, not 5.
	// find the decimalpoint if there is one.
	const WCHAR *decimalPoint = wcschr(p, '.');
	if (!decimalPoint || !*(decimalPoint+1)) return 0;
	// copy upto the next 3 chars to the local array
	++decimalPoint;
	int idx = 0;
	while (*decimalPoint >= '0' && *decimalPoint <= '9') {
		ms[idx++] = *decimalPoint;
		decimalPoint++;
		if (idx == 3) break;
	}
	return _wtoi(ms);
}
