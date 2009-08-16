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

#include "stdafx.h"
#include "copyHelpers.h"

/*
 *    ULONGLONG      VT_UI8									ULONGLONG ullVal;
 *    LONGLONG       VT_I8									LONGLONG llVal;
 *    LONG           VT_I4									LONG lVal;
 *    BYTE           VT_UI1									BYTE bVal;
 *    SHORT          VT_I2									SHORT iVal;
 *    FLOAT          VT_R4									FLOAT fltVal;
 *    DOUBLE         VT_R8									DOUBLE dblVal;
 *    VARIANT_BOOL   VT_BOOL								VARIANT_BOOL boolVal; _VARIANT_BOOL bool;
 *    SCODE          VT_ERROR								SCODE scode;
 *    CY             VT_CY									CY cyVal;
 *    DATE           VT_DATE								DATE date;
 *    BSTR           VT_BSTR								BSTR bstrVal;
 *    IUnknown *     VT_UNKNOWN								IUnknown __RPC_FAR *punkVal;
 *    IDispatch *    VT_DISPATCH							IDispatch __RPC_FAR *pdispVal;
 *    SAFEARRAY *    VT_ARRAY								SAFEARRAY __RPC_FAR *parray;
 *    BYTE *         VT_BYREF|VT_UI1						BYTE __RPC_FAR *pbVal;
 *    SHORT *        VT_BYREF|VT_I2							SHORT __RPC_FAR *piVal;
 *    LONG *         VT_BYREF|VT_I4							LONG __RPC_FAR *plVal;
 *    LONGLONG *     VT_BYREF|VT_I8							LONGLONG __RPC_FAR *pllVal;
 *    FLOAT *        VT_BYREF|VT_R4							FLOAT __RPC_FAR *pfltVal;
 *    DOUBLE *       VT_BYREF|VT_R8							DOUBLE __RPC_FAR *pdblVal;
 *    VARIANT_BOOL * VT_BYREF|VT_BOOL						VARIANT_BOOL __RPC_FAR *pboolVal; _VARIANT_BOOL __RPC_FAR *pbool;
 *    SCODE *        VT_BYREF|VT_ERROR						SCODE __RPC_FAR *pscode;
 *    CY *           VT_BYREF|VT_CY							CY __RPC_FAR *pcyVal;
 *    DATE *         VT_BYREF|VT_DATE						DATE __RPC_FAR *pdate;
 *    BSTR *         VT_BYREF|VT_BSTR						BSTR __RPC_FAR *pbstrVal;
 *    IUnknown **    VT_BYREF|VT_UNKNOWN					IUnknown __RPC_FAR *__RPC_FAR *ppunkVal;
 *    IDispatch **   VT_BYREF|VT_DISPATCH					IDispatch __RPC_FAR *__RPC_FAR *ppdispVal;
 *    SAFEARRAY **   VT_BYREF|VT_ARRAY						SAFEARRAY __RPC_FAR *__RPC_FAR *pparray;
 *    VARIANT *      VT_BYREF|VT_VARIANT					VARIANT __RPC_FAR *pvarVal;
 *    PVOID          VT_BYREF (Generic ByRef)				PVOID byref;
 *    CHAR           VT_I1									CHAR cVal;
 *    USHORT         VT_UI2									USHORT uiVal;
 *    ULONG          VT_UI4									ULONG ulVal;
 *    INT            VT_INT									INT intVal;
 *    UINT           VT_UINT								UINT uintVal;
 *    DECIMAL *      VT_BYREF|VT_DECIMAL					DECIMAL __RPC_FAR *pdecVal;
 *    CHAR *         VT_BYREF|VT_I1							CHAR __RPC_FAR *pcVal;
 *    USHORT *       VT_BYREF|VT_UI2						USHORT __RPC_FAR *puiVal;
 *    ULONG *        VT_BYREF|VT_UI4						ULONG __RPC_FAR *pulVal;	
 *    ULONGLONG *    VT_BYREF|VT_UI8						ULONGLONG __RPC_FAR *pullVal;
 *    INT *          VT_BYREF|VT_INT						INT __RPC_FAR *pintVal;
 *    UINT *         VT_BYREF|VT_UINT						UINT __RPC_FAR *puintVal;
*/

template<class T>
inline void TypedCopy(byte * pdest, const T &src)
{
	*(T *)pdest = src ;
}

template<class T>
inline void TypedCopy(T &dest, const byte * psrc)
{
	dest = *(T *)psrc ;
}

template<>
inline  void TypedCopy(BSTR &dest, const byte * psrc )
{
	dest = *(BSTR *)psrc ;
}

#ifndef _WIN32_WCE
void TypedCopy(byte * pdest, IRecordInfo * pri, void * pdata)
{
	pri->RecordCopy(pdata, pdest) ;
}
#endif

HRESULT TypedCopyHelper ( byte * pDest, const VARTYPE vt, const VARIANT * src )
{
	HRESULT hr = S_OK ;

	switch ( vt )
	{
		case VT_I1 :			TypedCopy(pDest, src->bVal ) ;		break ;
		case VT_I2 :			TypedCopy(pDest, src->iVal ) ;		break ;
		case VT_I4 :			TypedCopy(pDest, src->lVal ) ;		break ;

		// FFS, the definition of variant on pocketPC doesn't include these four		
#ifndef _WIN32_WCE
		case VT_I8 :			TypedCopy(pDest, src->llVal ) ;		break ;
		case VT_UI8 :			TypedCopy(pDest, src->ullVal ) ;	break ;
		case VT_RECORD	:		TypedCopy(pDest, src->pRecInfo, src->pvRecord ) ;	break ;
		case VT_USERDEFINED	:	TypedCopy(pDest, src->pRecInfo, src->pvRecord ) ;	break ;
#endif

		case VT_UI1 :			TypedCopy(pDest, src->cVal) ;		break ;
		case VT_UI2 :			TypedCopy(pDest, src->uiVal ) ;		break ;
		case VT_UI4 :			TypedCopy(pDest, src->ulVal ) ;		break ;

		case VT_R4 :			TypedCopy(pDest, src->fltVal ) ;	break ;
		case VT_R8 :			TypedCopy(pDest, src->dblVal ) ;	break ;

		case VT_CY :			TypedCopy(pDest, src->cyVal ) ;		break ;
		case VT_DATE :			TypedCopy(pDest, src->date ) ;		break ;
		case VT_BSTR :			TypedCopy(pDest, src->bstrVal ) ;	break ;

		case VT_BOOL :			TypedCopy(pDest, src->boolVal) ;	break ;
		case VT_DECIMAL :		TypedCopy(pDest, src->decVal ) ;	break ;
		case VT_ERROR :			TypedCopy(pDest, src->scode ) ;		break ;

		case VT_INT :			TypedCopy(pDest, src->intVal );		break ;
		case VT_UINT :			TypedCopy(pDest, src->uintVal) ;	break ;

		case VT_UNKNOWN:		TypedCopy(pDest, src->punkVal);		break ;
		case VT_DISPATCH:		TypedCopy(pDest, src->pdispVal);	break ;

		case VT_VARIANT	:		memcpy ( pDest, src, sizeof(VARIANT) ) ; break ;

		default :				hr = E_UNEXPECTED ;
	}

	return hr ;
}

HRESULT TypedCopyHelper(VARIANT * pDest, const VARTYPE vt, const byte * psrc)
{
	VariantInit(pDest) ;
	pDest->vt = vt ;

	HRESULT hr = S_OK ;

	switch ( vt )
	{
		case VT_I1 :			TypedCopy(pDest->bVal, psrc) ;		break ;
		case VT_I2 :			TypedCopy(pDest->iVal, psrc) ;		break ;
		case VT_I4 :			TypedCopy(pDest->lVal, psrc ) ;		break ;

		case VT_UI1 :			TypedCopy(pDest->cVal, psrc) ;		break ;
		case VT_UI2 :			TypedCopy(pDest->uiVal, psrc ) ;	break ;
		case VT_UI4 :			TypedCopy(pDest->ulVal, psrc ) ;	break ;

#ifndef _WIN32_WCE
		case VT_I8 :			TypedCopy(pDest->llVal, psrc ) ;	break ;
		case VT_UI8 :			TypedCopy(pDest->ullVal, psrc ) ;	break ;
		case VT_RECORD :		pDest->pvRecord = (void *)psrc ;	break ;
#endif

		case VT_R4 :			TypedCopy(pDest->fltVal, psrc ) ;	break ;
		case VT_R8 :			TypedCopy(pDest->dblVal, psrc ) ;	break ;

		case VT_CY :			TypedCopy(pDest->cyVal, psrc ) ;	break ;
		case VT_DATE :			TypedCopy(pDest->date, psrc ) ;		break ;
		case VT_BSTR :			TypedCopy(pDest->bstrVal, psrc ) ;	break ;

		case VT_BOOL :			TypedCopy(pDest->boolVal, psrc) ;	break ;
		case VT_DECIMAL :		TypedCopy(pDest->decVal, psrc ) ;	break ;
		case VT_ERROR :			TypedCopy(pDest->scode, psrc ) ;	break ;

		case VT_INT :			TypedCopy(pDest->intVal, psrc );	break ;
		case VT_UINT :			TypedCopy(pDest->uintVal, psrc) ;	break ;

		case VT_VARIANT:		*pDest = *(VARIANT *)psrc ;			break ;

		case VT_UNKNOWN :		TypedCopy(pDest->punkVal,  psrc) ;  break ;
		case VT_DISPATCH:		TypedCopy(pDest->pdispVal, psrc) ;  break ;

		default :				hr = E_UNEXPECTED ;
	}

	return hr ;
}
