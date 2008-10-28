/*  
$Header: c:/cvs/pocketsoap/wsdl/wsdlcl/wsdlcl.cpp,v 1.3 2004/06/22 04:25:08 simon Exp $

The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketSOAP WSDL Wizard.

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2002
Simon Fell. All Rights Reserved.

Contributor (s):
*/

// wsdlcl.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "CommandLineParser.h"

CComModule _Module;

struct wCommandLine : public StandardCommandLineParser
{
public:
    // Create flags and params
    ValueArg<tstring>       wsdl;
	FlagArg					fixImports ;
	ValueArg<tstring>		portName ;
	ValueArg<tstring>		prjName ;
	ValueArg<tstring>		clsName ;
	ValueArg<tstring>		pxServer ;
	ValueArg<int>			pxPort ;
	FlagArg					vc;

public:
    wCommandLine() :
        // Set names and descriptions for usage
        wsdl(		__T("wsdl"),		__T("The URL or FileName of the source WSDL document")),
		fixImports(	__T("fiximports"),	__T("Fix missing xsd:imports for the soap-enc namespace")),
		portName(	__T("port"),		__T("The name of the port to generate a proxy for")),
		prjName(	__T("project"),		__T("Overrides the name of the generated project")) ,
		clsName(	__T("class"),		__T("Overrides teh name of the generated proxy class")),
		pxServer(	__T("proxy"),		__T("Specifies a proxy server name  [only used in the generated code]")) ,
		pxPort(		__T("proxyPort"),	__T("Specifies a proxy server port# [only used in the generated code]")) ,
		vc(			__T("vc"),			__T("Generate [e]VC code rather than vb6 code"))

    {
        // Add params matched by position, e.g. foo
        AddParam(wsdl);
        // Add flags matched by name, e.g. /foo
		portName.SetDefaultValue("") ;
		AddFlag(portName) ;
		AddFlag(fixImports);
		AddFlag(vc);

		prjName.SetDefaultValue("") ;
		AddFlag(prjName) ;
		clsName.SetDefaultValue("") ;
		AddFlag(clsName) ;
		pxServer.SetDefaultValue("") ;
		AddFlag(pxServer) ;
		pxPort.SetDefaultValue(8080) ;
		AddFlag(pxPort);
    }
};

int dumpErr ( HRESULT hr, const std::string &msg )
{
	return dumpErr ( hr, msg.c_str() ) ;
}

int dumpErr ( HRESULT hr, const char * msg, const char *additional = NULL )
{
	USES_CONVERSION ;
	std::cout << "Error : " << msg;
	if(additional)
		std::cout << additional;

	CComPtr<IErrorInfo> ei ;
	GetErrorInfo(0, &ei) ;
	if ( ei )
	{
		CComBSTR d ;
		ei->GetDescription(&d) ;
		std::cout << ", " << OLE2A(d) ;
	}

	std::cout << " [hr=0x" << std::hex << hr << "]" << std::endl ;
	return -2 ;
}

int main(int argc, const char ** argv)
{
	USES_CONVERSION ;
	wCommandLine wcl ;
	wcl.Parse ( argc, argv ) ;

	if( !wcl.Continue() ) return -1;

	std::cout << wcl.Logo() ;

	CoInitialize(NULL);
	{
		// create the WSDL parser
		CComPtr<_Engine> wsdl ;
		HRESULT hr = wsdl.CoCreateInstance(__uuidof(Engine)) ;
		if (FAILED(hr))
			return dumpErr ( hr, "Unable to create instance of WSDL Parser class" ) ;

		std::cout << std::endl ;
		std::cout << "reading " << wcl.wsdl.Value() << std::endl ;

		// create the parser properties object
		CComPtr<IMapCollection> parserProps ;
		hr = parserProps.CoCreateInstance(__uuidof(MapCollection)) ;
		if (FAILED(hr))
			return dumpErr ( hr, "Unable to create instance of MapCollection class" ) ;

		parserProps->put_Value(CComVariant(L"fudge-imports"), CComVariant(wcl.fixImports)) ;

		CComPtr<_definitions> defs ;
		CComVariant vParserProps(parserProps.p) ;
		hr = wsdl->ParseWSDLFile(CComBSTR(wcl.wsdl.Value().c_str()), &vParserProps, &defs ) ;
		if (FAILED(hr))
			return dumpErr ( hr, "Error during parsing of WSDL file" ) ;

		std::cout << "Finished parsing WSDL document" << std::endl; 

		CComPtr<_port> thePort ;
		CComPtr<_Collection> ports ;
		defs->get_ports(&ports) ;

		std::string portName = wcl.portName.Value() ;

		if ( wcl.portName.Value().length() == 0 )
		{
			// default port name
			long portCount ;
			ports->Count(&portCount) ;
			CComVariant vPort ;
			ports->Item(&CComVariant(portCount), &vPort) ;
			vPort.punkVal->QueryInterface(&thePort) ;
			CComPtr<_qname> n ;
			CComBSTR ln ;
			thePort->get_Name(&n) ;
			n->get_localname(&ln) ;
			portName = OLE2A(ln) ;
		}
		else
		{
			// specified port
			CComPtr<_port> p ;
			CComPtr<_qname> n ;
			CComBSTR ln ;
			CComPtr<IUnknown> punkEnum ;
			ports->_NewEnum(&punkEnum) ;
		    for( variant_iterator i = variant_iterator(punkEnum); i != variant_iterator(); ++i )
			{
				VARIANT&    v = *i;
				v.punkVal->QueryInterface(&p) ;
				p->get_Name(&n) ;
				n->get_localname(&ln) ;
				if ( strcmp(OLE2A(ln), portName.c_str() ) == 0 )
				{
					thePort = p ;
					break ;
				}
				p.Release() ;
				n.Release() ;
				ln.Empty() ;
			}
		}

		if ( ! thePort )
			return dumpErr ( E_FAIL, "Unable to find port '" + portName + "'" ) ;

		std::cout << "Processing port " << portName << std::endl; 

		// get the binding object
		CComPtr<_binding> binding ;
		CComPtr<_qname> bindingQN ;
		CComBSTR bqnLocalName , bqnNamespace ;
		thePort->get_binding(&bindingQN) ;
		bindingQN->get_localname(&bqnLocalName) ;
		bindingQN->get_ns(&bqnNamespace) ;

		hr = defs->findBinding(bqnLocalName, bqnNamespace, &binding) ;
		if (FAILED(hr))
			return dumpErr(hr, "Unable to find binding for port '" + portName + "'" ) ;

		// get the portType object
		CComPtr<_portType> portType ;
		CComPtr<_qname> portTypeQN ;
		CComBSTR ptLocalName, ptNamespace ;
		binding->get_bindingType(&portTypeQN) ;
		portTypeQN->get_localname(&ptLocalName) ;
		portTypeQN->get_ns(&ptNamespace) ;

		hr = defs->findPortType(ptLocalName, ptNamespace, &portType) ;
		if (FAILED(hr))
			return dumpErr(hr, "Unable to find portType for port '" + portName + "'" ) ;

		// create the code gen object
		CComPtr<_ICodeGenTarget> cgt ;
		if(wcl.vc)
			hr = cgt.CoCreateInstance(L"PocketSOAP.WSDL.vcCodeGen");
		else
			hr = cgt.CoCreateInstance(__uuidof(vb6CodeGen)) ;
		if (FAILED(hr))
			return dumpErr(hr, wcl.vc ? "Unable to create instance of VC code generator class" : "Unable to create instance of VB6 code generator class" ) ;

		char currDir[MAX_PATH] ;
		GetCurrentDirectory( MAX_PATH, currDir ) ;
		strcat(currDir, "\\" ) ;

		// code gen context
		CComPtr<IMapCollection> ctx ;
		hr = ctx.CoCreateInstance(__uuidof(MapCollection)) ;
		if (FAILED(hr))
			return dumpErr ( hr, "Unable to create instance of MapCollection class" ) ;

		if ( wcl.prjName.Value().length() )
			ctx->put_Value(CComVariant(L"project"), CComVariant(wcl.prjName.Value().c_str()) ) ;
		if ( wcl.clsName.Value().length() ) 
			ctx->put_Value(CComVariant(L"class"),   CComVariant(wcl.clsName.Value().c_str()) ) ;
		if ( wcl.pxServer.Value().length() )
		{
			ctx->put_Value(CComVariant(L"proxyserver"), CComVariant(wcl.pxServer.Value().c_str()) ) ;
			ctx->put_Value(CComVariant(L"proxyport"),   CComVariant(wcl.pxPort.Value()) ) ;
		}

		// init the code gen
		hr = cgt->Initialize ( CComBSTR(currDir), CComBSTR(wcl.wsdl.Value().c_str()), defs, ctx ) ;
		if (FAILED(hr))
			return dumpErr(hr, "can't initialize the code generator" ) ;

		hr = cgt->StartProxy ( thePort, binding, portType ) ;
		if (FAILED(hr))
			return dumpErr(hr, "can't initialize the proxy writer" ) ;

		// code gen each operation
		CComPtr<_Collection> operations ;
		CComPtr<_operation> op ;
		CComBSTR  opName ;
		binding->get_operations(&operations) ;
		CComPtr<IUnknown> punkEnum ;
		operations->_NewEnum(&punkEnum) ;
		for( variant_iterator i = variant_iterator(punkEnum); i != variant_iterator(); ++i )
		{
			VARIANT&    v = *i;
			v.punkVal->QueryInterface(&op) ;
			op->get_Name(&opName) ;

			hr = cgt->Operation ( opName ) ;
			if (FAILED(hr))
				return dumpErr(hr, "code generation failed for operation ", OLE2A(opName) ) ;

			op.Release() ;
			opName.Empty();
		}

		cgt->FinalizeProxy() ;
		cgt->Finalize() ;

		std::cout << "Done !" << std::endl; 
	}

	CoUninitialize() ;
	return 0;
}

