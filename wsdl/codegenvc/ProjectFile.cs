using System;
using System.IO;

namespace PocketSOAP.WSDL
{
	public class ProjectFile
	{
		public ProjectFile(string fn)
		{
			fileName = fn;
		}
		
		public string fileName;

		public StreamWriter Create()
		{
			return new StreamWriter(fileName, false);
		}
	}

	public class ProjectClass
	{
		public ProjectClass(string className, ProjectFile header, ProjectFile impl, Interface itf)
		{
			this.className = className;
			this.header = header;
			this.impl = impl;
			this.proxyInterface = itf;
			clsid = Guid.NewGuid();
		}

		public string		className;
		public ProjectFile  header;
		public ProjectFile  impl;
		public Interface	proxyInterface;
		public Guid			clsid;
	}
}
