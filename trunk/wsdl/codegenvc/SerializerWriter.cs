using System;
using System.Collections;
using System.IO;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for SerializerWriter.
	/// </summary>
	public class SerializerWriter
	{
		string			m_cppName, m_serializeeName;
		StreamWriter	m_hdr, m_impl;
		bool			m_firstAttr = true;
		bool			m_gotValItf;
		ArrayList		m_deserA = null;
		ArrayList		m_deserE = null;
		ArrayList		m_deserC  = null;

		public SerializerWriter(vcProject project, wsdlParser.qname serializerQName, string serializerName, string serializeeName )
		{
			ProjectClass cls = project.AddClass(serializerName, new Interface("ISoapSerializer", "IUnknown"));
			m_hdr = cls.header.Create();
			m_impl = cls.impl.Create();
			m_cppName = serializerName;
			m_serializeeName = serializeeName;
			InitFiles();
			m_deserA = new ArrayList();
			m_deserE = new ArrayList();
			m_deserC = new ArrayList();
		}

		public String SerializerName
		{
			get { return m_cppName ; }
		}

		void InitFiles()
		{
			Templater t = CodeGenContext.Current.MakeTemplater("s_complextype.cpp.txt");
			doTemplate(t, m_impl);
			t = CodeGenContext.Current.MakeTemplater("s_complextype.h.txt");
			doTemplate(t, m_hdr);
		}

		void doTemplate(Templater t, StreamWriter sw)
		{
			t.Add("<<CLASS>>", m_cppName);
			t.Add("<<CLASS_UPPER>>", m_cppName.ToUpper());								
			t.Add("<<SERIALIZEE>>", m_serializeeName);
			t.CopyToStream(sw);
		}

		public void AddAttribute(string propertyName, CppType propType, MSXML2.ISchemaAttribute attr, wsdlParser.qname xmlType)
		{
			getValItf();
			if(m_firstAttr)
			{
				m_firstAttr = false;
				m_impl.WriteLine("\tCComQIPtr<ISerializerOutput2> dest2(dest);");
				m_deserA.Add("\tCComQIPtr<ISoapDeSerializerAttributes2> a(Attributes);");
				m_deserA.Add("\tCComVariant av;");
			}
			// serializer
			m_impl.WriteLine("\t{0} {1};", propType.CppName, propertyName);
			m_impl.WriteLine("\t_HR(obj->get_{0}(&{1}));", propertyName, propertyName);
			m_impl.WriteLine("\t_HR(dest2->SerializeAttribute(CComVariant({0}), CComBSTR(OLESTR(\"{1}\")), CComBSTR(OLESTR(\"{2}\"))));", propertyName, attr.name, attr.namespaceURI);

			// de-serializer
			m_deserA.Add(string.Format("\t_HR(a->get_AsValue(CComBSTR(OLESTR(\"{0}\")), CComBSTR(OLESTR(\"{1}\")), CComBSTR(OLESTR(\"{2}\")), CComBSTR(OLESTR(\"{3}\")), &av);", attr.name, attr.namespaceURI, xmlType.localname, xmlType.@namespace ));
			m_deserA.Add(string.Format("\t_HR(m_obj->put_{1}(av);"));
			m_deserA.Add("\tav.Clear();");
		}

		public void AddElement(string propertyName, CppType propType, MSXML2.ISchemaElement elem)
		{
			// serialization
			getValItf();
			if(propType.Array == CppType.ArrayType.Literal)
			{
			}
			else
			{
				m_impl.WriteLine("\t{0} {1};", propType.LocalStorageName, propertyName);
				m_impl.WriteLine("\t_HR(obj->get_{0}(&{0}));", propertyName ) ;
				m_impl.WriteLine("\t_HR(dest->SerializeValue(&CComVariant({0}), CComBSTR(OLESTR(\"{1}\")), CComBSTR(OLESTR(\"{2}\"))));", propertyName, elem.name, elem.namespaceURI );
			}

			// de-serialization
			m_deserE.Add(string.Format("\tif (wcscmp(name, L\"{0}\") == 0 && wcscmp(namespaceURI, L\"{1}\") == 0)", elem.name, elem.namespaceURI));
			m_deserE.Add("\t{");
			m_deserE.Add(string.Format("\t\treturn m_obj->put_{0}(v.{1});", propertyName, propType.VariantFieldName));
			m_deserE.Add("\t}");
																											   
			if(propType.Array == CppType.ArrayType.Literal)
			{
			}
			else
			{
			}
		}

		void getValItf()
		{
			if(!m_gotValItf)
			{
				m_gotValItf = true;
				m_impl.WriteLine("\tCComPtr<I{0}> obj;", m_serializeeName);
				m_impl.WriteLine("\tCComVariant vVal;");
				m_impl.WriteLine("\t_HR(VariantCopyInd(&vVal, val));");
				m_impl.WriteLine("\t_HR(vVal.punkVal->QueryInterface(&obj));");
			}
		}
		public void Complete()
		{
			// impl of Serialize
			m_impl.WriteLine("\treturn S_OK;");
			m_impl.WriteLine("}");
			m_impl.WriteLine("");

			m_impl.WriteLine("STDMETHODIMP {0}::Start(/*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns )", m_cppName);
			m_impl.WriteLine("{");
			m_impl.WriteLine("\t_HR(CreateObject());");
			m_impl.WriteLine("\t_HR(node->put_Value(CComVariant(m_obj)));");
			m_impl.WriteLine("\tm_refs.clear();");
			m_impl.WriteLine("");
			foreach (string ln in m_deserA)
				m_impl.WriteLine(ln);
			m_impl.WriteLine("\treturn S_OK;");
			m_impl.WriteLine("}");
			m_impl.WriteLine("");

			m_impl.WriteLine("STDMETHODIMP {0}::Characters( /*[in]*/ BSTR charData )", m_cppName);
			m_impl.WriteLine("{");
			foreach (string ln in m_deserC)
				m_impl.WriteLine(ln);
			m_impl.WriteLine("\treturn S_OK;");
			m_impl.WriteLine("}");
			m_impl.WriteLine("");

			m_impl.WriteLine("HRESULT {0}::NewNode(BSTR name, BSTR namespaceURI, ISOAPNode *node)", m_cppName);
			m_impl.WriteLine("{");
			m_impl.WriteLine("\tCComVariant v;");
			m_impl.WriteLine("\t_HR(node->get_Value(&v));");
			m_impl.WriteLine("\tif (v.vt == VT_NULL) return S_OK;");
			m_impl.WriteLine("");
			foreach (string ln in m_deserE)
				m_impl.WriteLine(ln);
			m_impl.WriteLine("\treturn S_OK;");
			m_impl.WriteLine("}");
			m_impl.Close();

			m_hdr.WriteLine("");
			m_hdr.WriteLine("#endif");
			m_hdr.Close();
		}
	}
}
