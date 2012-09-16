<%@ Page Language="C#" %>
<%@ Import namespace="System.IO" %>
<%@ Import namespace="ICSharpCode.SharpZipLib.GZip" %>
<%@ Import namespace="ICSharpCode.SharpZipLib.Zip.Compression" %>
<%@ Import namespace="ICSharpCode.SharpZipLib.Zip.Compression.Streams" %>
<%@ Import namespace="System.Threading" %>

<script runat='server'>

void Page_Load(object sender, EventArgs e)
{
	string srcCharSet = getCharSet();
	if ( srcCharSet == "" )
		Response.ContentType = "text/plain";
	else
		Response.ContentType = "text/plain; charset=" + srcCharSet;
	byte [] buf = new byte[4096] ;
	int cb ;
	Stream s = Request.InputStream ;
	if(Request.Headers["Content-Encoding"] != null)
	{
		if(Request.Headers["Content-Encoding"].ToLower() == "deflate")
		{
			s = new InflaterInputStream(s) ;
		}
		if(Request.Headers["Content-Encoding"].ToLower() == "gzip")
		{
			s = new GZipInputStream(s) ;
		}
	}
	if (Request.QueryString["to"] != null)
	{
		int to = int.Parse(Request.QueryString["to"]);
		Thread.Sleep(to * 1000);
	}
	do
	{
		cb = s.Read(buf, 0, buf.Length) ;
		if(cb>0)
			Response.OutputStream.Write(buf, 0, cb) ;
	}
	while(cb>0) ;
}

string getCharSet()
{
	string [] cts = Request.ContentType.Split(';');
	foreach ( string p in cts )
	{
		string t = p.Trim().ToLower();
		if(t.StartsWith("charset"))
		{
			string [] parts = t.Split('=');
			return parts[1].Trim();
		}
	}
	return "";
}
</script>