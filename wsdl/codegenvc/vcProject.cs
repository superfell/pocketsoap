using System;
using System.IO;
using System.Collections;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for vcProject.
	/// </summary>
	public class vcProject
	{
		private string m_directory;
		private string m_name;
		
		private ArrayList m_headers;
		private ArrayList m_cpp;
		private ArrayList m_classes;
		private IDL m_idl;

		public vcProject(string directory, string projectName)
		{
			m_directory = directory;
			m_name = projectName;

			Initialize();
			CreateStdAfx();
			m_idl = new IDL(m_directory, m_name);
			AddISoapProxyBaseImpl();
			AddTemplateClass("consts");
		}

		public static string safeClassName(string cn)
		{
			// TODO
			return cn;
		}
	
		public string ProjectName
		{
			get { return m_name ; }
		}

		public IDL IdlFile
		{
			get { return m_idl ; }
		}

		public ProjectClass AddClass(string className, Interface itf)
		{
			ProjectFile header = new ProjectFile(Path.Combine(m_directory, className + ".h"));
			ProjectFile impl   = new ProjectFile(Path.Combine(m_directory, className + ".cpp"));
			ProjectClass cls = new ProjectClass(className, header, impl, itf);
			m_classes.Add(cls);
			return cls;
		}

		public void Finalize()
		{
			m_idl.StartTypeLibrary();
			ResourceFile rf = new ResourceFile(m_directory, m_name);
			m_headers.Add(new ProjectFile("resource.h"));

			foreach ( ProjectClass c in m_classes )
			{
				m_idl.AddCoClass(c.className, c.clsid, c.proxyInterface);
				rf.AddRegistry(c.className, RgsWriter.GenerateRgs(m_name, m_idl.TlbId, m_directory, c));
			}
			m_idl.FinishTypeLibrary();
			m_idl.Close();
			rf.Close();

			AtlProjectCpp atlcpp = new AtlProjectCpp(m_directory, m_name);
			atlcpp.AddClasses(m_classes);
			atlcpp.Close();
			m_cpp.Add(new ProjectFile(atlcpp.FileName));

			DefFile.Generate(m_directory, m_name);

			// todo, generate project file
			dspProject dsp = new dspProject(m_directory, m_name);
			foreach ( ProjectClass c in m_classes )
				dsp.AddFile(c.impl.fileName);
			foreach ( ProjectFile f in m_cpp)
				dsp.AddFile(f.fileName);
			dsp.AddFile(m_name + ".idl");
			dsp.AddFile(m_name + ".rc");
			dsp.AddFile(m_name + ".def");

			dsp.StartHeaders();
			foreach ( ProjectClass c in m_classes )
				dsp.AddFile(c.header.fileName);
			foreach ( ProjectFile f in m_headers )
				dsp.AddFile(f.fileName);

			dsp.StartRgsSection();
			foreach ( ProjectClass c in m_classes )
				dsp.AddFile(c.className + ".rgs");
			dsp.Close();
		}
	
		private void Initialize()
		{
			Directory.CreateDirectory(m_directory);
			m_headers = new ArrayList();
			m_cpp = new ArrayList();
			m_classes = new ArrayList();
		}

		private void AddTemplateClass(string className)
		{
			m_headers.Add(MakeTemplateProjectFile(className + ".h"));
			m_cpp.Add(MakeTemplateProjectFile(className + ".cpp"));
		}

		private ProjectFile MakeTemplateProjectFile(string fileName)
		{
			ProjectFile pf = new ProjectFile(fileName);
			Templater t= CodeGenContext.Current.MakeTemplater(fileName + ".txt");
			using (StreamWriter sw = pf.Create())
				t.CopyToStream(sw);
			return pf;
		}

		private void AddISoapProxyBaseImpl()
		{
			m_headers.Add(MakeTemplateProjectFile("ISoapProxyBaseImpl.h"));
		}

		private void CreateStdAfx()
		{
			CreateStdAfxHeader();
			CreateStdAfxCpp();
		}
		
		// todo: this should be re-written to use the template support

		private void CreateStdAfxCpp()
		{
			string fn = Path.Combine(m_directory, "stdafx.cpp");
			using ( StreamWriter sw = new StreamWriter(fn, false))
			{
				sw.WriteLine("#include \"stdafx.h\"");
				sw.WriteLine("");
				sw.WriteLine("#ifdef _ATL_STATIC_REGISTRY");
				sw.WriteLine("#include <statreg.h>");
				sw.WriteLine("#include <statreg.cpp>");
				sw.WriteLine("#endif");
				sw.WriteLine("#include <atlimpl.cpp>");
			}
			m_cpp.Add(new ProjectFile(fn));
		}

		private void CreateStdAfxHeader()
		{
			string fn = Path.Combine(m_directory, "stdafx.h");
			using ( StreamWriter sw = new StreamWriter(fn, false))
			{
				sw.WriteLine ("#if !defined(AFX_STDAFX_H__65459711_3F7E_4AFB_AF15_D8F9A8AB101A__INCLUDED_)");
				sw.WriteLine ("#define AFX_STDAFX_H__65459711_3F7E_4AFB_AF15_D8F9A8AB101A__INCLUDED_");
				sw.WriteLine ("");
				sw.WriteLine ("#if _MSC_VER > 1000");
				sw.WriteLine ("#pragma once");
				sw.WriteLine ("#endif // _MSC_VER > 1000");
				sw.WriteLine ("");
				sw.WriteLine ("#define STRICT");
				sw.WriteLine ("#ifndef _WIN32_WINNT");
				sw.WriteLine ("#define _WIN32_WINNT 0x0400");
				sw.WriteLine ("#endif");
				sw.WriteLine ("#define _ATL_APARTMENT_THREADED");
				sw.WriteLine ("");
				sw.WriteLine ("#include <atlbase.h>");
				sw.WriteLine ("//You may derive a class from CComModule and use it if you want to override");
				sw.WriteLine ("//something, but do not change the name of _Module");
				sw.WriteLine ("extern CComModule _Module;");
				sw.WriteLine ("#include <atlcom.h>");
				sw.WriteLine ("#include <vector>");
				sw.WriteLine ("");
				sw.WriteLine ("#define _HR(x) { HRESULT hr = x; if(FAILED(hr)) return hr ; }");
				sw.WriteLine ("#endif // !defined(AFX_STDAFX_H__65459711_3F7E_4AFB_AF15_D8F9A8AB101A__INCLUDED_)");
			}
			m_headers.Add(new ProjectFile(fn));
		}
	}
}
