using System;
using System.IO;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for RgsWriter.
	/// </summary>
	public class RgsWriter
	{
		public static string GenerateRgs(string projectName, string tlbId, string baseDirectory, ProjectClass cls)
		{
			Templater t = new Templater("rgs.txt");
			t.Add("<<PROJECT>>", projectName);
			t.Add("<<CLASS>>", cls.className);
			t.Add("<<CLSID>>", cls.clsid.ToString().ToUpper());
			t.Add("<<TLBID>>", tlbId.ToUpper());

			string fn = cls.className + ".rgs";
			using ( StreamWriter sw = new StreamWriter(Path.Combine(baseDirectory, fn), false) )
			{
				t.CopyToStream(sw);
			}
			return fn;
		}
	}
}
