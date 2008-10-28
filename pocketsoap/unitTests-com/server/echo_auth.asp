<%@EnableSessionState=false%>
<%
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/server/echo_auth.asp,v 1.1 2003/05/17 23:26:04 simon Exp $
'
' This is a generic SOAP/COM Endpoint using the 4S4C dispatcher and configuration components

option explicit

if request.ServerVariables("REMOTE_USER") = "" then
	response.status = "401 Unauthorized"
	response.end
end if

dim oConfFactory, oRequest, oSoap, SoapRequest, oWSDLGen
response.expires = 0 

' look for the config.xml file in the same directory we're in
' feel free to change this if needed
' altough it looks like we might be pulling this file from disk every time
' the configFactory caches a parsed copy of it.
set oConfFactory = Server.CreateObject("4s4c.configFactory")
oConfFactory.SetConfigurationFile Server.MapPath("config.xml")

' set the correct response content type

' make sure its a POST request
if request.ServerVariables("REQUEST_METHOD") <> "POST" then
	' last chance, it might be for the WSDL file, check the querystring
	set oWSDLGen = Server.CreateObject("4s4c.wsdlGenerator")
	if ucase(trim(request.ServerVariables("QUERY_STRING"))) = "WSDL" then
		response.contentType = "text/xml; charset=UTF-8"
		response.write oWSDLGen.Generate ( oConfFactory, BuildURL() )
		response.end
	else
		' show the HTML version of the service info page
		dim tx
		set tx = CreateObject("4s4c.xslt")
		response.write tx.TransformMemFile ( oWSDLGen.Generate ( oConfFactory, BuildURL() ), Server.MapPath("wsdl.xslt") )
		response.end
	end if
end if

response.contentType = "text/xml; charset=UTF-8"

' this is a little helper component that will extract the POSTED soap request
' the ASP provided functions for getting at this data, seem to be functionally challenged
set oRequest = Server.CreateObject("4s4c.RequestHelper")
SoapRequest = oRequest.GetPostData

' create an instance of the dispatcher
set oSoap = Server.CreateObject("4s4c.Dispatcher")

' this sucker does all the work, it will take the soap request, and config info
' create an instance of the correct component, call the method, and build the
' soap response.
dim bFaulted
dim arrRes
arrRes = oSoap.ExecuteWithConfig2 ( SoapRequest, oConfFactory, bFaulted )
if bFaulted then response.status = "500 SOAP Error"
response.binaryWrite arrRes

' that's it !


' this is a little helper function, that builds a URL to this page, based on the server variables
' this allows the auto generated WSDL to have a decent soap:address without having to hard code it
Function BuildURL()
	dim sProto, iPort, sServer, sFile
	if request.ServerVariables("HTTPS") = "off" then
		sProto = "http://"
	else
		sProto = "https://"
	end if
	iPort   = Request.ServerVariables("SERVER_PORT")
	sServer = Request.ServerVariables("SERVER_NAME")
	sFile   = Request.ServerVariables("SCRIPT_NAME")

	BuildURL = sProto + sServer & ":" & iPort & sFile
end Function
%>
