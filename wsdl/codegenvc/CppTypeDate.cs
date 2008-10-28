using System;
using System.Text;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// This represents a VT_DATE (DATE) type
	/// </summary>
	public class CppTypeDate : CppType
	{
		internal CppTypeDate(int vt, string name, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType) : base(vt, name, array, style, optional, xmlType)
		{
		}

		public override string ConstructVariant(int currentIndent, string variantName, string paramName)
		{
			StringBuilder b = new StringBuilder();
			b.Append('\t', currentIndent);
			b.Append("CComVariant ").Append(variantName).Append(";\n");
			b.Append('\t', currentIndent);
			b.Append(variantName).Append(".vt = VT_DATE;\n");
			b.Append('\t', currentIndent);
			b.Append(variantName).Append(".date = ").Append(paramName).Append(";\n");
			return b.ToString();
		}
	}
}
