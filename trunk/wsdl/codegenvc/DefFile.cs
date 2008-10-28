using System;
using System.IO;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for DefFile.
	/// </summary>
	public class DefFile
	{
		public static void Generate(string directory, string name)
		{
			Templater t = new Templater("def.txt");
			using (StreamWriter sw = new StreamWriter(Path.Combine(directory, name + ".def"), false))
			{
				t.CopyToStream(sw);
			}
		}
	}
}
