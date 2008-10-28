<%@ Page Language="C#" %>

<script runat='server'>

void Page_Load(object sender, EventArgs e)
{
	Response.ContentType = "text/xml";
	Response.Write("<headers>\n");
	foreach ( string reqHdr in Request.Headers )
	{
		Response.Write(string.Format("<{0}>", reqHdr));
		Response.Write(Request.Headers[reqHdr]);
		Response.Write(string.Format("</{0}>\n", reqHdr));
	}
	Response.Write("</headers>");
}
</script>