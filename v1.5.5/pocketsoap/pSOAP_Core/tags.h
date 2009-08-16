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
Portions created by Simon Fell are Copyright (C) 2000, 2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#pragma once

_declspec(selectany) LPCWSTR XSD99_URI					= L"http://www.w3.org/1999/XMLSchema" ;
_declspec(selectany) LPCWSTR XSI99_URI					= L"http://www.w3.org/1999/XMLSchema-instance" ;
_declspec(selectany) LPCWSTR XSD01_URI					= L"http://www.w3.org/2001/XMLSchema" ;
_declspec(selectany) LPCWSTR XSI01_URI					= L"http://www.w3.org/2001/XMLSchema-instance" ;

_declspec(selectany) LPCWSTR XSD99_SCHEMA_INSTANCE_TYPE = L"http://www.w3.org/1999/XMLSchema-instance!type" ;
_declspec(selectany) LPCWSTR XSD01_SCHEMA_INSTANCE_TYPE = L"http://www.w3.org/2001/XMLSchema-instance!type" ;
_declspec(selectany) LPCWSTR XSD99_SCHEMA_INSTANCE_NULL = L"http://www.w3.org/1999/XMLSchema-instance!null" ;
_declspec(selectany) LPCWSTR XSD01_SCHEMA_INSTANCE_NULL = L"http://www.w3.org/2001/XMLSchema-instance!nil" ;

_declspec(selectany) LPCWSTR POCKETSOAP_TYPES_URI		= L"http://www.pocketsoap.com/types" ;

_declspec(selectany) LPCWSTR SOAP_ENVELOPE_11_URI		= L"http://schemas.xmlsoap.org/soap/envelope/" ;
_declspec(selectany) LPCWSTR SOAP_ENCODING_11_URI		= L"http://schemas.xmlsoap.org/soap/encoding/" ;

_declspec(selectany) LPCWSTR SOAP_11_ENVELOPE_TAG		= L"http://schemas.xmlsoap.org/soap/envelope/!Envelope" ;
_declspec(selectany) LPCWSTR SOAP_11_BODY_TAG			= L"http://schemas.xmlsoap.org/soap/envelope/!Body" ;
_declspec(selectany) LPCWSTR SOAP_11_HEADER_TAG			= L"http://schemas.xmlsoap.org/soap/envelope/!Header" ;
_declspec(selectany) LPCWSTR SOAP_11_FAULT_TAG			= L"http://schemas.xmlsoap.org/soap/envelope/!Fault" ;
_declspec(selectany) LPCWSTR SOAP_11_MUSTUNDERSTAND_TAG = L"http://schemas.xmlsoap.org/soap/envelope/!mustUnderstand" ;
_declspec(selectany) LPCWSTR SOAP_11_ACTOR_TAG			= L"http://schemas.xmlsoap.org/soap/envelope/!actor" ;
_declspec(selectany) LPCWSTR SOAP_11_ENCSTYLE_TAG		= L"http://schemas.xmlsoap.org/soap/envelope/!encodingStyle" ;

_declspec(selectany) LPCWSTR SOAP_FAULT_NAME			= L"Fault" ;

_declspec(selectany) LPCWSTR ENC_SOAP_11_ARRAY			= L"http://schemas.xmlsoap.org/soap/encoding/!arrayType" ;
_declspec(selectany) LPCWSTR ENC_SOAP_11_ROOT			= L"http://schemas.xmlsoap.org/soap/encoding/!root" ;
_declspec(selectany) LPCWSTR ENC_SOAP_11_OFFSET			= L"http://schemas.xmlsoap.org/soap/encoding/!offset" ;
_declspec(selectany) LPCWSTR ENC_SOAP_11_POSITION		= L"http://schemas.xmlsoap.org/soap/encoding/!position" ;


_declspec(selectany) LPCWSTR SOAP_ENVELOPE_12_URI		= L"http://www.w3.org/2003/05/soap-envelope" ;
_declspec(selectany) LPCWSTR SOAP_ENCODING_12_URI		= L"http://www.w3.org/2003/05/soap-encoding" ;
_declspec(selectany) LPCWSTR SOAP_RPC_12_URI			= L"http://www.w3.org/2003/05/soap-rpc";

_declspec(selectany) LPCWSTR SOAP_12_ENVELOPE_TAG		= L"http://www.w3.org/2003/05/soap-envelope!Envelope" ;
_declspec(selectany) LPCWSTR SOAP_12_BODY_TAG			= L"http://www.w3.org/2003/05/soap-envelope!Body" ;
_declspec(selectany) LPCWSTR SOAP_12_HEADER_TAG			= L"http://www.w3.org/2003/05/soap-envelope!Header" ;
_declspec(selectany) LPCWSTR SOAP_12_MUSTUNDERSTAND_TAG = L"http://www.w3.org/2003/05/soap-envelope!mustUnderstand" ;
_declspec(selectany) LPCWSTR SOAP_12_ROLE_TAG			= L"http://www.w3.org/2003/05/soap-envelope!role" ;
_declspec(selectany) LPCWSTR SOAP_12_RELAY_TAG			= L"http://www.w3.org/2003/05/soap-envelope!relay" ;
_declspec(selectany) LPCWSTR SOAP_12_ENCSTYLE_TAG		= L"http://www.w3.org/2003/05/soap-envelope!encodingStyle" ;

_declspec(selectany) LPCWSTR ENC_SOAP_12_ID				= L"http://www.w3.org/2003/05/soap-encoding!id" ;
_declspec(selectany) LPCWSTR ENC_SOAP_12_REF			= L"http://www.w3.org/2003/05/soap-encoding!ref" ;
_declspec(selectany) LPCWSTR ENC_SOAP_12_ARRAYSIZE		= L"http://www.w3.org/2003/05/soap-encoding!arraySize" ;
_declspec(selectany) LPCWSTR ENC_SOAP_12_ITEMTYPE		= L"http://www.w3.org/2003/05/soap-encoding!itemType"  ;
_declspec(selectany) LPCWSTR ENC_SOAP_12_NODETYPE		= L"http://www.w3.org/2003/05/soap-encoding!nodeType"  ;