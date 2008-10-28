using System;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for CodeGenContext.
	/// </summary>
	public class CodeGenContext
	{
		public static CodeGenContext Current
		{
			get
			{
				return theContext;
			}
		}

		private static CodeGenContext theContext = new CodeGenContext();

		private CodeGenContext()
		{
		}

		private string projectName, directory, wsdlUrl;

		// PROJECT
		public string ProjectName
		{
			get { return projectName ; }
			set { projectName = value; }
		}

		// DIRECTORY
		public string Directory
		{
			get { return directory ; }
			set { directory = value; }
		}

		// WSDL_URL
		public string WsdlUrl
		{
			get { return wsdlUrl ; }
			set { wsdlUrl = value ;}
		}

		/// <summary>
		/// Creates and initializes a new Templater object, the Templater replacements are initialized with all the known context values
		/// </summary>
		/// <param name="templateFile"></param>
		/// <returns></returns>
		public Templater MakeTemplater(string templateFile)
		{
			Templater t= new Templater(templateFile);
			t.Add("<<WSDL_URL>>", WsdlUrl);
			t.Add("<<DIRECTORY>>", Directory);
			t.Add("<<PROJECT>>", ProjectName);
			return t;
		}
	}
}
