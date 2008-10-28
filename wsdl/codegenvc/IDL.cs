using System;
using System.IO;
using System.Collections ;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for IDL.
	/// </summary>
	public class IDL
	{
		private StreamWriter m_stm;
		private string m_name;
		private string m_filename;
		private Guid m_tlbid;

		public IDL(string directory, string projectName)
		{
			m_name = projectName;
			m_filename = Path.Combine(directory, projectName + ".idl");
			m_stm = new StreamWriter(m_filename, false);
			m_tlbid = Guid.NewGuid();
			WriteHeader();
		}

		public string TlbId
		{
			get { return m_tlbid.ToString().ToUpper(); }
		}

		public void Close()
		{
			m_stm.Close();
		}

		private void WriteHeader()
		{
			m_stm.WriteLine("import \"oaidl.idl\";");
			m_stm.WriteLine("import \"ocidl.idl\";");
			m_stm.WriteLine("import \"{0}\\psoap.idl\";", Settings.PocketSOAPDirectory);
			m_stm.WriteLine("");
			writeItfHeader("9240D5D7-A65B-4f1a-B07C-392D2B0EA072", "Base interface for all generated proxies", "ISoapProxyBase", "IUnknown");
			m_stm.WriteLine("	[id(15000), propget] HRESULT Transport([out,retval] ISOAPTransport ** Transport);");
			m_stm.WriteLine("	[id(15000), propputref] HRESULT Transport([in] ISOAPTransport *Transport);");
			m_stm.WriteLine("	[id(15001), propget] HRESULT Url([out,retval]BSTR *Url);");
			m_stm.WriteLine("	[id(15001), propput] HRESULT Url(BSTR Url);");
			m_stm.WriteLine("	[id(15002), propget] HRESULT SerializerFactory([out,retval]ISerializerFactoryConfig2 **Factory);");
			m_stm.WriteLine("	[id(15002), propputref] HRESULT SerializerFactory(ISerializerFactoryConfig2 *Factory);");
			FinishInterface();
		}

		private void StartInterface(Interface itf)
		{
			m_stm.WriteLine("");
			writeItfHeader(itf.iid.ToString(), itf.itfName, itf.itfName, itf.itfBase);
		}

		public void AddInterface(Interface itf)
		{
			StartInterface(itf);
			foreach ( Interface.Method m in itf.Methods )
				Operation(m.Name, m.Parameters);
			FinishInterface();
		}

		private void Operation(string name, ArrayList parameters)
		{
			m_stm.Write("    {0} ( ", name ) ;
			bool first = true ;
			foreach ( Parameter p in parameters )
			{
				m_stm.Write ("{0}[{1}] {2} {3} {4}", first ? "" : ", ", p.idlDirection.ToString().ToLower(), p.cppType.FQIDLName, p.IsOutParam ? "*" : "", p.cppName ) ;
				first = false ;
			}
			m_stm.WriteLine(" );");
		}

		private void FinishInterface()
		{
			m_stm.WriteLine("};");
		}

		public void StartTypeLibrary()
		{
			m_stm.WriteLine("[");
			m_stm.WriteLine("uuid({0}),", m_tlbid.ToString().ToUpper());
			m_stm.WriteLine("version(1.0),");
			m_stm.WriteLine("helpstring(\"{0} : PocketSOAP Generated Proxy\")", m_name);
			m_stm.WriteLine("]");
			m_stm.WriteLine("library {0}", m_name);
			m_stm.WriteLine("{");
			m_stm.WriteLine("\timportlib(\"stdole32.tlb\");");
			m_stm.WriteLine("\timportlib(\"stdole2.tlb\");");
			m_stm.WriteLine("\timportlib(\"{0}\\psoap32.dll\");", Settings.PocketSOAPDirectory );
		}

		public void AddCoClass(string clsName, Guid clsid, Interface itf)
		{
			m_stm.WriteLine("\n\t[");
			m_stm.WriteLine("\tuuid({0}),", clsid.ToString().ToUpper());
			m_stm.WriteLine("\thelpstring(\"MapSerializer class\")");
			m_stm.WriteLine("\t]");
			m_stm.WriteLine("\tcoclass {0}", clsName);
			m_stm.WriteLine("\t{");		
			m_stm.WriteLine("\t\t[default] interface {0};", itf.itfName );
			m_stm.WriteLine("\t};");
		}

		public void FinishTypeLibrary()
		{
			m_stm.WriteLine("};");
		}
		private void writeItfHeader(string guid, string helpString, string itfName, string itfBase)
		{
			m_stm.WriteLine("[\n\tobject, \n\tuuid({0}), \n\toleautomation,", guid.ToUpper());
			m_stm.WriteLine("\thelpstring(\"{0}\"),\n\tpointer_default(unique)\n]", helpString);
			m_stm.WriteLine("interface {0} : {1}", itfName, itfBase);
			m_stm.WriteLine("{");

		}
	}
}
