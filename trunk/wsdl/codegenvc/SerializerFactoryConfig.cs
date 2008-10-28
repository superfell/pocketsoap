using System;
using System.Collections.Specialized;
using System.IO;

namespace PocketSOAP.WSDL
{
	public enum IdType
	{
		Namespace,
		Localname,
		Type,
		ProgId
	}

	/// <summary>
	/// Summary description for SerializerFactoryConfig.
	/// </summary>
	public class SerializerFactoryConfig
	{
		private StringCollection m_factorySetup;
		private StringPool []	m_pools;

		public SerializerFactoryConfig()
		{
			m_factorySetup = new StringCollection();
			m_pools = new StringPool[4];
			m_pools[0] = new StringPool("ns");
			m_pools[1] = new StringPool("op");
			m_pools[2] = new StringPool("type");
			m_pools[3] = new StringPool("progid");
		}
	
		public string IdForString(string s, IdType t)
		{
			StringPool sp = m_pools[(int)t];
			bool newString;
			string id = sp.IdForString(s, out newString);
			if(newString)
				m_factorySetup.Add(string.Format("CComBSTR {0}(OLESTR(\"{1}\"));", id, s)) ;
			return id;
		}

		public void Add(string s)
		{
			m_factorySetup.Add(s);
		}

		public void Serialize(StreamWriter sw)
		{
			foreach(string ln in m_factorySetup)
			{
				sw.Write("\t");
				sw.WriteLine(ln);
			}
		}
	}
}
