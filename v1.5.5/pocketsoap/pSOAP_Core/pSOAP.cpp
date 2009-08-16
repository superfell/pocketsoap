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

// pSOAP.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//		To build a separate proxy/stub DLL, 
//		run nmake -f pSOAPps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "psoap.h"
#include "pSOAP_i.c"
#include "tags.h"

#include "Envelope.h"
#include "HTTPTransport.h"
#include "rawtcp.h"
#include "SOAPNode.h"
#include "SerializerFactory.h"
#include "SerializerSimple.h"
#include "SerializerArray.h"
#include "SerializerNode.h"
#include "SerializerB64.h"
#include "SerializerDate.h"
#include "SerializerPB.h"
#include "SerializerNull.h"
#include "SerializerHexBin.h"
#include "SerializerQName.h"
#include "QName.h"
#include "InterfaceFinder.h"
#include "xsdLong.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CoEnvelope,				CEnvelope)
	OBJECT_ENTRY(CLSID_CoEnvelope12,			CEnvelope)
	OBJECT_ENTRY(CLSID_HTTPTransport,			CHTTPTransport)
	OBJECT_ENTRY(CLSID_RawTcpTransport,			Crawtcp)
	OBJECT_ENTRY(CLSID_CoSoapNode,				CSOAPNode)
	OBJECT_ENTRY(CLSID_CoSerializerFactory,		CSerializerFactory)
	OBJECT_ENTRY(CLSID_CoSerializerSimple,		CSerializerSimple)
	OBJECT_ENTRY(CLSID_CoSerializerArray,		CSerializerArray)
	OBJECT_ENTRY(CLSID_CoSerializerArray12,		CSerializerArray12)
	OBJECT_ENTRY(CLSID_CoDeserializerArray,		CDeserializerArray)
	OBJECT_ENTRY(CLSID_CoDeserializerArray12,	CDeserializerArray12)
	OBJECT_ENTRY(CLSID_CoSerializerB64,			CSerializerB64)
	OBJECT_ENTRY(CLSID_CoSerializerNode,		CSerializerNode)
	OBJECT_ENTRY(CLSID_CoSerializerDate,		CSerializerDate)
	OBJECT_ENTRY(CLSID_CoSerializerBoolean,		CSerializerBoolean)
	OBJECT_ENTRY(CLSID_CoSerializerPB,			CSerializerPB)
	OBJECT_ENTRY(CLSID_CoSerializerNull,		CSerializerNull)
	OBJECT_ENTRY(CLSID_CoSerializerHexBin,		CSerializerHexBin)
	OBJECT_ENTRY(CLSID_CoSerializerQName,		CSerializerQName)
	OBJECT_ENTRY(CLSID_CoDeserializerXsdLong,	CDeserializerXsdLong)
	OBJECT_ENTRY(CLSID_CoQName,					CQName)
	OBJECT_ENTRY(CLSID_CoInterfaceFinder,		CInterfaceFinder)
	OBJECT_ENTRY(CLSID_CoXsdLong,				CXsdLong)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, (HINSTANCE)hInstance, &LIBID_PocketSOAP);
#ifndef UNDER_CE
		DisableThreadLibraryCalls((HINSTANCE)hInstance);
#endif
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();
	return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

void InitCF ( void ** ppv, LPCWSTR envURI, bool &initDone, CComAutoCriticalSection &lock)
{
	lock.Lock();
	if ( ! initDone )
	{
		CComQIPtr<IClassFactoryVersion> cfv((IUnknown *)*ppv) ;
		cfv->setVersion(envURI) ;
		initDone = true ;
	}
	lock.Unlock() ;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	static CComAutoCriticalSection cfLock ;
	static bool cf11 = false ;
	static bool cf12 = false ;
	HRESULT hr = _Module.GetClassObject(rclsid, riid, ppv);

	if (( !cf11 ) && ( rclsid == CLSID_CoEnvelope ))
		InitCF(ppv, SOAP_ENVELOPE_11_URI, cf11, cfLock) ;

	else if ( (!cf12) && ( rclsid == CLSID_CoEnvelope12 ))
		InitCF(ppv, SOAP_ENVELOPE_12_URI, cf12, cfLock) ;
	
	return hr ;
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}
