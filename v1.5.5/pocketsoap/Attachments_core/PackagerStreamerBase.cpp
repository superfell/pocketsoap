// PackagerStreamerBase.cpp: implementation of the PackagerStreamerBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "attachments.h"
#include "PackagerStreamerBase.h"

static const long WRITE_HEADER = -1 ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template<class T>
PackagerStreamerImpl<T>::PackagerStreamerImpl() : m_pos(0)
{
}

template<class T>
PackagerStreamerImpl<T>::~PackagerStreamerImpl()
{
}

template<class T>
STDMETHODIMP PackagerStreamerImpl<T>::Read( void *pvx, ULONG cb, ULONG *pcbRead ) 
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

template<class T>
STDMETHODIMP PackagerStreamerImpl<T>::Reset()
{
	m_currentRecord = m_payload.begin() ;
	m_pos = WRITE_HEADER ;
	for ( PARTS::iterator i = m_payload.begin() ; i != m_payload.end() ; i++ )
		i->payload->Reset() ;

	return S_OK ;
}

template<class T>
void PackagerStreamerImpl<T>::CreatePart ( ISoapAttachment * a, PART &p )
{
	p.part = a ;
	CComQIPtr<IPartPayloadProvider> pp(a) ;
	p.payload.Release() ;
	pp->PartPayload(&p.payload) ;
	p.payload->Lock() ;
	p.last = false ;
}

template<class T>
BUFFER PackagerStreamerImpl<T>::AddCurrentHeader ( T* pT, BYTE * &pv, DWORD &size ) 
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
