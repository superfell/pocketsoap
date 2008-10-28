using System;
using Microsoft.Win32;

namespace PocketSOAP.WSDL
{
	/// <summary>
	/// Summary description for Settings.
	/// </summary>
	public class Settings
	{
		public Settings()
		{
		}

		public static string PocketSOAPDirectory
		{
			get
			{
				if(pocketSoapDir==null)
				{
					using(RegistryKey rk = Registry.ClassesRoot.OpenSubKey(@"pocketsoap.envelope\\CLSID", false))
					{
						string clsid = string.Format(@"CLSID\{0}\InprocServer32", rk.GetValue(""));
						using(RegistryKey inproc = Registry.ClassesRoot.OpenSubKey(clsid, false))
						{
							pocketSoapDir = System.IO.Path.GetDirectoryName((string)inproc.GetValue(""));
						}
					}
				}
				return pocketSoapDir;
			}
		}

		private static string pocketSoapDir = null;
	}
}
