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
Portions created by Simon Fell are Copyright (C) 2000,2004
Simon Fell. All Rights Reserved.

Contributor(s):
*/

static const DWORD CB_BUFFER_MAX = 256;

class VariantSourceCracker
{
public:
	VariantSourceCracker(VARIANT src, BSTR charEncoding, HRESULT &hr) ;
	~VariantSourceCracker() ;

	XML_Char * characterEncoding()
	{
		return charEnc ;
	}

	// the start of our next chunk
	char * source()
	{
		return src + pos ;
	}

	// the size of our next chunk
	size_t size()
	{
		return min ( CB_BUFFER_MAX, cb - pos ) ;
	}
	
	// move to the next chunk
	HRESULT Next(bool &isMore);

	// we've read everything we're interested in, make sure to clean up
	void Flush();

private:
	size_t					cb, pos;
	char					*src ;
	CComBSTR				charEnc ;
	SAFEARRAY				*arr ;
	bool					ownArray, ownSrc ;
	CComVariant				vSrc ;
	CComPtr<IStreamReader>	stream;
};

VariantSourceCracker::~VariantSourceCracker()
{
	if ( arr )
	{
		SafeArrayUnaccessData ( arr ) ;
		if ( ownArray )
			SafeArrayDestroy ( arr ) ;
	}
	if ( ownSrc )
		free(src) ;
}

VariantSourceCracker::VariantSourceCracker(VARIANT vsrc, BSTR charEncoding, HRESULT &hr) : cb(0), src(0), arr(0), ownArray(false), ownSrc(false)
{
	charEnc = charEncoding ;
	VariantCopyInd ( &vSrc, &vsrc ) ;
	pos =0 ;
	CComPtr<ISOAPTransport> st ;
	CComPtr<ISwATransport> swa;
	switch ( vSrc.vt )
	{
		case VT_BSTR :
			cb = SysStringByteLen(vSrc.bstrVal) ;
			src = (char *)vSrc.bstrVal ;
			charEnc = OLESTR("utf-16") ;
			break ;
			
		case VT_UNKNOWN : 
		case VT_DISPATCH :
			hr = vSrc.punkVal->QueryInterface(__uuidof(swa), (void **)&swa);
			if(swa)
			{
				CComPtr<IUnknown> punk;
				hr = swa->Receive( &charEnc, &punk ) ;
				if(FAILED(hr)) return ;
				punk->QueryInterface( __uuidof(stream), (void **)&stream);
				src = (char *)malloc(CB_BUFFER_MAX);
				ownSrc = true;
				bool more;
				Next(more);
				break;
			}

			hr = vSrc.punkVal->QueryInterface(IID_ISOAPTransport, (void **)&st) ;
			if (st)
			{
				hr = st->Receive ( &charEnc, &arr ) ;
				if (FAILED(hr)) return ;
				ownArray = true ;
			}
			else
				break ;

		case VT_ARRAY | VT_UI1 :
			if ( ! arr ) arr = vSrc.parray ;
			long ub, lb ;
			SafeArrayGetUBound ( arr, 1, &ub ) ;
			SafeArrayGetLBound ( arr, 1, &lb ) ;
			cb = ub - lb + 1 ;
			SafeArrayAccessData ( arr, (void **)&src ) ;
			break ;
	}
}

HRESULT VariantSourceCracker::Next(bool &isMore)
{
	if(!stream)
	{
		pos += size();
		isMore = pos < cb ;
		return S_OK ;
	}

	DWORD rcb ;
	HRESULT hr = stream->Read ( src, CB_BUFFER_MAX, &rcb ) ;
	isMore = rcb > 0 ;
	cb = rcb ;
	return hr;
}

void VariantSourceCracker::Flush()
{
	if (stream)
	{
		// we need to read the rest of the HTTP stream so that
		// the connection is re-used correctly
		bool isMore;
		HRESULT hr;
		do {
			hr = Next(isMore);
		} while (SUCCEEDED(hr) && isMore);	
	}
}