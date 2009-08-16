<%@ WebService Language="C#" class="echoHeaders" debug="true" %>

using System ;
using System.Web ;
using System.Web.Services;
using System.Collections ;

[WebService(Namespace="http://unittests.pocketsoap.com/")]
public class echoHeaders
{
	public class hdr
	{
		public hdr()
		{
		}
		public hdr ( string n, string v )
		{
			name = n ;
			val = v ; 
		}
		
		public string name ;
		public string val ;
	}
	 
	[WebMethod]
	public hdr [] Headers()
	{
		ArrayList res = new ArrayList() ;
		foreach ( string h in HttpContext.Current.Request.Headers )
		{
			res.Add ( new hdr(h, HttpContext.Current.Request.Headers[h]) ) ;
		}
		// return res ;
		hdr [] hr = (hdr [])res.ToArray(typeof(hdr)) ;
		return  hr ; // (hdr [])res.ToArray(typeof(hdr [])) ;	
	}
}