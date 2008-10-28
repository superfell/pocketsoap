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


#include "stdafx.h"
#include "psoap.h"
#include "SerializerFactory.h"

#include "tags.h"
#include "serializerSimple.h"
#include "soapnode.h"

//statics
SerializerFactoryConfigMgr CSerializerFactory::m_cfgMgr ;

bool operator < ( const stringBuff_W &a, const stringBuff_W &b )
{
	return wcscmp ( a.c_str(), b.c_str() ) < 0 ;
}

/////////////////////////////////////////////////////////////////////////////
// CSerializerFactory
/////////////////////////////////////////////////////////////////////////////
static const CComBSTR xsd01 ( XSD01_URI ) ;
static const CComBSTR xsd99 ( XSD99_URI ) ;

static const CComBSTR xsi01 ( XSD01_SCHEMA_INSTANCE_TYPE ) ;
static const CComBSTR xsi99 ( XSD99_SCHEMA_INSTANCE_TYPE ) ;

static const CComBSTR ss    ( OLESTR("pocketSOAP.SimpleSerializer.1") ) ;
static const CComBSTR as    ( OLESTR("pocketSOAP.ArraySerializer.1")  ) ;
static const CComBSTR as12  ( OLESTR("pocketSOAP.ArraySerializer12.1")) ;
static const CComBSTR ads   ( OLESTR("pocketSOAP.ArrayDeserializer.1")) ;
static const CComBSTR ads12 ( OLESTR("pocketSOAP.ArrayDeserializer12.1")) ;
static const CComBSTR ns    ( OLESTR("pocketSOAP.NodeSerializer.1")   ) ;
static const CComBSTR b64	( OLESTR("pocketSOAP.SerializerB64.1")    ) ;
static const CComBSTR ds	( OLESTR("pocketSOAP.SerializerDate.1")   ) ;
static const CComBSTR bools	( OLESTR("pocketSOAP.SerializerBoolean.1")) ;
static const CComBSTR pbs	( OLESTR("pocketSOAP.SerializerPB.1")     ) ;
static const CComBSTR hexbin( OLESTR("pocketSOAP.SerializerHexBin.1") ) ;
static const CComBSTR qname ( OLESTR("pocketSOAP.SerializerQName.1")  ) ;
static const CComBSTR array ( OLESTR("Array") ) ;
static const CComBSTR null ;

static const CComBSTR SOAPENC ( SOAP_ENCODING_11_URI ) ;
static const CComBSTR PSTYPES ( POCKETSOAP_TYPES_URI ) ;
static const CComBSTR SOAPENV ( SOAP_ENVELOPE_11_URI ) ;

bool bstrEqual ( BSTR a, BSTR b )
{
	return wcscmp ( a ? a : L"" , b ? b : L"" ) == 0  ;
}

/////////////////////////////////////////////////////////////////////////////
// CSerializerFactoryConfigMgr
/////////////////////////////////////////////////////////////////////////////
SerializerFactoryConfigMgr::SerializerFactoryConfigMgr() : 
	m_std11(0), 
	m_scripting11(0),
	m_std12(0),
	m_scripting12(0)
{
}

SerializerFactoryConfigMgr::~SerializerFactoryConfigMgr()
{
	if ( m_std11 )
		delete m_std11 ;
	if ( m_scripting11 ) 
		delete m_scripting11 ;
	if ( m_std12 )
		delete m_std12 ;
	if ( m_scripting12 ) 
		delete m_scripting12 ;
}

SerializerFactoryConfig * SerializerFactoryConfigMgr::Getter(sfConfigOptions configMode, SerializerFactoryConfig ** ourCopy) 
{
	if ( *ourCopy )
		return *ourCopy ;
	SerializerFactoryConfig * c = NULL ;
	m_lock.Lock() ;
	if ( *ourCopy )
		c = *ourCopy ;
	else
	{
		c = new SerializerFactoryConfig ;
		c->Init ( configMode ) ;
		*ourCopy = c ;
	}
	m_lock.Unlock() ;
	return c ;
}

SerializerFactoryConfig * SerializerFactoryConfigMgr::Config(sfConfigOptions mode) 
{
	switch ( mode )
	{
		case sfcNormal_11    : return Getter(mode, &m_std11) ; 
		case sfcScripting_11 : return Getter(mode, &m_scripting11) ; 
		case sfcNormal_12    : return Getter(mode, &m_std12) ; 
		case sfcScripting_12 : return Getter(mode, &m_scripting12) ; 
	}
	ATLASSERT("Invalid mode passed to SerializerFactoryConfigMgr::Config()") ;
	return 0 ;
}

/////////////////////////////////////////////////////////////////////////////
// CSerializerFactoryConfig
/////////////////////////////////////////////////////////////////////////////
SerializerFactoryConfig::SerializerFactoryConfig() : m_Loaded(false)
{
}

SerializerFactoryConfig::SerializerFactoryConfig(const SerializerFactoryConfig &rhs) : 
	m_Loaded	(rhs.m_Loaded),
	m_simpleSer	(rhs.m_simpleSer),
	m_ifaceSer	(rhs.m_ifaceSer),
	m_des		(rhs.m_des),
	m_etypes	(rhs.m_etypes),
	m_localtypes(rhs.m_localtypes) 
{
}

SerializerFactoryConfig::~SerializerFactoryConfig()
{
}

void SerializerFactoryConfig::Init( sfConfigOptions configMode )
{
	if ( m_Loaded ) return ;

	bool ScriptingMode = ( configMode == sfcScripting_11 ) || ( configMode == sfcScripting_12 ) ;
	bool soap12 = ( configMode == sfcNormal_12 ) || ( configMode == sfcScripting_12 ) ;
	const CComBSTR & arrSer = soap12 ? as12 : as ;
	const CComBSTR & arrds  = soap12 ? ads12 : ads ;

	// de-serializers
	// SOAP 1.1. soap encoding
	if ( ! soap12 )
	{
		AddDeser ( SOAPENC, CComBSTR(L"binary"),		false, VT_UI1 | VT_ARRAY, b64 ) ;
		AddDeser ( SOAPENC, CComBSTR(L"boolean"),		false, VT_BOOL, 	bools ) ;
		AddDeser ( SOAPENC, CComBSTR(L"float"),			false, VT_R4,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"double"),		false, VT_R8,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"decimal"),		false, VT_DECIMAL,	ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"uriReference"),	false, VT_BSTR,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"int"),			false, VT_I4,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"short"),			false, VT_I2,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"byte"),			false, VT_I1,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"unsignedLong"),	false, VT_UI4,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"unsignedInt"),	false, VT_UI4,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"unsignedShort"),	false, VT_UI2,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"unsignedByte"),	false, VT_UI1,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"timeInstant"),	false, VT_DATE,		ss ) ;
		AddDeser ( SOAPENC, CComBSTR(L"string"),		false, VT_BSTR,		ss ) ;
	}
	// todo, equivilent of above needed for SOAP 1.2 ?


	// only do 1999 schema for SOAP 1.1
	if ( ! soap12 )
	{
		AddDeser ( xsd99, CComBSTR(L"QName"),			false, L"pocketSOAP.QName.1", qname ) ;
		AddDeser ( xsd99, CComBSTR(L"decimal"),			false, VT_DECIMAL,	ss ) ;
		AddDeser ( xsd99, CComBSTR(L"uriReference"),	false, VT_BSTR,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"byte"),			false, VT_I1,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"short"),			false, VT_I2,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"int"),				false, VT_I4,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"long"),			false, VT_I8,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"unsignedByte"),	false, VT_UI1,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"unsignedShort"),	false, VT_UI2,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"unsignedInt"),		false, VT_UI4,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"unsignedLong"),	false, VT_UI8,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"boolean"),			false, VT_BOOL,		bools ) ;
		AddDeser ( xsd99, CComBSTR(L"float"),			false, VT_R4,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"double"),			false, VT_R8,		ss ) ;
		AddDeser ( xsd99, CComBSTR(L"ur-type"),			false, VT_VARIANT,	ss ) ;
		AddDeser ( xsd99, CComBSTR(L"timeInstant"),		false, VT_DATE,		ds ) ;
		AddDeser ( xsd99, CComBSTR(L"time"),			false, VT_DATE,		ds ) ;
		AddDeser ( xsd99, CComBSTR(L"date"),			false, VT_DATE,		ds ) ;
		AddDeser ( xsd99, CComBSTR(L"string"),			false, VT_BSTR,		ss ) ;

		if ( ! ScriptingMode )
		{
			AddDeser ( xsd99, CComBSTR(L"decimal"),			true,  VT_DECIMAL,	ads ) ;
			AddDeser ( xsd99, CComBSTR(L"uriReference"),	true,  VT_BSTR,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"short"),			true,  VT_I2,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"byte"),			true,  VT_I1,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"unsignedLong"),	true,  VT_UI4,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"unsignedInt"),		true,  VT_UI4,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"unsignedShort"),	true,  VT_UI2,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"unsignedByte"),	true,  VT_UI1,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"boolean"),			true,  VT_BOOL,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"int"),				true,  VT_I4,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"float"),			true,  VT_R4,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"double"),			true,  VT_R8,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"timeInstant"),		true,  VT_DATE,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"time"),			true,  VT_DATE,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"date"),			true,  VT_DATE,		ads ) ;
			AddDeser ( xsd99, CComBSTR(L"string"),			true,  VT_BSTR,		ads ) ;
		}
		AddDeser ( xsd99, CComBSTR(L"ur-type"),			true,  VT_VARIANT,	ads ) ;
		
		// although this is an array, it doesn't follow the array wire type, hence the false
		AddDeser ( xsd99, CComBSTR(L"base64Binary"),	false, VT_UI1 | VT_ARRAY, b64 ) ;	
	}


	// soap encoded arrays
	if ( ! ScriptingMode )
	{
		AddDeser ( xsd01, CComBSTR(L"decimal"),			true,  VT_DECIMAL,	arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"anyURI"),			true,  VT_BSTR,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"byte"),			true,  VT_I1,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"short"),			true,  VT_I2,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"int"),				true , VT_I4,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"long"),			true , VT_I8,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"unsignedLong"),	true,  VT_UI8,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"unsignedInt"),		true,  VT_UI4,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"unsignedShort"),	true,  VT_UI2,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"unsignedByte"),	true,  VT_UI1,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"boolean"),			true,  VT_BOOL,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"float"),			true , VT_R4,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"double"),			true , VT_R8,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"dateTime"),		true , VT_DATE,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"time"),			true,  VT_DATE,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"date"),			true,  VT_DATE,		arrds ) ;
		AddDeser ( xsd01, CComBSTR(L"string"),			true , VT_BSTR,		arrds ) ;
	}
	AddDeser ( xsd01, CComBSTR(L"anyType"),			true , VT_VARIANT,	arrds ) ;

	// although this is an array, it doesn't follow the array wire type, hence the false
	AddDeser ( SOAPENC,	CComBSTR(L"base64"),		false, VT_UI1 | VT_ARRAY, b64 ) ;
	AddDeser ( xsd01, CComBSTR(L"base64Binary"),	false, VT_UI1 | VT_ARRAY, b64 ) ;
	AddDeser ( xsd01, CComBSTR(L"hexBinary"),		false, VT_UI1 | VT_ARRAY, hexbin ) ;

	AddDeser ( xsd01, CComBSTR(L"QName"),			false, L"pocketSOAP.QName.1", qname ) ;
	AddDeser ( xsd01, CComBSTR(L"decimal"),			false, VT_DECIMAL,	ss ) ;
	AddDeser ( xsd01, CComBSTR(L"anyURI"),			false, VT_BSTR,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"byte"),			false, VT_I1,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"short"),			false, VT_I2,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"int"),				false, VT_I4,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"long"),			false, VT_I8,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"unsignedLong"),	false, VT_UI8,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"unsignedInt"),		false, VT_UI4,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"unsignedShort"),	false, VT_UI2,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"unsignedByte"),	false, VT_UI1,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"boolean"),			false, VT_BOOL,		bools ) ;
	AddDeser ( xsd01, CComBSTR(L"float"),			false, VT_R4,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"double"),			false, VT_R8,		ss ) ;
	AddDeser ( xsd01, CComBSTR(L"anyType"),			false, VT_VARIANT,	ss ) ;
	AddDeser ( xsd01, CComBSTR(L"time"),			false, VT_DATE,		ds ) ;
	AddDeser ( xsd01, CComBSTR(L"date"),			false, VT_DATE,		ds ) ;
	AddDeser ( xsd01, CComBSTR(L"dateTime"),		false, VT_DATE,		ds ) ;
	AddDeser ( xsd01, CComBSTR(L"string"),			false, VT_BSTR,		ss ) ;

	// todo
	//	uriReference
	
	// todo, check list in the SOAP spec / schema

	// setup types / mappings for the SOAP 1.1 Fault message
	AddGlobalType ( CComBSTR(L"Fault"),	SOAPENV, CComBSTR(L"Fault"), SOAPENV ) ;
	AddLocalType  ( SOAPENV, CComBSTR(L"Fault"), CComBSTR(L""), CComBSTR(L"faultcode"),   CComBSTR(L"QName"),  xsd01 ) ;
	AddLocalType  ( SOAPENV, CComBSTR(L"Fault"), CComBSTR(L""), CComBSTR(L"faultstring"), CComBSTR(L"string"), xsd01 ) ;

	if ( ! soap12 )
	{
		// soap 1.1 encoding element name to type mappings
		AddGlobalType ( CComBSTR(L"boolean"),		SOAPENC, CComBSTR(L"boolean"),		xsd01 ) ;
		AddGlobalType ( CComBSTR(L"float"),			SOAPENC, CComBSTR(L"float"),		xsd01 ) ;
		AddGlobalType ( CComBSTR(L"double"),		SOAPENC, CComBSTR(L"double"),		xsd01 ) ;
		AddGlobalType ( CComBSTR(L"decimal"),		SOAPENC, CComBSTR(L"decimal"),		xsd01 ) ;
		AddGlobalType ( CComBSTR(L"binary"),		SOAPENC, CComBSTR(L"base64Binary"),	xsd01 ) ;
		AddGlobalType ( CComBSTR(L"uriReference"),	SOAPENC, CComBSTR(L"anyURI"),		xsd01 ) ;
		AddGlobalType ( CComBSTR(L"long"),			SOAPENC, CComBSTR(L"long"),			xsd01 ) ;
		AddGlobalType ( CComBSTR(L"int"),			SOAPENC, CComBSTR(L"int"),			xsd01 ) ;
		AddGlobalType ( CComBSTR(L"short"),			SOAPENC, CComBSTR(L"short"),		xsd01 ) ;
		AddGlobalType ( CComBSTR(L"byte"),			SOAPENC, CComBSTR(L"byte"),			xsd01 ) ;
		AddGlobalType ( CComBSTR(L"unsignedLong"),	SOAPENC, CComBSTR(L"unsignedLong"),	xsd01 ) ;
		AddGlobalType ( CComBSTR(L"unsignedInt"),	SOAPENC, CComBSTR(L"unsignedInt"),	xsd01 ) ;
		AddGlobalType ( CComBSTR(L"unsignedShort"),	SOAPENC, CComBSTR(L"unsignedShort"),xsd01 ) ;
		AddGlobalType ( CComBSTR(L"unsignedByte"),	SOAPENC, CComBSTR(L"unsignedByte"),	xsd01 ) ;
		AddGlobalType ( CComBSTR(L"timeInstant"),	SOAPENC, CComBSTR(L"dateTime"),		xsd01 ) ;
		AddGlobalType ( CComBSTR(L"string"),		SOAPENC, CComBSTR(L"string"),		xsd01 ) ;
	}

	// serializers
	// only do 1999 schema for soap 1.1
	if ( ! soap12 )
	{
		AddSerVT ( VT_NULL,	   ss,	CComBSTR(L""),				CComBSTR(L"") ) ;
		AddSerVT ( VT_DECIMAL, ss,	CComBSTR(L"decimal"),		xsd99 ) ;
		AddSerVT ( VT_BOOL, bools,	CComBSTR(L"boolean"),		xsd99 ) ;
		AddSerVT ( VT_UI1,  ss,		CComBSTR(L"unsignedByte"),	xsd99 ) ;
		AddSerVT ( VT_UI2,  ss,		CComBSTR(L"unsignedShort"),	xsd99 ) ;
		AddSerVT ( VT_UI4,  ss,		CComBSTR(L"unsignedInt"),	xsd99 ) ;
		AddSerVT ( VT_I1,   ss,		CComBSTR(L"byte"),			xsd99 ) ;
		AddSerVT ( VT_I2,   ss,		CComBSTR(L"short"),			xsd99 ) ;
		AddSerVT ( VT_I4,   ss,		CComBSTR(L"int"),			xsd99 ) ;
		AddSerVT ( VT_R4,   ss,		CComBSTR(L"float"),			xsd99 ) ;
		AddSerVT ( VT_R8,   ss,		CComBSTR(L"double"),		xsd99 ) ;
		AddSerVT ( VT_DATE, ds,		CComBSTR(L"date"),			xsd99 ) ;
		AddSerVT ( VT_DATE, ds,		CComBSTR(L"time"),			xsd99 ) ;
		AddSerVT ( VT_DATE, ds,		CComBSTR(L"timeInstant"),	xsd99 ) ;
		AddSerVT ( VT_BSTR, ss,		CComBSTR(L"string"),		xsd99 ) ;
		
		AddSerVT ( VT_ARRAY | VT_DECIMAL,			as,	 CComBSTR(L"decimal"),		xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_BOOL,				as,	 CComBSTR(L"boolean"),		xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_UI2,				as,	 CComBSTR(L"unsignedShort"),xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_UI4,				as,	 CComBSTR(L"unsignedInt"),	xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_I2,				as,	 CComBSTR(L"short"),		xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_I4,				as,	 CComBSTR(L"int"),			xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_R4,				as,	 CComBSTR(L"float"),		xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_R8,				as,	 CComBSTR(L"double"),		xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_DATE,				as,	 CComBSTR(L"timeInstant"),	xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_BSTR,				as,	 CComBSTR(L"string"),		xsd99	) ;
		AddSerVT ( VT_ARRAY | VT_VARIANT,			as,	 CComBSTR(L"ur-type"),		xsd99	) ;
	}

	AddSerVT ( VT_DECIMAL, ss,	CComBSTR(L"decimal"),		xsd01 ) ;
	AddSerVT ( VT_BOOL, bools,	CComBSTR(L"boolean"),		xsd01 ) ;
	AddSerVT ( VT_UI1,  ss,		CComBSTR(L"unsignedByte"),	xsd01 ) ;
	AddSerVT ( VT_UI2,  ss,		CComBSTR(L"unsignedShort"),	xsd01 ) ;
	AddSerVT ( VT_UI4,  ss,		CComBSTR(L"unsignedInt"),	xsd01 ) ;
	AddSerVT ( VT_UI8,  ss,		CComBSTR(L"unsignedLong"),	xsd01 ) ;
	AddSerVT ( VT_I1,   ss,		CComBSTR(L"byte"),			xsd01 ) ;
	AddSerVT ( VT_I2,   ss,		CComBSTR(L"short"),			xsd01 ) ;
	AddSerVT ( VT_I4,   ss,		CComBSTR(L"int"),			xsd01 ) ;
	AddSerVT ( VT_I8,   ss,		CComBSTR(L"long"),			xsd01 ) ;
	AddSerVT ( VT_R4,   ss,		CComBSTR(L"float"),			xsd01 ) ;
	AddSerVT ( VT_R8,   ss,		CComBSTR(L"double"),		xsd01 ) ;
	AddSerVT ( VT_DATE, ds,		CComBSTR(L"date"),			xsd01 ) ;
	AddSerVT ( VT_DATE, ds,		CComBSTR(L"time"),			xsd01 ) ;
	AddSerVT ( VT_DATE, ds,		CComBSTR(L"dateTime"),		xsd01 ) ;
	AddSerVT ( VT_BSTR, ss,		CComBSTR(L"string"),		xsd01 ) ;

	AddSerVT ( VT_ARRAY | VT_DECIMAL,			arrSer,	 CComBSTR(L"decimal"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_BOOL,				arrSer,	 CComBSTR(L"boolean"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_UI2,				arrSer,	 CComBSTR(L"unsignedShort"),xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_UI4,				arrSer,	 CComBSTR(L"unsignedInt"),	xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_UI8,				arrSer,	 CComBSTR(L"unsignedLong"),	xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_I2,				arrSer,	 CComBSTR(L"short"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_I4,				arrSer,	 CComBSTR(L"int"),			xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_I8,				arrSer,	 CComBSTR(L"long"),			xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_R4,				arrSer,	 CComBSTR(L"float"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_R8,				arrSer,	 CComBSTR(L"double"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_DATE,				arrSer,	 CComBSTR(L"dateTime"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_BSTR,				arrSer,	 CComBSTR(L"string"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_VARIANT,			arrSer,	 CComBSTR(L"anyType"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_UNKNOWN,			arrSer,	 CComBSTR(L"anyType"),		xsd01	) ;
	AddSerVT ( VT_ARRAY | VT_DISPATCH,			arrSer,	 CComBSTR(L"anyType"),		xsd01	) ;

	AddSerVT ( VT_ARRAY | VT_UI1,				b64, CComBSTR(L"base64"),		SOAPENC ) ;
	AddSerVT ( VT_ARRAY | VT_I1,				b64, CComBSTR(L"base64"),		SOAPENC ) ;

	AddSerVT ( VT_ARRAY | VT_UI1,				b64, CComBSTR(L"base64Binary"),	xsd01 ) ;
	AddSerVT ( VT_ARRAY | VT_I1,				b64, CComBSTR(L"base64Binary"),	xsd01 ) ;

	AddSerIID ( __uuidof(IXmlQName),			qname,	CComBSTR(L"QName"),		    xsd99   ) ;
	AddSerIID ( __uuidof(IXmlQName),			qname,	CComBSTR(L"QName"),		    xsd01   ) ;
	AddSerIID ( __uuidof(ISOAPNode),			ns,		null,						null	) ;
	AddSerIID ( __uuidof(IPersistPropertyBag),	pbs,	null,						null	) ;

	AddSerVT  ( VT_EMPTY,				ss, null, null ) ;
	AddSerVT  ( VT_UNKNOWN,				ss, null, null ) ;
	AddSerVT  ( VT_DISPATCH,			ss, null, null ) ;
	
	m_Loaded = true ;
}

void SerializerFactoryConfig::AddSerVT  ( const VARTYPE vt,  BSTR progID, BSTR xmlType, BSTR xmlTypeNS )
{
	SimpleSerializer s ;
	s.vt = vt ;
	s.progID = progID ;
	s.xmlType = xmlType ;
	s.xmlTypeNS = xmlTypeNS ;
	s.clsidOK = false ;
	m_simpleSer.push_back(s) ;
}

void SerializerFactoryConfig::AddSerIID ( REFIID riid, BSTR progID, BSTR xmlType, BSTR xmlTypeNS )
{
	InterfaceSerializer s ;
	s.iid = riid ;
	s.progID = progID ;
	s.xmlType = xmlType ;
	s.xmlTypeNS = xmlTypeNS ;
	s.clsidOK = false ;
	m_ifaceSer.push_back(s) ;
}

void SerializerFactoryConfig::AddDeser  ( BSTR ns, BSTR xmlType, bool IsArray, VARIANT * comType, BSTR progID ) 
{
	if ( comType->vt == VT_BSTR ) 
		AddDeser ( ns, xmlType, IsArray, comType->bstrVal, progID ) ;
	else if ( comType->vt == ( VT_BSTR | VT_BYREF ) )
		AddDeser ( ns, xmlType, IsArray, *comType->pbstrVal, progID ) ;
	else
		AddDeser ( ns, xmlType, IsArray, (USHORT)comType->lVal, progID ) ;
}

void SerializerFactoryConfig::AddDeser  ( BSTR ns, BSTR xmlType, bool IsArray, BSTR comType, BSTR progID )
{
	DeserializerEntry d ;
	d.Namespace = ns ;
	d.XmlType = xmlType ;
	d.IsArray = IsArray ;
	d.ComType = comType ;
	d.progID  = progID ;
	StoreDeser(d) ;
}

void SerializerFactoryConfig::AddDeser  ( BSTR ns, BSTR xmlType, bool IsArray, VARTYPE comType, BSTR progID )
{
	DeserializerEntry d ;
	d.Namespace = ns ;
	d.XmlType = xmlType ;
	d.IsArray = IsArray ;
	d.ComType = comType ;
	d.progID  = progID ;
	StoreDeser(d) ;
}

void SerializerFactoryConfig::StoreDeser ( DeserializerEntry & d ) 
{
	d.clsidOK = false ;
	m_des[makeKey(d.XmlType, d.Namespace, d.IsArray)] = d ;
}

stringBuff_W SerializerFactoryConfig::makeKey ( BSTR Type, BSTR ns, bool Array ) 
{
	stringBuff_W k ;
	k.Allocate ( SysStringLen(Type) + SysStringLen(ns) + 4 ) ;
	k.Append ( Type, SysStringLen(Type) ) ;
	if ( Array )
		k.Append(L"!a!") ;
	else
		k.Append(L"!s!") ;
	k.Append (ns, SysStringLen(ns) ) ;
	return k ;
}

void SerializerFactoryConfig::AddGlobalType  ( BSTR name, BSTR ns, BSTR XmlType, BSTR TypeNamespace )
{
	ElementMap m ;
	m.Name = name ;
	m.Namespace = ns ;
	m.XmlType = XmlType ;
	m.XmlNamespace = TypeNamespace ;
	m_etypes.push_back(m);
}

void SerializerFactoryConfig::AddLocalType ( BSTR ParentNS, BSTR ParentType, BSTR ChildNS, BSTR ChildName, BSTR xmlType, BSTR xmlTypeNS )
{
	ElementMap m ;
	m.Name = ChildName ;
	m.Namespace = ChildNS ;
	m.XmlType = xmlType ;
	m.XmlNamespace = xmlTypeNS ;
	stringBuff_W k = makeKey(ParentType, ParentNS) ;
	MAP_ELEMENTTYPES::iterator i = m_localtypes.find(k) ;
	if ( i == m_localtypes.end() )
	{
		ELEMENTTYPES t ;
		t.push_back(m) ;
		m_localtypes[k] = t ;
	}
	else
		i->second.push_back(m) ;
}

/////////////////////////////////////////////////////////////////////////////
// CPool
/////////////////////////////////////////////////////////////////////////////
bool operator < ( const GUID &a, const GUID &b)
{
	return memcmp(&a, &b, sizeof(GUID)) < 0 ;
}

CPool::CPool()
{
	requests = 0 ;
	hits = 0 ;
}

CPool::~CPool()
{
}

HRESULT CPool::fetch ( /*[in]*/ REFCLSID clsid, /*[out,retval]*/ IUnknown ** ppUnk ) 
{
	// check the pool first
	++requests ;
	READYPOOL::iterator i = ready.find(clsid) ;
	if ( i != ready.end() )
	{
		i->second.CopyTo(ppUnk) ;
		++hits ;
		active.insert(ACTIVEPOOL::value_type(i->second, clsid)) ;
		ready.erase(i) ;
		return S_OK ;
	}

	// nope, ok, lets create one
	CComPtr<IUnknown> punk ;
	HRESULT hr = punk.CoCreateInstance(clsid) ;
	if (FAILED(hr)) return hr ;
	punk.CopyTo(ppUnk) ;
	active[punk] = clsid ;
	return S_OK ;
}

void CPool::completed ( /*[in]*/ IUnknown * punk ) 
{
	IUnknown * pId = 0 ;
	punk->QueryInterface(__uuidof(pId),(void **)&pId) ;
	
	ACTIVEPOOL::iterator i = active.find(pId) ;
	if ( i != active.end() )
	{
		ready.insert(READYPOOL::value_type(i->second, pId)) ;
		active.erase(i);
	}
	pId->Release() ;
}

size_t	CPool::size() 
{
	return ready.size() + active.size() ;
}

void CPool::clear()
{
	ATLTRACE(_T("CPool::clear() : Pool stat's : requests=%d\t\tpool hits=%d\t\tpool size=%d\n"), requests, hits, size() ) ;
	ready.clear() ;
	active.clear() ;
}

/////////////////////////////////////////////////////////////////////////////
// CSerializerFactory
/////////////////////////////////////////////////////////////////////////////
CSerializerFactory::CSerializerFactory() : 
	m_rootFirst(VARIANT_TRUE), 
	m_mode(sfcNormal_11), 
	m_prefNamespace(xsd01), 
	m_state(csNotSet), 
	m_cfg(0),
	m_understoodHeadersSorted(true)	// there are no entries, so they are already in order !
{
	ATLTRACE(_T("CSerializerFactory::CSerializerFactory()\n")) ;
}

CSerializerFactory::~CSerializerFactory()
{
	ClearConfig() ;
	ATLTRACE(_T("CSerializerFactory::~CSerializerFactory()\n")) ;
	CSOAPNode::DumpActivityStats() ;
}

void CSerializerFactory::ClearConfig()
{
	if ( m_state == csCustom )
		delete m_cfg ;
	m_cfg = 0 ;
	m_state = csNotSet ;
}

void CSerializerFactory::InitConfig(bool custom)
{
	if ( ( m_state == csCustom ) || (( m_state == csLoaded ) && ( ! custom )) )
		return ;
	SerializerFactoryConfig * base = m_cfgMgr.Config(m_mode) ;
	if ( custom )
	{
		m_cfg = new SerializerFactoryConfig(*base) ;
		m_state = csCustom ;
	}
	else
	{
		m_cfg = base ;
		m_state = csLoaded ;
	}
}

STDMETHODIMP CSerializerFactory::SetConfig( /*[in]*/ VARIANT config )
{
	CComVariant v ;
	HRESULT hr = v.ChangeType( VT_I4, &config ) ;
	if (SUCCEEDED(hr))
	{
		if ( ( v.lVal == sfcNormal_11 ) || ( v.lVal == sfcScripting_11 ) ||
			 ( v.lVal == sfcNormal_12 ) || ( v.lVal == sfcScripting_12 ) ) 
		{
			sfConfigOptions newMode = (sfConfigOptions)v.lVal ;
			if ( (( newMode != m_mode ) && (m_state == csLoaded )) ||  m_state == csCustom ) 
				ClearConfig() ;
			
			m_mode = newMode ;
			return S_OK ;
		}
	}
	return AtlReportError ( GetObjectCLSID(), OLESTR("Invalid config option"), __uuidof(ISerializerFactoryConfig), E_INVALIDARG ) ;
}

STDMETHODIMP CSerializerFactory::IsAnyType( /*[in]*/ BSTR XmlType, /*[in]*/ BSTR XmlTypeNamespace, /*[out,retval]*/ VARIANT_BOOL * IsAnyType )
{
	static const CComBSTR urType(L"ur-type") ;
	static const CComBSTR anyType(L"anyType") ;

	if ( ( bstrEqual ( XmlType, anyType ) && bstrEqual ( XmlTypeNamespace, xsd01 ) ) ||
		 ( bstrEqual ( XmlType, urType  ) && bstrEqual ( XmlTypeNamespace, xsd99 ) ) )
		 *IsAnyType = VARIANT_TRUE ;
	else
		*IsAnyType = VARIANT_FALSE ;
	return S_OK ;
}

// given two Xml types, is the de-serialized COM type equivilant ?
STDMETHODIMP CSerializerFactory::AreEqualComTypes( /*[in]*/ BSTR XmlTypeA, /*[in]*/ BSTR XmlTypeNSA, /*[in]*/ BSTR XmlTypeB, /*[in]*/ BSTR XmlTypeNSB, /*[out,retval]*/ VARIANT_BOOL * Match )
{
	if ( ! Match ) return E_POINTER ;
	*Match = VARIANT_FALSE ;
	InitConfig(false);

	DESERIALIZERS::const_iterator iA = m_cfg->m_des.find(m_cfg->makeKey(XmlTypeA, XmlTypeNSA, false)) ;
	DESERIALIZERS::const_iterator iB = m_cfg->m_des.find(m_cfg->makeKey(XmlTypeB, XmlTypeNSB, false)) ;
	if ( iA == m_cfg->m_des.end() || iB == m_cfg->m_des.end() )
		return S_OK ;

	const CComVariant &TypeA = iA->second.ComType ;
	const CComVariant &TypeB = iB->second.ComType ;
	if ( TypeA.vt != TypeB.vt )
		return S_OK ;

	if ( TypeA.vt == VT_I4 && TypeA.lVal == TypeB.lVal )
		*Match = VARIANT_TRUE ;

	if ( TypeA.vt == VT_BSTR && bstrEqual ( TypeA.bstrVal, TypeB.bstrVal ) )
		*Match = VARIANT_TRUE ;

	return S_OK ;
}

STDMETHODIMP CSerializerFactory::FindComType( /*[in]*/ BSTR XmlType, /*[in]*/ BSTR XmlTypeNamespace, /*[out,retval]*/ VARIANT * comType )
{
	if ( ! comType ) return E_POINTER ;
	VariantInit(comType) ;
	InitConfig(false) ;
	DESERIALIZERS::const_iterator i = m_cfg->m_des.find(m_cfg->makeKey(XmlType, XmlTypeNamespace, false)) ;
	if ( i == m_cfg->m_des.end() )
		return E_INVALIDARG ;

	return VariantCopy ( comType, (VARIANT *)&i->second.ComType ) ;
}

STDMETHODIMP CSerializerFactory::Deserializer( /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNamespace, /*[in]*/ VARIANT_BOOL ArrayOf, /*[in]*/ VARIANT ComType, /*[in]*/ BSTR ProgID )
{
	InitConfig(true) ;
	m_cfg->AddDeser(TypeNamespace, Type, ArrayOf == VARIANT_TRUE, &ComType, ProgID) ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::ElementMapping( /*[in]*/ BSTR ElementName, /*[in]*/ BSTR ElementNamespace, /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNamespace )
{
	InitConfig(true) ;
	m_cfg->AddGlobalType ( ElementName, ElementNamespace, Type, TypeNamespace )  ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::Serializer( /*[in]*/ VARIANT ComType, /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNamespace, /*[in]*/ BSTR ProgID )
{
	InitConfig(true) ;
	if ( ComType.vt == VT_BSTR )
	{
		GUID refiid ;
		// this really should be IIDFromString, but it doesn't link on PPC for some reason
		CLSIDFromString(ComType.bstrVal, &refiid) ;
		m_cfg->AddSerIID ( refiid, ProgID, Type, TypeNamespace ) ;
	}
	else
	{
		m_cfg->AddSerVT ( (USHORT)ComType.lVal, ProgID, Type, TypeNamespace ) ;
	}
	return S_OK ;
}


// TODO: ok, so this is not the most efficient way to do this, but it works for now
HRESULT CSerializerFactory::FindSimpleSerializer ( const VARTYPE vt, BSTR preferedType, BSTR preferedTypeNS, searchStage stage, ISoapSerializer ** s, BSTR * type, BSTR * typeNS ) 
{
	for ( SIMPLE_SERIALIZERS::reverse_iterator i = m_cfg->m_simpleSer.rbegin() ; i != m_cfg->m_simpleSer.rend() ; i++ )
	{
		if ( vt == i->vt )
		{
			if ( ( stage == ssAny ) || 
				 ((stage == ssPreferedXsd) && bstrEqual(i->xmlTypeNS, m_prefNamespace)) ||
				 ((stage == ssPreferedType) && bstrEqual(i->xmlType, preferedType) && bstrEqual(i->xmlTypeNS, preferedTypeNS)) )
			{
				CComPtr<IUnknown> ss ;
				HRESULT hr ;
				if ( ! i->clsidOK ) 
				{
					hr = CLSIDFromProgID(i->progID, &i->clsid ) ;
					if (FAILED(hr))
						return ReportError ( hr, L"Failed creaating an instance of the serializer '%s'", i->progID ) ;
					i->clsidOK = true ;
				}
				hr = m_pool.fetch ( i->clsid, &ss ) ;
				if (FAILED(hr))
					return ReportError ( hr, L"Failed creaating an instance of the serializer '%s'", i->progID ) ;

				CComQIPtr<ITypesInit> t(ss) ;
				if (t)
					t->Initialize ( i->xmlType, i->xmlTypeNS, CComVariant() ) ;
				if ( type )
				{
					ATLASSERT(*type == NULL) ;
					i->xmlType.CopyTo(type) ;
				}
				if ( typeNS ) 
				{
					ATLASSERT(*typeNS == NULL) ;
					i->xmlTypeNS.CopyTo(typeNS) ;
				}

				return ss.QueryInterface(s) ;
			}
		}
	}
	if ( ssPreferedType == stage )
		return FindSimpleSerializer ( vt, preferedType, preferedTypeNS, ssPreferedXsd, s, type, typeNS ) ;
	if ( ssPreferedXsd == stage  )
		return FindSimpleSerializer ( vt, preferedType, preferedTypeNS, ssAny,         s, type, typeNS ) ;
	return E_NO_SERIALIZER ;
}

HRESULT CSerializerFactory::FindSimpleSerializer ( const VARTYPE vt, BSTR preferedType, BSTR preferedTypeNS, ISoapSerializer ** s, BSTR * type, BSTR * typeNS ) 
{
	InitConfig(false);
	searchStage stage = ssPreferedType ;
	if ( SysStringLen(preferedType) == 0 )
		stage = ssPreferedXsd ;

	return FindSimpleSerializer ( vt, preferedType, preferedTypeNS, stage, s, type, typeNS ) ;
}

HRESULT CSerializerFactory::FindInterfaceSerializer ( IUnknown * punk, BSTR prefType, BSTR prefTypeNS, ISoapSerializer ** s, BSTR * type, BSTR * typeNS )
{
	InitConfig(false);
	HRESULT hr ;
	IUnknown * dest = 0 ;
	for ( IFACE_SERIALIZERS::reverse_iterator i = m_cfg->m_ifaceSer.rbegin() ; i != m_cfg->m_ifaceSer.rend() ; i++ )
	{
		hr = punk->QueryInterface(i->iid, (void **)&dest ) ;
		if (SUCCEEDED(hr))
		{
			dest->Release() ;
			CComPtr<IUnknown> ss ;
			if ( ! i->clsidOK )
			{
				hr = CLSIDFromProgID ( i->progID, &i->clsid ) ;
				if (FAILED(hr))
					return ReportError ( hr, L"Failed creating an instance of the serializer '%s'", i->progID ) ;
				i->clsidOK = true ;
			}
			hr = m_pool.fetch(i->clsid, &ss ) ;
			if (FAILED(hr))
				return ReportError ( hr, L"Failed creating an instance of the serializer '%s'", i->progID ) ;

			CComQIPtr<ITypesInit> t(ss) ;
			if (t)
				t->Initialize ( i->xmlType, i->xmlTypeNS, CComVariant() ) ;
			if (type)
			{
				ATLASSERT(*type == NULL) ;
				i->xmlType.CopyTo(type) ;
			}
			if ( typeNS) 
			{
				ATLASSERT(*typeNS == NULL) ;
				i->xmlTypeNS.CopyTo(typeNS) ;
			}

			return ss.QueryInterface(s) ;
		}
	}
	return E_NO_SERIALIZER ;
}

STDMETHODIMP CSerializerFactory::SerializerForValue( /*[in]*/ VARIANT * v, /*[in]*/ BSTR ParentType, /*[in]*/ BSTR ParentTypeNS, /*[in]*/ BSTR name, /*[in]*/ BSTR Namespace, /*[out]*/ BSTR * type, /*[out]*/ BSTR * typeNamespace, /*[out,retval]*/ ISoapSerializer ** s ) 
{
	if ( ! type ) return E_POINTER ; 
	if ( ! typeNamespace ) return E_POINTER ; 
	InitConfig(false);
	CComVariant vd ;
	VariantCopyInd ( &vd, v ) ;
	VARTYPE vt = vd.vt ;

	CComBSTR prefType, prefTypeNS ;
	FindType ( ParentType, ParentTypeNS, name, Namespace, &prefType, &prefTypeNS ) ;

	if ( ( vt == VT_DISPATCH || vt == VT_UNKNOWN ) && vd.punkVal )
		return FindInterfaceSerializer ( vd.punkVal, prefType, prefTypeNS, s, type, typeNamespace ) ;
	else
		return FindSimpleSerializer ( vt, prefType, prefTypeNS, s, type, typeNamespace ) ;
}

STDMETHODIMP CSerializerFactory::SerializerForNode( /*[in]*/ ISOAPNode * n, /*[in]*/ BSTR ParentType,  /*[in]*/ BSTR ParentTypeNS, /*[out]*/ BSTR * type, /*[out]*/ BSTR * typeNamespace, /*[out,retval]*/ ISoapSerializer ** s ) 
{
	InitConfig(false);
	CComVariant v ;
	n->get_Value(&v) ;
	CComBSTR name, ns ;
	n->get_Name(&name) ;
	n->get_Namespace(&ns) ;
	return SerializerForValue ( &v, ParentType, ParentTypeNS, name, ns, type, typeNamespace, s ) ;
}

STDMETHODIMP CSerializerFactory::ReturnSerializer( /*[in]*/ ISoapSerializer * s )
{
	m_pool.completed(s) ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::SerializerForValue( /*[in]*/ VARIANT * v, /*[out,retval]*/ ISoapSerializer ** s )
{
	return SerializerForValue ( v, NULL, NULL, NULL, NULL, NULL, NULL, s ) ;
}

STDMETHODIMP CSerializerFactory::SerializerForNode( /*[in]*/ ISOAPNode * n, /*[out,retval]*/ ISoapSerializer ** s )
{
	return SerializerForNode ( n, NULL, NULL, NULL, NULL, s ) ;
}


STDMETHODIMP CSerializerFactory::DeserializerForType( /*[in]*/ BSTR XmlType, /*[in]*/ BSTR XmlTypeNamespace, /*[in] */ VARIANT_BOOL IsArray, /*[out,retval]*/ ISoapDeSerializer ** s ) 
{
	InitConfig(false);
	if ( SysStringLen(XmlType) == 0 && SysStringLen(XmlTypeNamespace) == 0 )
		return E_NO_DESERIALIZER ;

	DESERIALIZERS::iterator i = m_cfg->m_des.find(m_cfg->makeKey(XmlType, XmlTypeNamespace, IsArray == VARIANT_TRUE)) ;
	if ( i == m_cfg->m_des.end() )
		return E_NO_DESERIALIZER ;

	HRESULT hr ;
	CComPtr<IUnknown> ss ;
	if ( ! i->second.clsidOK )
	{
		hr = CLSIDFromProgID(i->second.progID, &i->second.clsid ) ;
		if (FAILED(hr))
			return ReportError ( hr, L"Failed to create an instance of de-serializer '%s'", i->second.progID ) ;
		i->second.clsidOK = true ;
	}
	hr = m_pool.fetch(i->second.clsid, &ss ) ;
	if (FAILED(hr))
		return ReportError ( hr, L"Failed to create an instance of de-serializer '%s'", i->second.progID ) ;

	CComQIPtr<ITypesInit> init(ss) ;
	if ( init )
	{
		hr = init->Initialize ( XmlType, XmlTypeNamespace, i->second.ComType ) ;
		if (FAILED(hr))
			return ReportError ( hr, L"Failed to initialize instance of de-serializer '%s'", i->second.progID ) ;
	}
	return ss.QueryInterface(s) ;
}

STDMETHODIMP CSerializerFactory::DeserializerForElement( /*[in]*/ BSTR elementName, /*[in]*/ BSTR elementNamespace, /*[in]*/ VARIANT_BOOL IsArray, /*[out,retval]*/ ISoapDeSerializer ** s )
{
	return DeserializerForElement ( elementName, elementNamespace, IsArray, NULL, NULL, s ) ;
}

HRESULT CSerializerFactory::DeserializerForElement  ( /*[in]*/ BSTR elementName, /*[in]*/ BSTR elementNamespace, /*[in]*/ VARIANT_BOOL IsArray, BSTR * XmlType, BSTR * XmlTypeNS, /*[out,retval]*/ ISoapDeSerializer ** s )
{
	InitConfig(false);
	ATLTRACE(_T("DeserializerForElement [%ls]:%ls %d\n"), elementNamespace, elementName, IsArray ) ;
	HRESULT hr = FindType ( m_cfg->m_etypes, elementName, elementNamespace, IsArray, XmlType, XmlTypeNS, s ) ;
	if (SUCCEEDED(hr)) return hr ;

	// todo : do we want to keep this, or throw an error ?
	if ( IsArray == VARIANT_TRUE ) 
	{
		static CComBSTR anyType(L"anyType") ;
		hr = DeserializerForType ( anyType, xsd01, VARIANT_TRUE, s ) ;
		if ( SUCCEEDED(hr))
		{
			if ( XmlType )   *XmlType   = anyType.Copy();
			if ( XmlTypeNS ) *XmlTypeNS = xsd01.Copy() ;
		}
		return hr ;
	}
	
	CComPtr<ISoapDeSerializer> ss ;
	CComPtr<IUnknown> punk ;
	hr = m_pool.fetch ( CLSID_CoSerializerNode, &punk ) ;
	if (FAILED(hr)) return hr ;
	punk->QueryInterface(__uuidof(ss), (void **)&ss) ;
	CComQIPtr<ITypesInit> init(ss) ;
	if ( init )
	{
		hr = init->Initialize ( CComBSTR(), CComBSTR(), CComVariant() ) ;
		if (FAILED(hr))
			return ReportError ( hr, L"Failed to initialize instance of de-serializer '%ls'", CComBSTR(CLSID_CoSerializerNode) ) ;
	}
	return ss.QueryInterface(s) ;
}

STDMETHODIMP CSerializerFactory::FindType( /*[in]*/ BSTR ParentType,  /*[in]*/ BSTR ParentTypeNS, /*[in]*/ BSTR elementName, /*[in]*/ BSTR elementNamespace, /*[out]*/ BSTR * XmlType, /*[out]*/ BSTR * XmlTypeNS ) 
{
	if ( ! XmlType ) return E_POINTER ;
	if ( ! XmlTypeNS ) return E_POINTER ;
	ATLASSERT(*XmlType==0) ;
	ATLASSERT(*XmlTypeNS==0);
	*XmlType = 0 ;
	*XmlTypeNS = 0 ;
	InitConfig(false) ;
	// passing VARIANT_FALSE for IsArray doesn't matter, as that only affects the de-serializer creation
	if ( SysStringLen(ParentType) > 0 )
	{
		// find the local mappings for the parent type
		MAP_ELEMENTTYPES::iterator i = m_cfg->m_localtypes.find(m_cfg->makeKey(ParentType, ParentTypeNS)) ;
		if ( i != m_cfg->m_localtypes.end() )
		{	
			HRESULT hr = FindType ( i->second, elementName, elementNamespace, VARIANT_FALSE, XmlType, XmlTypeNS, NULL ) ;
			if (SUCCEEDED(hr)) return hr ;
		}
	}
	// otherwise look for a global mapping
	HRESULT hr = FindType ( m_cfg->m_etypes, elementName, elementNamespace, VARIANT_FALSE, XmlType, XmlTypeNS, NULL ) ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::DeserializerForElement( /*[in]*/ BSTR ParentType,  /*[in]*/ BSTR ParentTypeNS,     /*[in]*/ BSTR elementName,     /*[in]*/ BSTR elementNamespace, /*[in]*/ VARIANT_BOOL IsArray, /*[out]*/ BSTR * XmlType, /*[out]*/ BSTR * XmlTypeNS, /*[out,retval]*/ ISoapDeSerializer ** s ) 
{
	return DeserializerForChild(ParentType, ParentTypeNS, elementName, elementNamespace, IsArray, XmlType, XmlTypeNS, s ) ;
}

STDMETHODIMP CSerializerFactory::DeserializerForChild( /*[in]*/ BSTR ParentType, 
													   /*[in]*/ BSTR ParentTypeNS, 
													   /*[in]*/ BSTR elementName, 
													   /*[in]*/ BSTR elementNamespace, 
													   /*[in]*/ VARIANT_BOOL IsArray, 
													   /*[in,out]*/ BSTR * XmlType, 
													   /*[in,out]*/ BSTR * XmlTypeNS, 
													   /*[out,retval]*/ ISoapDeSerializer ** s ) 
{
	if ( ! s ) return E_POINTER ;
	if ( ! XmlType ) return E_POINTER ;
	if ( ! XmlTypeNS ) return E_POINTER ;

	InitConfig(false) ;
	// find the local mappings for the parent type
	MAP_ELEMENTTYPES::iterator i = m_cfg->m_localtypes.find(m_cfg->makeKey(ParentType, ParentTypeNS)) ;
	if ( i != m_cfg->m_localtypes.end() )
	{	
		HRESULT hr = FindType ( i->second, elementName, elementNamespace, IsArray, XmlType, XmlTypeNS, s ) ;
		if (SUCCEEDED(hr)) return hr ;
	}
	// otherwise look for a global mapping
	return DeserializerForElement ( elementName, elementNamespace, IsArray, XmlType, XmlTypeNS, s ) ;
}


HRESULT CSerializerFactory::FindType ( const ELEMENTTYPES &types, BSTR Name, BSTR NameNS, VARIANT_BOOL IsArray, BSTR * XmlType, BSTR * XmlTypeNS, ISoapDeSerializer ** s )
{
	for ( ELEMENTTYPES::const_reverse_iterator i = types.rbegin(); i != types.rend() ; i++ )
	{
		if ( bstrEqual ( Name, i->Name ) && bstrEqual ( NameNS, i->Namespace ) )
		{
			if ( XmlType )		*XmlType   = i->XmlType.Copy() ;
			if ( XmlTypeNS )	*XmlTypeNS = i->XmlNamespace.Copy(); 
			HRESULT hr = S_OK ;
			if ( s ) 
				hr = DeserializerForType ( i->XmlType, i->XmlNamespace, IsArray, s ) ;
			return hr ;
		}
	}
	return E_NO_DESERIALIZER ;
}

STDMETHODIMP CSerializerFactory::ReturnDeSerializer( /*[in]*/ ISoapDeSerializer * s )
{
	m_pool.completed(s) ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::Reset()
{
	m_pool.clear() ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::Fetch( /*[in]*/ REFCLSID clsid, /*[out,retval]*/ IUnknown ** ppUnk ) 
{
	return m_pool.fetch(clsid, ppUnk) ;
}

STDMETHODIMP CSerializerFactory::XsiForPrimaryNS( /*[out,retval]*/ BSTR * uri )
{
	if ( ! uri ) return E_POINTER ;
	if ( bstrEqual ( m_prefNamespace, xsd01 ))
		*uri = SysAllocString ( XSI01_URI ) ;
	else if ( bstrEqual ( m_prefNamespace, xsd99 ))
		*uri = SysAllocString ( XSI99_URI ) ;
	else
		*uri = SysAllocString ( XSI01_URI ) ;

	return S_OK ;
}

STDMETHODIMP CSerializerFactory::get_PrimarySchema( /*[out,retval]*/ BSTR * uri )
{
	return m_prefNamespace.CopyTo(uri) ;
}

STDMETHODIMP CSerializerFactory::put_PrimarySchema( /*[in]*/ BSTR uri ) 
{
	if ( bstrEqual ( uri, xsd01 ) || bstrEqual ( uri, xsd99 ) )
	{
		m_prefNamespace = uri ;
		return S_OK ;
	}
	return ReportError ( E_UNKNOWN_PRIMARYSCHEMA, L"Unknown primary schema URI, primary schema can be '%s' or '%s'", xsd01, xsd99 ) ;
}

STDMETHODIMP CSerializerFactory::get_RootFirst( /*[out,retval]*/ VARIANT_BOOL * rootFirst )
{
	if ( ! rootFirst ) return E_POINTER ;
	*rootFirst = m_rootFirst ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::put_RootFirst( /*[in]*/ VARIANT_BOOL rootFirst )
{
	m_rootFirst = rootFirst ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::LocalTypeMapping( /*[in]*/ BSTR ParentXmlType, /*[in]*/ BSTR ParentXmlTypeNS, /*[in]*/ BSTR ChildName,  /*[in]*/ BSTR ChildNamespace, /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNamespace ) 
{
	InitConfig(true) ;
	m_cfg->AddLocalType ( ParentXmlTypeNS, ParentXmlType, ChildNamespace, ChildName, Type, TypeNamespace ) ;
	return S_OK ;
}

STDMETHODIMP CSerializerFactory::understoodHeader( /*[in]*/ BSTR NamespaceURI, /*[in]*/ BSTR localName )
{
	m_understoodHeaders.push_back(makeHeaderKey(NamespaceURI, localName)) ;
	m_understoodHeadersSorted = false ;
	return S_OK;
}

STDMETHODIMP CSerializerFactory::isUnderstood( /*[in]*/ BSTR NamespaceURI, /*[in]*/ BSTR localName, /*[out,retval]*/ VARIANT_BOOL *understood )
{
	if ( ! understood ) return E_POINTER ;

	if ( m_understoodHeadersSorted )
	{
		std::sort ( m_understoodHeaders.begin(), m_understoodHeaders.end()) ;
		m_understoodHeadersSorted = true ;
	}

	*understood = std::binary_search ( m_understoodHeaders.begin(), m_understoodHeaders.end(), makeHeaderKey(NamespaceURI, localName)) ? VARIANT_TRUE : VARIANT_FALSE ;
	return S_OK ;
}

std::wstring CSerializerFactory::makeHeaderKey(BSTR NamespaceURI, BSTR localName) 
{
	std::wstring h ;
	DWORD cns = SysStringLen(NamespaceURI) ;
	DWORD cln = SysStringLen(localName) ;
	h.reserve(cns+cln+1) ;
	h.append(localName, cln) ;
	h.append(1, '#') ;
	h.append(NamespaceURI, cns ) ;
	return h ;
}

