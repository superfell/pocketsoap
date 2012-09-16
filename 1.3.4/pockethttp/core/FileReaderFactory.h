/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketHTTP

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2005
Simon Fell. All Rights Reserved.

Contributor(s):
	Chris P. Vigelius
*/

/////////////////////////////////////////////////////////////////////////////
// FileReaderFactory.h : Declaration of FileReader, CFileReaderFactory
//
// FileReader is a minimal implementation of IStream, which only
// supports the methods Read(), Seek() and Stat(). All other methods
// return E_NOTIMPL.
//
// CFileReaderFactory is a factory class for FileReader objects.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FILEREADERFACTORY_H_
#define __FILEREADERFACTORY_H_

#include "resource.h"     

class ATL_NO_VTABLE FileReader: 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStream
{
public:
	FileReader();
	~FileReader();

BEGIN_COM_MAP(FileReader)
	COM_INTERFACE_ENTRY(IStream)
END_COM_MAP()

public:
// class
	HRESULT Attach(HANDLE h);

// IStream
	// note: we support only Read(), Seek() and Stat()
	STDMETHOD(Read)(void *pv, ULONG cb, ULONG * pcbRead);
	STDMETHOD(Write)(const void * pv,ULONG cb,ULONG * pcbWritten);
	STDMETHOD(Seek)(LARGE_INTEGER dlibMove,DWORD dwOrigin,ULARGE_INTEGER * plibNewPosition);
	STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
	STDMETHOD(CopyTo)(IStream * pstm,ULARGE_INTEGER cb,ULARGE_INTEGER * pcbRead,ULARGE_INTEGER * pcbWritten);
	STDMETHOD(Commit)(DWORD grfCommitFlags);
	STDMETHOD(Revert)(void);
	STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType);
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType);
	STDMETHOD(Stat)(STATSTG * pstatstg,DWORD grfStatFlag);
	STDMETHOD(Clone)(IStream ** ppstm);

private:
	HANDLE m_handle;
	void Close();
};

class ATL_NO_VTABLE CFileReaderFactory : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CFileReaderFactory, &CLSID_CoFileReaderFactory>,
	public ISupportErrorInfoImpl<&IID_IFileReaderFactory>,
	public IsfDelegatingDispImpl<IFileReaderFactory>
{
public:
	CFileReaderFactory(); 
	~CFileReaderFactory(); 

DECLARE_REGISTRY_RESOURCEID(IDR_FILEREADERFACTORY)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFileReaderFactory)
	COM_INTERFACE_ENTRY(IDispatch)	
	COM_INTERFACE_ENTRY(IFileReaderFactory)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

public:
// IFileReader
	STDMETHOD(CreateReaderFromFile)  (/*[in]*/ BSTR path,          /*[out, retval]*/ IUnknown **pReader);
	STDMETHOD(CreateReaderFromHandle)(/*[in]*/ OLE_HANDLE hHandle, /*[out, retval]*/ IUnknown **pReader);
};

#endif //__FILEREADERFACTORY_H_