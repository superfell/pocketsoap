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

// $Header: c:/cvs/pocketsoap/pocketsoap/pSOAP_PPC/pocketpc.cpp,v 1.3 2005/08/18 05:12:47 simon Exp $

#include "stdafx.h"
#include "pocketpc.h"

// todo: review conditional compl. / install issues for detecting PPC2202 vs PPC2000
// PPC2002 only
// #include <Iphlpapi.h>
// #pragma comment(lib, "Iphlpapi.lib" ) 


// see http://www.opengroup.org/onlinepubs/9629399/apdxa.htm#tagcjh_20
//
// typedef struct _GUID {          // size is 16
//		DWORD  Data1;
//		WORD   Data2;
//		WORD   Data3;
//		BYTE   Data4[8];
// } GUID;
//

// for the leapyear stuff, see
// http://groups.google.com/groups?q=leap+years+in+1582&hl=en&ie=ISO-8859-1&oe=ISO-8859-1&selm=334E45B3%40gatekeeper.imsint.com&rnum=1
void GetCorrectedTimeStamp(__int64 *ft)
{
	SYSTEMTIME t ;
	GetSystemTime(&t) ;
	SystemTimeToFileTime(&t, (FILETIME *)ft) ;
	// apply correction, UUID spec has Tz's starting at 00:00:00.00, 15 October 1582 
	// FILETIMEs start at January 1, 1601.
	// 1584 is a leap year, so there are these leap years to take into account as well
	//		1584, 1588, 1592, 1596, 1600 [1600 doesn't get excluded on the div by 100 rule as its div by 400]
	// 78 days betwwen 15 October & Jan 1
	__int64 offset = ( 78 + 5 + (18 * 365)) * 24 * 60 * 60 ;
	offset *= 10000 ;

	*ft += offset ;
}

// try and find out what our MAC address is
void GetBigMac(BYTE  mac[]) 
{
	memset ( mac, 0, 6 ) ;
/*  See comments at top re: PPC2000 vs PPC2002
	
	DWORD er, cb = 0 ;
	BYTE * buf = NULL ;
	int tries = 0 ;
	static const int max_tries = 3 ;
	while ( ERROR_BUFFER_OVERFLOW == ( er = GetAdaptersInfo ( (IP_ADAPTER_INFO *)buf, &cb ) ) )
	{
		buf = (BYTE *)realloc ( buf, cb ) ;
		if( ++tries > max_tries ) break ;
	}
	if ( tries <= max_tries )
	{
		IP_ADAPTER_INFO * p = (IP_ADAPTER_INFO *)buf ;
		while ( p ) 
		{
			if ( (p->AddressLength == 6) && ( memcmp ( mac, p->Address, 6 ) != 0 ) )
			{
				memcpy ( mac, p->Address , 6 ) ;
				break ;
			}
			p = p->Next ;
		} ;
	}
	free ( buf ) ;
	*/
}

HRESULT CoCreateGuid ( GUID * pguid ) 
{
	static __int64				_lastTz ;
	static long					_clockSeq ;
	static unsigned long		_clkAdj ;
	static BYTE					_mac[6] ;
	static bool					_init ;
	static CComCriticalSection	_lock ;
	static const long MAX_CLOCK_SEQ = 16384 ;

	if ( ! _init )
	{
		_lock.Lock() ;
		if ( ! _init )
		{			
			GetCorrectedTimeStamp(&_lastTz) ;
			srand(*(unsigned int *)&_lastTz) ;
			_clockSeq = rand() % MAX_CLOCK_SEQ ;	
			_clkAdj = 0 ;
			GetBigMac(_mac) ;
			_init = true ;
		}
		_lock.Unlock() ;
	}

	__int64 tz ;
	GetCorrectedTimeStamp(&tz) ;
	_lock.Lock() ;
	if ( tz < _lastTz )
		_clockSeq = ( _clockSeq + 1 ) % MAX_CLOCK_SEQ ;
	else if ( tz == _lastTz )
		++_clkAdj;
	else
	{
		_clkAdj = 0 ;
		_lastTz = tz ;
	}
	tz |= _clkAdj ;
	_lock.Unlock() ;

	byte * ptz = (byte *)&tz ;
	memcpy ( &pguid->Data1, ptz, 4 ) ;		// timestamp
	memcpy ( &pguid->Data2, ptz + 4, 2 ) ;

	memcpy ( &pguid->Data3, ptz + 6, 2 ) ;
	pguid->Data3 &= 0x1FFF ;	// top 4 bits should 0001
	pguid->Data3 |= 0x1000 ;

	memcpy ( &pguid->Data4[0], &_clockSeq, 2 ) ;
	pguid->Data4[1] &= 0xBF ;	// top 2 bits should be 10
	pguid->Data4[1] |= 0x80 ;
	memcpy ( &pguid->Data4[2], &_mac, 6 ) ;	// the mac'n'cheese

	return S_OK ;
}

wchar_t * _i64tow ( __int64 i64, wchar_t * buff, int radix )
{
	unsigned __int64 ui64 = i64;
	if ( i64 < 0 )
	{
		*buff++ = '-';
		ui64 = -ui64;
	}
	wchar_t * initBuff = buff;
	wchar_t * firstNumChar = buff;
	do
	{
		unsigned char digit = (unsigned char)(ui64 % radix);
		if (digit < 10)
			*buff++ = digit + '0';
		else
			*buff++ = digit + 'A' - 10;
		ui64 = ui64 / radix;
	} while (ui64 > 0);

	*buff = 0;

	// now reverse the string
	wchar_t tmp;
	--buff;
	do
	{
		tmp = *firstNumChar;
		*firstNumChar = *buff;
		*buff = tmp;
		++firstNumChar;
		--buff;
	} while (firstNumChar < buff);

	return initBuff;
}