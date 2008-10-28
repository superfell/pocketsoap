using System;
using System.IO;
using System.Collections;
using System.Collections.Specialized ;
using System.Runtime.InteropServices;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for ProxyWriter.
	/// </summary>
	public class ProxyWriter
	{
		private string m_wsdlUrl;						// the URL of the source WSDL
		private wsdlParser.definitions m_wsdl;			// the entire WSDL
		private wsdlParser.port m_port;					// the port we're generating from
		private wsdlParser.binding m_binding;			// the binding we're generating from
		private wsdlParser.portType m_portType;			// the port type we're generating from

		private vcProject m_project;					// the vcProject we're generating
		private string m_className;						// the name of the proxy class
		private Interface m_interface;					// the interface of the generated proxy class
		private StreamWriter m_proxyFileHeader;			// the proxy header file
		private StreamWriter m_proxyFileCpp;			// the proxy implementation

		private SerializerFactoryConfig m_factory;		// the code needed to configure the serializer factory
		private Hashtable m_typesCache;					// type QName -> CppType
		private Hashtable m_namemap;					// type QName -> CPP Name
		private PocketSOAP.ISerializerFactory2 m_sf;	// the serializer factory config, does the core type mappings for us

		public ProxyWriter(string wsdlUrl, wsdlParser.definitions def, wsdlParser.port port, wsdlParser.binding binding, wsdlParser.portType portType, vcProject project)
		{
			m_wsdlUrl = wsdlUrl;
			m_wsdl = def;
			m_port = port ;
			m_binding = binding;
			m_portType = portType;
			m_project = project;
			m_factory = new SerializerFactoryConfig();
			m_sf = new PocketSOAP.CoSerializerFactoryClass();
			m_typesCache = new Hashtable();
			m_namemap = new Hashtable();
			Initialize();
		}

		private void Initialize()
		{
			m_className = vcProject.safeClassName(m_portType.Name.localname);
			m_interface = new Interface("I" + m_className, "ISoapProxyBase");
			ProjectClass cls = m_project.AddClass(m_className, m_interface);
			m_proxyFileHeader = cls.header.Create();
			m_proxyFileCpp = cls.impl.Create();
			StartHeader();
			StartCpp();
		}

		public void FinalizeProxy()
		{
			m_project.IdlFile.AddInterface(m_interface);

			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("void {0}::ConfigSerializerFactory()", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("	m_sf.CoCreateInstance(__uuidof(CoSerializerFactory));");
			m_factory.Serialize(m_proxyFileCpp);

			m_proxyFileCpp.WriteLine("}");

			m_proxyFileHeader.WriteLine("};");
			m_proxyFileHeader.WriteLine("");
			string defName = string.Format("__{0}_H_", m_className.ToUpper());
			m_proxyFileHeader.WriteLine("#endif // {0}", defName);
			m_proxyFileHeader.Close();
			m_proxyFileCpp.Close();
		}

		public void Operation(string opName)
		{
			wsdlParser.portTypeOperation pto = null;
			wsdlParser.operation bo = null;
			foreach ( wsdlParser.portTypeOperation pto2 in m_portType.operations )
			{
				if ( pto2.Name == opName )
				{
					pto = pto2;
					break;
				}
			}
			foreach ( wsdlParser.operation bo2 in m_binding.operations )
			{
				if ( bo2.Name == opName )
				{
					bo = bo2;
					break;
				}
			}
			// check for doc/literal or rpc/enc
			bool isEnc, isRpc;
			isEnc = bo.inputBody.use != "literal";
			// remember that rpc/document can be set at the binding level, but overriden at the operation level
			if (( bo.style != null) && (bo.style.Length > 0 ))
				isRpc = (bo.style == "rpc");
			else
				isRpc = (m_binding.bindingStyle == "rpc");

			if ( isRpc && !isEnc )
				throw new Exception("Sorry only encoded is supported for RPC style operations");
			if ( !isRpc && isEnc )
				throw new Exception("Sorry only literal messages are supported for document style operations");

			if (isRpc)
				GenerateRpcEncOperation(opName, pto, bo);
			else
				GenerateDocLitOperation(opName, pto, bo);

			m_proxyFileHeader.Flush();
			m_proxyFileCpp.Flush();
		}

		private void GenerateRpcEncOperation(string opName, wsdlParser.portTypeOperation pto, wsdlParser.operation bo)
		{
			wsdlParser.message msgIn, msgOut = null;
			msgIn = m_wsdl.findMessage(pto.inputMessage.msg.localname, pto.inputMessage.msg.@namespace);
			if(pto.outputMessage.Name.Length>0)
				msgOut = m_wsdl.findMessage(pto.outputMessage.msg.localname, pto.outputMessage.msg.@namespace);

			// serializerFactory setup goo
			string ln = string.Format("// type mappings for operation {0}", opName);
			m_factory.Add("");
			m_factory.Add(ln);

			string opIn = m_factory.IdForString(opName, IdType.Localname);
			string opOut= m_factory.IdForString(opName + "Response", IdType.Localname);
			string opInNS = m_factory.IdForString(bo.inputBody.@namespace, IdType.Namespace);
			string opOutNS= m_factory.IdForString(bo.outputBody.@namespace, IdType.Namespace);

			ln = string.Format("m_sf->ElementMapping({0}, {1}, {0}, {1});", opIn, opInNS ) ;
			m_factory.Add(ln);
			ln = string.Format("m_sf->ElementMapping({0}, {1}, {0}, {1});", opOut, opOutNS ) ;
			m_factory.Add(ln);

			// work out all the parameters
			ArrayList parameters = new ArrayList();

			// first the body parameters
			foreach ( wsdlParser.part p in msgIn.parts )
			{
				CppType type = mapType(p.XmlType);
				// add type mappings
				string pn = m_factory.IdForString(p.Name, IdType.Localname);
				string nons  = m_factory.IdForString("", IdType.Namespace);
				string xmlln = m_factory.IdForString(type.XmlType.localname, IdType.Type);
				string xmlns = m_factory.IdForString(type.XmlType.@namespace, IdType.Namespace);
				ln = string.Format("m_sf->LocalTypeMapping({0}, {1}, {2}, {3}, {4}, {5});", opIn, opInNS, pn, nons, xmlln, xmlns) ;
				m_factory.Add(ln);
				parameters.Add(new Parameter(type, p.Name, p.XmlType, false, IdlDirection.In));
			}
			// now the headers
			foreach ( wsdlParser.soapHeader header in bo.inputHeaders )
			{
				wsdlParser.message hdrMsg = m_wsdl.findMessage(header.message.localname, header.message.@namespace);
				foreach ( wsdlParser.part hdrPart in hdrMsg.parts )
				{
					if ( hdrPart.Name == header.part )
					{
						CppType type = mapPart(hdrPart);
						parameters.Add(new Parameter(type, hdrPart.Name, hdrPart.XmlType, true, IdlDirection.In));
					}
				}

			}
			// finally, any return parameter ?
			Parameter retParam = null;
			if (msgOut != null)
			{
				if(msgOut.parts.Count() > 0)
				{
					object idxOne = 1 ;
					wsdlParser.part p = (wsdlParser.part)msgOut.parts.Item(ref idxOne);
					CppType type = mapType(p.XmlType);
					
					// add type mappings
					string pn = m_factory.IdForString(p.Name, IdType.Localname);
					string nons  = m_factory.IdForString("", IdType.Namespace);
					string xmlln = m_factory.IdForString(type.XmlType.localname, IdType.Type);
					string xmlns = m_factory.IdForString(type.XmlType.@namespace, IdType.Namespace);
					ln = string.Format("m_sf->LocalTypeMapping({0}, {1}, {2}, {3}, {4}, {5});", opOut, opOutNS, pn, nons, xmlln, xmlns) ;
					m_factory.Add(ln);
					retParam = new Parameter(type, p.Name, p.XmlType, false, IdlDirection.Out | IdlDirection.Retval );
					parameters.Add(retParam);
				}
			}
		
			// now generate the stub
			m_proxyFileHeader.Write("\tSTDMETHOD({0})(", opName);
			m_proxyFileCpp.Write("STDMETHODIMP {0}::{1}(", m_className, opName);
			bool first = true;
			foreach ( Parameter p in parameters )
			{
				string prm = string.Format("{0}{1} {2} {3}", first ? "" : ", ", p.cppType.CPPParameterName, p.IsOutParam ? "*" : "", p.cppName);
				m_proxyFileHeader.Write(prm);
				m_proxyFileCpp.Write(prm);
				first = false;
			}
			m_proxyFileHeader.WriteLine(");");
			m_proxyFileCpp.WriteLine(")\n{");
			// update the interface
			m_interface.AddMethod("HRESULT " + opName, parameters);
			// generate the stub body
			m_proxyFileCpp.WriteLine("\tHRESULT hr;");
			m_proxyFileCpp.WriteLine("\tCComPtr<ISOAPEnvelope> env = newEnvelope(CComBSTR(OLESTR(\"{0}\")), CComBSTR(OLESTR(\"{1}\")), hr);", opName, bo.inputBody.@namespace );
			m_proxyFileCpp.WriteLine("\tif (FAILED(hr)) return hr;");
			bool gotParams = false, gotHeader = false;
			foreach ( Parameter p in parameters )
			{
				if(p.IsOutParam)
					continue;
				if(p.isHeader)
				{
					if(!gotHeader)
					{
						m_proxyFileCpp.WriteLine("\tCComPtr<ISOAPNodes> headers;");
						m_proxyFileCpp.WriteLine("\tenv->get_Headers(&headers);");
						gotHeader = true ;
					}
					m_proxyFileCpp.WriteLine("\tif({0} != null)");
					m_proxyFileCpp.WriteLine(p.cppType.ConstructVariant(1, "v" + p.cppName, p.cppName));
					m_proxyFileCpp.WriteLine("\t\theaders->Create(CComBSTR(OLESTR(\"{0}\")), v{1}, NULL, NULL, NULL, NULL);", p.xmlName.localname, p.cppName ) ;
				}
				else
				{
					if(!gotParams)
					{
						m_proxyFileCpp.WriteLine("\tCComPtr<ISOAPNodes> params;");
						m_proxyFileCpp.WriteLine("\tenv->get_Parameters(&params);");
						gotParams = true;
					}
					m_proxyFileCpp.WriteLine(p.cppType.ConstructVariant(1, "v" + p.cppName, p.cppName));
					m_proxyFileCpp.WriteLine("\tparams->Create(CComBSTR(OLESTR(\"{0}\")), v{1}, NULL, NULL, NULL, NULL);", p.cppName, p.cppName );
				}
			}
			m_proxyFileCpp.WriteLine("\thr = SendRecv( env, CComBSTR(OLESTR(\"{0}\")));", bo.inputBody.encodingStyle );
			m_proxyFileCpp.WriteLine("\tif (FAILED(hr)) return hr;");
			// decode the return parameter
			if(retParam != null)
			{
				m_proxyFileCpp.WriteLine("\thr = extractRetVal{3}(env, CComBSTR(\"{0}\"), CComBSTR(\"{1}\"), {2});", retParam.xmlName.localname, retParam.xmlName.@namespace, retParam.cppName, retParam.cppType.ExtracRetValSuffix );
			}
			m_proxyFileCpp.WriteLine("\treturn hr;");
			m_proxyFileCpp.WriteLine("}\n");
		}

		internal CppType mapType(wsdlParser.qname type)
		{
			if(m_sf.IsAnyType(type.localname, type.@namespace))
				return CppType.Create((int)VarEnum.VT_VARIANT, "VARIANT", CppType.ArrayType.NotArray, CppType.TypeStyle.Primative, false, type);
			CppType t = null;
			try
			{
				Object ct = m_sf.FindComType(type.localname, type.@namespace);
				if(ct is int)
					return CppType.TypeFromVariantType((int)ct, type);
				return CppType.Create((int)VarEnum.VT_VARIANT, "VARIANT", type);
			} 
			catch(Exception ex)
			{
				// todo, catch specific exception
				// todo, handle complex types.
				// Console.WriteLine(type.ExpandedName + " : " + ex.Message);
			}
			// handle non-primitive types

			// first off, check the cache
			t = (CppType)m_typesCache[type.ExpandedName];
			if(t!=null)
				return t;

			// look for a schema definition
			MSXML2.ISchema schema = SchemaForType(type);
			MSXML2.ISchemaType oType = (MSXML2.ISchemaType)schema.types.itemByQName(type.localname, type.@namespace);
			if ( oType is MSXML2.ISchemaComplexType )
				t = CreateComplexType(type, (MSXML2.ISchemaComplexType)oType);
			else if ( oType.enumeration.length > 0 )
				t = CreateEnumType(type, oType);
			else if (( oType.itemType == MSXML2.SOMITEMTYPE.SOMITEM_SIMPLETYPE ) && ( oType.derivedBy == MSXML2.SCHEMADERIVATIONMETHOD.SCHEMADERIVATIONMETHOD_RESTRICTION ))
				t = CreateRestrictedSimpleType(type, oType);
			else
				throw new ApplicationException("Sorry, this type " + type.ExpandedName + " is not yet supported");
			return t;
		}

		CppType CreateComplexType(wsdlParser.qname typeName, MSXML2.ISchemaComplexType typeDesc)
		{
			// look to see if its a section 5 encoded array first
			foreach ( MSXML2.ISchemaType bt in typeDesc.baseTypes )
			{
				if (( bt.name == "Array" ) && ( bt.namespaceURI == Consts.SOAP_11_ENCODING ))
					return HandleArrayType(typeName, typeDesc);
			}
    
			CppType t = CppType.Create((int)VarEnum.VT_UNKNOWN, NameBuilder(typeName), CppType.ArrayType.NotArray, CppType.TypeStyle.Object, false, typeName);

			// a qname for the serializer class
			wsdlParser.qname tSerQName= new wsdlParser.qnameClass();
			tSerQName.localname = "s" + typeName.localname;
			tSerQName.@namespace = "http://serializers.schemas.pocketsoap.com/";

			// register the type mapping now, incase there is some recursive nonsense going on
			m_typesCache[typeName.ExpandedName] = t;

			// a complexTypeWriter
			ComplexTypeWriter ctw = new ComplexTypeWriter(m_project, typeName.ExpandedName, t.CppName);

			// a SerializerWriter
			SerializerWriter sw = new SerializerWriter(m_project, tSerQName, NameBuilder(tSerQName), t.CppName );

			MSXML2.ISchemaComplexType complexType = typeDesc;
			MSXML2.ISchemaModelGroup smg = complexType.contentModel;

			// walk through each contained particle
			
			// attributes
			foreach ( MSXML2.ISchemaAttribute attr in complexType.attributes)
			{
				wsdlParser.qname ptype = new wsdlParser.qnameClass();
				ptype.@namespace = attr.type.namespaceURI;
				ptype.localname  = attr.type.name;
				CppType itemType = mapType(ptype);
				string itemName = vcProject.safeClassName(attr.name);
				ctw.AddPropertyField ( itemName, itemType );
				sw.AddAttribute ( itemName, itemType, attr, ptype );
			}

			AddElementsToContainer(typeName, smg, ctw, sw);

			ctw.Complete();
			sw.Complete();

			// add type registrations
			string serProgId = m_project.ProjectName + "." + sw.SerializerName;
			string clsProgId = m_project.ProjectName + "." + ctw.ComplexTypeName;
			string clsItfIID = string.Format("CComVariant(OLESTR(\"{{{0}}}\"))", ctw.InterfaceIID);
			m_factory.Add(string.Format("m_sf->Serializer({0}, CComBSTR(OLESTR(\"{1}\")), CComBSTR(OLESTR(\"{2}\")), CComBSTR(OLESTR(\"{3}\")));", clsItfIID, typeDesc.name, typeDesc.namespaceURI, serProgId));

			string typeLN = string.Format("CComBSTR(OLESTR(\"{0}\"))", typeName.localname);
			string typeNS = string.Format("CComBSTR(OLESTR(\"{0}\"))", typeName.@namespace);
			m_factory.Add(string.Format("m_sf->Deserializer({0}, {1}, VARIANT_FALSE, CComVariant(OLESTR(\"{2}\")), {3});", typeLN, typeNS, clsProgId, makeBstr(serProgId)));
			return t;
		}

		string makeBstr(string str)
		{
			return string.Format("CComBSTR(OLESTR(\"{0}\"))", str);
		}

		void AddElementsToContainer(wsdlParser.qname xmlName, MSXML2.ISchemaModelGroup smg, ComplexTypeWriter ctw, SerializerWriter sw)
		{
			foreach ( MSXML2.ISchemaParticle particle in smg.particles )
			{
				if (( particle.itemType == MSXML2.SOMITEMTYPE.SOMITEM_ALL ) || ( particle.itemType == MSXML2.SOMITEMTYPE.SOMITEM_SEQUENCE ))
				{
					AddElementsToContainer(xmlName, particle as MSXML2.ISchemaModelGroup, ctw, sw);
				}
				else if ( particle.itemType == MSXML2.SOMITEMTYPE.SOMITEM_ELEMENT )
				{
					MSXML2.ISchemaElement e = (MSXML2.ISchemaElement)particle;
					CppType itemType = CppType.FromParticle(this, particle);
					string itemName  = vcProject.safeClassName(particle.name);

					ctw.AddPropertyField(itemName, itemType);
					sw.AddElement(itemName, itemType, e);
				}		   
			}
		}

		string NameBuilder(wsdlParser.qname qn)
		{
			if(m_namemap.ContainsKey(qn.ExpandedName))
				return (string)m_namemap[qn.ExpandedName];

			string name = vcProject.safeClassName(qn.localname);
			string suffix = "";
			int num = 0;
			string newName = name ;
			while(m_namemap.ContainsValue(newName))
			{
				suffix = "";
				while(num>0)
				{
					suffix = suffix + ((num % 26) + 'A');
					num = num /26;
				}
				++num;					
				newName = name + suffix;
			}

			m_namemap[qn.ExpandedName] = newName;
			return newName;
		}

		CppType HandleArrayType(wsdlParser.qname typeName, MSXML2.ISchemaComplexType typeDesc)
		{
			MSXML2.ISchemaComplexType ct = typeDesc;
			MSXML2.ISchemaItem encArrayType = ct.attributes.itemByQName("arrayType", Consts.SOAP_11_ENCODING);
			string wsdlArrayType = null;
			for ( int idx =0 ; idx < encArrayType.unhandledAttributes.length ; idx++ )
			{
				if ( ( encArrayType.unhandledAttributes.getLocalName(idx) == "arrayType" ) &&
					( encArrayType.unhandledAttributes.getURI(idx) == Consts.WSDL_URL ))
				{
					wsdlArrayType = encArrayType.unhandledAttributes.getValue(idx);
					break;
				}
			}
			if(wsdlArrayType == null)
				throw new ApplicationException("SOAP-ENC:Array derivation must include a wsdl:arrayType attribute");

			wsdlParser.qname qn = new wsdlParser.qnameClass();
			int lastColon = wsdlArrayType.LastIndexOf(":");
			qn.@namespace = wsdlArrayType.Substring(0,lastColon);
			int rank = wsdlArrayType.IndexOf("[", lastColon);
			qn.localname = wsdlArrayType.Substring(lastColon+1, rank-lastColon-1);
			CppType itemType = mapType(qn);

			CppType t = CppType.CreateArray(itemType, CppType.ArrayType.Encoded, false, qn);

			m_typesCache.Add(typeName.ExpandedName, t);

			if(t.Style == CppType.TypeStyle.Object )
			{
				string qnln = m_factory.IdForString (qn.localname, IdType.Localname );
				string qnns = m_factory.IdForString (qn.@namespace,IdType.Namespace);
				string psad = m_factory.IdForString ("PocketSOAP.ArrayDeserializer.1", IdType.ProgId);
				string psas = m_factory.IdForString ("PocketSOAP.ArraySerializer.1", IdType.ProgId);
				m_factory.Add(string.Format("m_sf->Deserializer({0}, {1}, VARIANT_TRUE, VT_UNKNOWN, {2});", qnln, qnns, psad));
				m_factory.Add(string.Format("m_sf->Serializer(VT_UNKNOWN | VT_ARRAY, {0}, {1}, {2});", qnln, qnns, psas));

			}
			return t;
		}

		CppType CreateEnumType(wsdlParser.qname typeName, MSXML2.ISchemaType typeDesc)
		{
			throw new ApplicationException("Sorry, this type " + typeName.ExpandedName + " is not yet supported");
		}

		CppType CreateRestrictedSimpleType(wsdlParser.qname typeName, MSXML2.ISchemaType typeDesc)
		{
			throw new ApplicationException("Sorry, this type " + typeName.ExpandedName + " is not yet supported");
		}

		// note that sc.getSchema(someNS) fails when xs:import is used, as imported types will appear in the
		// schema that imported them, not in the original schmea (which isn't in the schema collection)
		private MSXML2.ISchema SchemaForType(wsdlParser.qname qn)
		{
			MSXML2.IXMLDOMSchemaCollection2 sc = m_wsdl.Schemas;
			MSXML2.ISchema s;
			for ( int idx = 0 ; idx < sc.length ; idx++ )
			{
				s = sc.getSchema(sc[idx]);
				foreach ( MSXML2.ISchemaType t in s.types )
				{
					if (( t.name == qn.localname ) && ( t.namespaceURI == qn.@namespace ))
					{
						return s;
					}
				}
			}
			throw new ArgumentException ("Unable to find schema for " + qn.ExpandedName);				  
		}
		
		private CppType mapPart(wsdlParser.part part)
		{
			// either the part has a type attribute, in which case its an rpc/enc type
			if(part.XmlType.localname.Length >0)
				return mapType(part.XmlType);
			else
				return genDocLiteralType(part.Element);
		}

		private CppType genDocLiteralType(wsdlParser.qname elementName)
		{
			// todo
			return CppType.Create("VARIANT");
		}

		private void GenerateDocLitOperation(string opName, wsdlParser.portTypeOperation pto, wsdlParser.operation bo)
		{
			// todo
		}

		private void StartHeader()
		{
			string defName = string.Format("__{0}_H_", m_className.ToUpper());
			m_proxyFileHeader.WriteLine("#ifndef {0}", defName);
			m_proxyFileHeader.WriteLine("#define {0}", defName);
			m_proxyFileHeader.WriteLine("");
			m_proxyFileHeader.WriteLine("#include \"resource.h\"       // main symbols");
			m_proxyFileHeader.WriteLine("#include \"{0}.h\"			   // idl header", m_project.ProjectName);
			m_proxyFileHeader.WriteLine("#include \"ISoapProxyBaseImpl.h\"");
			m_proxyFileHeader.WriteLine("#include \"c:\\soap_src\\simonfell\\3rdparty\\dispimpl2.h\"");
			m_proxyFileHeader.WriteLine("");
			m_proxyFileHeader.WriteLine("// " + m_className);
			m_proxyFileHeader.WriteLine("");
			m_proxyFileHeader.WriteLine("class ATL_NO_VTABLE {0} : ", m_className);
			m_proxyFileHeader.WriteLine("    public CComObjectRootEx<CComMultiThreadModel>,");
			m_proxyFileHeader.WriteLine("    public CComCoClass<{0}, &CLSID_{0}>,", m_className);
			m_proxyFileHeader.WriteLine("    public IDelegatingDispImpl<I{0}>,", m_className);
			m_proxyFileHeader.WriteLine("    public ISoapProxyBaseImpl");
			m_proxyFileHeader.WriteLine("{");
			m_proxyFileHeader.WriteLine("public:");
			m_proxyFileHeader.WriteLine("");
			m_proxyFileHeader.WriteLine("DECLARE_PROTECT_FINAL_CONSTRUCT()");
			m_proxyFileHeader.WriteLine("DECLARE_REGISTRY_RESOURCEID(IDR_{0})", m_className.ToUpper());
			m_proxyFileHeader.WriteLine("");
			m_proxyFileHeader.WriteLine("BEGIN_COM_MAP({0})", m_className);
			m_proxyFileHeader.WriteLine("	COM_INTERFACE_ENTRY(I{0})", m_className);
			m_proxyFileHeader.WriteLine("	COM_INTERFACE_ENTRY(ISoapProxyBase)");
			m_proxyFileHeader.WriteLine("	COM_INTERFACE_ENTRY(IDispatch)");
			m_proxyFileHeader.WriteLine("END_COM_MAP()");
			m_proxyFileHeader.WriteLine("");
			m_proxyFileHeader.WriteLine("	HRESULT FinalConstruct();");
			m_proxyFileHeader.WriteLine("");
			m_proxyFileHeader.WriteLine("//ISoapProxyBase");
			m_proxyFileHeader.WriteLine("	STDMETHOD(get_Transport)(ISOAPTransport ** Transport);");
			m_proxyFileHeader.WriteLine("	STDMETHOD(putref_Transport)(ISOAPTransport * Transport);");
			m_proxyFileHeader.WriteLine("	STDMETHOD(get_Url)(BSTR *Url);");
			m_proxyFileHeader.WriteLine("	STDMETHOD(put_Url)(BSTR Url);");
			m_proxyFileHeader.WriteLine("	STDMETHOD(get_SerializerFactory)( ISerializerFactoryConfig2 **Factory);");
			m_proxyFileHeader.WriteLine("	STDMETHOD(putref_SerializerFactory)( ISerializerFactoryConfig2 *Factory);");
			m_proxyFileHeader.WriteLine("");
			m_proxyFileHeader.WriteLine("protected:");
			m_proxyFileHeader.WriteLine("	virtual void ConfigSerializerFactory();");
			m_proxyFileHeader.WriteLine("private:");
/*			m_proxyFileHeader.WriteLine("	CComBSTR						   m_url;");
			m_proxyFileHeader.WriteLine("	CComPtr<ISOAPTransport>			   m_tp;");
			m_proxyFileHeader.WriteLine("	CComPtr<ISerializerFactoryConfig2> m_sf;");
			m_proxyFileHeader.WriteLine("");
*/			m_proxyFileHeader.WriteLine("	HRESULT SendRecv(ISOAPEnvelope * env, CComBSTR &soapAction);");
			m_proxyFileHeader.WriteLine("   CComPtr<ISOAPEnvelope> newEnvelope(CComBSTR &methodName, CComBSTR &ns, HRESULT &hr);");
			m_proxyFileHeader.WriteLine("   HRESULT extractRetVal(ISOAPEnvelope *env, VARIANT *pVal);");
			m_proxyFileHeader.WriteLine("public:");
			m_proxyFileHeader.WriteLine("// I{0}", m_className);
		}

		private void StartCpp()
		{
			m_proxyFileCpp.WriteLine("#include \"stdafx.h\"");
			m_proxyFileCpp.WriteLine("#include \"{0}.h\"", m_className);
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("HRESULT {0}::FinalConstruct()", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("	m_url = OLESTR(\"{0}\");", m_port.SoapURL);
			m_proxyFileCpp.WriteLine("	return S_OK;");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("STDMETHODIMP {0}::get_Transport(ISOAPTransport ** Transport)", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("	return ISoapProxyBaseImpl::get_Transport(Transport);");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("STDMETHODIMP {0}::putref_Transport(ISOAPTransport * Transport)", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("	return ISoapProxyBaseImpl::putref_Transport(Transport);");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("STDMETHODIMP {0}::get_Url(BSTR *Url)", m_className);
			m_proxyFileCpp.WriteLine("{\n\treturn ISoapProxyBaseImpl::get_Url(Url);\n}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("STDMETHODIMP {0}::put_Url(BSTR Url)", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("	return ISoapProxyBaseImpl::put_Url(Url);");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("STDMETHODIMP {0}::get_SerializerFactory(ISerializerFactoryConfig2 **Factory)", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("	return ISoapProxyBaseImpl::get_SerializerFactory(Factory);");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("STDMETHODIMP {0}::putref_SerializerFactory(ISerializerFactoryConfig2 *Factory)", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("	return ISoapProxyBaseImpl::putref_SerializerFactory(Factory);");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("HRESULT {0}::SendRecv(ISOAPEnvelope * env, CComBSTR &soapAction)", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("	CComPtr<ISOAPTransport> soap;");
			m_proxyFileCpp.WriteLine("	ISoapProxyBaseImpl::get_Transport(&soap);");
			m_proxyFileCpp.WriteLine("	CComQIPtr<IHTTPTransport> http(soap);");
			m_proxyFileCpp.WriteLine("	if(http)");
			m_proxyFileCpp.WriteLine("		http->put_SOAPAction(soapAction);");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("	// todo: detect PS1.5 and do the right thing.");
			m_proxyFileCpp.WriteLine("	CComBSTR sEnv, enc;");
			m_proxyFileCpp.WriteLine("	HRESULT hr = env->Serialize(&sEnv);");
			m_proxyFileCpp.WriteLine("	if(FAILED(hr)) return hr;");
			m_proxyFileCpp.WriteLine("	hr = soap->Send(m_url, sEnv);");
			m_proxyFileCpp.WriteLine("	if(FAILED(hr)) return hr;");
			m_proxyFileCpp.WriteLine("	sEnv.Empty();");
			m_proxyFileCpp.WriteLine("	return env->Parse(CComVariant(soap), enc);");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("CComPtr<ISOAPEnvelope> {0}::newEnvelope(CComBSTR &methodName, CComBSTR &ns, HRESULT &hr)", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("\tCComPtr<ISOAPEnvelope> env;");
			m_proxyFileCpp.WriteLine("\thr = env.CoCreateInstance(__uuidof(CoEnvelope));");
			m_proxyFileCpp.WriteLine("\tCComPtr<ISerializerFactoryConfig2> sfc;");
			m_proxyFileCpp.WriteLine("\tISoapProxyBaseImpl::get_SerializerFactory(&sfc);");
			m_proxyFileCpp.WriteLine("\tif(env)");
			m_proxyFileCpp.WriteLine("\t{");
			m_proxyFileCpp.WriteLine("\t\tenv->putref_SerializerFactory(sfc);");
			m_proxyFileCpp.WriteLine("\t\tenv->SetMethod(methodName, ns);");
			m_proxyFileCpp.WriteLine("\t}");
			m_proxyFileCpp.WriteLine("\treturn env;");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
			m_proxyFileCpp.WriteLine("HRESULT {0}::extractRetVal(ISOAPEnvelope *env, VARIANT *pVal)", m_className);
			m_proxyFileCpp.WriteLine("{");
			m_proxyFileCpp.WriteLine("\tCComPtr<ISOAPNodes> params;");
			m_proxyFileCpp.WriteLine("\tCComPtr<ISOAPNode> node;");
			m_proxyFileCpp.WriteLine("\tenv->get_Parameters(&params);");
			m_proxyFileCpp.WriteLine("\tparams->get_Item(0, &node);");
			m_proxyFileCpp.WriteLine("\treturn node->get_Value(pVal);");
			m_proxyFileCpp.WriteLine("}");
			m_proxyFileCpp.WriteLine("");
		}
	}
}
