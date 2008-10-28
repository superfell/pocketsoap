using System;
using System.Collections ;
using System.Collections.Specialized ;

namespace PocketSOAP.WSDL
{
	[Flags]
	public enum IdlDirection
	{
		In = 1,
		Out = 2,
		Retval = 4
	}

	class Parameter
	{
		public Parameter(CppType cppType, string cppName, wsdlParser.qname xmlName, bool isHeader, IdlDirection dir)
		{
			populateWords();
			this.cppType = cppType;
			this.cppName = safeCppName(cppName);
			this.xmlName = xmlName;
			this.isHeader = isHeader;
			this.idlDirection = dir;
		}

		public CppType			cppType;
		public string			cppName;
		public wsdlParser.qname	xmlName;
		public bool				isHeader;
		public IdlDirection		idlDirection;

		public bool IsOutParam
		{
			get { return ( idlDirection & IdlDirection.Out ) > 0 ; }
		}

		private string safeCppName(string n)
		{
			if(words.ContainsKey(n))
				return safeCppName("_" + n);
			return n;
		}

		private void populateWords()
		{
			if(words == null)
			{
				words = new StringDictionary();
				words["return"] = null;
				words["public"] = null;
				words["private"] = null;
				words["protected"] = null;
				words["for"] = null;
				words["if"] = null;
				words["while"] = null;
				words["do"] = null;
				words["case"] = null;
				words["goto"] = null;
				words["BSTR"] = null;
				words["char"] = null;
				words["wchar"] = null;
				words["int"] = null;
				words["short"] = null;
				words["long"] = null;
				words["VARIANT"] = null;
				words["struct"] = null;
				words["class"] = null;
			}
		}

		private static StringDictionary words;
	}
}
