// DataStruct.h : Declaration of the CDataStruct

#ifndef __DATASTRUCT_H_
#define __DATASTRUCT_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CDataStruct
class ATL_NO_VTABLE CDataStruct : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDataStruct, &CLSID_DataStruct>,
	public IDispatchImpl<IDataStruct, &IID_IDataStruct, &LIBID_VCSERIALIZERDEMOLib>
{
public:
	CDataStruct()
	{
		m_int = 0;
		m_float = 0.0f;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_DATASTRUCT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDataStruct)
	COM_INTERFACE_ENTRY(IDataStruct)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IDataStruct
public:
	STDMETHOD(get_Float)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Float)(/*[in]*/ float newVal);
	STDMETHOD(get_Int)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Int)(/*[in]*/ long newVal);
	STDMETHOD(get_String)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_String)(/*[in]*/ BSTR newVal);

private:
	CComBSTR m_str;
	int		 m_int;
	float	 m_float;
};

#endif //__DATASTRUCT_H_
