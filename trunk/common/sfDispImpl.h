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

#pragma once
#include "dispimpl2.h"


template <class T, const IID* piid = &__uuidof(T), class D = IDispatch,
          const GUID* plibid = &CComModule::m_libid, WORD wMajor = 1,
          WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
class ATL_NO_VTABLE IsfDelegatingDispImpl : public IDelegatingDispImpl<T, piid, D, plibid, wMajor, wMinor, tihclass>
{
public:
	typedef IDelegatingDispImpl<T, piid, D, plibid, wMajor, wMinor, tihclass> _base ;

	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		HRESULT hr = _base::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr ) ;
		if (FAILED(hr))
			return hr ;

		if ( pvarResult && pvarResult->vt == VT_UNKNOWN )
		{
			IDispatch * pd ;
			if ( pvarResult->punkVal )
			{
				HRESULT _hr = pvarResult->punkVal->QueryInterface(IID_IDispatch, (void **)&pd) ;
				if (SUCCEEDED(_hr))
				{
					pvarResult->vt = VT_DISPATCH ;
					pvarResult->punkVal->Release() ;
					pvarResult->pdispVal = pd ;
				}
			}
		}
		return hr ;
	}

};
