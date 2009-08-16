VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcSerialization"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/TCTestContainer.cls,v 1.5 2004/11/30 05:27:53 simon Exp $
'
' various SOAP 1.1 serialization tests

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testStringNull", "TestDateNull", "TestNil", _
                                    "testValueAs", "testDeserTypes", "testDeserLocalTypes", "testDeserRoot", "testDeserFault", _
                                    "testSerTypes", "testSerTypes2", "testSerLocalTypes", "testSerNodeType", "TestSerNil", "testSerNoEncoding", _
                                    "testReserialize", "testEncStyle", "testDeserArray", "testDeserDefNS")
End Property

' Run the specified test case methods in this test container
Public Sub ITestContainer_RunTestCase(oTestCase As ITestCase, oTestResult As TestResult)
    On Error GoTo ErrorHandler
    InvokeHook Me, oTestCase.Name, INVOKE_FUNC, oTestResult
    Exit Sub
ErrorHandler:
    oTestResult.AddError Err.Number, Err.Source, Err.Description
End Sub

'Initialize the test fixture
Public Sub ITestContainer_Setup()
End Sub

'Destroy the test fixture
Public Sub ITestContainer_TearDown()
End Sub


' tests that when the encoding style is turned off, section 5 encoding doesn't take place
Public Sub testSerNoEncoding(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.SetMethod "doFoo", "http://www.foo.org/"
    e.EncodingStyle = ""
    CreateStruct e, "a", "b", "c"

    Dim d As New DOMDocument40
    d.validateOnParse = False
    tr.Assert d.loadXML(e.Serialize)
    tr.AssertEqualsLong 0, d.selectNodes("//*[@href]").length, "message has href's in it"
End Sub

Function CreateStruct(env, a, b, c)
    Dim n As Object
    Set n = env.Parameters.Create("foo", Empty)
    n.Nodes.Create "a", a
    n.Nodes.Create "b", b
    n.Nodes.Create "c", c
End Function

' tests that the encodingStyle stack is serialized / de-serialized correctly
Public Sub testEncStyle(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.SetMethod "doFoo", "http://www.foo.org/"
    e.Parameters.Create "foo", 5

    e.EncodingStyle = ""
    e.Headers.Create "doFooHeader", "hello", "http://www.foo.org/"
    e.Headers.Create "h2", ""

    e.Parameters.Create "bar", 5

    Dim e2 As Object
    Set e2 = CreateObject("PocketSOAP.Envelope.2")
    e2.parse e.Serialize

    tr.AssertEqualsString "", e2.EncodingStyle, "envelope encoding style"
    tr.AssertEqualsString "", e2.Headers.Item(0).EncodingStyle, "header 0 encoding Style"
    tr.AssertEqualsString "", e2.Headers.Item(1).EncodingStyle, "header 1 encoding Style"
    tr.AssertEqualsString SOAP_11_ENC, e2.Parameters.Item(0).EncodingStyle, "parameter 0 encoding Style"
    tr.AssertEqualsString SOAP_11_ENC, e2.Parameters.Item(1).EncodingStyle, "parameter 1 encoding Style"
End Sub

' tests that faultdetection / throwing is handled proeprty
Public Sub testDeserFault(ByVal tr As TestResult)
    Dim env As String
    env = "<S:Envelope xmlns:S='http://schemas.xmlsoap.org/soap/envelope/'>" + _
            "<S:Body><S:Fault>" + _
            "<faultcode>S:Server</faultcode>" + _
            "<faultstring>this is a drill</faultstring>" + _
            "</S:Fault>" + _
            "</S:Body></S:Envelope>"
            
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    On Error Resume Next
    e.parse env
    tr.Assert Err.Number <> 0, "Error not thrown"
    tr.Assert InStr(1, Err.Description, "this is a drill", vbTextCompare) > 0, "faultstring not in COM error description"
    On Error GoTo 0
End Sub

' tests that serializerFactory local types are applied during de-serialization
Public Sub testDeserLocalTypes(ByVal tr As TestResult)
    Dim env As String
    env = "<S:Envelope xmlns:S='http://schemas.xmlsoap.org/soap/envelope/' " + _
            "SOAP-ENV:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/' xmlns:E='http://schemas.xmlsoap.org/soap/encoding/' " + _
            "xmlns:d='http://soapinterop.org/' " + _
            "xmlns:xs='http://www.w3.org/2001/XMLSchema' " + _
            "xmlns:xi='http://www.w3.org/2001/XMLSchema-instance'>" + _
            "<S:Body><d:testFooResponse>" + _
            "<a xi:type='d:one'><item>String</item></a>" + _
            "<b xi:type='d:two'><item>1414</item></b>" + _
            "<c><item>33.33</item></c>" + _
            "</d:testFooResponse></S:Body></S:Envelope>"
    
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.SerializerFactory.ElementMapping "c", "", "three", "http://soapinterop.org/"
    e.SerializerFactory.LocalTypeMapping "one", "http://soapinterop.org/", "item", "", "string", XSD
    e.SerializerFactory.LocalTypeMapping "two", "http://soapinterop.org/", "item", "", "int", XSD
    e.SerializerFactory.LocalTypeMapping "three", "http://soapinterop.org/", "item", "", "float", XSD
    
    e.parse env
    
    tr.AssertEqualsLong 3, e.Parameters.Count, "incorrect # of parameters"
    tr.AssertEqualsLong vbString, VarType(e.Parameters.Item(0).Nodes.Item(0).Value), "/a/item wrong type"
    tr.AssertEqualsLong vbLong, VarType(e.Parameters.Item(1).Nodes.Item(0).Value), "/b/item wrong type"
    tr.AssertEqualsLong vbSingle, VarType(e.Parameters.Item(2).Nodes.Item(0).Value), "/c/item wrong type"

End Sub

' This tests if pocketSOAP can re-serialize a soap message it parsed correctly
Public Sub testReserialize(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.SetMethod "Add", "http://simon.fell.com/calc/"
    e.Parameters.Create "a", 10
    e.Parameters.Create "b", 20
    e.MethodName = "Subtract"

    Dim e2 As Object
    Set e2 = CreateObject("pocketSOAP.Envelope.2")
    e2.parse e.Serialize

    tr.AssertEqualsString e.MethodName, e2.MethodName, "methodName doesn't match"
    tr.AssertEqualsString e.URI, e2.URI, "methodName URI doesn't match"
    tr.AssertEqualsLong e.Parameters.Count, e2.Parameters.Count, "parameters count don't match"
    Dim i As Long
    For i = 0 To e.Parameters.Count - 1
        tr.AssertEqualsString e.Parameters.Item(i).Name, e2.Parameters.Item(i).Name, "paramater " & i & " name doesn't match"
        tr.AssertEqualsLong VarType(e.Parameters.Item(i).Value), VarType(e2.Parameters.Item(i).Value), "parameter " & i & " varType doesn't match"
        tr.AssertEqualsVariant e.Parameters.Item(i).Value, e2.Parameters.Item(i).Value, "parameter " & i & " values doesn't match"
    Next
End Sub

' tests that the root attribute is properly handled during de-serialization
Public Sub testDeserRoot(ByVal tr As TestResult)
    Dim env As String
    env = "<S:Envelope" + vbCrLf + _
        "   S:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'" + vbCrLf + _
        "   xmlns:E='http://schemas.xmlsoap.org/soap/encoding/'" + vbCrLf + _
        "   xmlns:S='http://schemas.xmlsoap.org/soap/envelope/'" + vbCrLf + _
        "   xmlns:a='http://soapinterop.org/'" + vbCrLf + _
        "   xmlns:c='http://soapinterop.org/xsd'" + vbCrLf + _
        "   xmlns:b='http://www.w3.org/2001/XMLSchema'" + vbCrLf + _
        "   xmlns:d='http://www.w3.org/2001/XMLSchema-instance'>" + vbCrLf + _
        "<S:Body><c:SOAPStruct E:root='0' id='2c92d8c' d:type='c:SOAPStruct'><varInt d:type='b:int'>1</varInt>" + vbCrLf + _
        "<varFloat d:type='b:float'>2</varFloat>" + vbCrLf + _
        "<varString d:type='b:string'>wilma</varString>" + vbCrLf + _
        "</c:SOAPStruct>" + vbCrLf + _
        "<a:echoStructArray><inputStructArray E:arrayType='b:anyType[3]'><fred href='#2c92d8c'/>" + vbCrLf + _
        "<i href='#2c92d8c'/>" + vbCrLf + _
        "v<i href='#2c92d8c'/>" + vbCrLf + _
        "</inputStructArray>" + vbCrLf + _
        "</a:echoStructArray>" + vbCrLf + _
        "</S:Body></S:Envelope>"

    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.parse env

    tr.AssertEqualsString "echoStructArray", e.MethodName, "methodName is wrong"
    tr.AssertEqualsString "http://soapinterop.org/", e.URI, "methodName URI is wrong"
End Sub

' tests that node types are serialized into the XML
Public Sub testSerNodeType(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.MethodName = "AddItem"
    e.URI = "urn:radiohost-service"
    
    Dim theNodes As Object
    Set theNodes = e.Parameters.Create("meth4_inType1", "")
    theNodes.TypeNS = "http://www.radiohostservice.com/RadioHost-interface/types/"
    theNodes.Type = "Item"
    theNodes.Nodes.Create "name", "Simon"
    theNodes.Nodes.Create "barCode", "1234"
    
    Dim d As New DOMDocument40
    d.validateOnParse = False
    tr.Assert d.loadXML(e.Serialize), "Unable to load serialized XML into DOM"
    Dim dn As IXMLDOMNode
    Set dn = d.selectSingleNode("//meth4_inType1")
    If Not dn.Attributes.getNamedItem("href") Is Nothing Then
        ' is href'd, chase it down
        Dim href As String
        href = dn.Attributes.getNamedItem("href").Text
        tr.AssertEqualsString "#", Left$(href, 1), "href attribute missing required #"
        href = Right$(href, Len(href) - 1)
        Set dn = d.selectSingleNode("//*[@id='" + href + "']")
    End If
    tr.Assert InStr(1, dn.Attributes.getQualifiedItem("type", XSI).Text, "Item", vbTextCompare), "xsi:type missing"
End Sub

' tests that the serializer takes notice of the serializerFactory config type mappings during serialization
Public Sub testSerTypes(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.SetMethod "echofoo", "http://bar.org/"
    
    Dim a, f As Object
    a = Array("one", "two", "three", "four")
    e.Parameters.Create "s", a
    Set f = e.SerializerFactory
    f.ElementMapping "echofoo", "http://bar.org/", "echofoo", "http://bar.org/"
    f.LocalTypeMapping "echofoo", "http://bar.org/", "s", "", "string", "http://www.w3.org/2001/XMLSchema"
    f.Serializer vbArray Or vbVariant, "string", "http://www.w3.org/2001/XMLSchema", "pocketSOAP.ArraySerializer.1"
    
    Dim dom As MSXML2.DOMDocument40
    Set dom = New DOMDocument40
    dom.validateOnParse = False
    tr.Assert dom.loadXML(e.Serialize), "failed to load serialized XML into the DOM"
    
    Dim dn As IXMLDOMNode
    Set dn = dom.selectSingleNode("//s")
    tr.AssertEqualsString "XS:string[4]", dn.Attributes.getQualifiedItem("arrayType", SOAP_11_ENC).Text, "arrayType attribute has wrong value"
End Sub

Public Sub testSerTypes2(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    Dim s As Object
    Set s = e.SerializerFactory
    s.ElementMapping "foo", "http://bar.org/", "foo", "http://bar.org/"
    s.LocalTypeMapping "foo", "http://bar.org/", "strings", "", "string", "http://www.w3.org/2001/XMLSchema"
    s.LocalTypeMapping "foo", "http://bar.org/", "nums", "", "integer", "http://www.w3.org/2001/XMLSchema"
    s.Serializer vbArray + vbVariant, "string", "http://www.w3.org/2001/XMLSchema", "PocketSOAP.ArraySerializer"
    s.Serializer vbArray + vbVariant, "integer", "http://www.w3.org/2001/XMLSchema", "pocketSOAP.ArraySerializer"
    Dim a1, a2
    a1 = Array("one", "two", "three")
    a2 = Array(1, 2, 3)
    e.SetMethod "foo", "http://bar.org/"
    e.Parameters.Create "strings", a1
    e.Parameters.Create "nums", a2
    
    Dim d As New DOMDocument40
    d.validateOnParse = False
    tr.Assert d.loadXML(e.Serialize), "failed to load XML into DOM"
    Dim dn As IXMLDOMNode
    Set dn = d.selectSingleNode("//strings")
    tr.AssertEqualsString "XS:string[3]", dn.Attributes.getQualifiedItem("arrayType", SOAP_11_ENC).Text, "string array type"
    Set dn = d.selectSingleNode("//nums")
    tr.AssertEqualsString "XS:integer[3]", dn.Attributes.getQualifiedItem("arrayType", SOAP_11_ENC).Text, "int array type"
End Sub

Public Sub testSerLocalTypes(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    Dim s As Object
    Set s = e.SerializerFactory
    s.ElementMapping "foo", "http://bar.org/", "foo", "http://bar.org/"
    s.LocalTypeMapping "foo", "http://bar.org/", "string", "", "NMTOKEN", XSD
    s.LocalTypeMapping "foo", "http://bar.org/", "num", "", "integer", XSD
    s.Serializer vbString, "NMTOKEN", XSD, "pocketSOAP.SimpleSerializer"
    s.Serializer vbInteger, "integer", XSD, "pocketSOAP.SimpleSerializer"
    s.Serializer vbInteger, "short", XSD, "pocketSOAP.SimpleSerializer"
    Dim a1, a2
    a1 = "TOKEN"
    a2 = 42
    e.SetMethod "foo", "http://bar.org/"
    e.Parameters.Create "string", a1
    e.Parameters.Create "num", a2
    e.Parameters.Create "myInt", a2
    
    Dim d As New DOMDocument40
    d.validateOnParse = False
    tr.Assert d.loadXML(e.Serialize), "failed to load XML into DOM"
    Dim dn As IXMLDOMNode
    Set dn = d.selectSingleNode("//string")
    tr.AssertEqualsString "XS:NMTOKEN", dn.Attributes.getQualifiedItem("type", XSI).Text, "NMTOKEN type"
    Set dn = d.selectSingleNode("//num")
    tr.AssertEqualsString "XS:integer", dn.Attributes.getQualifiedItem("type", XSI).Text, "int type"
    Set dn = d.selectSingleNode("//myInt")
    tr.AssertEqualsString "XS:short", dn.Attributes.getQualifiedItem("type", XSI).Text, "short type"
End Sub

' tests that untyped elements pick up their types from the serializer factory config
Public Sub testDeserTypes(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("PocketSOAP.Envelope.2")

    ' treat a <varFloat> as a {http://www.w3.org/2001/XMLSchema}float
    ' treat a <varInt>   as a {http://www.w3.org/2001/XMLSchema}int
    ' note that string is the default if no mapping can be found, so we don't need to
    ' specifically add a mapping for <varString>
    e.SerializerFactory.ElementMapping "varFloat", "", "float", XSD
    e.SerializerFactory.ElementMapping "varInt", "", "int", XSD

    e.parse getTypesEnv
    tr.AssertEqualsLong 3, e.Parameters.Count, "wrong # of parameters de-serialized"
    tr.AssertEqualsLong vbSingle, VarType(e.Parameters.itemByName("varFloat").Value), "varFloat wrong type"
    tr.AssertEqualsLong vbLong, VarType(e.Parameters.itemByName("varInt").Value), "varInt wrong type"
    tr.AssertEqualsLong vbString, VarType(e.Parameters.itemByName("varString").Value), "varString wrong type"
    
    tr.Assert e.Parameters.itemByName("varFloat").Value = -42.42, "varFloat wrong value"
    tr.AssertEqualsLong 420042, e.Parameters.itemByName("varInt").Value, "varInt wrong value"
    tr.AssertEqualsString "Orbital", e.Parameters.itemByName("varString").Value, "varString wrong value"
End Sub

' tests that valueAs works for simple types
Public Sub testValueAs(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketsoap.envelope.2")
    e.parse getTypesEnv
    
    tr.AssertEqualsLong 3, e.Parameters.Count, "wrong # of parameters de-serialized"
    tr.Assert VarType(e.Parameters.itemByName("varFloat").ValueAs("float", XSD)) = vbSingle, "varFloat not expected type"
    tr.Assert VarType(e.Parameters.itemByName("varInt").ValueAs("int", XSD)) = vbLong, "varInt not expected type"
    tr.Assert VarType(e.Parameters.itemByName("varString").ValueAs("string", XSD)) = vbString, "varString not expected type"
    tr.Assert e.Parameters.itemByName("varFloat").ValueAs("float", XSD) = -42.42, "varFloat wrong value"
    tr.AssertEqualsLong 420042, e.Parameters.itemByName("varInt").ValueAs("int", XSD), "varInt wrong value"
    tr.AssertEqualsString "Orbital", e.Parameters.itemByName("varString").ValueAs("string", XSD), "varString wrong value"
End Sub

Private Function getTypesEnv() As String
    getTypesEnv = "<SOAP-ENV:Envelope SOAP-ENV:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/' " + _
                    "xmlns:SOAP-ENV='http://schemas.xmlsoap.org/soap/envelope/'>" + _
                    "<SOAP-ENV:Body><m:GotStruct xmlns:m='http://examples.pocketsoap.com/'>" + _
                    "<varFloat>-42.42</varFloat>" + _
                    "<varString>Orbital</varString>" + _
                    "<varInt>420042</varInt>" + _
                    "</m:GotStruct></SOAP-ENV:Body></SOAP-ENV:Envelope>"
End Function

' serializes a 'Nothing' and makes sure that its serialized as xsi:nil='true'
Public Sub testSerNil(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.MethodName = "foo"
    e.Parameters.Create "myVar", Nothing
    Dim xml As String, myVar As Integer, endmyVar As Integer
    xml = e.Serialize
    myVar = InStr(1, xml, "<myVar")
    endmyVar = InStr(myVar, xml, ">")
    tr.Assert InStr(1, Mid$(xml, myVar, endmyVar - myVar), ":nil=""true""", vbTextCompare), "xsi:nil missing from serialized message"
End Sub

' tests that a null string gets deserialized into a zero length string, not VT_NULL [this is a special case for the NULL de-serializer]
Public Sub testStringNull(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.parse getStringNull
    tr.AssertEqualsLong 1, e.Parameters.Count, "wrong # of parameters"
    tr.AssertEqualsLong vbString, VarType(e.Parameters.Item(0).Value), "unexpected type"
End Sub

' tests that a null date, gets de-serialized into a VT_NULL [included because there was a report this didn't work]
Public Sub TestDateNull(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.parse getDateNull
    
    tr.AssertEqualsLong 1, e.Parameters.Count, "wrong # of parameters returned"
    tr.AssertEqualsLong vbNull, VarType(e.Parameters.Item(0).Value), "unexpected type"
End Sub

' tests that an element with xsi:nil or xsi:null [for 1999 schema messages]
Public Sub TestNil(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.parse getNil
    
    tr.AssertEqualsLong 3, e.Parameters.Count, "wrong # of parameters returned"
    tr.AssertEqualsLong vbNull, VarType(e.Parameters.Item(0).Value), "unexpected type [2001 schema test]"
    tr.AssertEqualsLong vbString, VarType(e.Parameters.Item(1).Value), "unexpected type [string]"
    tr.AssertEqualsLong vbNull, VarType(e.Parameters.Item(2).Value), "unexpected type [1999 schema test]"
End Sub

Public Sub TestDeserArray(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketsoap.envelope.2")
    e.parse getArray
    Dim a
    a = e.Parameters.Item(0).Value
    tr.AssertEqualsLong 4, UBound(a) - LBound(a) + 1
    tr.AssertEqualsLong 0, LBound(a)
    tr.AssertEqualsLong 3, UBound(a)
End Sub

Public Sub testDeserDefNS(ByVal tr As TestResult)
    Dim e As Object
    Set e = CreateObject("pocketsoap.envelope.2")
    e.parse getMessage("defns")
    tr.AssertEqualsLong 1, e.Parameters.Count
    tr.AssertEqualsString "Login Info is here", e.Parameters.Item(0).Value
    
End Sub

Function getArray() As String
    getArray = "<?xml version=""1.0"" encoding=""UTF-8"" standalone=""no""?>" + _
               "<SOAP-ENV:Envelope xmlns:SOAPSDK1=""http://www.w3.org/2001/XMLSchema"" xmlns:SOAPSDK2=""http://www.w3.org/2001/XMLSchema-instance"" xmlns:SOAPSDK3=""http://schemas.xmlsoap.org/soap/encoding/"" xmlns:SOAP-ENV=""http://schemas.xmlsoap.org/soap/envelope/"">" + _
               "<SOAP-ENV:Body SOAP-ENV:encodingStyle=""http://schemas.xmlsoap.org/soap/encoding/""><SOAPSDK4:GetDocListResponse xmlns:SOAPSDK4=""http://tempuri.org/FWISPSServer/message/"">" + _
               "<Result SOAPSDK3:arrayType=""SOAPSDK1:string[4]"" SOAPSDK3:offset=""[1]"" SOAPSDK2:type=""SOAPSDK3:Array"">" + _
               "<SOAPSDK3:string>Client_Number~106~8!Doc_Type~00100~8!F_ARCHIVEDATE~~1!F_CLOSED~False~11!F_DELETEDATE~~1!F_DOCCLASSNAME~WFLoans~8!F_DOCCLASSNUMBER~3~18!F_DOCFORMAT~image/tiff;name=&quot;KofaxSample.tif&quot;~8!F_DOCLOCATION~~1!F_DOCNUMBER~100008~19!F_DOCTYPE~0~13!F_ENTRYDATE~11/14/2002~7!F_PAGES~1~18!F_RETENTBASE~0~13!F_RETENTDISP~49~13!F_RETENTOFFSET~12~19!Loan_Number~123456~8!TestIndex~~1</SOAPSDK3:string><SOAPSDK3:string>Client_Number~472~8!Doc_T" + _
               "ype~00100~8!F_ARCHIVEDATE~~1!F_CLOSED~False~11!F_DELETEDATE~~1!F_DOCCLASSNAME~WFLoans~8!F_DOCCLASSNUMBER~3~18!F_DOCFORMAT~image/tiff;name=&quot;TrafficReport1.tif&quot;~8!F_DOCLOCATION~~1!F_DOCNUMBER~100009~19!F_DOCTYPE~0~13!F_ENTRYDATE~11/14/2002~7!F_PAGE" + _
               "S~1~18!F_RETENTBASE~0~13!F_RETENTDISP~49~13!F_RETENTOFFSET~12~19!Loan_Number~123456~8!TestIndex~~1</SOAPSDK3:string><SOAPSDK3:string>Client_Number~106~8!Doc_Type~00300~8!F_ARCHIVEDATE~~1!F_CLOSED~False~11!F_DELETEDATE~~1!F_DOCCLASSNAME~WFLoans~8!F_DOCCLASSNUMBER~3~18!F_DOCFORMAT~image/tiff;name=&quot;TrafficReport2.tif&quot;~8!F_DOCLOCATION~~1!F_DOCNUMBER~100010~19!F_DOCTYPE~0~13!F_ENTRYDATE~11/14/2002~7!F_PAGES~1~18!F_RETENTBASE~0~13!F_RETENTDISP~49~13!F_RETENTOFFSET~12~19!Loan_Number~123456~8!TestIndex~~1</SOAPSDK3:string></Result></SOAPSDK4:GetDocListResponse></SOAP-ENV:Body></SOAP-ENV:Envelope>"
End Function

Function getNil()
    getNil = "<SOAP-ENV:Envelope SOAP-ENV:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/' " + _
                "xmlns:SOAP-ENV='http://schemas.xmlsoap.org/soap/envelope/'>" + _
                "<SOAP-ENV:Body xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>" + _
                "<m:GotStruct xmlns:m='http://examples.pocketsoap.com/'>" + _
                "<one xsi:nil='true'></one>" + _
                "<two>Orbital</two>" + _
                "<three xmlns:xsi='http://www.w3.org/1999/XMLSchema-instance' xmlns:xsd='http://www.w3.org/1999/XMLSchema' xsi:null='true' />" + _
                "</m:GotStruct></SOAP-ENV:Body></SOAP-ENV:Envelope>"
End Function

Private Function getDateNull() As String
    getDateNull = "<SOAP-ENV:Envelope SOAP-ENV:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/' " + _
                    "xmlns:SOAP-ENV='http://schemas.xmlsoap.org/soap/envelope/'>" + _
                    "<SOAP-ENV:Body xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>" + _
                    "<m:GotStruct xmlns:m='http://examples.pocketsoap.com/' xmlns:xsd='http://www.w3.org/2001/XMLSchema'>" + _
                    "<one xsi:type='xsd:dateTime' xsi:nil='true' />" + _
                    "</m:GotStruct></SOAP-ENV:Body></SOAP-ENV:Envelope>"
End Function

Private Function getStringNull() As String
    getStringNull = "<SOAP-ENV:Envelope SOAP-ENV:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/' " + _
                    "xmlns:SOAP-ENV='http://schemas.xmlsoap.org/soap/envelope/'>" + _
                    "<SOAP-ENV:Body xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>" + _
                    "<m:GotStruct xmlns:m='http://examples.pocketsoap.com/' xmlns:xsd='http://www.w3.org/2001/XMLSchema'>" + _
                    "<one xsi:type='xsd:string' xsi:nil='true' />" + _
                    "</m:GotStruct></SOAP-ENV:Body></SOAP-ENV:Envelope>"
End Function

