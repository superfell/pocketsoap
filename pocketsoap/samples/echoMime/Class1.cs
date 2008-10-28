using System;
using PocketSOAP ;
using PocketSOAPAttachments ;

namespace echoMime
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			// the file to echo
			string fileName = @"c:\344.txt" ;
			if ( args.Length > 0 )
				fileName = args[0] ;

			// create the envelope
			CoEnvelope e = new CoEnvelopeClass() ;
			e.SetMethod("echo", "urn:EchoAttachmentsService")  ;

			// create attachments manager
			CoAttachmentManager mgr = new CoAttachmentManagerClass() ;
			mgr.Format = AttachmentFormat.formatMime ;
			
			// create the parameter, and point it at the attachment
			CoSoapNode n = e.Parameters.Create("source", null, "","","") ;
			n.href = mgr.Request.Create(fileName, TypeNameFormat.tnfMediaType ,"application/octetstream").Uri ;

			// create and configure the transport
			IHTTPTransport t = new HTTPTransportClass() ;
			t.SOAPAction = "test" ;
			mgr.Transport = t ;
			ISOAPTransport st = mgr as ISOAPTransport ;
			st.Send ("http://soap.4s4c.com/axis/servlet/AxisServlet", e.Serialize() ) ;
			string enc = "" ;
			e.Parse (st, enc) ;
	
			// get the returned attachment and dump some info about it
			CoSoapAttachment  att = mgr.Response.Find(e.Parameters.get_Item(0).href) ;
			Console.WriteLine ( "Attachment TypeName  {0}", att.TypeName ) ;
			Console.WriteLine ( "Attachment ContentId {0}", att.ContentId ) ;
		}
	}
}
