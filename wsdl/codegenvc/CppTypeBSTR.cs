using System;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for CppTypeBSTR.
	/// </summary>
	public class CppTypeBSTR : CppType
	{
		internal CppTypeBSTR(int vt, string name, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType) : base(vt, name, array, style, optional, xmlType)
		{
		}

		public override string LocalStorageName
		{
			get { return "CComBSTR" ; }
		}	

		public override string AssignCopy(string src, string dst)
		{
			return string.Format("{0}.CopyTo({1});", src, dst);
		}

	}
}
