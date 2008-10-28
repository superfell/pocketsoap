// echoStructApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main(int argc, char* argv[])
{
	printf("Starting echoStruct demo application\n");
	CoInitialize(NULL);
	{
		// create a soap envelope
		CComPtr<ISOAPEnvelope> env;
		HRESULT hr = env.CoCreateInstance(__uuidof(CoEnvelope));
		if(FAILED(hr)) { printf("failed to create envelope 0x%x08\n", hr); return hr; }

		// get the serializer factory interface
		CComPtr<ISerializerFactoryConfig> sf;
		env->get_SerializerFactory(&sf);
		CComBSTR SOAP_INTEROP_ORG(L"http://soapinterop.org/");
		CComBSTR SOAP_INTEROP_XSD(L"http://soapinterop.org/xsd");
		// configure the serializer ( the {18A6F... is the interface ID of the IDataStruct interface, you can find this in the IDL file)
		sf->Serializer ( CComVariant(L"{18A6F10A-8833-440D-9361-4754FE47C397}"), CComBSTR(L"SOAPStruct"), SOAP_INTEROP_XSD, CComBSTR(L"VcSerializerDemo.DataStructSerializer.1"));
		sf->Deserializer( CComBSTR(L"SOAPStruct"), SOAP_INTEROP_XSD, VARIANT_FALSE, CComVariant(L"VcSerializerDemo.DataStruct.1"), CComBSTR(L"VcSerializerDemo.DataStructSerializer.1"));


		// setup the soap method call
		env->SetMethod(CComBSTR(L"echoStruct"), SOAP_INTEROP_ORG);
		// create an instance of our data class
		CComPtr<IDataStruct> data;
		data.CoCreateInstance(L"VcSerializerDemo.DataStruct");
		// set some values
		data->put_String(CComBSTR(L"Hello World"));
		data->put_Int(42);
		data->put_Float(42.42f);
		// set it as a parameter
		CComPtr<ISOAPNodes> params;
		env->get_Parameters(&params);
		params->Create(CComBSTR(L"inputStruct"), CComVariant(data), NULL, NULL, NULL, NULL);

		// create a http transport
		CComPtr<IHTTPTransportAdv> http;
		http.CoCreateInstance(__uuidof(HTTPTransport));
		// set the proxy if you want to
		// http->SetProxy(CComBSTR(L"localhost"), 5049);

		// send the request
		CComBSTR sEnv;
		env->Serialize(&sEnv);
		hr = http->Send( CComBSTR(L"http://soap.4s4c.com/ilab/soap.asp"), sEnv );
		if(FAILED(hr)) { printf("HTTP Send failed with error 0x%x08\n", hr); return hr; }

		// parse the response
		CComBSTR charEnc;
		hr = env->Parse(CComVariant(http), charEnc);
		if(FAILED(hr)) { printf("Parsing the HTTP response failed with error 0x%x08\n", hr); return hr; }

		// get the returned data object
		params.Release();
		env->get_Parameters(&params);
		CComPtr<ISOAPNode> node;
		params->get_Item(0, &node);
		data.Release();
		CComVariant vVal;
		node->get_Value(&vVal);
		vVal.punkVal->QueryInterface(&data);

		// dump the returned values
		CComBSTR sVal;
		long iVal;
		float fVal;
		data->get_String(&sVal);
		data->get_Int(&iVal);
		data->get_Float(&fVal);

		printf("returned values are string=%S, int=%d, float=%f\n", sVal, iVal, fVal);
	}

	CoUninitialize();
	return 0;
}
