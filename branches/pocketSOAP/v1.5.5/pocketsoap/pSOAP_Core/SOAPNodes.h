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

#if !defined(AFX_NODES_H__ACD85366_8D00_4E65_B2A9_0064E653C14D__INCLUDED_)
#define AFX_NODES_H__ACD85366_8D00_4E65_B2A9_0064E653C14D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSOAPNode ;

class ATL_NO_VTABLE CNodes  :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IsfDelegatingDispImpl<ISOAPNodes>,
	public ISupportErrorInfoImpl<&__uuidof(ISOAPNodes)>
{
public:
	CNodes();
	~CNodes();

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNodes)
	COM_INTERFACE_ENTRY(ISOAPNodes)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// ISOAPNodes
	STDMETHOD(get__NewEnum)		( /*[out, retval]*/ IUnknown **pVal);
	STDMETHOD(get_Item)			( /*[in]*/ long idx,  /*[out,retval]*/ ISOAPNode ** node ) ;
	STDMETHOD(get_ItemByName)	( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[out,retval]*/ ISOAPNode ** node ) ;
	STDMETHOD(get_Count)		( /*[out,retval]*/ long * pCount ) ;
	STDMETHOD(Append)			( /*[in]*/ ISOAPNode * newNode);
	STDMETHOD(Clear)			( );
	STDMETHOD(Create)			(	/*[in]*/					BSTR Name, 
									/*[in]*/					VARIANT Val, 
									/*[in,defaultvalue(L"")]*/	BSTR Namespace,
									/*[in,defaultvalue(L"")]*/	BSTR Type, 
									/*[in,defaultvalue(L"")]*/	BSTR TypeNamespace,
									/*[out,retval]*/			ISOAPNode ** newNode ) ;

// class
	void SetParent ( CSOAPNode * p ) ;

private:
	typedef std::vector<ISOAPNode *> NODES ;
	NODES m_nodes ;
	CSOAPNode				  * m_parent ;
};

#endif // !defined(AFX_NODES_H__ACD85366_8D00_4E65_B2A9_0064E653C14D__INCLUDED_)
