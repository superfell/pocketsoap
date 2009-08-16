<%@ WebHandler class="soap12Headers" language="C#" %>

// $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/server/headers.ashx,v 1.1 2005/03/01 05:55:36 simon Exp $

using System ;
using System.Xml ;
using System.Web  ;

class soap12Headers : IHttpHandler
{
	public bool IsReusable
	{
		get { return false ; }
	}
	
	public void ProcessRequest(HttpContext ctx)
	{	
		const string SOAP_12 = "http://www.w3.org/2003/05/soap-envelope" ;
		const string TESTS   = "http://unittests.pocketsoap.com/" ;
		
		ctx.Response.ContentType= "application/soap+xml; charset=utf-8" ;
		
		XmlTextWriter xw = new XmlTextWriter(ctx.Response.OutputStream, null) ;
		xw.Formatting = Formatting.Indented ;
		
		xw.WriteStartElement("e", "Envelope", SOAP_12) ;
		xw.WriteStartElement("Body" ,    SOAP_12) ;
		xw.WriteStartElement("m",  "Headers",  TESTS ) ;
		
		foreach ( string h in ctx.Request.Headers )
		{
			xw.WriteStartElement( "header", TESTS ) ;
			xw.WriteElementString( "name", TESTS, h ) ;
			xw.WriteElementString( "val", TESTS, ctx.Request.Headers[h] ) ;
			xw.WriteEndElement();
		}
		
		xw.WriteEndElement() ;
		xw.WriteEndElement() ;
		xw.WriteEndElement() ;
		xw.Close() ;
	}
}