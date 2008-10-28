VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcSOAP12Deser"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' COMUnit 1.1 - TestContainer Class
' tests for SOAP 1.2 deserialization

Option Explicit

' Interface declaration
Implements ITestContainer


' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testSpecExample", "testHeader", "testIdRef", "testIdRefFwd", "testIdRefNoEnc", "testArray", "testBase64")
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

' deserialize an array
Public Sub testArray(ByVal tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.parse getMessage("testSoap12Array")
    
    tr.AssertEqualsString SOAP_12_ENV, e.EnvelopeVersion
    tr.AssertEqualsString "urn:tests", e.URI
    tr.AssertEqualsString "foo", e.MethodName
    Dim n As ISOAPNode3
    Set n = e.Body.Item(0)
    tr.AssertEqualsString SOAP_12_ENC, n.EncodingStyle
    tr.AssertEqualsLong 5, e.Parameters.Count
    
    ' first array, specific arraySize & itemType
    testArrayNode e.Parameters.Item(0), "a", tr
    
    ' second array, uses an unbound size
    testArrayNode e.Parameters.Item(1), "b", tr
    
    ' third array, has no arraySize, should default to unbound array
    testArrayNode e.Parameters.Item(2), "c", tr
    
    ' forth array, has no arraySize or itemType, but has a nodeType
    Set n = e.Parameters.Item(3)
    tr.AssertEqualsString "d", n.Name, "Array Node Name"
    tr.AssertEqualsString "", n.Namespace, "Array Node Namespace"
    tr.AssertEqualsLong vbArray Or vbVariant, VarType(n.Value), "Array Type"
    tr.AssertEqualsLong 2, UBound(n.Value), "Array Size"
    Dim i() As Variant
    i = n.Value
    tr.AssertEqualsLong vbLong, VarType(i(0))
    tr.AssertEqualsLong vbLong, VarType(i(1))
    tr.AssertEqualsLong vbLong, VarType(i(2))
    
    tr.AssertEqualsLong 1, CLng(i(0)), "Array Value 0"
    tr.AssertEqualsLong 2, CLng(i(1)), "Array Value 1"
    tr.AssertEqualsLong 3, CLng(i(2)), "Array Value 2"
    
    ' fifth array, has itemType, array values have no xsi-type
    testArrayNode e.Parameters.Item(4), "e", tr
End Sub

Private Sub testArrayNode(ByVal n As ISOAPNode3, ByVal expectedName As String, ByVal tr As TestResult)
    Dim i() As Long
    tr.AssertEqualsString expectedName, n.Name, "Array Node Name"
    tr.AssertEqualsString "", n.Namespace, "Array Node Namespace"
    tr.AssertEqualsLong vbArray Or vbLong, VarType(n.Value), "Array Type"
    tr.AssertEqualsLong 2, UBound(n.Value), "Array Size"
    i = n.Value
    tr.AssertEqualsLong 1, i(0), "Array Value 0"
    tr.AssertEqualsLong 2, i(1), "Array Value 1"
    tr.AssertEqualsLong 3, i(2), "Array Value 2"
End Sub

' id/refs in the message, but no encoding style, should be ignored
Public Sub testIdRefNoEnc(ByVal tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.parse getMessage("testIdRefNoEnc")
    testIdRefCommon e, tr
    
    tr.AssertEqualsString "Pick up Mary at school at 2pm", e.Parameters.Item(0).Value
    tr.AssertEqualsString "a", e.Parameters.Item(0).id
    tr.AssertEqualsString "a", e.Parameters.Item(1).href
    tr.AssertEqualsString "", e.Parameters.Item(1).Value
End Sub

' simple id/ref test
Public Sub testIdRef(ByVal tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.parse getMessage("testIdRef")
    testIdRefCommon e, tr
    
    tr.AssertEqualsString "a", e.Parameters.Item(0).id
    tr.AssertEqualsString "a", e.Parameters.Item(1).href
    Dim n As ISOAPNode12
    Set n = e.Parameters.Item(1)
    tr.AssertEqualsString "a", n.ref
    tr.AssertEqualsString "Pick up Mary at school at 2pm", e.Parameters.Item(1).Value
End Sub

' id/ref test with fwd lookup
Public Sub testIdRefFwd(ByVal tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.parse getMessage("testIdRefFwd")
    testIdRefCommon e, tr

    tr.AssertEqualsString "a", e.Parameters.Item(0).href
    tr.AssertEqualsString "a", e.Parameters.Item(1).id
    Dim n As ISOAPNode12
    Set n = e.Parameters.Item(0)
    tr.AssertEqualsString "a", n.ref
    tr.AssertEqualsString "Pick up Mary at school at 2pm", e.Parameters.Item(1).Value
End Sub

Private Sub testIdRefCommon(ByVal e As ISOAPEnvelope2, ByVal tr As TestResult)
    tr.AssertEqualsString SOAP_12_ENV, e.EnvelopeVersion
    tr.AssertEqualsString "", e.EncodingStyle
    tr.AssertEqualsString "alert", e.MethodName
    tr.AssertEqualsString "http://example.org/alert", e.URI
    tr.AssertEqualsLong 2, e.Parameters.Count
    tr.AssertEqualsString "msg", e.Parameters.Item(0).Name
    tr.AssertEqualsString e.URI, e.Parameters.Item(0).Namespace
    tr.AssertEqualsString "Pick up Mary at school at 2pm", e.Parameters.Item(0).Value
    tr.AssertEqualsString "msg", e.Parameters.Item(1).Name
    tr.AssertEqualsString e.URI, e.Parameters.Item(1).Namespace
End Sub

Public Sub testHeader(ByVal tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    Dim sf As ISerializerFactoryConfig3
    Set sf = e.SerializerFactory
    sf.understoodHeader "http://example.org/alertcontrol", "alertcontrol"
    e.parse getMessage("testHeader")
    
    tr.AssertEqualsString SOAP_12_ENV, e.EnvelopeVersion
    tr.AssertEqualsString "", e.EncodingStyle
    tr.AssertEqualsLong 1, e.Headers.Count
    Dim h As ISOAPNode3
    Set h = e.Headers.Item(0)
    Dim h12 As ISOAPNode12
    Set h12 = h
    
    tr.AssertEqualsString "alertcontrol", h.Name
    tr.AssertEqualsString "http://example.org/alertcontrol", h.Namespace
    tr.AssertEqualsLong 1, h.Nodes.Count
    tr.Assert h.mustUnderstand, "mustUnderstand"
    tr.Assert h12.relay, "relay"
    tr.AssertEqualsString "urn:role", h12.role
    tr.AssertEqualsString "urn:role", h.actor
    tr.AssertEqualsString "urn:encoding", h.EncodingStyle
    tr.Assert Not h.root, "soap 11 root"
    
End Sub

Public Sub testSpecExample(ByVal tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.parse getMessage("testSpecExample")
    
    tr.AssertEqualsString SOAP_12_ENV, e.EnvelopeVersion
    tr.AssertEqualsString "", e.EncodingStyle
    
    tr.AssertEqualsString "alert", e.MethodName
    tr.AssertEqualsString "http://example.org/alert", e.URI
    tr.AssertEqualsLong 1, e.Parameters.Count
    tr.AssertEqualsString "msg", e.Parameters.Item(0).Name
    tr.AssertEqualsString e.URI, e.Parameters.Item(0).Namespace
    tr.AssertEqualsString "Pick up Mary at school at 2pm", e.Parameters.Item(0).Value
    
    tr.AssertEqualsLong 1, e.Headers.Count
    tr.AssertEqualsString "alertcontrol", e.Headers.Item(0).Name
    tr.AssertEqualsString "http://example.org/alertcontrol", e.Headers.Item(0).Namespace
    Dim h As ISOAPNode3
    Set h = e.Headers.Item(0)
    tr.AssertEqualsLong 2, h.Nodes.Count
    tr.AssertEqualsString "priority", h.Nodes.Item(0).Name
    tr.AssertEqualsString h.Namespace, h.Nodes.Item(0).Namespace
    tr.AssertEqualsString "1", h.Nodes.Item(0).Value
    tr.AssertEqualsString "expires", h.Nodes.Item(1).Name
    tr.AssertEqualsString h.Namespace, h.Nodes.Item(1).Namespace
    tr.AssertEqualsString "2001-06-22T14:00:00-05:00", h.Nodes.Item(1).Value
    
End Sub

Public Sub testBase64(ByVal tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.parse getMessage("testBase64")
    
    tr.AssertEqualsLong 2, e.Parameters.Count, "Wrong number of parameters in message"
    tr.AssertEqualsLong vbArray + vbByte, VarType(e.Parameters.Item(1).Value)
    Dim d() As Byte
    d = e.Parameters.Item(1).Value
    ' array should be 501 bytes long, and contain values of index mod 255
    tr.AssertEqualsLong 501, UBound(d) - LBound(d) + 1, "base64 decoded data incorrect length"
    Dim i As Integer
    For i = LBound(d) To UBound(d)
        tr.AssertEqualsVariant i Mod 255, d(i), "data at offset " & i & " wrong"
    Next
End Sub
