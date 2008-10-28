using System;
using System.Collections;
using System.Collections.Specialized ;
using System.IO;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for ComplexTypeWriter.
	/// </summary>
	public class ComplexTypeWriter
	{
		private string			m_cppName;
		private StreamWriter	m_impl, m_hdr;
		IDL						m_idl;
		Interface				m_itf;

		private StringCollection	m_vars;
		private StringCollection	m_finalconstruct;

		private const string PROP_PREFIX = "m_p";
		private const string OPT_PREFIX  = "m_o";

		public ComplexTypeWriter(vcProject project, string xmlName, string cppName)
		{
			m_itf = new Interface("I" + cppName, "IDispatch");
			ProjectClass cls = project.AddClass(cppName, m_itf);
			m_cppName = cppName;
			m_impl = cls.impl.Create();
			m_hdr  = cls.header.Create();
			m_vars = new StringCollection();
			m_finalconstruct = new StringCollection();
			m_idl = project.IdlFile;
			InitFiles(xmlName);
		}

		public string ComplexTypeName
		{
			get { return m_cppName ; }
		}

		public Guid InterfaceIID
		{
			get { return m_itf.iid; }
		}

		void InitFiles(string xmlName)
		{
			Templater t = CodeGenContext.Current.MakeTemplater("complextype.cpp.txt");
			t.Add ("<<CLASS>>", m_cppName);
			t.Add ("<<XML_TYPE_NAME>>", xmlName);
			t.CopyToStream(m_impl);
			t = CodeGenContext.Current.MakeTemplater("complextype.h.txt");
			t.Add ("<<CLASS>>", m_cppName);
			t.Add ("<<CLASS_UPPER>>", m_cppName.ToUpper());
			t.Add ("<<XML_TYPE_NAME>>", xmlName);
			t.CopyToStream(m_hdr);
		}

		public void AddPropertyField( string propertyName, CppType propertyType )
		{
			m_vars.Add ( string.Format("{0} {1}{2};", propertyType.LocalStorageName, PROP_PREFIX, propertyName ));
			GeneratePropertyAccessor(propertyName, PROP_PREFIX + propertyName, OPT_PREFIX + propertyName, propertyType);
			if(propertyType.Optional)
			{
				m_vars.Add ( string.Format("VARIANT_BOOL {0}{1};", OPT_PREFIX, propertyName));
				m_finalconstruct.Add(string.Format("{0}{1} = VARIANT_FALSE;", OPT_PREFIX, propertyName));
				GenerateOptionalAccessor(propertyName);
			}
		}

		public void Complete()
		{
			m_impl.WriteLine("HRESULT {0}::FinalConstruct()", m_cppName);
			m_impl.WriteLine("{");
			foreach ( String fc in m_finalconstruct )
			{
				m_impl.Write("\t");
				m_impl.WriteLine(fc);
			}
			m_impl.WriteLine("\treturn S_OK;");
			m_impl.WriteLine("}\n");
			m_impl.Close();
			m_hdr.WriteLine("\nprivate:");
			foreach ( string v in m_vars )
			{
				m_hdr.Write("\t");
				m_hdr.WriteLine(v);
			}
			m_hdr.WriteLine("};");
			m_hdr.WriteLine("");
			m_hdr.WriteLine("#endif");
			m_hdr.Close();
			m_idl.AddInterface(m_itf);
		}

		void GenerateOptionalAccessor(string propertyName)
		{
			CppType variant_bool = CppType.Create("VARIANT_BOOL");
			GeneratePropertyAccessor(propertyName + Consts.OPTIONAL_PROPERTY_SUFFIX, OPT_PREFIX + propertyName, "", variant_bool);
		}
		void GeneratePropertyAccessor(string propertyName, string memberName, string optionalMemberName, CppType propType)
		{
			m_hdr.WriteLine("\tSTDMETHOD(get_{0})({1} *val);", propertyName, propType.CppName);
			m_impl.WriteLine("STDMETHODIMP {0}::get_{1}({2} *val)", m_cppName, propertyName, propType.CppName);
			m_impl.WriteLine("{");
			m_impl.WriteLine("\tif(!val) return E_POINTER;");
			m_impl.WriteLine("\t{0}", propType.AssignCopy(memberName, "val"));
			m_impl.WriteLine("\treturn S_OK;");
			m_impl.WriteLine("}\n");
			m_hdr.WriteLine("\tSTDMETHOD(put_{0})({1} val);", propertyName, propType.CppName);
			m_impl.WriteLine("STDMETHODIMP {0}::put_{1}({2} val)", m_cppName, propertyName, propType.CppName);
			m_impl.WriteLine("{");
			m_impl.WriteLine("\t{0} = val;", memberName);
			if(propType.Optional)
				m_impl.WriteLine("\t{0} = VARIANT_TRUE;", optionalMemberName);
			m_impl.WriteLine("\treturn S_OK;");
			m_impl.WriteLine("}\n");

			ArrayList props = new ArrayList();
			props.Add(new Parameter(propType, propertyName, null, false, IdlDirection.In));
			m_itf.AddMethod("[propput] HRESULT " + propertyName, props);
			props = new ArrayList();
			props.Add(new Parameter(propType, propertyName, null, false, IdlDirection.Retval | IdlDirection.Out ));
			m_itf.AddMethod("[propget] HRESULT " + propertyName, props);
		}
	}
}
