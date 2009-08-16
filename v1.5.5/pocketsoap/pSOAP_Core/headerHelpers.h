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
Portions created by Simon Fell are Copyright (C) 2002-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#pragma once

class headerVal
{
public:
	headerVal(const std::string &v) : value(v)
	{
	}

	std::string		value ;
} ;

class ciStringCompare
{
public:
	bool operator()(const std::string &a, const std::string &b) const 
	{
		return _stricmp ( a.c_str(), b.c_str() ) < 0 ;
	}
};

// typedef std::vector<header> headers ;
typedef std::multimap<std::string, headerVal, ciStringCompare> Headers ;


class headerHelpers
{
public:
	// parses the set of headers into a headers object
	// if payload contains the end of header marker [\r\n\r\n], we'll stop there
	// or you can pass in just the headers
	static Headers Parse(const char * payload)
	{
		Headers hdrs ;
		if ( strncmp ( payload, "HTTP/", 5 ) == 0 )
		{
			// we don't extract the HTTP status line
			payload = strstr(payload, "\r\n" ) ;
			if ( ! payload ) return hdrs ;
			payload += 2 ;
		}
		const char * endOfHeaders = strstr ( payload, "\r\n\r\n" ) ;
		std::string h, name, val ;
		if ( endOfHeaders )
			h.append ( payload, endOfHeaders + 2 ) ;
		else
		{
			h.append ( payload ) ;
			if (( h.at(h.size()-2) != '\r') && ( h.at(h.size()-1) != '\n' ))
				h.append ("\r\n") ;
		}
		
		// h now contains the headers and they are all CRLF terminated
		size_t soh = 0, eoh = 0 ;
		while ( soh < h.size() )
		{
			// find the end of the first end, take line folding into account
			eoh = soh ;
			do
			{
				eoh = h.find ( "\r\n", eoh+2 )  ;
			} while ( eoh != std::string::npos && IsLWS( h.begin()+(eoh+2) ) ) ;

			SplitHeader ( h.begin()+soh, h.begin()+eoh, name, val ) ;
			hdrs.insert ( Headers::value_type ( name, val ) ) ;

			// remember eat the CRLF
			soh = eoh +2 ;
		}
		return hdrs;
	}


	static HRESULT SplitHeader(const char * startOfHeader, const char * endOfHeader, std::string &name, std::string &value) 
	{
		name.erase() ;
		value.erase() ;
		const char * colon = strchr(startOfHeader, ':' ) ;
		if ( colon && ( colon < endOfHeader ))
		{
			// extract name
			const char * eon = colon -1 ;
			while ( headerHelpers::IsLWS(eon) )
				--eon ;
			name.append ( startOfHeader, eon - startOfHeader +1 ) ;

			// extract value
			++colon ;
			while ( headerHelpers::IsLWS(colon) )
				++colon ;

			value.append ( colon, endOfHeader - colon ) ;
			return S_OK ;
		}
		return E_UNEXPECTED ;
	}

	// is a Linear white space character
	static bool IsLWS( const char * c )
	{
		return ( *c == ' ' || *c == '\t' ) ;
	}

	static bool IsIgnoreableChar ( const char * c )
	{
		if ( IsLWS(c) || *c == 0 || *c == '=' || *c == '\'' || *c == '\"' )
			return true ;
		return false ;
	}

	static stringBuff_A ExtractHeaderVal ( const char * payload, const char * hdrName )
	{
		stringBuff_A res ;
		stringBuff_A hdrs ;
		size_t lwrOffset = 0 ;
		const char * endOfHeaders = strstr ( payload, "\r\n\r\n" ) ;
		// insert a leading \r\n into the headers if needed
		if ( (payload[0] != '\r') && ( payload[1] != '\n' ))
		{
			hdrs.Append("\r\n") ;
			lwrOffset = 2 ;
		}

		// take into account that there might not be any payload
		if ( endOfHeaders ) 
			hdrs.Append ( payload, endOfHeaders - payload ) ;
		else
			hdrs.Append ( payload ) ;

		hdrs.ToLower() ;

		// header start to start on a new line
		stringBuff_A hdr ("\r\n") ;
		hdr.Append(hdrName) ;
		hdr.ToLower() ;

		char * p = hdrs.buffer() ;
		char * eoh = NULL ;
		int offset ;
		while ( p )
		{
			p = strstr(p, hdr.c_str() ) ;
			if ( p )
			{
				// p now points to the start of the header line, find the end of this
				// header, taking into account the fact that the header might be folded.

				// skip over the actual header name
				p += hdr.Size() ;
				eoh = p-1 ;
				do
				{
					eoh = strstr ( eoh+1, "\r\n" ) ;
				} while ( eoh && IsLWS(eoh+2) ) ;
	
				// could be the last header
				if ( ! eoh )
					eoh = hdrs.buffer() + hdrs.Size() ;

				while ( ( IsIgnoreableChar(p) ) || *p == ':' )
					++p ;
				if ( p ) 
				{
					if ( res.Size() ) 
						res.Append ( ";" ) ;
					offset = p - hdrs.c_str() ;

					res.Append ( payload + offset - lwrOffset , eoh-p ) ;
					p = eoh ;
				}
			}
		}
		// unfold multiple lines back into one.
		std::replace_if ( res.buffer(), res.buffer() + res.Size(), IsCrOrLf(), ' ' ) ;
		return res ;
	}

private:
	class IsCrOrLf
	{
	public:
		bool operator()(char x)
		{
			return ( x== '\r' || x== '\n' ) ;
		}
	};
} ;



