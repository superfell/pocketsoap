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
Portions created by Simon Fell are Copyright (C) 2001-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

/////////////////////////////////////////////////////////////////////////////
// SerializerFactory.h : Declaration of the CSerializerFactory
/////////////////////////////////////////////////////////////////////////////

#ifndef __SERIALIZERFACTORY_H_
#define __SERIALIZERFACTORY_H_

#include "resource.h"       // main symbols
#include "reportErrorImpl.h"
#include "stringBuff.h"

typedef struct SimpleSerializer
{
	VARTYPE		vt		 ;	
	CComBSTR	progID	 ;	// the progID of the serializer 
	CComBSTR	xmlType ;
	CComBSTR	xmlTypeNS ;
	CLSID		clsid ;
	bool		clsidOK ;
} SimpleSerializer ;

typedef struct InterfaceSerializer
{
	IID			iid ;
	CComBSTR	progID ;
	CComBSTR	xmlType ;
	CComBSTR	xmlTypeNS ;
	CLSID		clsid ;
	bool		clsidOK ;
} InterfaceSerializer ;

typedef struct DeserializerEntry
{
	CComBSTR	Namespace ; // the NamespaceURI of the XML type ( e.g. http://www.w3.org/2001/XMLSchema )
	CComBSTR	XmlType	  ; // the Type name ( e.g. string )
	bool		IsArray   ; // Array of XmlType
	CComVariant	ComType	  ; // the progID or CLSID of the target object
	CComBSTR	progID ;	// the progID of the de-serializer ;
	CLSID		clsid ;		// cached clsid of the de-serializer ;
	bool		clsidOK ;	// the clsid has already been calc'd
} DeserializerEntry ;

typedef struct ElementMap 
{
	CComBSTR	Namespace ;
	CComBSTR	Name ;
	CComBSTR	XmlType ;
	CComBSTR	XmlNamespace ;
} ElementMap ;

typedef std::vector<ElementMap>						ELEMENTTYPES ;
typedef std::vector<SimpleSerializer>				SIMPLE_SERIALIZERS ;
typedef std::vector<InterfaceSerializer>			IFACE_SERIALIZERS ;
typedef std::map<stringBuff_W, DeserializerEntry>	DESERIALIZERS ;		// key is 'XmlType![a|s]!Namespace'	// a = array, s= single
typedef std::map<stringBuff_W, ELEMENTTYPES>		MAP_ELEMENTTYPES;	// key is 'XmlType!s!Namespace' 
// NB: we use stringBuff_W in the map rather then std::wstring, as using
// std::wstring in the map causes an internal compiler error with the pocketPC MIPS compiler

/////////////////////////////////////////////////////////////////////////////
// CSerializerFactoryConfig
/////////////////////////////////////////////////////////////////////////////
class SerializerFactoryConfig
{
public:
	SerializerFactoryConfig() ;
	SerializerFactoryConfig(const SerializerFactoryConfig &rhs) ;
	~SerializerFactoryConfig() ;

	void Init	   ( sfConfigOptions configMode ) ;
	void AddSerVT  ( const VARTYPE vt,		BSTR progID, BSTR xmlType, BSTR xmlTypeNS ) ;
	void AddSerIID ( REFIID riid,			BSTR progID, BSTR xmlType, BSTR xmlTypeNS ) ;
	void AddDeser  ( BSTR ns, BSTR xmlType, bool IsArray, BSTR		comType, BSTR progID ) ;
	void AddDeser  ( BSTR ns, BSTR xmlType, bool IsArray, VARTYPE	comType, BSTR progID ) ;
	void AddDeser  ( BSTR ns, BSTR xmlType, bool IsArray, VARIANT * comType, BSTR progID ) ;
	void AddGlobalType ( BSTR ns, BSTR name,    BSTR XmlType, BSTR TypeNamespace ) ;
	void AddLocalType  ( BSTR ParentNS, BSTR ParentType, BSTR ChildNS, BSTR ChildName, BSTR xmlType, BSTR xmlTypeNS ) ;

	SIMPLE_SERIALIZERS	m_simpleSer ;	// simple serializers
	IFACE_SERIALIZERS	m_ifaceSer ;	// object serializers
	DESERIALIZERS		m_des ;			// de-serialziers
	ELEMENTTYPES		m_etypes ;		// global element name -> type mappings
	MAP_ELEMENTTYPES	m_localtypes ;	// element name -> type mappings for specific types

	static stringBuff_W makeKey ( BSTR Type, BSTR ns, bool Array = false ) ;

private:
	bool				m_Loaded ;
	void StoreDeser ( DeserializerEntry & d ) ;
};

/////////////////////////////////////////////////////////////////////////////
// SerializerFactoryConfigMgr
/////////////////////////////////////////////////////////////////////////////
class SerializerFactoryConfigMgr
{
public:
	SerializerFactoryConfigMgr() ;
	~SerializerFactoryConfigMgr() ;

	SerializerFactoryConfig * Config(sfConfigOptions mode) ;

private:
	SerializerFactoryConfig *m_std11 , *m_scripting11 ;
	SerializerFactoryConfig *m_std12 , *m_scripting12 ;
	CComAutoCriticalSection m_lock ;

	SerializerFactoryConfig * Getter(sfConfigOptions configMode, SerializerFactoryConfig ** ourCopy) ;
};

/////////////////////////////////////////////////////////////////////////////
// CPool - The current pool of [de]serializers for a particular request
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CPool 
{
public:
	CPool() ;
	~CPool() ;
	HRESULT fetch     ( /*[in]*/ REFCLSID clsid, /*[out,retval]*/ IUnknown ** ppUnk ) ;
	void	completed ( /*[in]*/ IUnknown * punk ) ;
	size_t	size      () ;
	void	clear	  () ;

private:
	typedef struct PoolItem
	{
		CLSID				clsid ;
		CComPtr<IUnknown>	punk ;
	} PoolItem ;

	// typedef std::vector<PoolItem> POOL ;
	typedef std::multimap<CLSID, CComPtr<IUnknown> > READYPOOL ;
	typedef std::map<CComPtr<IUnknown>, CLSID>  ACTIVEPOOL ;
	READYPOOL	ready ;
	ACTIVEPOOL  active ;
	long		requests, hits ;
};

/////////////////////////////////////////////////////////////////////////////
// CSerializerFactory
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSerializerFactory : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSerializerFactory, &CLSID_CoSerializerFactory>,
	public ISupportErrorInfoImpl<&__uuidof(ISerializerFactory)>,	// todo, move to multi itf support
	public ISerializerFactory2,
	public ISerializerFactoryPool2,
	public IsfDelegatingDispImpl<ISerializerFactoryConfig3>,
	public reportErrorImpl<CSerializerFactory>,
	public ISerializerFactoryEx,
	public ISerializerFactoryHeaders
{
public:
	CSerializerFactory()  ;
	~CSerializerFactory() ;

DECLARE_REGISTRY_RESOURCEID(IDR_SERIALIZERFACTORY)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSerializerFactory)
	COM_INTERFACE_ENTRY(ISerializerFactoryEx)
	COM_INTERFACE_ENTRY(ISerializerFactory2)
	COM_INTERFACE_ENTRY(ISerializerFactory)
	COM_INTERFACE_ENTRY(ISerializerFactoryConfig3)
	COM_INTERFACE_ENTRY(ISerializerFactoryConfig2)
	COM_INTERFACE_ENTRY(ISerializerFactoryConfig)
	COM_INTERFACE_ENTRY(ISerializerFactoryPool2)
	COM_INTERFACE_ENTRY(ISerializerFactoryPool)
	COM_INTERFACE_ENTRY(ISerializerFactoryHeaders)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISerializerFactory
	STDMETHOD(SerializerForValue) ( /*[in]*/ VARIANT * v,   /*[out,retval]*/ ISoapSerializer ** s ) ;
	STDMETHOD(SerializerForNode)  ( /*[in]*/ ISOAPNode * n, /*[out,retval]*/ ISoapSerializer ** s ) ;
	STDMETHOD(ReturnSerializer)   ( /*[in]*/ ISoapSerializer * s ) ;

	STDMETHOD(DeserializerForType)	 ( /*[in]*/ BSTR XmlType,	  /*[in]*/ BSTR XmlTypeNamespace, /*[in]*/ VARIANT_BOOL IsArray, /*[out,retval]*/ ISoapDeSerializer ** s ) ;
	STDMETHOD(DeserializerForElement)( /*[in]*/ BSTR elementName, /*[in]*/ BSTR elementNamespace, /*[in]*/ VARIANT_BOOL IsArray, /*[out,retval]*/ ISoapDeSerializer ** s ) ;
	STDMETHOD(ReturnDeSerializer)    ( /*[in]*/ ISoapDeSerializer * s ) ;

	STDMETHOD(XsiForPrimaryNS) ( /*[out,retval]*/ BSTR * uri ) ;

	STDMETHOD(IsAnyType)	   ( /*[in]*/ BSTR XmlType,  /*[in]*/ BSTR XmlTypeNamespace, /*[out,retval]*/ VARIANT_BOOL * IsAnyType ) ;
	STDMETHOD(AreEqualComTypes)( /*[in]*/ BSTR XmlTypeA, /*[in]*/ BSTR XmlTypeNSA, /*[in]*/ BSTR XmlTypeB, /*[in]*/ BSTR XmlTypeNSB, /*[out,retval]*/ VARIANT_BOOL * Match ) ;
	STDMETHOD(FindComType)	   ( /*[in]*/ BSTR XmlType, /*[in]*/ BSTR XmlTypeNamespace, /*[out,retval]*/ VARIANT * comType ) ;

// ISerializerFactory2
	STDMETHOD(DeserializerForChild)( /*[in]*/ BSTR ParentType, /*[in]*/ BSTR ParentTypeNS, /*[in]*/ BSTR elementName, /*[in]*/ BSTR elementNamespace, /*[in]*/ VARIANT_BOOL IsArray, /*[in,out]*/ BSTR * XmlType, /*[in,out]*/ BSTR * XmlTypeNS, /*[out,retval]*/ ISoapDeSerializer ** s ) ;

// ISerializerFactoryEx
	STDMETHOD(SerializerForValue)	 ( /*[in]*/ VARIANT * v, /*[in]*/ BSTR ParentType,  /*[in]*/ BSTR ParentTypeNS, /*[in]*/ BSTR name, /*[in]*/ BSTR Namespace, /*[out]*/ BSTR * type, /*[out]*/ BSTR * typeNamespace, /*[out,retval]*/ ISoapSerializer ** s ) ;
	STDMETHOD(SerializerForNode)	 ( /*[in]*/ ISOAPNode * n, /*[in]*/ BSTR ParentType,  /*[in]*/ BSTR ParentTypeNS, /*[out]*/ BSTR * type, /*[out]*/ BSTR * typeNamespace, /*[out,retval]*/ ISoapSerializer ** s ) ;
	STDMETHOD(DeserializerForElement)( /*[in]*/ BSTR ParentType,  /*[in]*/ BSTR ParentTypeNS, /*[in]*/ BSTR elementName, /*[in]*/ BSTR elementNamespace, /*[in]*/ VARIANT_BOOL IsArray, /*[out]*/ BSTR * XmlType, /*[out]*/ BSTR * XmlTypeNS, /*[out,retval]*/ ISoapDeSerializer ** s ) ;
	STDMETHOD(FindType)				 ( /*[in]*/ BSTR ParentType,  /*[in]*/ BSTR ParentTypeNS, /*[in]*/ BSTR elementName, /*[in]*/ BSTR elementNamespace, /*[out]*/ BSTR * XmlType, /*[out]*/ BSTR * XmlTypeNS ) ;


// ISerializerFactoryPool/2
	STDMETHOD(Reset)();
	STDMETHOD(Fetch)( /*[in]*/ REFCLSID clsid, /*[out,retval]*/ IUnknown ** ppUnk ) ;

// ISerializerFactoryConfig
	STDMETHOD(get_PrimarySchema) ( /*[out,retval]*/ BSTR * uri ) ;
	STDMETHOD(put_PrimarySchema) ( /*[in]*/ BSTR uri ) ;

	STDMETHOD(get_RootFirst)	 ( /*[out,retval]*/ VARIANT_BOOL * rootFirst ) ;
	STDMETHOD(put_RootFirst)	 ( /*[in]*/ VARIANT_BOOL rootFirst ) ;

	STDMETHOD(ElementMapping)	 ( /*[in]*/ BSTR ElementName, /*[in]*/ BSTR ElementNamespace, /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNamespace ) ;
	STDMETHOD(Deserializer)		 ( /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNamespace, /*[in]*/ VARIANT_BOOL ArrayOf, /*[in]*/ VARIANT ComType, /*[in]*/ BSTR ProgID ) ;
	STDMETHOD(Serializer)		 ( /*[in]*/ VARIANT ComType, /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNamespace, /*[in]*/ BSTR ProgID ) ;
	STDMETHOD(SetConfig)		 ( /*[in]*/ VARIANT config ) ;

// ISerializerFactoryConfig2
	STDMETHOD(LocalTypeMapping)  ( /*[in]*/ BSTR ParentXmlType, /*[in]*/ BSTR ParentXmlTypeNS, /*[in]*/ BSTR ChildName,  /*[in]*/ BSTR ChildNamespace, /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNamespace ) ;

// ISerializerFactoryConfig3
	STDMETHOD(understoodHeader) ( /*[in]*/ BSTR NamespaceURI, /*[in]*/ BSTR localName ) ;

// ISerializerFactoryHeaders
	STDMETHOD(isUnderstood)		( /*[in]*/ BSTR NamespaceURI, /*[in]*/ BSTR localName, /*[out,retval]*/ VARIANT_BOOL *understood ) ;

private:
	typedef enum cfgState
	{
		csNotSet = 0,
		csLoaded,
		csCustom
	} cfgState ;

	static SerializerFactoryConfigMgr m_cfgMgr ;
	VARIANT_BOOL					  m_rootFirst ;
	sfConfigOptions					  m_mode ;
	CComBSTR						  m_prefNamespace ;
	cfgState						  m_state ;
	SerializerFactoryConfig			* m_cfg ;
	CPool							  m_pool ;
	std::vector<std::wstring>		  m_understoodHeaders ;
	bool							  m_understoodHeadersSorted ;

	void	ClearConfig() ;
	void    InitConfig(bool custom) ;

	typedef enum searchStage
	{
		ssPreferedType,
		ssPreferedXsd,
		ssAny
	} searchStage ;

	HRESULT FindSimpleSerializer    ( VARTYPE vt, BSTR preferedType, BSTR preferedTypeNS, searchStage stage, ISoapSerializer ** s, BSTR * type = 0, BSTR * typeNS = 0 ) ;
	HRESULT FindSimpleSerializer    ( VARTYPE vt, BSTR preferedType, BSTR preferedTypeNS, ISoapSerializer ** s, BSTR * type = 0, BSTR * typeNS = 0 ) ;
	HRESULT FindInterfaceSerializer ( IUnknown * punk, BSTR prefType, BSTR prefTypeNS, ISoapSerializer ** s, BSTR * type = 0, BSTR * typeNS = 0 ) ;
	HRESULT FindType ( const ELEMENTTYPES &types, BSTR Name, BSTR NameNS, VARIANT_BOOL IsArray, BSTR * XmlType, BSTR * XmlTypeNS, ISoapDeSerializer ** s ) ;
	HRESULT DeserializerForElement  ( /*[in]*/ BSTR elementName, /*[in]*/ BSTR elementNamespace, /*[in]*/ VARIANT_BOOL IsArray, BSTR * XmlType, BSTR * XmlTypeNS, /*[out,retval]*/ ISoapDeSerializer ** s ) ;

	std::wstring makeHeaderKey(BSTR NamespaceURI, BSTR localName) ;
};

#endif //__SERIALIZERFACTORY_H_
