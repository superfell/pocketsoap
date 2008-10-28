<%@ WebHandler class="compEcho" language="C#" %>

// $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/server/deflateecho.ashx,v 1.1 2005/03/01 05:55:36 simon Exp $

using System ;
using System.Xml ;
using System.Web  ;
using System.IO;
using ICSharpCode.SharpZipLib.Zip.Compression.Streams;

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
		if ( string.Compare(ctx.Request.Headers["Content-Encoding"], "deflate", true ) == 0 ) {
			rs = new InflaterInputStream(rs);
		}
		
		Stream os = ctx.Response.OutputStream;
		if ( ctx.Request.Headers["Accept-Encoding"].IndexOf("deflate") >= 0) {
			os = new DeflaterOutputStream(os);
			ctx.Response.AddHeader("content-encoding","deflate");
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