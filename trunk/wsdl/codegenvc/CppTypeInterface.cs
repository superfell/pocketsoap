using System;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for CppTypeInterface.
	/// </summary>
	public class CppTypeInterface : CppType
	{
		internal CppTypeInterface(int vt, string name, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType) : base(vt, name, array, style, optional, xmlType)
		{
		}

		public override string LocalStorageName
		{
			get { return "CComPtr<" + base.LocalStorageName + ">"; }
		}

		public override string FQIDLName
		{
			get { return "I" + base.FQIDLName + " *" ; }
		}

		public override string CPPParameterName
		{
			get	{ return "I" + base.CPPParameterName + " *"; }
		}

		public override string ExtracRetValSuffix
		{
			get
			{
				// ExtractRetVal for objects uses a templated function called ExtractRetValInterface
				return "Interface";
			}
		}

	}
}
