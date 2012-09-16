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

#include "Base64.h"

// this gets around the VC++ 6 compiler bug
template base64<char> ;
template base64<WCHAR>;

// Map 8-bit character to 6-bit byte
#define INVALID_BYTE64  255
#define OFFSET_BYTE64   '+'
static const byte g_byteMap64[] =
{
    62, // +
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    63, // /
    52, // 0
    53, // 1
    54, // 2
    55, // 3
    56, // 4
    57, // 5
    58, // 6
    59, // 7
    60, // 8
    61, // 9
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    0,  // A
    1,  // B
    2,  // C
    3,  // D
    4,  // E
    5,  // F
    6,  // G
    7,  // H
    8,  // I
    9,  // J
    10, // K
    11, // L
    12, // M
    13, // N
    14, // O
    15, // P
    16, // Q
    17, // R
    18, // S
    19, // T
    20, // U
    21, // V
    22, // W
    23, // X
    24, // Y
    25, // Z
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    INVALID_BYTE64,
    26, // a
    27, // b
    28, // c
    29, // d
    30, // e
    31, // f
    32, // g
    33, // h
    34, // i
    35, // j
    36, // k
    37, // l
    38, // m
    39, // n
    40, // o
    41, // p
    42, // q
    43, // r
    44, // s
    45, // t
    46, // u
    47, // v
    48, // w
    49, // x
    50, // y
    51, // z
};
static const g_byteMapSize = sizeof(g_byteMap64)/sizeof(*g_byteMap64) ;

// Map 6-bit byte to 8-bit character
void GetMap(const char ** p)
{
	static const char  szMap64[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	*p = szMap64 ;
}

void GetMap(const WCHAR ** p)
{
	static const WCHAR szMap64[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	*p = szMap64 ;
}

// Turn up to three bytes into four characters in the range [A-Za-z0-9+/],
// using = as the padding character.
template<class T>
HRESULT base64<T>::Encode64(T* encData, const BYTE* srcData, size_t numBytes )    // Number of bytes to encode (1, 2 or 3)
{
	if ( numBytes < 1 || numBytes > 3 )
		return E_INVALIDARG ;

	if ( ! encData ) return E_POINTER ;
	if ( ! srcData ) return E_POINTER ;

    // Break out three 8-bit bytes into four characters using 6-bit bytes,
    // padding with the = character should less than 3 bytes be encoded.
	const T * szMap ;
	GetMap(&szMap) ;
	encData[0] = szMap[((srcData[0] >> 2))];
    switch( numBytes )
    {
    case 3:
        encData[1] = szMap[((srcData[0] & 0x03) << 4) | (srcData[1] >> 4)];
        encData[2] = szMap[((srcData[1] & 0x0f) << 2) | (srcData[2] >> 6)];
        encData[3] = szMap[((srcData[2] & 0x3f))];
		break;

    case 2:
        encData[1] = szMap[((srcData[0] & 0x03) << 4) | (srcData[1] >> 4)];
        encData[2] = szMap[((srcData[1] & 0x0f) << 2)];
        encData[3] = '=';
		break;

    case 1:
        encData[1] = szMap[((srcData[0] & 0x03) << 4)];
        encData[2] = '=';
        encData[3] = '=';
    }
    return S_OK;
}


// Turn four characters in the range [A-Za-z0-9+/] into n bytes,
// stopping when the = padding character is reached.
template<class T>
HRESULT base64<T>::Decode64(BYTE* decData, size_t* numBytes, const T* srcData)     
{
	if ( ! decData ) return E_POINTER ;
	if ( ! numBytes) return E_POINTER ;
	if ( ! srcData ) return E_POINTER ;

    // Translate four characters into four 6-bit bytes
	*numBytes = 0 ;
    BYTE    rgbTmp[4];
    size_t  cch = 0;    // Count of characters to decode (stop at padding)
    HRESULT hr = S_OK;

    for( int i = 0; SUCCEEDED(hr) && srcData[i] != '=' && i < 4; i++, cch++ )
    {
        size_t  n = srcData[i] - OFFSET_BYTE64;
        if( n >= g_byteMapSize || ((rgbTmp[i] = g_byteMap64[n]) == INVALID_BYTE64) )
            hr = E_UNEXPECTED;
    }

    if( SUCCEEDED(hr) )
    {
        // Translate 6-bit bytes into 8-bit bytes
        switch( cch )
        {
        case 4:
            decData[0] = (rgbTmp[0] << 2) | (rgbTmp[1] >> 4);
            decData[1] = (rgbTmp[1] << 4) | (rgbTmp[2] >> 2);
            decData[2] = (rgbTmp[2] << 6) | (rgbTmp[3] >> 0);
            *numBytes = 3;
			break;

        case 3:
            decData[0] = (rgbTmp[0] << 2) | (rgbTmp[1] >> 4);
            decData[1] = (rgbTmp[1] << 4) | (rgbTmp[2] >> 2);
            *numBytes = 2;
		    break;

        case 2:
            decData[0] = (rgbTmp[0] << 2) | (rgbTmp[1] >> 4);
            *numBytes = 1;
	        break;

        default:
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

// Base64 encode a buffer of bytes where cch >= (cb/3) * 4.
template<class T>
HRESULT base64<T>::BufferEncode64(T* destData, size_t destLen, const BYTE*  srcData, size_t srcLen)
{
	ATLASSERT ( destLen %4 == 0 ) ;
	ATLASSERT ( destLen >= (srcLen/3 * 4) ) ;

	if ( ! srcData )  return E_POINTER ;
	if ( ! destData ) return E_POINTER ;

    HRESULT hr = S_OK;
    size_t  nRaw;
    size_t  nEncoded;

    for( nRaw = 0, nEncoded = 0; SUCCEEDED(hr) && (nRaw + 2) < srcLen; nRaw += 3, nEncoded += 4)
        hr = Encode64(destData + nEncoded, srcData + nRaw, 3);

    // Catch the last 1 or 2 bytes
    if( SUCCEEDED(hr) && ( srcLen - nRaw ) )
		hr = Encode64(destData + nEncoded, srcData + nRaw, srcLen - nRaw);

    return hr;
}


// Base64 decode a buffer of characters where *pcb >= (cch/4) * 3.
// NOTE: On input, *destLen is assumed to be the maximum size of destData.
template<class T>
HRESULT base64<T>::BufferDecode64(BYTE* destData, size_t* destLen, const T* srcData, size_t srcLen)
{
	if ( ! destData ) return E_POINTER ;
	if ( ! srcData  ) return E_POINTER ;
	if ( ! srcLen   ) return E_POINTER ;

	ATLASSERT ( *destLen >= (srcLen/4 * 3) - 2 ) ;

    *destLen = 0;
    HRESULT hr = S_OK;
    size_t  nRaw;
    size_t  nEncoded;
	size_t  cb;

    for( nRaw = 0, nEncoded = 0; SUCCEEDED(hr) && nEncoded < srcLen; nRaw += 3, nEncoded += 4)
    {    
        hr = Decode64(destData + nRaw, &cb, srcData + nEncoded);
        *destLen += cb;
        if( cb < 3 )
            break;
    }

    return hr;
}
