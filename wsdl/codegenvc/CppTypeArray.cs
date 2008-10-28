using System;
using System.Text;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for CppTypeArray.
	/// </summary>
	public class CppTypeArray : CppType
	{
		private CppType arrayItemType;

		public CppTypeArray(int vt, CppType arrayItem, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType) : base (vt, "SAFEARRAY *", array, style, optional, xmlType)
		{
			this.arrayItemType = arrayItem;
		}

		public override string FQIDLName
		{
			get { return "SAFEARRAY(" + arrayItemType.FQIDLName + ")"; }
		}

		public override string CppNameNoPointer
		{
			get { return "SAFEARRAY"; }
		}

		public override string ConstructVariant(int currentIndent, string variantName, string paramName)
		{
			StringBuilder b = new StringBuilder();
			b.Append('\t', currentIndent);
			b.Append("CComVariant ").Append(variantName).Append(";\n");
			b.Append('\t', currentIndent);
			b.Append(variantName).Append(".vt = VT_ARRAY | ").Append(this.arrayItemType.VT).Append(";\n");
			b.Append('\t', currentIndent);
			b.Append(variantName).Append(".parray = ").Append(paramName).Append(";\n");
			return b.ToString();
		}

	}
}
