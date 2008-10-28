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

#include "stdafx.h"
#include "attachments.h"
#include "dime.h"

namespace Dime
{
	//  ***********************************************************************

	ByteArray HeaderBuilder::BuildHeader (	bool			msgBegin, 
											bool			chunked, 
											bool			msgEnd, 
											TypeNameFormat	tnf, 
											BSTR			typeName, 
											BSTR			id, 
											DWORD			bodySize )
	{
		USES_CONVERSION ;
		std::string szId, szTn ;
		szId = OLE2A(id) ;
		szTn = OLE2A(typeName) ;
		return BuildHeader ( msgBegin, chunked, msgEnd, tnf, szTn, szId, bodySize ) ;
	}

	ByteArray HeaderBuilder::BuildHeader ( DimeHeader &hdr ) 
	{
		return BuildHeader ( hdr.messageBegin, hdr.chunked, hdr.messageEnd, hdr.typeNameFormat, hdr.typeName, hdr.id, hdr.length ) ;
	}

	ByteArray HeaderBuilder::BuildHeader (	bool			msgBegin, 
											bool			chunked, 
											bool			msgEnd, 
											TypeNameFormat	tnf, 
											std::string		&typeName, 
											std::string		&id, 
											DWORD			bodySize )
	{
	    long nIdLength       = GetAlignedSize(id.size());
	    long nTypeNameLength = GetAlignedSize(typeName.size());
		long nSize           = HEADER_SIZE + nIdLength + nTypeNameLength ;
		ByteArray payload(nSize);

		DWORD Block1 = 0, Block2 = 0 , Block3 = 0 ;

		Dime::SetVersion		(Block1) ;
		Dime::SetMessageBegin	(Block1, msgBegin);
		Dime::SetMessageEnd		(Block1, msgEnd);
		Dime::SetChunkedFlag	(Block1, chunked);
		Dime::SetTypeNameFormat	(Block1, tnf);
		Dime::SetOptionsLength  (Block1, 0 ) ;

		Dime::SetIdLength		(Block2, id.size());
		Dime::SetTypeLength		(Block2, typeName.size());

		Dime::SetDataLength		(Block3, bodySize);

		//  copy header, remember to cope with endianess
		u_long b1 = htonl(Block1) ;
		u_long b2 = htonl(Block2) ;
		u_long b3 = htonl(Block3) ;
		::memcpy((void*)(&payload[0]), &b1, sizeof(b1));
		::memcpy((void*)(&payload[4]), &b2, sizeof(b2));
		::memcpy((void*)(&payload[8]), &b3, sizeof(b3));

		// options would go here when/if we support them

		//  copy id
		::memcpy((void*)(&payload[HEADER_SIZE]), id.c_str(), id.size());

		//  copy type
		::memcpy((void*)(&payload[HEADER_SIZE + nIdLength]), typeName.c_str(), typeName.size());

		return payload ;
	}

	//  ***********************************************************************

	// parse the fixed 96bit part of the RECORD header, this lets us find out the sizes.
	void HeaderReader::ParseFixedHeader ( const BYTE * record, DimeHeader &hdr )
	{
		DWORD block1 = ntohl( *((u_long*)record) ) ;
		DWORD block2 = ntohl( *((u_long*)(record + sizeof(DWORD)) )) ;
		DWORD block3 = ntohl( *((u_long*)(record + sizeof(DWORD)*2) )) ;

		hdr.version         = GetVersion(block1) ;
		hdr.messageBegin	= GetMessageBegin(block1);
		hdr.messageEnd		= GetMessageEnd(block1);
		hdr.chunked			= GetChunkedFlag(block1);
		hdr.typeNameFormat	= GetTypeNameFormat(block1);
		hdr.opLength		= GetOptionsLength(block1) ;

		hdr.idLength        = GetIdLength(block2);
		hdr.tnLength        = GetTypeLength(block2);

		hdr.length			= GetDataLength(block3);
	}

	// parse a full RECORD header
	void HeaderReader::Parse ( const BYTE * record, DimeHeader &hdr ) 
	{
		ParseFixedHeader ( record, hdr ) ;

		DWORD idOffset = HEADER_SIZE + GetAlignedSize(hdr.opLength) ;
		hdr.id.assign ( (const char *)(record + idOffset), hdr.idLength ) ;
		hdr.typeName.assign ( (const char *)(record + idOffset + GetAlignedSize(hdr.idLength)), hdr.tnLength ) ;
	}
};
