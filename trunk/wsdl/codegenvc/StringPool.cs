using System;
using System.Collections ;
using System.IO;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for StringPool.
	/// </summary>
	public class StringPool
	{
		private string prefix;
		private int nextId = 1 ;
		private SortedList strings = new SortedList();

		public StringPool(string prefix)
		{
			this.prefix = prefix;
		}

		public string IdForString(string s, out bool newId)
		{
			string sid = null ;
			object id = strings[s];
			if(id == null)
			{
				sid = string.Format("{0}_{1}", prefix, nextId++);
				strings[s] = sid ;
				newId = true;
			}
			else
			{
				sid = (string)id;
				newId = false;
			}
			return sid;
		}

		public void Serialize(StreamWriter sw)
		{
			foreach ( DictionaryEntry e in strings )
				sw.WriteLine("\tstatic const CComBSTR {0}(OLESTR(\"{1}\"));", e.Value, e.Key) ;
		}
	}
}
