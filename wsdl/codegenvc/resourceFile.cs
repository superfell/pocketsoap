using System;
using System.IO;
using System.Collections;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for resourceFile.
	/// </summary>
	public class ResourceFile
	{
		private StreamWriter m_stm;
		private string m_name;
		private StreamWriter m_resource_h;
		private int next_id = 201;

		public ResourceFile(string directory, string name)
		{
			m_stm = new StreamWriter(Path.Combine(directory, name+".rc"), false);
			m_resource_h = new StreamWriter(Path.Combine(directory, "resource.h"), false);

			m_name = name;

			Templater t = new Templater("rc.top.txt");
			t.Add("<<PROJECT>>", name);
			t.CopyToStream(m_stm);

			m_resource_h.WriteLine("//{{NO_DEPENDENCIES}}");
			m_resource_h.WriteLine("// Microsoft Developer Studio generated include file.");
			m_resource_h.WriteLine("// Used by cgt.rc");
			m_resource_h.WriteLine("//");
			m_resource_h.WriteLine("#define IDS_PROJNAME                    100");


		}


		public void AddRegistry(string name, string regFile)
		{
			m_stm.WriteLine("IDR_{0} REGISTRY DISCARDABLE    \"{1}\"", name.ToUpper(), regFile);
			m_resource_h.WriteLine("#define IDR_{0} {1}", name.ToUpper(), next_id++);
		}

		public void Close()
		{
			Templater t = new Templater("rc.bot.txt");
			t.Add("<<PROJECT>>", m_name);
			t.CopyToStream(m_stm);
			m_stm.Close();

			m_resource_h.WriteLine("");
			m_resource_h.WriteLine("// Next default values for new objects");
			m_resource_h.WriteLine("// ");
			m_resource_h.WriteLine("#ifdef APSTUDIO_INVOKED");
			m_resource_h.WriteLine("#ifndef APSTUDIO_READONLY_SYMBOLS");
			m_resource_h.WriteLine("#define _APS_NEXT_RESOURCE_VALUE        {0}", next_id);
			m_resource_h.WriteLine("#define _APS_NEXT_COMMAND_VALUE         32768");
			m_resource_h.WriteLine("#define _APS_NEXT_CONTROL_VALUE         201");
			m_resource_h.WriteLine("#define _APS_NEXT_SYMED_VALUE           101");
			m_resource_h.WriteLine("#endif");
			m_resource_h.WriteLine("#endif");
			m_resource_h.Close();
		}
	}
}
