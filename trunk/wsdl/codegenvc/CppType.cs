using System;
using System.Collections;
using System.Runtime.InteropServices;
using System.Text;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for CppType.
	/// 
	/// todo, pull all construction / factory goop out into its own class
	/// </summary>
	public class CppType
	{
		public enum TypeStyle
		{
			Primative,
			Object
		};
    
		public enum ArrayType
		{
			NotArray,
			Encoded,
			Literal
		};
	
		static CppType()
		{
			mappings = new Hashtable();
			mappings[VarEnum.VT_I1]			= new Mapping(VarEnum.VT_I1,		"cVal",		"BYTE");
			mappings[VarEnum.VT_I2]			= new Mapping(VarEnum.VT_I2,		"iVal",		"short");
			mappings[VarEnum.VT_I4]			= new Mapping(VarEnum.VT_I4,		"lVal",		"long");
			mappings[VarEnum.VT_UI1]		= new Mapping(VarEnum.VT_UI1,		"bVal",		"BYTE");
			mappings[VarEnum.VT_UI2]		= new Mapping(VarEnum.VT_UI2,		"uiVal",	"USHORT");
			mappings[VarEnum.VT_UI4]		= new Mapping(VarEnum.VT_UI4,		"ulVal",	"ULONG");
			mappings[VarEnum.VT_R4]			= new Mapping(VarEnum.VT_R4,		"fltVal",	"float");
			mappings[VarEnum.VT_R8]			= new Mapping(VarEnum.VT_R8,		"dblVal",	"double");
			mappings[VarEnum.VT_BOOL]		= new Mapping(VarEnum.VT_BOOL,		"boolVal",	"VARIANT_BOOL");
			mappings[VarEnum.VT_BSTR]		= new Mapping(VarEnum.VT_BSTR,		"bstrVal",	"BSTR");
			mappings[VarEnum.VT_DATE]		= new Mapping(VarEnum.VT_DATE,		"date",		"DATE");
			mappings[VarEnum.VT_DISPATCH]	= new Mapping(VarEnum.VT_DISPATCH,	"pdispVal",	"IDispatch", TypeStyle.Object);
			mappings[VarEnum.VT_UNKNOWN]	= new Mapping(VarEnum.VT_UNKNOWN,	"punkVal",	"IUnknown", TypeStyle.Object);
		}

		internal class Mapping
		{
			internal Mapping(VarEnum vt, string varFieldName, string name): this(vt, varFieldName, name, TypeStyle.Primative)
			{
			}

			internal Mapping(VarEnum vt, string varFieldName, string name, TypeStyle style)
			{
				this.vt = vt;
				this.varFieldName = varFieldName;
				this.name = name;
				this.style = style;
			}

			internal VarEnum	vt;
			internal string		name;
			internal string		varFieldName;
			internal TypeStyle	style;
		}

		static Hashtable mappings;

		public static CppType Create(string name)
		{
			return Factory(VTFromName(name), name, ArrayType.NotArray, TypeStyle.Primative, false, null);
		}

		public static CppType Create(int vt, string name, wsdlParser.qname xmlType)
		{
			return Factory(vt, name, ArrayType.NotArray, TypeStyle.Primative, false, xmlType);
		}

		public static CppType Create(int vt, string name, wsdlParser.qname xmlType, TypeStyle style)
		{
			return Factory(vt, name, ArrayType.NotArray, style, false, xmlType);
		}

		public static CppType Create(int vt, string name, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType)
		{
			return Factory(vt, name, array, style, optional, xmlType);
		}

		public static CppType CreateArray(CppType arrayItem, ArrayType array, bool optional, wsdlParser.qname xmlType)
		{
			return Factory(arrayItem, array, TypeStyle.Primative, optional, xmlType);
		}

		public static CppType TypeFromVariantType(int vt, wsdlParser.qname xmlType)
		{
			CppType t= null;
			Mapping m = (Mapping)mappings[(VarEnum)vt];
			if(m != null)
				t = Create((int)m.vt, m.name, xmlType, m.style);
			else
				t = Create(vt, "VARIANT", xmlType, TypeStyle.Primative); 

			if (( vt & (int)VarEnum.VT_ARRAY ) > 0 )
				return Factory(t, ArrayType.Encoded, TypeStyle.Primative, false, xmlType);
			return t;
		}

		private static int VTFromName(string typeName)
		{
			foreach ( Mapping m in mappings.Values )
			{
				if (String.Compare(m.name, typeName, true) == 0)
					return (int)m.vt;
			}
			throw new ArgumentException("Unable to find a VT mapping for the type '" + typeName + "'", typeName);
		}

		// todo: fix all the VarEnum/int casting nonsense
		private static CppType Factory(CppType arrayItem, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType)
		{
			return new CppTypeArray((int)VarEnum.VT_ARRAY, arrayItem, array, style, optional, xmlType);
		}

		private static CppType Factory(int vt, string name, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType)
		{
			if(name == "BSTR")
				return new CppTypeBSTR(vt, name, array, style, optional, xmlType);

			if(name == "DATE")
				return new CppTypeDate(vt, name, array, style, optional, xmlType);

			if(name == "VARIANT_BOOL")
				return new CppTypeBool(vt, name, array, style, optional, xmlType);

			if ( style == TypeStyle.Object )
				return new CppTypeInterface(vt, name, array, style, optional, xmlType);

			return new CppType(vt, name, array, style, optional, xmlType);
		}

		public static CppType FromParticle(ProxyWriter pw, MSXML2.ISchemaParticle p)
		{
			MSXML2.ISchemaElement e = p as MSXML2.ISchemaElement;
			wsdlParser.qname ptype = new wsdlParser.qnameClass();
			ptype.localname = e.type.name;
			ptype.@namespace = e.type.namespaceURI;
			CppType itemType = pw.mapType(ptype);
			int minOccurs = int.Parse(p.minOccurs.ToString());
			int maxOccurs = int.Parse(p.maxOccurs.ToString());
			if ((maxOccurs == 1) && (minOccurs == 0))
				itemType.Optional = true;
			else if ((maxOccurs != 1) || (minOccurs != 1))
				itemType.Array = ArrayType.Literal;
			else if (e != null && e.isNillable)
				itemType.Optional = true;

			return itemType;
		}

		private CppType()
		{
		}
		
		public CppType Clone()
		{
			return (CppType)MemberwiseClone();
		}

		protected CppType(int vt, string name, ArrayType array, TypeStyle style, bool optional, wsdlParser.qname xmlType)
		{
			this.cppName= name;
			this.Array = array;
			this.Style = style;
			this.Optional = optional;
			this.XmlType = xmlType;
			this.vt = vt;
		}
	
		protected string cppName;
		protected int	 vt;

		/// <summary>
		/// The General CPP name of this type, e.g. int, long, BSTR, SAFEARRAY *
		/// </summary>
		public virtual string CppName
		{
			get { return cppName ; }
		}

		/// <summary>
		/// The General CPP name of this type without any pointers (e.g. int, long, BSTR, SAFEARRAY)
		/// </summary>
		public virtual string CppNameNoPointer
		{
			get { return cppName; }
		}

		/// <summary>
		/// The name of this type as it appears in IDL, e.g. int, long, BSTR, SAFEARRAY(..)
		/// </summary>
		public virtual string IdlName
		{
			get { return cppName; }
		}
		
		/// <summary>
		/// The name of this type as we store it in local vars, member variables etc, e.g. int, long, CComBSTR, etc.
		/// </summary>
		public virtual string LocalStorageName
		{
			get { return cppName; }
		}

		/// <summary>
		/// returns the c++ code need to assign a copy of this type to another variable, assume dst is a pointer
		/// </summary>
		/// <returns></returns>
		public virtual string AssignCopy(string src, string dst)
		{
			return string.Format("*{0} = {1};", dst, src);
		}

		/// <summary>
		/// The fully qualified IDLName, includes *, I prefix's etc
		/// </summary>
		public virtual string FQIDLName
		{
			get { return cppName; }
		}

		public virtual string ConstructVariant(int currentIndent, string variantName, string paramName) 
		{
			StringBuilder b = new StringBuilder();
			b.Append('\t', currentIndent);
			b.AppendFormat("CComVariant {0}({1});", variantName, paramName);
			return b.ToString();
		}

		/// <summary>
		/// As it should appear as a parameter, includes *, I prefix's etc.
		/// </summary>
		public virtual string CPPParameterName
		{
			get { return cppName; }
		}

		/// <summary>
		/// a stringified version of this type VariantType, e.g. "VT_BSTR"
		/// </summary>
		public virtual string VT 
		{
			get
			{
				return ((VarEnum)vt).ToString();
			}
		}

		/// <summary>
		/// The suffix that should be applied to the extracRetValSuffixXXXX Method name for this type
		/// </summary>
		public virtual string ExtracRetValSuffix
		{
			get { return this.CppNameNoPointer; }
		}

		/// <summary>
		/// returns the fieldName of this type in the VARIANT union type (i.e. for a CppType for VT_BSTR it returns bstrVal)
		/// </summary>
		public virtual string VariantFieldName
		{
			get { return ((Mapping)mappings[((VarEnum)vt)]).varFieldName; }
		}

		public ArrayType		Array;
		public TypeStyle		Style;
		public bool				Optional;
		public wsdlParser.qname	XmlType;
	}
}
