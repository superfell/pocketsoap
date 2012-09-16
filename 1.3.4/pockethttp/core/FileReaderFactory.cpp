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

#include "stdafx.h"
#include "pocketHTTP.h"
#include "FileReaderFactory.h"

/////////////////////////////////////////////////////////////////////////////
// FileReader: Implementation
/////////////////////////////////////////////////////////////////////////////

FileReader::FileReader() : m_handle(INVALID_HANDLE_VALUE)
{
}

FileReader::~FileReader()
{
	Close();
}

void FileReader::Close()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}

STDMETHODIMP FileReader::Read(void *pv, ULONG cb, ULONG * pcbRead)
{
	BOOL bOk=::ReadFile(m_handle, pv, cb, pcbRead, NULL);
	if (!bOk)
		return HRESULT_FROM_WIN32(::GetLastError());		
	return S_OK;
}

STDMETHODIMP FileReader::Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin,ULARGE_INTEGER * plibNewPosition)
{		
	DWORD res = ::SetFilePointer(m_handle, (long)dlibMove.QuadPart, NULL, dwOrigin);
	if (INVALID_SET_FILE_POINTER == res) return HRESULT_FROM_WIN32(::GetLastError());
	return S_OK;
}

STDMETHODIMP FileReader::Stat(STATSTG *pstatstg,DWORD grfStatFlag)
{
	// minimal implementation, returns only size
	DWORD lFileSizeLower = 0, lFileSizeUpper = 0;
	lFileSizeLower = ::GetFileSize(m_handle, &lFileSizeUpper);
	ATLASSERT(lFileSizeUpper==0); //we do not support files > 4 GB
	
	pstatstg->cbSize.QuadPart = (__int64) lFileSizeLower;
	return S_OK;
}

STDMETHODIMP FileReader::Write(const void * pv,ULONG cb,ULONG * pcbWritten)
{
	return E_NOTIMPL;
}


STDMETHODIMP FileReader::SetSize(ULARGE_INTEGER libNewSize)
{
	return E_NOTIMPL;
}

STDMETHODIMP FileReader::CopyTo(IStream * pstm,ULARGE_INTEGER cb,ULARGE_INTEGER * pcbRead,ULARGE_INTEGER * pcbWritten)
{
	return E_NOTIMPL;
}

STDMETHODIMP FileReader::Commit(DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP FileReader::Revert(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP FileReader::LockRegion(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType)
{
	return E_NOTIMPL;
}

STDMETHODIMP FileReader::UnlockRegion(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType)
{
	return E_NOTIMPL;
}

STDMETHODIMP FileReader::Clone(IStream ** ppstm)
{
	return E_NOTIMPL;
}

HRESULT FileReader::Attach(HANDLE h)
{
	if (h == INVALID_HANDLE_VALUE) return E_FAIL;
	Close();
	m_handle = h;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CFileReaderFactory: Implementation
/////////////////////////////////////////////////////////////////////////////

CFileReaderFactory::CFileReaderFactory()
{
}

CFileReaderFactory::~CFileReaderFactory()
{
}

HRESULT CreateReaderFromHandle(HANDLE fh, IUnknown ** out)
{
	if (!out) return E_POINTER;
	*out = NULL;
	CComObject<FileReader> * reader = 0;
	HRESULT hr;
	_HR(reader->CreateInstance(&reader));
	reader->AddRef();
	hr = reader->Attach(fh);
	if (SUCCEEDED(hr))
		reader->QueryInterface(out);
	reader->Release();
	return hr;
}

STDMETHODIMP CFileReaderFactory::CreateReaderFromHandle(OLE_HANDLE handle, IUnknown **out)
{
	return ::CreateReaderFromHandle((HANDLE)handle, out);
}

STDMETHODIMP CFileReaderFactory::CreateReaderFromFile(BSTR path, IUnknown **out)
{
	USES_CONVERSION;
	HANDLE fh = ::CreateFile(OLE2T(path), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, CREATE_FILE_FLAGS, NULL);
	if (fh==INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(::GetLastError());
	return ::CreateReaderFromHandle(fh, out);
}
