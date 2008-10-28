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
Portions created by Simon Fell are Copyright (C) 2002
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "PSOAP.h"
#include "QName.h"

/////////////////////////////////////////////////////////////////////////////
// CQName


STDMETHODIMP CQName::get_Name(BSTR *pVal)
{
	return m_name.CopyTo(pVal) ;
}

STDMETHODIMP CQName::put_Name(BSTR newVal)
{
	m_name = newVal ;
	return S_OK;
}

STDMETHODIMP CQName::get_Namespace(BSTR *pVal)
{
	return m_namespace.CopyTo(pVal) ;
}

STDMETHODIMP CQName::put_Namespace(BSTR newVal)
{
	m_namespace = newVal ;
	return S_OK;
}


STDMETHODIMP CQName::Set(BSTR Name, BSTR Namespace)
{
	m_name= Name ;
	m_namespace = Namespace ;
	return S_OK;
}
