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
Portions created by Simon Fell are Copyright (C) 2000,2003-2004
Simon Fell. All Rights Reserved.

Contributor(s):
*/

//////////////////////////////////////////////////////////////////////
// stringBuff.h: interface for the stringBuff class.
//
// stringBuff is a simple string buffer template class that
// over allocates in an attempt to reduce heap contention
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGBUFF_H__E89A62FE_2F7B_4F0D_9348_3FD0745FDDB0__INCLUDED_)
#define AFX_STRINGBUFF_H__E89A62FE_2F7B_4F0D_9348_3FD0745FDDB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "hrtrace.h"

template<class T>
class stringBuff  
{
public:
	stringBuff() : m_s(0), m_pos(0), m_alloc(0)  { }

	// these 3 are deprecated, because they hide any error that Append might return */
	stringBuff(const stringBuff &rhs) : m_s(0), m_pos(0), m_alloc(0)
	{
		Append ( rhs ) ;
	}
	
	stringBuff(const T* rhs) : m_s(0), m_pos(0), m_alloc(0)
	{
		Append ( rhs ) ;
	}

	stringBuff & operator = ( const stringBuff &rhs )
	{
		Clear() ;
		Append ( rhs ) ;
		return *this ;
	}

	~stringBuff()
	{	
		if ( m_s )
			free(m_s) ;
	}

	bool operator == ( const stringBuff &rhs ) const
	{
		return strcompare ( c_str(), rhs.c_str() )   ;
	}

	// clears the string buffer, but doesn't free it
	void Clear()
	{
		if ( m_s )
		{
			m_s[0] =0 ;
			m_pos = 0 ;
		}
	}

	// this forces us to de-allocate any memory we don't need
	void ReleaseMemory()
	{
		if ( m_s )
		{
			// we don't have any string at all, we can complete dump the buffer
			if ( 0 == m_pos )
			{
				free(m_s);
				m_s = 0 ;
				m_alloc = 0 ;
			}
			else
			{
				m_alloc = m_pos + 1 ;
				m_s = (T *)realloc(m_s, m_alloc) ;
			}
		}
	}

	// make the underlying buffer at least minSize (in sizeof(T))
	HRESULT Allocate(size_t minSize)
	{
		if ( minSize <= m_alloc )
			return S_OK;

		size_t newAlloc = m_alloc * 2 ;
		if ( newAlloc < minSize )
			newAlloc = minSize ;

		m_s = (T *)realloc(m_s, newAlloc * sizeof(T) ) ;
		if(!m_s)
			return E_OUTOFMEMORY;

		m_alloc = newAlloc ;
		return S_OK;
	}

	HRESULT Append(const T * str)
	{
		if(str == NULL)
			return S_OK;
		return Append(str, stringLength(str)) ;
	}

	HRESULT Append(const T * str, size_t len)
	{
		if ( ! len )
			return S_OK;
		size_t reqSize = (m_pos + len + 1) ;
		_HR(Allocate(reqSize));
		memcpy(&m_s[m_pos], str, len * sizeof(T) ) ;
		m_pos += len ;
		m_s[m_pos] = 0 ;
		return S_OK;
	}
	
	HRESULT Append(const stringBuff<T> &rhs )
	{
		return Append ( rhs.c_str(), rhs.Size() ) ;
	}

	// removes [begin,end) and shuffles remaining contents down
	void erase(size_t begin, size_t end)
	{
		ATLASSERT ( begin < end && "stringBuff::erase() end is before begin !" ) ;
		if ( begin > m_pos ) begin = m_pos ;
		if ( end > m_pos ) end = m_pos ;
		memmove ( &m_s[begin], &m_s[end], (m_pos - end ) * sizeof(T) ) ;
		m_pos -= ( end - begin ) ;
		m_s[m_pos] = 0 ;
	}

	size_t Size() const
	{
		return m_pos ;
	}

	const T * c_str(void) const
	{
		return m_s ? m_s : NullString((T*)0) ;
	}

	// this lets you mess with the buffer contents directly if you really must
	HRESULT Size(size_t newSize)
	{
		_HR(Allocate(newSize+1));
		m_pos = newSize ;
		m_s[m_pos] = 0 ;
		return S_OK;
	}

	T * buffer(void)
	{
		return m_s ;
	}

	void ToLower(void)
	{
		if (m_s)
			ToLower(m_s) ;
	}

private:
	T		*m_s ;
	size_t	m_alloc, m_pos ;

	WCHAR * NullString(const WCHAR * x) const
	{
		return L"" ;
	}
	
	char * NullString(char * x) const
	{
		return "" ;
	}

	size_t stringLength(const char * str) const
	{
		return strlen(str) ;
	}

	size_t stringLength(const WCHAR * str) const 
	{
		return wcslen(str) ;
	}

	void ToLower(char * str)
	{
		_strlwr(str) ;
	}

	void ToLower(wchar_t * str)
	{
		__wcslwr(str) ;
	}

	bool strcompare ( const char * a, const char * b) const 
	{
		return ( strcmp ( a, b ) == 0 ) ;
	}

	bool strcompare ( const wchar_t * a, const wchar_t * b) const 
	{
		return ( wcscmp ( a, b ) == 0 ) ;
	}
};

typedef stringBuff<char>	stringBuff_A ;
typedef stringBuff<wchar_t> stringBuff_W ;
typedef stringBuff<TCHAR>	stringBuff_T ;

//////////////////////////////////////////////////////////////////////
// arrayOfString
//////////////////////////////////////////////////////////////////////
template<class T>
class arrayOfString
{
private:
	typedef stringBuff<T>  STRING ;
	typedef std::vector<STRING *> ITEMS ;
	ITEMS m_items ;

public:
	arrayOfString() { }
	~arrayOfString()
	{
		RemoveAll() ;
	}

	STRING * Add()
	{
		STRING * p = new STRING ;
		m_items.push_back(p) ;
		return p ;
	}

	STRING * Add(const STRING &rhs)
	{
		STRING * p = new STRING(rhs) ;
		m_items.push_back(p) ;
		reutrn p ;
	}

	size_t GetSize() const 
	{
		return m_items.size() ;
	}

	STRING * operator [] (const long index) const
	{
		return m_items[index] ;
	}

	void RemoveAt(const long index)
	{
		STRING * p = m_items[index] ;
		m_items.erase(m_items.begin() + index) ;
		delete p ;
	}

	void RemoveAll()
	{
		for ( unsigned long i = 0 ; i < m_items.size() ; ++i )
			delete m_items[i] ;
		m_items.clear() ;
	}

};

#endif // !defined(AFX_STRINGBUFF_H__E89A62FE_2F7B_4F0D_9348_3FD0745FDDB0__INCLUDED_)
