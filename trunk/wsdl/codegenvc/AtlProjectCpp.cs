using System;
using System.IO;
using System.Collections;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for AtlProjectCpp.
	/// </summary>
	public class AtlProjectCpp
	{
		private StreamWriter m_stm;
		private string m_name;
		private string m_fn;

		public AtlProjectCpp(string directory, string name)
		{
			m_name = name;
			m_fn = name+".cpp";
			m_stm = new StreamWriter(Path.Combine(directory, m_fn), false);
			WriteHead();
		}

		public string FileName
		{
			get { return m_fn ; }
		}

		public void AddClasses(ArrayList classes)
		{
			foreach(ProjectClass c in classes )
			{
				m_stm.WriteLine("#include \"{0}.h\"", c.className);
			}
			m_stm.WriteLine("CComModule _Module;");
			m_stm.WriteLine("");
			m_stm.WriteLine("BEGIN_OBJECT_MAP(ObjectMap)");
			foreach(ProjectClass c in classes )
			{
				m_stm.WriteLine("	OBJECT_ENTRY(CLSID_{0}, {0})", c.className);
			}
			m_stm.WriteLine("END_OBJECT_MAP()");
			m_stm.WriteLine("");
		}

		public void Close()
		{
			WriteTrailer();
			m_stm.Close();
		}

		private void WriteHead()
		{
			m_stm.WriteLine("#include \"stdafx.h\"");
			m_stm.WriteLine("#include \"resource.h\"");
			m_stm.WriteLine("#include <initguid.h>");
			m_stm.WriteLine("#include \"{0}.h\"", m_name);
			m_stm.WriteLine("#include \"{0}_i.c\"", m_name);
			m_stm.WriteLine("");
		}

		private void WriteTrailer()
		{
			m_stm.WriteLine("/////////////////////////////////////////////////////////////////////////////");
			m_stm.WriteLine("// DLL Entry Point");
			m_stm.WriteLine("");
			m_stm.WriteLine("extern \"C\"");
			m_stm.WriteLine("BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)");
			m_stm.WriteLine("{");
			m_stm.WriteLine("    if (dwReason == DLL_PROCESS_ATTACH)");
			m_stm.WriteLine("    {");
			m_stm.WriteLine("        _Module.Init(ObjectMap, (HINSTANCE)hInstance, &LIBID_{0});", m_name);
			m_stm.WriteLine("        DisableThreadLibraryCalls((HINSTANCE)hInstance);");
			m_stm.WriteLine("    }");
			m_stm.WriteLine("    else if (dwReason == DLL_PROCESS_DETACH)");
			m_stm.WriteLine("        _Module.Term();");
			m_stm.WriteLine("    return TRUE;    // ok");
			m_stm.WriteLine("}");
			m_stm.WriteLine("");
			m_stm.WriteLine("/////////////////////////////////////////////////////////////////////////////");
			m_stm.WriteLine("// Used to determine whether the DLL can be unloaded by OLE");
			m_stm.WriteLine("");
			m_stm.WriteLine("STDAPI DllCanUnloadNow(void)");
			m_stm.WriteLine("{");
   			m_stm.WriteLine("return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;");
			m_stm.WriteLine("}");
			m_stm.WriteLine("");
			m_stm.WriteLine("/////////////////////////////////////////////////////////////////////////////");
			m_stm.WriteLine("// Returns a class factory to create an object of the requested type");
			m_stm.WriteLine("");
			m_stm.WriteLine("STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)");
			m_stm.WriteLine("{");
   			m_stm.WriteLine("return _Module.GetClassObject(rclsid, riid, ppv);");
			m_stm.WriteLine("}");
			m_stm.WriteLine("");
			m_stm.WriteLine("/////////////////////////////////////////////////////////////////////////////");
			m_stm.WriteLine("// DllRegisterServer - Adds entries to the system registry");
			m_stm.WriteLine("");
			m_stm.WriteLine("STDAPI DllRegisterServer(void)");
			m_stm.WriteLine("{");
			m_stm.WriteLine("    // registers object, typelib and all interfaces in typelib");
			m_stm.WriteLine("    return _Module.RegisterServer(TRUE);");
			m_stm.WriteLine("}");
			m_stm.WriteLine("");
			m_stm.WriteLine("/////////////////////////////////////////////////////////////////////////////");
			m_stm.WriteLine("// DllUnregisterServer - Removes entries from the system registry");
			m_stm.WriteLine("");
			m_stm.WriteLine("STDAPI DllUnregisterServer(void)");
			m_stm.WriteLine("{");
			m_stm.WriteLine("    return _Module.UnregisterServer(TRUE);");
			m_stm.WriteLine("}");
		}
	}
}
