<%@ WebHandler class="compEcho" language="C#" %>

// $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/server/gzipecho.ashx,v 1.1 2005/03/01 05:55:36 simon Exp $

using System ;
using System.Xml ;
using System.Web  ;
using System.IO;
using ICSharpCode.SharpZipLib.GZip;

// echo's request, handles inbound & outbound gzip compression

class compEcho : IHttpHandler
{
	public bool IsReusable
	{
		get { return false ; }
	}
	
	public void ProcessRequest(HttpContext ctx)
	{	
		Stream rs = ctx.Request.InputStream;
		if ( string.Compare(ctx.Request.Headers["Content-Encoding"], "gzip", true ) == 0 ) {
			rs = new GZipInputStream(rs);
		}
		
		Stream os = ctx.Response.OutputStream;
		if ( ctx.Request.Headers["Accept-Encoding"] != null && ctx.Request.Headers["Accept-Encoding"].IndexOf("gzip") >= 0) {
			os = new GZipOutputStream(os);
			ctx.Response.AddHeader("content-encoding","gzip");
		}
		
		ctx.Response.ContentType = ctx.Request.Headers["Content-Type"];

		byte [] buff = new byte[1024];
		int cb = 0;
		do
		{
			cb = rs.Read(buff, 0, buff.Length);
			os.Write(buff,0,cb);
		} while ( cb > 0 );		
		os.Flush();
		os.Close();
	}
}