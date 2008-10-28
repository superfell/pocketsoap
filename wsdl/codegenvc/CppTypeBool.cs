using System;
using System.Text;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for CppTypeBool.
	/// </summary>
	public class CppTypeBool : CppType
	{
		internal CppTypeBool(int vt, string name, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType) : base(vt, name, array, style, optional, xmlType)
		{
		}

		public override string ConstructVariant(int currentIndent, string variantName, string paramName)
		{
			StringBuilder b = new StringBuilder();
			b.Append('\t', currentIndent);
			b.Append("CComVariant ").Append(variantName).Append(";\n");
			b.Append('\t', currentIndent);
			b.Append(variantName).Append(".vt = VT_BOOL;\n");
			b.Append('\t', currentIndent);
			b.Append(variantName).Append(".boolVal = ").Append(paramName).Append(";\n");
			return b.ToString();
		}
	}
}
