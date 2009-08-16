VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcMultiref11"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/tcMultiref.cls,v 1.4 2004/01/05 04:52:05 simon Exp $
'
' section 5 multi-ref serialization tests

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testSerialize", "testSimple", "testForward", "testBackward", "testRingBuffer", "testUnusedId", "testExternalHref")
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

' test that multi-refs are serialized correctly, the bill & ship nodes should both point to the the address node
Public Sub testSerialize(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.SetMethod "foo", "urn:tests"
    Dim n As New CoSoapNode
    n.Name = "address"
    n.Namespace = "urn:tests"
    n.Nodes.Create "city", "san francisco"
    n.Nodes.Create "state", "california"
    e.Parameters.Create "bill", n
    e.Parameters.Create "ship", n
    
    Dim d As DOMDocument40
    Set d = getNewDom
    tr.Assert (d.loadXML(e.Serialize))
    
    Dim bill As IXMLDOMElement, ship As IXMLDOMElement, adr As IXMLDOMElement
    Set bill = d.selectSingleNode("//bill")
    Set ship = d.selectSingleNode("//ship")
    Set adr = d.selectSingleNode("//a:address")
    
    Dim href As String
    href = "#" + adr.Attributes.getQualifiedItem("id", "").Text
    tr.AssertEqualsString href, bill.Attributes.getQualifiedItem("href", "").Text
    tr.AssertEqualsString href, ship.Attributes.getQualifiedItem("href", "").Text
    tr.AssertEqualsLong 0, bill.childNodes.length
    tr.AssertEqualsLong 0, ship.childNodes.length
    tr.AssertEqualsString SOAP_11_ENC, adr.Attributes.getQualifiedItem("encodingStyle", SOAP_11_ENV).Text
End Sub

Public Sub testSimple(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.parse simpleMsg
    tr.AssertEqualsString "Add", e.MethodName
    tr.AssertEqualsString "http://simon.fell.com/calc", e.URI
    tr.AssertEqualsString "the string", e.Parameters.itemByName("head", "").Value
    tr.AssertEqualsString "#12c5a04", e.Parameters.itemByName("head", "").href
    tr.AssertEqualsLong 1, e.Parameters.Count
End Sub

Public Sub testForward(ByVal tr As TestResult)
    test3itemArray forwardMsg, tr
End Sub

Public Sub testBackward(ByVal tr As TestResult)
    test3itemArray backwardMsg, tr
End Sub

Private Sub test3itemArray(msg As String, ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.parse msg
    tr.AssertEqualsString "Add", e.MethodName
    tr.AssertEqualsString "http://simon.fell.com/calc", e.URI
    tr.AssertEqualsLong 1, e.Parameters.Count
    tr.AssertEqualsString "#12c5a04", e.Parameters.itemByName("head", "").href
    tr.AssertEqualsString "head", e.Parameters.Item(0).Name
    tr.AssertEqualsLong vbArray + vbString, VarType(e.Parameters.Item(0).Value)
    tr.AssertEqualsLong 2, UBound(e.Parameters.Item(0).Value)
    Dim s() As String
    s = e.Parameters.Item(0).Value
    tr.AssertEqualsString "string 3", s(0)
    tr.AssertEqualsString "string 4", s(1)
    tr.AssertEqualsString "string 3", s(2)
    ' note s(0) & s(2) point to two different string instance because string array item's aren't multi-ref'd in COM
End Sub

Public Sub testRingBuffer(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.parse ringMsg
    
    tr.AssertEqualsString "Add", e.MethodName
    tr.AssertEqualsString "http://simon.fell.com/calc", e.URI
    tr.AssertEqualsLong 1, e.Parameters.Count
    tr.AssertEqualsString "#12c5a04", e.Parameters.itemByName("head", "").href
    tr.AssertEqualsString "head", e.Parameters.Item(0).Name
    
    Dim n As CoSoapNode
    Set n = e.Parameters.Item(0).Value ' head/tail
    Dim expectedValue As Long
    expectedValue = 5
    While (expectedValue > 0)
        tr.AssertEqualsLong 2, n.Nodes.Count
        tr.AssertEqualsLong expectedValue, n.Nodes.itemByName("value", "").Value
        Set n = n.Nodes.itemByName("next", "").Value
        expectedValue = expectedValue - 1
    Wend
    
End Sub

Public Sub testUnusedId(ByVal tr As TestResult)
    test3itemArray unusedIdMsg, tr
End Sub

Public Sub testExternalHref(ByVal tr As TestResult)
    test3itemArray externalHrefMsg, tr
    Dim e As New CoEnvelope
    e.parse externalHrefMsg
    Dim n As CoSoapNode
    Set n = e.Body.itemByName("att", "http://simon.fell.com/calc")
    tr.AssertEqualsString "cid:foo", n.href
End Sub

Private Function msgStart() As String
    msgStart = "<S:Envelope " + _
                "S:encodingStyle=""http://schemas.xmlsoap.org/soap/encoding/"" " + _
                "xmlns:Enc=""http://schemas.xmlsoap.org/soap/encoding/"" " + _
                "xmlns:S=""http://schemas.xmlsoap.org/soap/envelope/"" " + _
                "xmlns:a=""http://simon.fell.com/calc"" " + _
                "xmlns:b=""http://soapinterop.org"" " + _
                "xmlns:XS=""http://www.w3.org/2001/XMLSchema"" " + _
                "xmlns:XI=""http://www.w3.org/2001/XMLSchema-instance"">" + _
                "<S:Body>"
End Function

Private Function msgEnd() As String
    msgEnd = "</S:Body></S:Envelope>"
End Function

Private Function simpleMsg() As String
    simpleMsg = msgStart + _
                "<a:Add><head href=""#12c5a04""/>" + _
                "</a:Add>" + _
                "<b:linkedlist Enc:root=""0"" id=""12c5a04"" XI:type=""XS:string"">the string</b:linkedlist>" + _
                msgEnd
End Function

Private Function forwardMsg() As String
    forwardMsg = msgStart + _
                "<a:Add><head href=""#12c5a04""/>" + _
                "</a:Add>" + _
                "<b:linkedlist Enc:root=""0"" id=""12c5a04"" Enc:arrayType='XS:string[3]'>" + _
                "<i href='#3'/>" + _
                "<i href='#4'/>" + _
                "<i href='#3'/>" + _
                "</b:linkedlist>" + _
                "<s id='3'>string 3</s>" + _
                "<s id='4'>string 4</s>" + _
                msgEnd
End Function

Private Function backwardMsg() As String
    backwardMsg = msgStart + _
                "<s id='3' Enc:root=""0"">string 3</s>" + _
                "<s id='4' Enc:root=""0"">string 4</s>" + _
                "<b:linkedlist Enc:root=""0"" id=""12c5a04"" Enc:arrayType='XS:string[3]'>" + _
                "<i href='#3'/>" + _
                "<i href='#4'/>" + _
                "<i href='#3'/>" + _
                "</b:linkedlist>" + _
                "<a:Add><head href=""#12c5a04""/></a:Add>" + _
                msgEnd
End Function

Private Function ringMsg() As String
    ringMsg = msgStart + _
             "<a:Add><head href=""#12c5a04""/>" + _
             "</a:Add>" + _
             "<b:linkedlist Enc:root=""0"" id=""12c277c"" XI:type=""b:linkedlist""><value></value>" + _
             "<next href=""#12c5a04""/>" + _
             "</b:linkedlist>" + _
             "<b:linkedlist Enc:root=""0"" id=""12c502c"" XI:type=""b:linkedlist""><value XI:type=""XS:short"">1</value>" + _
             "<next href=""#12c277c""/>" + _
             "</b:linkedlist>" + _
             "<b:linkedlist Enc:root=""0"" id=""12c5254"" XI:type=""b:linkedlist""><value XI:type=""XS:short"">2</value>" + _
             "<next href=""#12c502c""/>" + _
             "</b:linkedlist>" + _
             "<b:linkedlist Enc:root=""0"" id=""12c54e4"" XI:type=""b:linkedlist""><value XI:type=""XS:short"">3</value>" + _
             "<next href=""#12c5254""/>" + _
             "</b:linkedlist>" + _
             "<b:linkedlist Enc:root=""0"" id=""12c5774"" XI:type=""b:linkedlist""><value XI:type=""XS:short"">4</value>" + _
             "<next href=""#12c54e4""/>" + _
             "</b:linkedlist>" + _
             "<b:linkedlist Enc:root=""0"" id=""12c5a04"" XI:type=""b:linkedlist""><value XI:type=""XS:short"">5</value>" + _
             "<next href=""#12c5774""/>" + _
             "</b:linkedlist>" + _
             msgEnd
End Function

Private Function unusedIdMsg() As String
    unusedIdMsg = msgStart + _
                "<s id='3' Enc:root=""0"">string 3</s>" + _
                "<s id='4' Enc:root=""0"">string 4</s>" + _
                "<b:linkedlist Enc:root=""0"" id=""12c5a04"" Enc:arrayType='XS:string[3]'>" + _
                "<i href='#3'/>" + _
                "<i href='#4'/>" + _
                "<i href='#3'/>" + _
                "</b:linkedlist>" + _
                "<a:Add id='1234'><head href=""#12c5a04""/></a:Add>" + _
                "<b:linkedlist Enc:root=""0"" id=""5678"">foo</b:linkedlist>" + _
                msgEnd
End Function

Private Function externalHrefMsg() As String
    externalHrefMsg = msgStart + _
                "<s id='3' Enc:root=""0"">string 3</s>" + _
                "<s id='4' Enc:root=""0"">string 4</s>" + _
                "<b:linkedlist Enc:root=""0"" id=""12c5a04"" Enc:arrayType='XS:string[3]'>" + _
                "<i href='#3'/>" + _
                "<i href='#4'/>" + _
                "<i href='#3'/>" + _
                "</b:linkedlist>" + _
                "<a:Add id='1234'><head href=""#12c5a04""/></a:Add>" + _
                "<a:att Enc:root=""0"" href=""cid:foo"" />" + _
                msgEnd
End Function

