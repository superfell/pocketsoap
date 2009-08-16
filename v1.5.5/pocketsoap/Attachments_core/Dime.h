/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is PocketSOAP.

The Initial Developer of the Original Code is David Buksbaum.
Portions created by David Buksbaum are Copyright (C) 2001
David Buksbaum. All Rights Reserved.

Contributor(s):
Simon Fell. 
	Updated to match the joint IBM / MSFT DIME spec
		http://msdn.microsoft.com/library/en-us/dnglobspec/html/draft-nielsen-dime-02.txt
	Updated to match the revised DIME spec
		http://www.gotdotnet.com/team/xml_wsspecs/dime/draft-nielsen-dime-01.txt
	Fixed endianess issues with header
	Added HeaderBuilder to generate record headers separate to record bodies
	Added HeaderReader  to parse record headers separate from record bodies
*/

#ifndef __DIME_H__
#define __DIME_H__

//  ***********************************************************************

#include <string>
#include <vector>

//  ***********************************************************************

namespace Dime
{
	typedef std::vector<unsigned char>  ByteArray;
	typedef ByteArray::pointer          ByteArrayPtr;

	//  ***********************************************************************

	static const int HEADER_SIZE			= 12;
	static const unsigned char DIME_VERSION = 0x01 ;

	//  ***********************************************************************

	enum MaskValues32
	{
		//  Header 0-15
		mv32Version			= 0xF8000000, //  1111100000000000  0000000000000000
		mv32MessageBegin    = 0x04000000, //  0000010000000000  0000000000000000
		mv32MessageEnd      = 0x02000000, //  0000001000000000  0000000000000000
		mv32ChunkedFlag     = 0x01000000, //  0000000100000000  0000000000000000
		mv32TypeNameFormat  = 0x00F00000, //  0000000011110000  0000000000000000
		//  Header 16-31
		mv32OptionsLength   = 0x0000FFFF, //  0000000000000000  1111111111111111
		//  Header 32-63
		mv32IdLength        = 0xFFFF0000, //  1111111111111111  0000000000000000
		// Header 64-95
		mv32TypeLength      = 0x0000FFFF, //  0000000000000000  1111111111111111
		// Header 96-128
		mv32DataLength      = 0xFFFFFFFF  //  1111111111111111  1111111111111111
	};

	//  ***********************************************************************

	inline DWORD GetAlignedSize(DWORD size)
	{
		//  aligns on 4 octet boundries
		DWORD adjustment = (size % 4) == 0 ? 0 : 1;
		if ( ! adjustment )
			return size ;
		DWORD newSize = (size > 0) ? (((size  / 4) + adjustment) * 4) : 0;
		return newSize ;
	}

	//  ***********************************************************************
	//  32 bit manipulators
	inline unsigned char GetVersion(DWORD dataBlock)
	{
		return (unsigned char)(( dataBlock & mv32Version ) >> 27) ;
	}

	inline void SetVersion(DWORD &dataBlock)
	{
		dataBlock |= ( DIME_VERSION << 27 ) ;
	}

	//  ***********************************************************************

	inline bool GetMessageBegin(DWORD dataBlock)
	{ 
		return((dataBlock & mv32MessageBegin) != 0);
	}

	inline void SetMessageBegin(DWORD &dataBlock, bool bValue)
	{
		if(bValue)
			dataBlock |= mv32MessageBegin;
		else
			dataBlock &= ~mv32MessageBegin; 
	}

	//  ***********************************************************************

	inline bool GetMessageEnd(DWORD dataBlock)
	{
		return((dataBlock & mv32MessageEnd) != 0);
	}

	inline void SetMessageEnd(DWORD& dataBlock, bool bValue)
	{
		if(bValue)
			dataBlock |= mv32MessageEnd;
		else
			dataBlock &= ~mv32MessageEnd; 
	}

	//  ***********************************************************************

	inline bool GetChunkedFlag(DWORD dataBlock)
	{
		return((dataBlock & mv32ChunkedFlag) != 0);
	}

	inline void SetChunkedFlag(DWORD &dataBlock, bool bValue)
	{
		if(bValue)
			dataBlock |= mv32ChunkedFlag;
		else
			dataBlock &= ~mv32ChunkedFlag; 
	}

	//  ***********************************************************************

	inline TypeNameFormat GetTypeNameFormat(DWORD dataBlock)
	{
		DWORD v = (dataBlock & mv32TypeNameFormat);
		v >>= 20; //  HACK: Magic Number representing starting bit position from right.
		return ((TypeNameFormat)v); 
	}

	inline void SetTypeNameFormat(DWORD &dataBlock, TypeNameFormat tnfValue)
	{
		//  HACK: Magic Number representing starting bit position from right.
		DWORD v = tnfValue << 20 ;
		dataBlock |= (v & mv32TypeNameFormat); 
	}

	//  ***********************************************************************

	inline unsigned short GetOptionsLength(DWORD dataBlock)
	{
		return (unsigned short)( dataBlock & mv32OptionsLength ) ;
	}

	inline void SetOptionsLength(DWORD &dataBlock, short length)
	{
		dataBlock |= ( length & mv32OptionsLength ) ;
	}

	//  ***********************************************************************

	inline short GetIdLength(DWORD dataBlock)
	{
		DWORD v = (dataBlock & mv32IdLength);
		v >>= 16; //  HACK: Magic Number representing starting bit position from right.
		return ((short)v); 
	}

	inline void SetIdLength(DWORD &dataBlock, unsigned short nValue)
	{
		DWORD v = (nValue << 16); //  HACK: Magic Number representing starting bit position from right.
		dataBlock |= (v & mv32IdLength); 
	}

	//  ***********************************************************************

	inline short GetTypeLength(DWORD dataBlock)
	{
		DWORD v = (dataBlock & mv32TypeLength);
		return ((short)v); 
	}

	inline void SetTypeLength(DWORD &dataBlock, unsigned short nValue)
	{
		dataBlock |= ((nValue) & mv32TypeLength); 
	}

	//  ***********************************************************************

	inline DWORD GetDataLength(DWORD dataBlock)
	{
		return dataBlock ;
	}

	inline void SetDataLength(DWORD &dataBlock, DWORD nValue)
	{
		dataBlock = nValue;
	}

	//  ***********************************************************************

	//  ***********************************************************************
	//  This lets you build a DIME record header,  
	//  without having to have the payload in memory
	//  ***********************************************************************
	struct DimeHeader
	{
		DimeHeader() : 
			version(DIME_VERSION),
			messageBegin(false), 
			messageEnd(false), 
			chunked(false), 
			typeNameFormat(tnfUnknown), 
			idLength(0),
			tnLength(0),
			opLength(0),
			length(0)
		{
		}

		DimeHeader(bool begin, bool end, bool Chunked, TypeNameFormat fmt, std::string &TypeName, std::string ID, DWORD len ) :
			version(DIME_VERSION),
			messageBegin(begin), 
			messageEnd(end),
			chunked(chunked),
			typeNameFormat(fmt),
			typeName(TypeName),
			id(ID),
			length(len),
			opLength(0)
		{
			idLength = id.length() ;
			tnLength = typeName.length() ;
		}

		DWORD HeaderSize(bool incFixedPart)
		{
			return (incFixedPart ? HEADER_SIZE : 0) + GetAlignedSize(idLength) + GetAlignedSize(tnLength) + GetAlignedSize(opLength) ;
		}

		// is a version of DIME we're supporting ?
		bool VersionIsOk()
		{
			return version == DIME_VERSION ;
		}

		unsigned char	version ;
		bool			messageBegin ;
		bool			messageEnd ;
		bool			chunked ;
		TypeNameFormat	typeNameFormat ;
		unsigned short	idLength, tnLength, opLength ;
		std::string		typeName ;
		std::string		id ;
		DWORD			length ;
	};

	// builds a serialized RECORD header
	class HeaderBuilder
	{
	public:
		static ByteArray BuildHeader ( bool msgBegin, bool chunked, bool msgEnd, TypeNameFormat tnf, BSTR typeName, BSTR id, DWORD bodySize ) ;
		static ByteArray BuildHeader ( bool msgBegin, bool chunked, bool msgEnd, TypeNameFormat tnf, std::string &typeName, std::string &id, DWORD bodySize ) ;
		static ByteArray BuildHeader ( DimeHeader &hdr ) ;
	};

	// parses a serialzied DIME record from the stream, and return a DimeHeader object, and the # of bytes eaten 
	// You are left to your own devices to handle the record body
	class HeaderReader
	{
	public:
		static void	ParseFixedHeader ( const BYTE * record, DimeHeader &hdr ) ;
		static void Parse			 ( const BYTE * record, DimeHeader &hdr ) ;
	};
}

#endif  //  __DIME_H__