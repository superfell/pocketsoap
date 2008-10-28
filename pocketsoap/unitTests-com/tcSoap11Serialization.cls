VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcSOAP11Serialization"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/tcSoap11Serialization.cls,v 1.4 2004/08/21 19:19:24 simon Exp $
'
' tests for basic SOAP 1.1. envelope serialization

Option Explicit


' Interface declaration
Implements ITestContainer

' Fixture Member Variables

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ' TODO: add the names of your test methods as a parameter into the Array() function
    ITestContainer_TestCaseNames = Array("testEnvelope", "testHeader", "testOffset", "testContentType", "testSoapEncOnBody")
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
    ' TODO: initialize your test fixture here
End Sub

'Destroy the test fixture
Public Sub ITestContainer_TearDown()
    ' TODO: destruct your test fixture here
End Sub

Public Sub testSoapEncOnBody(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.parse getMessage("soapEncOnBody")
    
    Dim dom As DOMDocument40
    Set dom = getNewDom
    tr.Assert (dom.loadXML(e.Serialize))
    
    Dim bodyChild As MSXML2.IXMLDOMElement
    Set bodyChild = dom.selectSingleNode("//a:echoEnumResponse")
    Dim soapEnc As IXMLDOMNode
    Set soapEnc = bodyChild.Attributes.getQualifiedItem("encodingStyle", SOAP_11_ENV)
    tr.Assert Not soapEnc Is Nothing, "couldn't find soapencoding attribute"
    tr.AssertEqualsString SOAP_11_ENC, soapEnc.Text, "soapEncoding has wrong value"
End Sub

Public Sub testContentType(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.EncodingStyle = ""
    e.SetMethod "Headers", "http://unittests.pocketsoap.com/"
    Dim t As New HTTPTransport
    t.SOAPAction = e.URI + e.MethodName
    t.setProxy "localhost", 5049
    t.send TEST_SERVER_HTTP + "headers.asmx", e.Serialize
    e.parse t
    
    Dim n As Variant
    Dim ct As String, sa As String, cs As String
    Dim nm As String, vl As String
    For Each n In e.Parameters.Item(0).Nodes
        nm = n.Nodes.itemByName("name", e.URI).Value
        vl = n.Nodes.itemByName("val", e.URI).Value
        If (nm = "Content-Type") Then
            ct = vl
        ElseIf nm = "SOAPAction" Then
            sa = vl
        ElseIf nm = "Accept-Charset" Then
            cs = vl
        End If
    Next
    tr.AssertEqualsString "text/xml; charset=UTF-8", ct
    tr.AssertEqualsString """" + t.SOAPAction + """", sa
    tr.AssertEqualsString "UTF-8, UTF-16;q=0.8, iso-8859-1;q=0.8", cs
End Sub

Public Sub testOffset(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    Dim a(5 To 10) As Long
    e.SetMethod "foo", "urn:tests"
    e.Parameters.Create "arr", a
    
    Dim d As New DOMDocument40
    d.validateOnParse = False
    d.setProperty "SelectionLanguage", "XPath"
    tr.Assert (d.loadXML(e.Serialize))
    
    Dim arr As IXMLDOMElement
    Set arr = d.selectSingleNode("//arr")
    tr.AssertEqualsString "XS:int[11]", arr.Attributes.getQualifiedItem("arrayType", SOAP_11_ENC).Text
    tr.AssertEqualsString "[5]", arr.Attributes.getQualifiedItem("offset", SOAP_11_ENC).Text
End Sub

Public Sub testHeader(tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.EnvelopeVersion = SOAP_11_ENV
    e.SetMethod "foo", "urn:bar"
    Dim h As ISOAPNode2
    Set h = e.Headers.Create("myheader", 42, "urn:bar")
    h.actor = "urn:actor"
    h.mustUnderstand = True
    
    Dim d As New MSXML2.DOMDocument40
    tr.Assert (d.loadXML(e.Serialize))
    
    Dim hdr As IXMLDOMNode
    d.setProperty "SelectionLanguage", "XPath"
    d.setProperty "SelectionNamespaces", "xmlns:a='urn:bar'"
    Set hdr = d.selectSingleNode("//a:myheader")
    tr.AssertEqualsString "myheader", hdr.baseName
    tr.AssertEqualsString "urn:bar", hdr.namespaceURI
    
    Dim actor As IXMLDOMAttribute
    Set actor = hdr.Attributes.getQualifiedItem("actor", SOAP_11_ENV)
    tr.Assert Not actor Is Nothing, "actor attribute is missing"
    tr.AssertEqualsString "urn:actor", actor.Value
    
    Dim mu As IXMLDOMAttribute
    Set mu = hdr.Attributes.getQualifiedItem("mustUnderstand", SOAP_11_ENV)
    tr.Assert Not mu Is Nothing, "mustUnderstand attribute is missing"
    tr.AssertEqualsString "1", mu.Value
        
    tr.AssertEqualsString "42", hdr.childNodes.Item(0).Text
End Sub

Public Sub testEnvelope(tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.EnvelopeVersion = SOAP_11_ENV
    tr.AssertEqualsString SOAP_11_ENV, e.EnvelopeVersion
    e.SetMethod "foo", "urn:bar"
    e.Headers.Create "header", Empty, "urn:bar"
        
    Dim d As New MSXML2.DOMDocument40
    tr.Assert (d.loadXML(e.Serialize))
    
    Dim en As IXMLDOMNode
    
    ' soap:Envelope
    Set en = d.documentElement
    tr.AssertEqualsString "Envelope", en.baseName
    tr.AssertEqualsString SOAP_11_ENV, en.namespaceURI
    
    ' soap:Header
    Dim hdr As IXMLDOMNode
    Set hdr = en.childNodes.Item(0)
    tr.AssertEqualsString SOAP_11_ENV, hdr.namespaceURI
    tr.AssertEqualsString "Header", hdr.baseName
    
    ' a:header
    Dim he As IXMLDOMNode
    Set he = hdr.childNodes.Item(0)
    tr.AssertEqualsString "header", he.baseName
    tr.AssertEqualsString "urn:bar", he.namespaceURI
    Dim enc As IXMLDOMAttribute
    Set enc = he.Attributes.getQualifiedItem("encodingStyle", SOAP_11_ENV)
    tr.AssertEqualsString SOAP_11_ENC, enc.Value
    
    ' soap:Body
    Dim bd As IXMLDOMNode
    Set bd = en.childNodes.Item(1)
    tr.AssertEqualsString "Body", bd.baseName
    tr.AssertEqualsString SOAP_11_ENV, bd.namespaceURI
    
    ' a:foo
    Dim mn As IXMLDOMNode
    Set mn = bd.childNodes.Item(0)
    tr.AssertEqualsString "foo", mn.baseName
    tr.AssertEqualsString "urn:bar", mn.namespaceURI
    Set enc = mn.Attributes.getQualifiedItem("encodingStyle", SOAP_11_ENV)
    tr.AssertEqualsString SOAP_11_ENC, enc.Value

End Sub
