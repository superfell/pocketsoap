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
Portions created by Simon Fell are Copyright (C) 2001,2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

//////////////////////////////////////////////////////////////////////
// SOAPNode.h: interface for the CSOAPNode class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOAPNODE_H__FD7BED3D_CF34_43D1_BD58_9C3BC69345D0__INCLUDED_)
#define AFX_SOAPNODE_H__FD7BED3D_CF34_43D1_BD58_9C3BC69345D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class ATL_NO_VTABLE CSOAPNode : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSOAPNode, &CLSID_CoSoapNode>,
	public ISupportErrorInfoImpl<&IID_ISOAPNode>,
	public IsfDelegatingDispImpl<ISOAPNodeDisp>,
	public ISOAPNode3,
	public ISOAPNode12	
{
public:
	CSOAPNode();
	~CSOAPNode();

	DECLARE_REGISTRY_RESOURCEID(IDR_SOAPNODE)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSOAPNode)
	COM_INTERFACE_ENTRY(ISOAPNode)
	COM_INTERFACE_ENTRY(ISOAPNode2)
	COM_INTERFACE_ENTRY(ISOAPNode3)
	COM_INTERFACE_ENTRY(ISOAPNode12)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

	HRESULT FinalConstruct();
	void    FinalRelease() ;

// ISOAPNode
	STDMETHOD(get_Name)			( /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Namespace)	( /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Name)			( /*[in]*/ BSTR newVal);
	STDMETHOD(put_Namespace)	( /*[in]*/ BSTR newVal);

	STDMETHOD(get_Type)		(  /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_TypeNS)	(  /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Type)		(  /*[in]*/ BSTR newVal);
	STDMETHOD(put_TypeNS)	(  /*[in]*/ BSTR newVal);
	STDMETHOD(put_ArrayType)( BSTR newVal );
	STDMETHOD(get_ArrayType)( BSTR *pVal  );

	STDMETHOD(get_Value)	( /*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_Value)	( /*[in]*/ VARIANT newVal);
	STDMETHOD(get_ValueAs)	( /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNS, /*[out, retval]*/ VARIANT *pVal );
	STDMETHOD(putref_SerializerFactory) ( /*[in]*/			ISerializerFactoryConfig * sf ) ;

	STDMETHOD(get_root)			  ( /*[out,retval]*/ VARIANT_BOOL * IsRoot ) ;
	STDMETHOD(get_id)			  ( /*[out,retval]*/ BSTR * id ) ;
	STDMETHOD(get_href)			  ( /*[out,retval]*/ BSTR * href ) ;
	STDMETHOD(get_actor)	 	  ( /*[out,retval]*/ BSTR * actorURI ) ;
	STDMETHOD(get_mustUnderstand) ( /*[out,retval]*/ VARIANT_BOOL * MustUnderstand ) ;
	STDMETHOD(get_offset)		  ( /*[out,retval]*/ BSTR * offset ) ;
	STDMETHOD(get_position)		  ( /*[out,retval]*/ BSTR * position ) ;
	STDMETHOD(get_nil)			  ( /*[out,retval]*/ VARIANT_BOOL * IsNil ) ;

	STDMETHOD(put_root)			  ( /*[in]*/ VARIANT_BOOL IsRoot ) ;
	STDMETHOD(put_id)			  ( /*[in]*/ BSTR id ) ;
	STDMETHOD(put_href)			  ( /*[in]*/ BSTR href ) ;
	STDMETHOD(put_actor)		  ( /*[in]*/ BSTR actorURI ) ;
	STDMETHOD(put_mustUnderstand) ( /*[in]*/ VARIANT_BOOL MustUnderstand ) ;
	STDMETHOD(put_offset)		  ( /*[in]*/ BSTR offset ) ;
	STDMETHOD(put_position)		  ( /*[in]*/ BSTR position ) ;
	STDMETHOD(put_nil)			  ( /*[in]*/ VARIANT_BOOL IsNil ) ;

	STDMETHOD(get_Nodes)		  ( /*[out,retval]*/ ISOAPNodes ** ppNodes ) ;

// ISOAPNode2
	STDMETHOD(get_EncodingStyle)  ( /*[out,retval]*/ BSTR * encStyle );
	STDMETHOD(put_EncodingStyle)  ( /*[in]*/ BSTR encStyle ) ;

// ISOAPNode3
	STDMETHOD(get_explicitRoot)   ( /*[out,retval]*/ VARIANT_BOOL * explicitRoot ) ;
	STDMETHOD(put_explicitRoot)   ( /*[in]*/		 VARIANT_BOOL   explicitRoot ) ;

// ISOAPNode12
	STDMETHOD(get_relay)		  ( /*[out,retval]*/ VARIANT_BOOL * relay ) ;
	STDMETHOD(put_relay)		  ( /*[in]*/		 VARIANT_BOOL   relay ) ;
	STDMETHOD(get_role)			  ( BSTR * role ) ;
	STDMETHOD(put_role)			  ( BSTR newRole ) ;
	STDMETHOD(get_ref)			  ( BSTR * ref ) ;
	STDMETHOD(put_ref)			  ( BSTR ref ) ;

	// debugging help
	static void DumpActivityStats() ;

private:
	CComBSTR		m_Name ;
	CComBSTR		m_NameNS ;
	CComBSTR		m_Type ;
	CComBSTR		m_TypeNS ;
	CComBSTR		m_arrayType ;
	CComVariant		m_val ;
	VARIANT_BOOL	m_root, m_nil , m_relay, m_explicitRoot ;
	CComBSTR		m_id ;
	CComBSTR		m_href ;
	CComBSTR		m_actor ;
	VARIANT_BOOL	m_mustUnderstand ;
	CComBSTR		m_offset ;
	CComBSTR		m_position ;
	CComBSTR		m_encStyle ;

	CComPtr<ISOAPNodes>			m_nodes ;
	CComPtr<ISerializerFactory> m_serFactory ;
};

#endif // !defined(AFX_SOAPNODE_H__FD7BED3D_CF34_43D1_BD58_9C3BC69345D0__INCLUDED_)
