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

// PackagerStreamerBase.h: interface for the PackagerStreamerBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PACKAGERSTREAMERBASE_H__BB29A39F_D409_44C8_9E76_EAFEB968343A__INCLUDED_)
#define AFX_PACKAGERSTREAMERBASE_H__BB29A39F_D409_44C8_9E76_EAFEB968343A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

static const long WRITE_HEADER = -1 ;
typedef std::vector<unsigned char>	BUFFER ;

template<class T>
class PackagerStreamerImpl : public IStreamReader  
{
public:
	PackagerStreamerImpl() : m_pos(0) { }
	virtual ~PackagerStreamerImpl() { }

// IStreamReader
	STDMETHODIMP Read( void *pvx, ULONG cb, ULONG *pcbRead )
	{
		BYTE * pv = (BYTE *)pvx ;
		*pcbRead = cb ;
		DWORD cs = 0 ;
		if ( m_readBuffer.size() > 0 )
		{
			cs = min ( m_readBuffer.size(), cb ) ;
			memcpy ( pv, &m_readBuffer[0], cs ) ;
			cb -= cs ;
			pv += cs ;
			m_readBuffer.erase ( m_readBuffer.begin(), m_readBuffer.begin() + cs ) ;
		}

		T * pT = static_cast<T*>(this) ;
		while ( cb > 0 && (m_currentRecord != m_payload.end() ) )
		{
			if ( m_pos == WRITE_HEADER )
				AddCurrentHeader(pT, pv, cb) ;
			else
			{
				if ( pT->AddNextChunk ( pv, cb ) )
				{
					m_currentRecord++ ;
					m_pos = WRITE_HEADER ;
				}
			}
		}
		*pcbRead -= cb ;
		return S_OK ;
	}

	STDMETHODIMP Reset()
	{	
		m_currentRecord = m_payload.begin() ;
		m_pos = WRITE_HEADER ;
		HRESULT hr = S_OK ;
		for ( PARTS::iterator i = m_payload.begin() ; (i != m_payload.end()) && SUCCEEDED(hr); i++ )
			hr = i->payload->Reset() ;

		return hr ;
	}


protected:
	typedef struct
	{
		CComPtr<ISoapAttachment>	part ;
		CComPtr<IPartPayload>		payload ;
		bool						last ;
	} PART ;
	typedef std::vector<PART>		PARTS ;

	BUFFER						m_readBuffer ;

	PARTS						m_payload ;
	PARTS::iterator				m_currentRecord ;
	u_long						m_pos ;

	void CreatePart ( ISoapAttachment * a, PART &p ) 
	{
		p.part = a ;
		CComQIPtr<IPartPayloadProvider> pp(a) ;
		p.payload.Release() ;
		pp->PartPayload(&p.payload) ;
		p.payload->Lock() ;
		p.last = false ;
	}

	void AddCurrentHeader ( T * pT, BYTE *& pv, DWORD &size )
	{
		BUFFER hdr = pT->CreateCurrentHeader() ;

		DWORD ck = min ( size, hdr.size() ) ;
		memcpy ( pv, &hdr[0], ck ) ;
		pv += ck ;
		size -= ck ;

		// add the left over to the readBuffer, for next time around
		if ( ck < hdr.size() )
			m_readBuffer.insert ( m_readBuffer.end(), &hdr[ck], &hdr[hdr.size()] ) ;

		m_pos = 0 ;
	}
};

// couple of helpers that get used in various places
HRESULT StreamToBuffer ( IStreamReader * s, BYTE * buff, DWORD cbToRead ) ;
HRESULT PartToArray    ( ISoapAttachment * part, SAFEARRAY ** ppsa ) ;


#endif // !defined(AFX_PACKAGERSTREAMERBASE_H__BB29A39F_D409_44C8_9E76_EAFEB968343A__INCLUDED_)
