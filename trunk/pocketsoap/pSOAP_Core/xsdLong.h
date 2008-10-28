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
Portions created by Simon Fell are Copyright (C) 2005
Simon Fell. All Rights Reserved.

Contributor(s):
*/
	
/////////////////////////////////////////////////////////////////////////////
// xsdLong is a simple wrapper around an xsd:long (i.e. a signed 64 bit integer)
// its useful because many COM environments don't support the VT_I8 type
// directly
/////////////////////////////////////////////////////////////////////////////

#ifndef __XSDLONG_H_
#define __XSDLONG_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// xsdLong
class ATL_NO_VTABLE CXsdLong : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CXsdLong, &CLSID_CoXsdLong>,
	public ISupportErrorInfoImpl<&IID_IXsdLong>,
	public IsfDelegatingDispImpl<IXsdLong>,
	public INoMultirefSerializer
{
public:
	CXsdLong();

DECLARE_REGISTRY_RESOURCEID(IDR_XSDLONG)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CXsdLong)
	COM_INTERFACE_ENTRY(IXsdLong)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(INoMultirefSerializer)
END_COM_MAP()

// IXsdLong
public:
	STDMETHOD(get_String) ( /*[out,retval]*/ BSTR * pVal);
	STDMETHOD(put_String) ( /*[in]*/ BSTR val);

	STDMETHOD(get_HiDWord) ( /*[out,retval]*/ long * pVal);
	STDMETHOD(put_HiDWord) ( /*[in]*/ long val);

	STDMETHOD(get_LoDWord) ( /*[out,retval]*/ long * pVal);
	STDMETHOD(put_LoDWord) ( /*[in]*/ long val);

	STDMETHOD(Register)(/*[in]*/ ISerializerFactoryConfig * cfgFactory );

private:
	LONGLONG longlong;
};

#endif //__XSDLONG_H_
