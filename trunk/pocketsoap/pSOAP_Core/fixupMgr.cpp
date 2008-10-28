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
Portions created by Simon Fell are Copyright (C) 2000, 2002-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "psoap.h"
#include "fixupMgr.h"
#include "psParser.h"

/////////////////////////////////////////////////////////////////////////////
// FixupMgr 
/////////////////////////////////////////////////////////////////////////////
FixupMgr::FixupMgr() : parser(0)
{
}

FixupMgr::~FixupMgr()
{
	Clear() ;
}

void FixupMgr::Init( psParser * p )
{
	parser = p ;
}

void FixupMgr::Clear()
{
	for ( IDMAP::iterator i = idMap.begin(); i != idMap.end(); i++ )
		i->second->Release() ;
	idMap.clear() ;
	items.clear() ;
	elemDepends.clear() ;
	idDepends.clear() ;
}

void FixupMgr::Add ( unsigned long elemNum, CComPtr<ISOAPNode2> &n, CComPtr<ISoapDeSerializer> &s, const CComBSTR &href, unsigned long waitingFor )
{
	// look to see if we're already tracking data for this element #
	FIXUPS::iterator f = items.find(elemNum) ;
	if ( f == items.end() )
	{
		std::pair<FIXUPS::iterator, bool> res = items.insert ( FIXUPS::value_type(elemNum, FixupItem(elemNum))) ;
		f = res.first ;
		FixupItem &fi = f->second ;
		fi.node = n ;
		fi.ser  = s ;
	}

	if ( f != items.end() )
	{
		// add additional dependencies
		if ( href.Length() )
		{
			if ( std::find(f->second.hrefs.begin(), f->second.hrefs.end(), href ) == f->second.hrefs.end() )
			{
				f->second.hrefs.push_back(href) ;
				idDepends.insert ( ID_DEPENDS::value_type( href, &f->second ) ) ;
			}
		}
		if ( waitingFor > 0 )
		{
			if ( std::find(f->second.elems.begin(), f->second.elems.end(), waitingFor ) == f->second.elems.end() )
			{
				f->second.elems.push_back(waitingFor) ;
				elemDepends.insert ( ELEM_DEPENDS::value_type ( waitingFor, &f->second )) ;
			}
		}
	}
}

// we've just finished de-serializing a node with an id attribute
// elemNum is the element number of the just finished node
// id is its id attribute value
// n is the actual node
// s is the actual de-serializer for that node

// we need to find the nodes that are dependent on this node, and 
// get them to handle this node that they are waiting on.
// if this is the last node that that node is waiting for, then
// we get to go around again.
void FixupMgr::ProcessID ( unsigned long elemNum, const CComBSTR &id, CComPtr<ISOAPNode2> &n, CComPtr<ISoapDeSerializer> &s ) 
{
	DumpState ( L"FixupMgr::ProcessID" ) ;

	ProcessIDImpl ( elemNum, id, n, s ) ;

	for ( size_t i = 0 ; i < itemsToRemove.size() ; i++ )
		items.erase(itemsToRemove[i]) ;
	itemsToRemove.clear() ;

	DumpState ( L"~FixupMgr::ProcessID" ) ;
}

// get the node interface for the particular ID. n is left untouched if we can't find it
void FixupMgr::Lookup ( const CComBSTR &id, ISOAPNode2 ** n )
{
	IDMAP::iterator i = idMap.find(id) ;
	if ( i != idMap.end() )
		*n = i->second ;
	if (*n)
		(*n)->AddRef();
}

// this does the work of finding all the fixups waiting on this node, making the de-serialization
// call into the fixup node, removing the depenedency, and checking to see if we finished any new
// nodes (in which case we recurse around again)
void FixupMgr::ProcessIDImpl ( unsigned long elemNum, const CComBSTR &id, CComPtr<ISOAPNode2> &n, CComPtr<ISoapDeSerializer> &s )
{
	// the newly finished node goes in the id map for later as
	// there may be a href pointer to it further down the road
	ISOAPNode2 * newNode = n ;
	newNode->AddRef() ;
	idMap[id] = newNode ;

	// we don't use equal_range, as this gets called recursively and an inner call could
	// invalidate our range end iterator (if the inner call for for an adjacent value)
	// the == in the loop works in this case, but generally isn't correct.
	ID_DEPENDS::iterator j = idDepends.lower_bound(id) ;
	while ( j != idDepends.end() &&  j->first == id )
	{
		FixupItem &si = *j->second ;
		si.ser->Ref ( id, n ) ;
		si.hrefs.erase(std::find(si.hrefs.begin(), si.hrefs.end(), id)) ;
		if ( si.elems.size() == 0 && si.hrefs.size() == 0 )
		{
			// just because there are no more outstanding fixups, doesn't mean that node
			// is finished, we could still be chasing around inside that nodes tree.
			// however it doesn't seem readily apparent on how we can work this out
			// so there are no garuntee's that end will be called.
			if ( ! parser->elemNumInStack(si.elemNum) )
				si.ser->End() ;

			itemsToRemove.push_back(items.find(si.elemNum)) ;
			// check to see if there are any fixups waiting on this element#
			ProcessRemove(si) ;
			// if we have an ID, repeat the process for that ID
			CComBSTR id ;
			si.node->get_id(&id) ;
			if ( id.Length() )
				ProcessIDImpl ( si.elemNum, id, si.node, si.ser ) ;
		}
		idDepends.erase(j++) ;
	}
}

// the fixup item at index idx has just had all of its dependencies processed
// check to see if there are any fixups waiting on that element#
void FixupMgr::ProcessRemove ( FixupItem &finItem )
{
	unsigned int num = finItem.elemNum ;

	ELEM_DEPENDS::iterator j= elemDepends.lower_bound(num) ;
	while ( j != elemDepends.end() && j->first == num )
	{
		FixupItem &i = *j->second ;
		i.ser->ChildReady ( finItem.elemNum, finItem.node ) ;
		i.elems.erase(std::find(i.elems.begin(), i.elems.end(), num)) ;
		if ( i.hrefs.size() == 0 && i.elems.size() == 0 )
		{
			if ( ! parser->elemNumInStack(i.elemNum) )
				i.ser->End() ;

			itemsToRemove.push_back(items.find(i.elemNum)) ;
			CComBSTR id ;
			i.node ->get_id(&id) ;
			ProcessRemove(i) ;
			if ( id.Length() )
				ProcessIDImpl ( i.elemNum, id, i.node, i.ser ) ;
		}
		elemDepends.erase(j++) ;
	}
}


void FixupMgr::DumpState ( LPCWSTR location )
{
#ifdef _DEBUG
	ATLTRACE(_T("\n\n***********************************************************************************\n%ls\nIDMAP\n-----\n"), location ) ;

	CComBSTR name ;
	for ( IDMAP::iterator i = idMap.begin() ; i != idMap.end() ; i++ )
	{
		i->second->get_Name(&name) ;
		ATLTRACE(_T("id=%ls : node =%ls\n"), i->first , name ) ;
		name.Empty() ;
	}

	ATLTRACE(_T("\nFIXUPS\n------")) ;
	for ( FIXUPS::iterator j = items.begin() ; j != items.end(); j++ )
	{
		j->second.node->get_Name(&name) ;
		ATLTRACE(_T("\nElem # : %d : %ls\nwaiting for hrefs\n"), j->second.elemNum, name ) ;
		
		for ( unsigned int k = 0 ; k < j->second.hrefs.size() ; ++k ) 
			ATLTRACE(_T("\t%ls\n"), j->second.hrefs[k] ) ;

		ATLTRACE(_T("waiting for element #'s\n")) ;
		for ( k = 0 ; k < j->second.elems.size() ; ++k )
			ATLTRACE(_T("\t%d\n"), j->second.elems[k] ) ;

		name.Empty() ;
	}

	ATLTRACE(_T("\n\n***********************************************************************************\n")) ;
#endif
}