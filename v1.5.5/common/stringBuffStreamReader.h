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


#pragma once

// this provides an IResetableStream impl over a stringBuff_A based buffer
class ATL_NO_VTABLE stringBuffStreamReader :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IResetableStream
{
public:
	stringBuffStreamReader() : m_pos(0) { }

BEGIN_COM_MAP(stringBuffStreamReader)
	COM_INTERFACE_ENTRY(IResetableStream)
END_COM_MAP()

// IStreamReader
	STDMETHOD(Read)( void *pv,		// Pointer to buffer into which the stream is read
					 ULONG cb,			// Specifies the number of bytes to read
					 ULONG *pcbRead )	// Pointer to location that contains actual
										// number of bytes read
	{
		if ( ! pv ) return E_POINTER ;
		if ( ! pcbRead ) return E_POINTER ;

		*pcbRead = min ( cb, m_buff.Size() - m_pos ) ;
		memcpy ( pv, &m_buff.c_str()[m_pos], *pcbRead ) ;
		m_pos += *pcbRead ;
		return m_pos >= m_buff.Size() ? S_FALSE : S_OK ;
	}

	STDMETHOD(Reset)()
	{
		m_pos = 0 ;
		return S_OK ;
	}

// class
	stringBuff_A & Buffer()
	{
		return m_buff ;
	}

private:
	stringBuff_A  m_buff ;
	u_long		  m_pos ;
} ;
