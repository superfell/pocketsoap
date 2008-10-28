using System;
using System.IO;
using System.Collections;
using System.Reflection;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for Templater.
	/// </summary>
	public class Templater
	{
		private string m_srcFilename;
		private Hashtable m_replacements;

		public Templater(string srcFileName)
		{
			m_srcFilename = srcFileName;
			m_replacements = new Hashtable();
		}

		public void Add(string key, string replaceWith)
		{
			m_replacements.Add(key, replaceWith);
		}

		public void CopyToStream(StreamWriter sw)
		{
			using ( StreamReader sr = new StreamReader(findSrcFile()) )
			{
				while(true)
				{
					string ln = sr.ReadLine();
					if(ln == null) 
						break;
					sw.WriteLine(doReplacements(ln));
				}
			}
		}

		private string doReplacements(string src)
		{
			foreach ( DictionaryEntry de in m_replacements )
				src = src.Replace((string)de.Key, (string)de.Value);
			return src;
		}

		private string findSrcFile()
		{
			// we look for a templates directory under the app, if that doesn't exist we look for a templates directory in the parent dir
			Uri uri = new Uri(Assembly.GetAssembly(this.GetType()).CodeBase);
			string dir = uri.AbsolutePath;
			dir = Directory.GetParent(dir).FullName;
			while(!Directory.Exists(Path.Combine(dir, "templates")))
				dir = Directory.GetParent(dir).FullName;
			return Path.Combine(dir, "templates/" + m_srcFilename);
		}
	}
}
