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


// Note: Proxy/Stub Information
//		To build a separate proxy/stub DLL, 
//		run nmake -f psPorxy_PPCps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "psProxy.h"

#include "psProxy_i.c"
#include "Factory.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CoSoapFactory, CFactory)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		#ifndef UNDER_CE
			_Module.Init(ObjectMap, (HINSTANCE)hInstance, &LIBID_PSPROXYLib);
			DisableThreadLibraryCalls((HINSTANCE)hInstance);
		#else
			_Module.Init(ObjectMap, (HINSTANCE)hInstance);
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

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
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
	#ifndef UNDER_CE
		return _Module.UnregisterServer(TRUE);
	#else
		_Module.UnregisterServer();
		return S_OK;
	#endif
}


