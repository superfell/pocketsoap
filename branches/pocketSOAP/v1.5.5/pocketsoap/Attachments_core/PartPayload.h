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

//////////////////////////////////////////////////////////////////////
// PartPayload.h: interface for the Payload classes.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTPAYLOAD_H__2AC26877_9629_4EE5_B878_B058475DC43C__INCLUDED_)
#define AFX_PARTPAYLOAD_H__2AC26877_9629_4EE5_B878_B058475DC43C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ATL_NO_VTABLE FilePayload : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IPartPayload
{
public:
	FilePayload() : m_hf(INVALID_HANDLE_VALUE), m_lock(0) { } 
	~FilePayload()
	{
		CloseFile() ;
	}

BEGIN_COM_MAP(FilePayload)
	COM_INTERFACE_ENTRY(IPartPayload)
	COM_INTERFACE_ENTRY(IStreamReader)
END_COM_MAP()

public:
// IStreamReader
	STDMETHOD(Read)( void * pv, DWORD cb, DWORD * pcbRead ) ;
	STDMETHOD(Reset)() ;
// IPartPayload
	STDMETHOD(Lock)() ;	// this should lock the underlying resource, so it can't change size
	STDMETHOD(Unlock)() ;
	STDMETHOD(Size)(DWORD * cb) ;	

// class
	void Init ( BSTR fileName ) ;

private:
	CComBSTR	m_fileName ;
	HANDLE		m_hf ;
	long		m_lock ;

	HRESULT CloseFile() ;
	HRESULT OpenFile()  ;
};

class ATL_NO_VTABLE MemoryPayload : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IPartPayload
{
public:
	MemoryPayload() : m_psa(0), m_pos(0), m_data(0), m_size(0) { }

BEGIN_COM_MAP(MemoryPayload)
	COM_INTERFACE_ENTRY(IPartPayload)
	COM_INTERFACE_ENTRY(IStreamReader)
END_COM_MAP()

public:
// IStreamReader
	STDMETHOD(Read)( void * pv, DWORD cb, DWORD * pcbRead ) ;
	STDMETHOD(Reset)() ;
// IPartPayload
	STDMETHOD(Lock)() ;	// this should lock the underlying resource, so it can't change size
	STDMETHOD(Unlock)() ;
	STDMETHOD(Size)(DWORD * cb) ;	

// class
	void Init ( SAFEARRAY * psa ) ;

private:
	SAFEARRAY * m_psa ;
	DWORD		m_pos ;

	BYTE *		m_data ;
	DWORD		m_size ;
};

#endif // !defined(AFX_PARTPAYLOAD_H__2AC26877_9629_4EE5_B878_B058475DC43C__INCLUDED_)
