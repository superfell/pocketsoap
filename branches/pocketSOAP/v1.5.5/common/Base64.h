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
Portions created by Simon Fell are Copyright (C) 2000,2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

// a long time ago, in a galaxy far far away (aka Portland)
// Chris Sells wrote some base64 code for his monikers project
// this code was originally based on Chris's code, and has over the years
// recevied extensive tweaks from me.

#pragma once

#include <atlbase.h>

template<class T>
class base64
{
public:
	// Base64 encode a buffer of bytes where cch >= (cb/3) * 4.
	static HRESULT BufferEncode64(T*			destData,     // Array of encoded characters
								   size_t       destLen,      // Size of dest buffer
								   const BYTE*  srcData,      // Array of bytes to encode
								   size_t       srcLen);      // Number of bytes to encode

	// Base64 decode a buffer of characters where *pcb >= (cch/4) * 3.
	// NOTE: On input, *pcb is assumed to be the maximum size of rgb.
	static HRESULT BufferDecode64( BYTE*        destData,    // Array of decoded bytes
					               size_t*      destLen,     // Number of decoded bytes
							       const T*		srcData,     // Array of characters to decode
								   size_t       srcLen)  ;   // Number of characters to decode

	
	// Turn up to three bytes into four characters in the range [A-Za-z0-9+/],
	static HRESULT Encode64(T*				encData,      // 4 encoded characters
							 const BYTE*	srcData,      // bytes to encode
							 size_t			numBytes );   // Number of bytes to encode (1, 2 or 3)

	// Turn four characters in the range [A-Za-z0-9+/] into n bytes,
	static HRESULT Decode64(BYTE*			decData,     // Decoded bytes
							 size_t*		numBytes,    // Number of decoded bytes (1, 2 or 3)
							 const T*		srcData);    // Array of 4 characters to decode
};
