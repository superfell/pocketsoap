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
// streamStreamReader: Definition
//
// streamStreamReader implements IResetableStream and forwards all calls to
// its hosted IStream object. 
/////////////////////////////////////////////////////////////////////////////

class ATL_NO_VTABLE streamStreamReader :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IResetableStream
{
public:
	streamStreamReader(): m_size(-1) 
	{
	}

	~streamStreamReader()	
	{
	}

BEGIN_COM_MAP(stringBuffStreamReader)
	COM_INTERFACE_ENTRY(IResetableStream)
	COM_INTERFACE_ENTRY(IReadableStream)
END_COM_MAP()

// class
	HRESULT Attach(IStream *stream)
	{
		m_stream = stream;
		if (!m_stream) return E_NOINTERFACE;
		_HR(Reset());
		// determine size
		STATSTG stat;
		HRESULT hr = m_stream->Stat(&stat, STATFLAG_NONAME);
		if(FAILED(hr))
			return AtlReportError(__uuidof(CoFileReaderFactory), _T("The call to IStream::Stat() failed, please make sure the IStream implementation you're using implements this meethod"), IID_NULL, hr);
		m_size = stat.cbSize.LowPart; // todo: support upload of files > 4 GB ;-)		
		return S_OK;
	}

	long GetSize()
	{
		return m_size;
	}


// IResetableStream
	STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead)
	{
		if (!pv || !pcbRead ) return E_POINTER;		
		// todo, do we need to map between the different semantics ?
		return m_stream->Read(pv,cb, pcbRead);		
	}
	
	STDMETHOD(Reset)()
	{
		LARGE_INTEGER lint;
		lint.QuadPart = 0;		
		HRESULT hr = m_stream->Seek(lint, STREAM_SEEK_SET, NULL);
		if(FAILED(hr))
			return AtlReportError(__uuidof(CoFileReaderFactory), _T("The call to IStream::Seek() failed please make sure the IStream implementation you're using implements this method"), IID_NULL, hr);
		return S_OK;
	}

private:
	CComQIPtr<IStream> m_stream;
	long m_size;
};