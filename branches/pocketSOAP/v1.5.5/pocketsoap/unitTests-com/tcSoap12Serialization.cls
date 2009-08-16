VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcSoap12Serialization"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/tcSoap12Serialization.cls,v 1.4 2004/09/08 05:29:38 simon Exp $

' tests for SOAP 1.2 serialization

Option Explicit


' Interface declaration
Implements ITestContainer

' Fixture Member Variables
Private e As ISOAPEnvelope2
Private d As DOMDocument40

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ' TODO: add the names of your test methods as a parameter into the Array() function
    ITestContainer_TestCaseNames = Array("testEnvelope", "testHeader", "testBody", "testArray", "testProgID", "testContentType")
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
    Set e = New CoEnvelope
    e.EnvelopeVersion = SOAP_12_ENV
    Set d = getNewDom
End Sub

'Destroy the test fixture
Public Sub ITestContainer_TearDown()
    ' TODO: destruct your test fixture here
End Sub

Public Sub testProgID(tr As TestResult)
    Dim e As Object
    Set e = CreateObject("PocketSOAP.Envelope.12")
    tr.AssertEqualsString SOAP_12_ENV, e.EnvelopeVersion
    
    Set e = CreateObject("PocketSOAP.Envelope.11")
    tr.AssertEqualsString SOAP_11_ENV, e.EnvelopeVersion
End Sub

Public Sub testArray(tr As TestResult)
    e.SetMethod "foo", "urn:tests"
    e.Parameters.Create "a", Array("a", "b", "c")
    
    tr.Assert (d.loadXML(e.Serialize)), "failed to load XML into DOM"
    
    Dim a As IXMLDOMNode
    Set a = d.selectSingleNode("//a")
    tr.AssertEqualsString "a", a.baseName
    tr.AssertEqualsString "", a.namespaceURI
    
    Dim t As IXMLDOMAttribute
    Set t = a.Attributes.getQualifiedItem("itemType", SOAP_12_ENC)
    tr.Assert Not t Is Nothing
    tr.AssertEqualsString "XS:anyType", t.Text
    
    Dim s As IXMLDOMAttribute
    Set s = a.Attributes.getQualifiedItem("arraySize", SOAP_12_ENC)
    tr.Assert Not s Is Nothing
    tr.AssertEqualsString "3", s.Text
    
    Dim ai As IXMLDOMNode
    Set ai = a.childNodes.Item(0)
    tr.AssertEqualsString "a", ai.Text
    tr.AssertEqualsString "XS:string", ai.Attributes.getQualifiedItem("type", XSI).Text
    
    Set ai = a.childNodes.Item(1)
    tr.AssertEqualsString "b", ai.Text
    tr.AssertEqualsString "XS:string", ai.Attributes.getQualifiedItem("type", XSI).Text
    
    Set ai = a.childNodes.Item(2)
    tr.AssertEqualsString "c", ai.Text
    tr.AssertEqualsString "XS:string", ai.Attributes.getQualifiedItem("type", XSI).Text
    
End Sub

Public Sub testBody(tr As TestResult)
    e.SetMethod "foo", "urn:tests"
    e.Parameters.Create "a", "hay"
    e.Parameters.Create "b", "straw", "urn:bar"
    e.Parameters.Create "c", 42
    
    tr.Assert (d.loadXML(e.Serialize))
        
    Dim b As IXMLDOMNode
    Set b = d.selectSingleNode("//s12:Body")
    tr.AssertEqualsLong 1, b.childNodes.length
    Dim f As IXMLDOMNode
    Set f = b.childNodes.Item(0)
    tr.AssertEqualsString "foo", f.baseName
    tr.AssertEqualsString "urn:tests", f.namespaceURI
    tr.AssertEqualsLong e.Parameters.Count, f.childNodes.length
    
    Dim p As IXMLDOMNode
    Set p = f.childNodes.Item(0)
    tr.AssertEqualsString "a", p.baseName
    tr.AssertEqualsString "", p.namespaceURI
    tr.AssertEqualsString "hay", p.Text
    
    Set p = f.childNodes.Item(1)
    tr.AssertEqualsString "b", p.baseName
    tr.AssertEqualsString "urn:bar", p.namespaceURI
    tr.AssertEqualsString "straw", p.Text
    
    Set p = f.childNodes.Item(2)
    tr.AssertEqualsString "c", p.baseName
    tr.AssertEqualsString "", p.namespaceURI
    tr.AssertEqualsString "42", p.Text
    
    
End Sub

Public Sub testHeader(tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.EnvelopeVersion = SOAP_12_ENV
    e.SetMethod "foo", "urn:tests"
    Dim h2 As ISOAPNode12
    Set h2 = e.Headers.Create("myheader", 42, "urn:tests")
    h2.role = "urn:role"
    h2.relay = True
    Dim h As ISOAPNode3
    Set h = h2
    h.mustUnderstand = True
    
    Debug.Print e.Serialize
    Dim d As New MSXML2.DOMDocument40
    tr.Assert (d.loadXML(e.Serialize))
    
    Dim hdr As IXMLDOMNode
    d.setProperty "SelectionLanguage", "XPath"
    d.setProperty "SelectionNamespaces", "xmlns:a='urn:tests'"
    Set hdr = d.selectSingleNode("//a:myheader")
    tr.AssertEqualsString "myheader", hdr.baseName
    tr.AssertEqualsString "urn:tests", hdr.namespaceURI
    
    Dim role As IXMLDOMAttribute
    Set role = hdr.Attributes.getQualifiedItem("role", SOAP_12_ENV)
    tr.Assert Not role Is Nothing, "role attribute is missing"
    tr.AssertEqualsString "urn:role", role.Value
    
    Dim mu As IXMLDOMAttribute
    Set mu = hdr.Attributes.getQualifiedItem("mustUnderstand", SOAP_12_ENV)
    tr.Assert Not mu Is Nothing, "mustUnderstand attribute is missing"
    tr.AssertEqualsString "true", mu.Value
    
    Dim rl As IXMLDOMAttribute
    Set rl = hdr.Attributes.getQualifiedItem("relay", SOAP_12_ENV)
    tr.Assert Not rl Is Nothing, "relay attribute is missing"
    tr.AssertEqualsString "true", rl.Value
    
    tr.AssertEqualsString "42", hdr.childNodes.Item(0).Text
    
End Sub

Public Sub testEnvelope(tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.EnvelopeVersion = SOAP_12_ENV
    tr.AssertEqualsString SOAP_12_ENV, e.EnvelopeVersion
    e.SetMethod "foo", "urn:tests"
    e.Headers.Create "header", Empty, "urn:tests"
    
    tr.AddTrace e.Serialize
    
    Dim d As New MSXML2.DOMDocument40
    tr.Assert (d.loadXML(e.Serialize))
    
    Dim en As IXMLDOMNode
    
    ' soap:Envelope
    Set en = d.documentElement
    tr.AssertEqualsString "Envelope", en.baseName
    tr.AssertEqualsString SOAP_12_ENV, en.namespaceURI
    Dim enc As IXMLDOMAttribute
    Set enc = en.Attributes.getQualifiedItem("encodingStyle", SOAP_12_ENV)
    tr.Assert enc Is Nothing, "encoding style not allowed on envelope element"
    
    ' soap:Header
    Dim hdr As IXMLDOMNode
    Set hdr = en.childNodes.Item(0)
    tr.AssertEqualsString SOAP_12_ENV, hdr.namespaceURI
    tr.AssertEqualsString "Header", hdr.baseName
    Set enc = hdr.Attributes.getQualifiedItem("encodingStyle", SOAP_12_ENV)
    tr.Assert enc Is Nothing, "encoding style not allowed on envelope element"
    
    ' a:header
    Dim he As IXMLDOMNode
    Set he = hdr.childNodes.Item(0)
    tr.AssertEqualsString "urn:tests", he.namespaceURI
    tr.AssertEqualsString "header", he.baseName
    Set enc = he.Attributes.getQualifiedItem("encodingStyle", SOAP_12_ENV)
    tr.AssertEqualsString SOAP_12_ENC, enc.Value
    
    ' soap:Body
    Dim bd As IXMLDOMNode
    Set bd = en.childNodes.Item(1)
    tr.AssertEqualsString "Body", bd.baseName
    tr.AssertEqualsString SOAP_12_ENV, bd.namespaceURI
    Set enc = bd.Attributes.getQualifiedItem("encodingStyle", SOAP_12_ENV)
    tr.Assert enc Is Nothing, "encoding style not allowed on body element"
    
    ' a:foo
    Dim mn As IXMLDOMNode
    Set mn = bd.childNodes.Item(0)
    tr.AssertEqualsString "foo", mn.baseName
    tr.AssertEqualsString "urn:tests", mn.namespaceURI
    Set enc = mn.Attributes.getQualifiedItem("encodingStyle", SOAP_12_ENV)
    tr.AssertEqualsString SOAP_12_ENC, enc.Value
    
End Sub

Public Sub testContentType(ByVal tr As TestResult)
    Dim e As New CoEnvelope12
    e.EncodingStyle = ""
    e.SetMethod "Headers", "http://unittests.pocketsoap.com/"
    Dim t As Object
    Set t = New HTTPTransport
    t.SOAPAction = e.URI + e.MethodName
    't.setProxy PROXY_SERVER, PROXY_PORT
    t.send TEST_SERVER_HTTP + "headers.ashx", e
    e.parse t
    
    Dim n As Variant
    Dim ct As String, sa As String, cs As String
    Dim saFound As Boolean
    Dim nm As String, vl As String
    For Each n In e.Body.Item(0).Nodes
        nm = n.Nodes.itemByName("name", e.URI).Value
        vl = n.Nodes.itemByName("val", e.URI).Value
        If (nm = "Content-Type") Then
            ct = vl
        ElseIf nm = "SOAPAction" Then
            sa = vl
            saFound = True
        ElseIf nm = "Accept-Charset" Then
            cs = vl
        End If
    Next
    tr.AssertEqualsString "application/soap+xml; charset=""utf-8""; action=""" + t.SOAPAction + """", ct
    tr.AssertEqualsString "", sa
    tr.AssertEqualsString "UTF-8, UTF-16;q=0.8, iso-8859-1;q=0.8", cs
    tr.Assert Not saFound, "found unexpected SOAPAction header"
End Sub
