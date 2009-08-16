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
Portions created by Simon Fell are Copyright (C) 2000-2002,2009
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "PSOAP.h"
#include "SerializerDate.h"
#include "ParseHelpers.h"

//
BOOL VariantTimeToSystemTimeWithMilliseconds (/*input*/ double dVariantTime, /*output*/SYSTEMTIME *st);
BOOL SystemTimeToVariantTimeWithMilliseconds (/*input*/ SYSTEMTIME st, /*output*/double *dVariantTime);

/////////////////////////////////////////////////////////////////////////////
// CSerializerDate
CSerializerDate::CSerializerDate()
{
	ATLTRACE(_T("CSerializerDate::CSerializerDate\n")) ;
}

CSerializerDate::~CSerializerDate()
{
	ATLTRACE(_T("CSerializerDate::~CSerializerDate()\n")) ;
}

// ITypesInit
STDMETHODIMP CSerializerDate::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType )
{
	m_type = xmlType ;
	m_typeNS = xmlTypeNamespace ;
	return S_OK ;
}

// ISimpleSoapSerializer
STDMETHODIMP CSerializerDate::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ BSTR * dest ) 
{
	SYSTEMTIME st ;
	double dt = val->vt & VT_BYREF ? *val->pdate : val->date;
	VariantTimeToSystemTimeWithMilliseconds(dt, &st) ;

	WCHAR buff[30] ;
	if ( wcscmp ( m_type , L"date" ) == 0 ) {
		swprintf ( buff, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay ) ;
	} else if ( wcscmp ( m_type, L"time" ) ==0 ) {
		if (st.wMilliseconds != 0) 
			swprintf ( buff, L"%02d:%02d:%02d.%03dZ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ) ;
		else
			swprintf ( buff, L"%02d:%02d:%02dZ", st.wHour, st.wMinute, st.wSecond ) ;
	} else {
		if (st.wMilliseconds != 0)
			swprintf ( buff, L"%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ) ;
		else
			swprintf ( buff, L"%04d-%02d-%02dT%02d:%02d:%02dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond ) ;
	}
	CComBSTR bstrDate(buff) ;
	*dest = bstrDate.Detach() ;

	return S_OK ;
}

// ISoapSerializer
STDMETHODIMP CSerializerDate::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	if ( m_type.Length() ) 
	{
		CComPtr<ISerializerFactory> sf ;
		ctx->get_SerializerFactory(&sf) ;
		CComBSTR xsi ;
		sf->XsiForPrimaryNS(&xsi) ;
		dest->QNameAttribute ( CComBSTR(OLESTR("type")), xsi, m_type, m_typeNS ) ;
	}
	CComBSTR date ;
	Serialize ( val, ctx, &date ) ;
	dest->WriteTextNoEncoding ( date ) ;
	return S_OK ;
}

HRESULT CSerializerDate::extractDate ( const WCHAR * p, const WCHAR ** pStopped, SYSTEMTIME &tm )
{
	tm.wYear = (unsigned short)_wtoi(p) ;
	p = wcschr(p, '-') ;
	if ( p && *++p )	// this checks we found the - and that there is a character after it
	{
		tm.wMonth = (unsigned short)_wtoi(p) ;
		p = wcschr(p, '-') ;
		if ( p && *++p )
		{
			tm.wDay = (UINT)wcstol(p, const_cast<WCHAR **>(pStopped), 10) ;
			return S_OK ;
		}
	}
	return E_INVALID_LEX_REP ;
}

HRESULT CSerializerDate::extractTime ( const WCHAR * p, SYSTEMTIME &tm, long &offsetMins )
{
	int dir = -1 , oMins = 0, oHrs = 0 ;
	tm.wHour = (unsigned short)_wtoi(p) ;
	p = wcschr(p,':') ;
	if ( p && *++p )
	{
		tm.wMinute = (unsigned short)_wtoi(p) ;
		p = wcschr(p,':') ;
		if ( p && *++p )
		{
			tm.wSecond = (unsigned short)_wtoi(p);
			tm.wMilliseconds = parseMilliseconds(p);
			const WCHAR * pOffsetStart = p;
			p = wcschr(p, '-') ;
			if ( p )
				dir =1 ;
			if ( ! p )
				p = wcschr(pOffsetStart,'+') ;
			if ( p && *++p )
			{
				oHrs = _wtoi(p) ;
				p = wcschr(p,':') ;
				if ( p && *++p ) 
					oMins = _wtoi(p) ;
			}
			offsetMins = ((oHrs * 60 ) + oMins) * dir ;
			return S_OK ;
		}
	}
	return E_INVALID_LEX_REP ;
}

HRESULT CSerializerDate::SystemTimeToVariantWithOffset ( SYSTEMTIME &tm, long offsetMins, VARIANT * dest )
{
	CComVariant vdate ;
	vdate.vt = VT_DATE ;
	SystemTimeToVariantTimeWithMilliseconds(tm, &vdate.date) ;
	// now apply the offset
	double offset = offsetMins / ( 24.0f * 60.0f ) ;
	vdate.date += offset ;
	return vdate.Detach(dest) ;
}

HRESULT CSerializerDate::parseDateTime( BSTR charData, VARIANT * dest )
{
	// looking for yyyy-mm-ddThh:mm:ss-hh:mm or yyyy-mm-ddThh:mm:ssZ
	// min length is 20
	bool converted = false ;
	const WCHAR * p = charData ;
	if ( SysStringLen(charData) >= 19 )
	{
		SYSTEMTIME tm = {0} ;
		long offsetMins ;
		if (SUCCEEDED(extractDate(charData, &p, tm )))
		{
			p = wcschr(p, 'T') ;
			if ( p && *++p )
			{
				if ( SUCCEEDED(extractTime(p, tm, offsetMins )))
					return SystemTimeToVariantWithOffset ( tm, offsetMins, dest ) ;
			}
		}
	}
	// as a fallback, try the VARIANTCHANGETYPE on the string
	CComVariant v(charData) ;
	HRESULT hr = v.ChangeType(VT_DATE) ;
	if (FAILED(hr))
		return AtlReportError ( GetObjectCLSID(), OLESTR("Invalid dateTime string, unable to convert to dateTime"), IID_NULL, E_INVALID_LEX_REP ) ;

	return v.Detach(dest) ;
}

HRESULT CSerializerDate::parseDate ( BSTR charData, VARIANT * dest ) 
{
	SYSTEMTIME tm = {0} ;
	const WCHAR * p = 0 ;
	if(SUCCEEDED(extractDate ( charData, &p, tm )))
		return SystemTimeToVariantWithOffset ( tm, 0, dest ) ;

	return AtlReportError ( GetObjectCLSID(), OLESTR("Invalid date string, unable to convert to date"), IID_NULL, E_INVALID_LEX_REP ) ;
}

HRESULT CSerializerDate::parseTime ( BSTR charData, VARIANT * dest ) 
{
	SYSTEMTIME tm = {0} ;
	long offsetMins ;
	if(SUCCEEDED(extractTime(charData, tm, offsetMins)))
	{
		if(SUCCEEDED(SystemTimeToVariantWithOffset ( tm, offsetMins, dest )))
		{
			dest->date -= (long)(dest->date) ;
			return S_OK ;
		}
	}
	return AtlReportError ( GetObjectCLSID(), OLESTR("Invalid time string, unable to convert to time"), IID_NULL, E_INVALID_LEX_REP ) ;
}

// ISimpleSoapDeSerializer
STDMETHODIMP CSerializerDate::Deserialize( BSTR charData, ISOAPNamespaces * ns, VARIANT * dest )
{
	if ( wcscmp ( m_type, L"date" ) == 0 )
		return parseDate ( charData, dest ) ;
	else if ( wcscmp ( m_type, L"time" ) ==0 )
		return parseTime ( charData, dest ) ;
	return parseDateTime ( charData, dest ) ;
}


// ISoapDeSerializer
STDMETHODIMP CSerializerDate::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	m_node = node ;
	m_node->put_Type(m_type) ;
	m_node->put_TypeNS(m_typeNS) ;
	return S_OK ;
}

STDMETHODIMP CSerializerDate::Characters( /*[in]*/ BSTR charData )
{
	CComVariant vdate ;
	HRESULT hr = Deserialize ( charData, NULL, &vdate ) ;
	if (FAILED(hr)) return hr ;
	return m_node->put_Value(vdate) ;
}

STDMETHODIMP CSerializerDate::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerDate::ChildReady( long id, /*[in]*/ ISOAPNode * childNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerDate::ChildRef( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerDate::Ref( BSTR id,	 /*[in]*/ ISOAPNode * idNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerDate::End()
{
	m_node.Release();
	return S_OK ;
}

// VariantTime <-> SystemTime has trouble with milliseconds
//		see http://support.microsoft.com/kb/297463
//
//
// variant/system time conversions with millisecond support
// see http://www.codeproject.com/KB/datetime/SysTimeToVarTimeWMillisec.aspx
//

#define ONETHOUSANDMILLISECONDS  .0000115740740740
/*
A variant time is stored as an 8-byte real value (double), representing a date between January 1, 1753 
and December 31, 2078,inclusive. The value 2.0 represents January 1, 1900; 3.0 represents January 2, 1900, and so on. 
Adding 1 to the value increments the date by a day. The fractional part of the value represents the time of day. 
Therefore, 2.5 represents noon on January 1, 1900; 3.25 represents 6:00 A.M. on January 2, 1900, and so on. 
so 0.5 represents 12 hours ie 12*60*60 seconds, hence 1 second = .0000115740740740
*/

BOOL VariantTimeToSystemTimeWithMilliseconds (/*input*/ double dVariantTime, /*output*/SYSTEMTIME *st)
{
    BOOL retVal = TRUE;

    double halfsecond = ONETHOUSANDMILLISECONDS / 2.0; 
    // ONETHOUSANDMILLISECONDS is equal to 0.0000115740740740

    // this takes care of rounding problem with 
    if(!VariantTimeToSystemTime(dVariantTime - halfsecond, st))
		return FALSE;

    // extracts the fraction part
    double fraction = dVariantTime - (int) dVariantTime; 

    double hours; 
    hours = fraction = (fraction - (int)fraction) * 24;

    double minutes;
    minutes = (hours - (int)hours) * 60;

    double seconds;
    seconds = (minutes - (int)minutes) * 60;

    double milliseconds;
    milliseconds = (seconds - (int)seconds) * 1000;

    milliseconds = milliseconds + 0.5; // rounding off millisecond to the 
                                       // nearest millisecond 

    if (milliseconds < 1.0 || milliseconds > 999.0) //Fractional 
                          // calculations may yield in results like
        milliseconds = 0; // 0.00001 or 999.9999 which should actually 
                          // be zero (slightly above or below limits 
                          // are actually zero)
    if (milliseconds) 
        st->wMilliseconds = (WORD) milliseconds;
    else  // if there is 0 milliseconds, then we don't have the problem !!
        retVal = VariantTimeToSystemTime(dVariantTime, st); // 

    return retVal;
}

BOOL SystemTimeToVariantTimeWithMilliseconds (/*input*/ SYSTEMTIME st, /*output*/double *dVariantTime)
{
    WORD wMilliSeconds = st.wMilliseconds; // save the milli second information

    st.wMilliseconds = 0; // pass 0 milliseconds to the function and get  
                          // the converted value without milliseconds
    double dWithoutms;
    if (!SystemTimeToVariantTime(&st, &dWithoutms))
		return FALSE;

    // manually convert the millisecond information into variant 
    // fraction and add it to system converted value
    *dVariantTime = dWithoutms + (ONETHOUSANDMILLISECONDS * wMilliSeconds / 1000.0);
    return TRUE;
}