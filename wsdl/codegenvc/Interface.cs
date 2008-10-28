using System;
using System.Collections;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for Interface.
	/// </summary>
	public class Interface
	{
		public Interface(string itfName, string itfBase)
		{
			this.itfName = itfName;
			this.itfBase = itfBase;
			methods = new ArrayList();
		}

		public Guid iid = Guid.NewGuid();
		public string itfName;
		public string itfBase;

		private ArrayList methods;

		public void AddMethod(string methodName, ArrayList parameters)
		{
			methods.Add(new Method(methodName, parameters));
		}

		internal ArrayList Methods
		{
			get { return methods; }
		}

		internal class Method
		{
			internal Method(string name, ArrayList props)
			{
				this.Name = name;
				this.Parameters = props;
			}

			internal string Name;
			internal ArrayList Parameters;
		}
	}
}
