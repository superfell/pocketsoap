/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketXML-RPC

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2002
Simon Fell. All Rights Reserved.

Contributor(s):
     Jim T. Row - Dallas TX

*/


#pragma once

static const LCID serLCID = MAKELCID ( MAKELANGID ( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT ) ;

typedef struct value
{
	value() { }
	value(const WCHAR * elemName) : name(elemName) { }
	CComVariant		val ;
	std::wstring	name ;
} value ;

class IValueDeserializer
{
public:
	virtual ~IValueDeserializer() { }
	virtual HRESULT Init ( const XML_Char * name ) = 0 ;
	virtual HRESULT Characters ( std::wstring &chardata ) = 0 ;
	virtual HRESULT Child ( value &child ) = 0 ;
	virtual HRESULT Done () = 0 ;
	virtual value   Value() = 0;
} ;

class containerDeserializer : public IValueDeserializer
{
public:
	containerDeserializer() { }
	virtual HRESULT Init ( const XML_Char * name )  
	{
		return S_OK ;
	}
	virtual HRESULT Characters ( std::wstring &chardata ) 
	{
		return S_OK ;
	}
	virtual HRESULT Child ( value &child )  
	{
		val = child ;
		return S_OK ;
	}
	virtual HRESULT Done () 
	{
		return S_OK ;
	}
	virtual value   Value() 
	{
		return val ;
	}
private:
	value	val ;
};

class arrayDeserializer : public IValueDeserializer
{
public:
	arrayDeserializer () { }
	virtual HRESULT Init ( const XML_Char * name )  
	{
		val.name= name ;
		return S_OK ;
	}
	virtual HRESULT Characters ( std::wstring &chardata ) 
	{
		return S_OK ;
	}
	virtual HRESULT Child ( value &child )  
	{
		vals.push_back(child.val) ;
		return S_OK ;
	}
	virtual HRESULT Done () 
	{
		SAFEARRAYBOUND rga ;
		rga.lLbound = 0 ;
		rga.cElements = vals.size();
		SAFEARRAY * psa = SafeArrayCreate(VT_VARIANT, 1, &rga ) ;
		VARIANT * dest = 0 ;
		SafeArrayAccessData(psa, (void **)&dest) ;
		for ( std::vector<CComVariant>::iterator i = vals.begin() ; i != vals.end() ; i++ )
		{
			i->Detach(dest) ;
			dest++ ;
		}
		SafeArrayUnaccessData(psa) ;
		val.val.vt = VT_ARRAY | VT_VARIANT ;
		val.val.parray = psa ;
		return S_OK ;
	}
	virtual value   Value() 
	{
		return val ;
	}
private:
	std::vector<CComVariant> vals ;
	value	val ;
};

class memberDeserializer : public IValueDeserializer
{
public:
	memberDeserializer() { }
	virtual HRESULT Init ( const XML_Char * name )  
	{
		return S_OK ;
	}
	virtual HRESULT Characters ( std::wstring &chardata ) 
	{
		return S_OK ;
	}
	virtual HRESULT Child ( value &child )  
	{
		if ( wcscmp ( child.name.c_str(), L"name" ) == 0 )
			val.name = child.val.bstrVal ;
		else
			val.val = child.val ;
		return S_OK ;
	}
	virtual HRESULT Done () 
	{
		return S_OK ;
	}
	virtual value   Value() 
	{
		return val ;
	}
private:
	value	val ;
};

class structDeserializer : public IValueDeserializer
{
public:
	structDeserializer() { }
	virtual HRESULT Init ( const XML_Char * name )  
	{
		return s.CoCreateInstance(CLSID_CoXmlRpcStruct) ;
	}
	virtual HRESULT Characters ( std::wstring &chardata ) 
	{
		return S_OK ;
	}
	virtual HRESULT Child ( value &child )  
	{
		CComBSTR n(child.name.c_str()) ;
		return s->put_Value(n, child.val );
	}
	virtual HRESULT Done () 
	{
		IDispatch * d= 0 ;
		HRESULT hr = s.QueryInterface(&d) ;
		if (FAILED(hr)) return hr ;
		val.val.vt = VT_DISPATCH ;
		val.val.pdispVal = d ;
		return hr ;
	}
	virtual value   Value() 
	{
		return val ;
	}
private:
	value	val ;
	CComPtr<IXmlRpcStruct> s ;
};


class simpleDeserializer : public IValueDeserializer
{
public:
	simpleDeserializer() : requiredType(VT_EMPTY) { }
	virtual HRESULT Init ( const XML_Char * name )  
	{
		val.name = name ;
		if ( ( wcscmp ( name, L"i4" ) == 0 ) || wcscmp ( name, L"int" ) == 0 )
			requiredType = VT_I4 ;
		else if ( wcscmp ( name , L"boolean" ) == 0 )
			requiredType = VT_BOOL ;
		else if ( wcscmp ( name, L"string" ) == 0 || wcscmp ( name, L"value" ) == 0 || wcscmp ( name, L"name" ) == 0 ) 
			requiredType = VT_BSTR ;
		else if ( wcscmp ( name, L"double" ) == 0 )
			requiredType = VT_R8 ;
		return S_OK ;
	}
	virtual HRESULT Characters ( std::wstring &chardata ) 
	{
		HRESULT hr = S_OK ;
		if ( val.val.vt != VT_EMPTY ) 
			return hr ;	// we got a child, so we don't care about character data.
		val.val.bstrVal = SysAllocStringLen(chardata.c_str(), chardata.size()) ;
		val.val.vt = VT_BSTR ;
		if ( requiredType != VT_BSTR ) 
			hr = VariantChangeTypeEx ( &val.val, &val.val, serLCID, 0, requiredType ) ;
		return hr;
	}
	virtual HRESULT Child ( value &child )  
	{
		val = child ;
		return S_OK ;
	}
	virtual HRESULT Done ()
	{
		return S_OK ;
	}
	virtual value   Value() 
	{
		return val ;
	}
private:
	value	val ;
	VARTYPE	requiredType ;
};

class dateDeserializer : public IValueDeserializer
{
public:
	virtual HRESULT Init ( const XML_Char * name )  
	{
		val.name = name ;
		return S_OK ;
	}
	virtual HRESULT Characters ( std::wstring &chardata ) 
	{
		// min rep is YYYYMMDD, alternatives include
		//	YYYY-MM-DD
		//  YYYY-MM-DDTHH:MM:SS
		bool converted = false ;
		if ( chardata.size() <= 8 )
			return E_INVALIDARG ;

		if ( chardata.at(4) != '-' && chardata.at(7) != '-' )
		{
			chardata.insert(4, L"-") ;
			chardata.insert(7, L"-") ;
		}
		const WCHAR * p = chardata.begin() ;
		if ( chardata.size() >= 8 )
		{
			SYSTEMTIME tm = {0} ;
			int dir=-1, offsetMins = 0, offsetHours = 0 ;
			tm.wYear = (WORD)_wtoi(p) ;
			p = wcschr(p, '-') ;
			if ( p && *++p )	// this checks we found the - and that there is a character after it
			{
				tm.wMonth = (WORD)_wtoi(p) ;
				p = wcschr(p, '-') ;
				if ( p && *++p )
				{
					tm.wDay = (WORD)_wtoi(p) ;
					p = wcschr(p, 'T') ;
					if ( p && *++p )
					{
						tm.wHour = (WORD)_wtoi(p) ;
						p = wcschr(p,':') ;
						if ( p && *++p )
						{
							tm.wMinute = (WORD)_wtoi(p) ;
							p = wcschr(p,':') ;
							if ( p && *++p )
							{
								tm.wSecond = (WORD)_wtoi(p) ;
								const WCHAR * pOffsetStart = p ;
								p = wcschr(p, '-') ;
								if ( p )
									dir =1 ;
								if ( ! p )
									p = wcschr(pOffsetStart,'+') ;
								if ( p && *++p )
								{
									offsetHours = _wtoi(p) ;
									p = wcschr(p,':') ;
									if ( p && *++p ) 
										offsetMins = _wtoi(p) ;
								}

								// wow, now put all the peices together
								val.val.vt = VT_DATE ;
								SystemTimeToVariantTime(&tm, &val.val.date) ;
								// now apply the offset
								double offset = (offsetHours * 60 + offsetMins) / ( 24 * 60.0f ) ;
								val.val.date += ( dir * offset ) ;
								return S_OK ;
							}
						}
					}
				}
			}
		}
		return E_INVALIDARG ;
	}
	virtual HRESULT Child ( value &child )  
	{
		return S_OK ;
	}
	virtual HRESULT Done ()
	{
		return S_OK ;
	}
	virtual value   Value() 
	{
		return val ;
	}
private:
	value	val ;
};

class base64Deserializer : public IValueDeserializer
{
public:
	virtual HRESULT Init ( const XML_Char * name )  
	{
		val.name = name ;
		return S_OK ;
	}
	virtual HRESULT Characters ( std::wstring &chardata ) 
	{
		if ( chardata.size() == 0 )
			return S_OK ;

		WCHAR * end =  chardata.end() ;
		WCHAR * start = chardata.begin() ;
		while ( *start == ' ' || *start == '\r' || *start == '\t' || *start == '\n' )
			start++ ;

		WCHAR * p = start ;
		//  
		// Drasticly improved handling of the removal of whitespace - Contributed by Jim T. Row - Dallas TX
		//
		WCHAR  *pSource =p;
		while (*p)
		{
			// Check current character
			//  
			if (((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '/' && *p <= '9') || *p == '+' || *p == '='))
			{
				// Character is fine.. 
				// Move to next character..
				++p;
			}
			++pSource;
			*p = *pSource;
		}

		SAFEARRAYBOUND rga ;
		rga.lLbound = 0 ;
		rga.cElements = ( p - start ) * 3 / 4 ;
		SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
		BYTE * dst = 0 ;
		SafeArrayAccessData( psa, (void **)&dst ) ;

		size_t destLen = rga.cElements + 1 ;
		HRESULT hr = base64<WCHAR>::BufferDecode64( dst, &destLen, start, ( p - start ) ) ;
		SafeArrayUnaccessData(psa);
		// if there were some trailing =='s in the string, then the actual data is slightly
		// smaller than our original calc, we need to shrink the array slightly
		if (SUCCEEDED(hr))
		{
			rga.cElements = destLen ;
			SafeArrayRedim ( psa, &rga ) ;
			val.val.parray = psa ;
			val.val.vt = VT_ARRAY | VT_UI1 ;
		}
		else
			SafeArrayDestroy(psa) ;

		return hr ;
	}
	virtual HRESULT Child ( value &child )  
	{
		return S_OK ;
	}
	virtual HRESULT Done ()
	{
		return S_OK ;
	}
	virtual value   Value() 
	{
		return val ;
	}
private:
	value	val ;
};
