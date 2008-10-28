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

#pragma once
#include "stringBuff.h"

class idMgr
{
public:
	idMgr() : m_id(0) { }

	const stringBuff_W & nextID() 
	{
		static const span = 26 ;
		long i = m_id++ ;
		m_txt.Allocate(i / span + 1 ) ;
		m_txt.Clear() ;
		WCHAR t[2] = { 0, 0 } ;
		while ( true )
		{
			t[0] = 'a' + ( i % span ) ;
			m_txt.Append ( t, 1 ) ;
			if ( i < span )
				break ;
			i = i / span -1;
		}
		return m_txt ;
	}

private:
	long			m_id ;
	stringBuff_W	m_txt ;
} ;