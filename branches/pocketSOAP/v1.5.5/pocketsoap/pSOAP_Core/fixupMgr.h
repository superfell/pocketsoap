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
Portions created by Simon Fell are Copyright (C) 2000, 2002
Simon Fell. All Rights Reserved.

Contributor(s):
patch for required use of CAdapt from Peter Jonsson
*/

#pragma once

class psParser ;


/////////////////////////////////////////////////////////////////////////////
// An item (Node) that's waiting to be fixed up 
// (i.e. it has outstanding forward references)
/////////////////////////////////////////////////////////////////////////////
class FixupItem
{
public:
	FixupItem(unsigned long eNum) : elemNum(eNum)
	{
	}
	
	FixupItem (const FixupItem &rhs) : node(rhs.node), ser(rhs.ser), hrefs(rhs.hrefs), elems(rhs.elems), elemNum(rhs.elemNum)
	{
	}

	FixupItem ( CComPtr<ISOAPNode2> &n, CComPtr<ISoapDeSerializer> &s, std::vector<CAdapt<CComBSTR> > refs, std::vector<unsigned long> e, unsigned long en ) :
		node(n), ser(s), hrefs(refs), elems(e), elemNum(en)
	{
	}

	~FixupItem() { }


	CComPtr<ISOAPNode2>				node ;		// the node that needs fixing up
	CComPtr<ISoapDeSerializer>		ser ;		// the de-serializer for this node
	std::vector<CAdapt<CComBSTR> >	hrefs ;		// a list of hrefs that we're dependent on
	std::vector<unsigned long>		elems ;		// a list of element #'s that we're dependent on
	unsigned long					elemNum ;	// the element # of node
} ;

/////////////////////////////////////////////////////////////////////////////
// The Fixup Manager, this keeps track of who's waiting on who
/////////////////////////////////////////////////////////////////////////////
class FixupMgr
{
public:
	FixupMgr() ;
	~FixupMgr() ;

	void Init	   ( psParser * p ) ;
	void Add	   ( unsigned long elemNum, CComPtr<ISOAPNode2> &n, CComPtr<ISoapDeSerializer> &s, const CComBSTR &href, unsigned long waitingFor ) ;
	void ProcessID ( unsigned long elemNum, const CComBSTR &id, CComPtr<ISOAPNode2> &n, CComPtr<ISoapDeSerializer> &s ) ;

	void Lookup    ( const CComBSTR &id, ISOAPNode2 ** n ) ;
	
private:
	typedef std::map<CComBSTR, ISOAPNode2 *> IDMAP ;
	IDMAP						idMap ;	// id to node
	psParser					*parser ;

	typedef std::map<unsigned long,		 FixupItem>   FIXUPS ;
	typedef std::multimap<unsigned long, FixupItem *> ELEM_DEPENDS ;
	typedef std::multimap<CComBSTR,		 FixupItem *> ID_DEPENDS ;

	FIXUPS			items ;
	ELEM_DEPENDS	elemDepends ;
	ID_DEPENDS		idDepends ;

	std::vector<FIXUPS::iterator>		itemsToRemove ;

	void ProcessRemove ( FixupItem &finItem ) ;
	void ProcessIDImpl ( unsigned long elemNum, const CComBSTR &id, CComPtr<ISOAPNode2> &n, CComPtr<ISoapDeSerializer> &s ) ;
	void Clear		   () ;

	void DumpState ( LPCWSTR location ) ;
};
