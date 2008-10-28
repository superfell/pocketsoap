using System;
using System.IO;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for vcpProject.
	/// </summary>
	public class dspProject
	{
		private StreamWriter m_stm;
		private string m_name;

		public dspProject(string directory, string name)
		{
			m_stm = new StreamWriter(Path.Combine(directory, name+".dsp"), false);
			m_name = name;

			Templater t = new Templater("dsp.top.txt");
			t.Add("<<PROJECT>>", name);
			t.Add("<<POCKETSOAPDIR>>", Settings.PocketSOAPDirectory);
			t.CopyToStream(m_stm);
		}

		public void AddFile(string fileName)
		{
			fileName = Path.GetFileName(fileName);
			m_stm.WriteLine("# Begin Source File");
			m_stm.WriteLine("SOURCE={0}", fileName);
			if(fileName.EndsWith(".idl"))
				m_stm.WriteLine("# ADD MTL /tlb \"{0}.tlb\"", m_name);
			else if ( fileName.EndsWith(".rc"))
			{
				m_stm.WriteLine("# ADD BASE RSC /l 0x409");
				m_stm.WriteLine("# ADD RSC /l 0x409 /d \"RC_NOTCE\"");
			}
			else if ( fileName == "stdafx.cpp" )
				m_stm.WriteLine("# ADD CPP /Yc\"stdafx.h\"");

			m_stm.WriteLine("# End Source File");
			m_stm.WriteLine("");
		}

		public void StartHeaders()
		{
			m_stm.WriteLine("# End Group\n");
			m_stm.WriteLine("# Begin Group \"Header Files\"");
			m_stm.WriteLine("# PROP Default_Filter \"h;hpp;hxx;hm;inl\"\n");
		}

		public void StartRgsSection()
		{
			m_stm.WriteLine("# End Group\n");
			m_stm.WriteLine("# Begin Group \"Resource Files\"");
			m_stm.WriteLine("# PROP Default_Filter \"ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe\"\n");
		}

		public void Close()
		{
			Templater t = new Templater("dsp.bot.txt");
			t.Add("<<PROJECT>>", m_name);
			t.CopyToStream(m_stm);
			m_stm.Close();
		}
	}
}
