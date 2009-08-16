VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcHeaders"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' COMUnit 1.1 - TestContainer Class
'
' known Header tests

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables
Private m_sf As ISerializerFactoryConfig3

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testMu0", "testMu1", "testKnownHeader", "testNodeDispatch")
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
    Set m_sf = New CoSerializerFactory
    m_sf.understoodHeader "http://www.pocketsoap.com/tests", "id"
End Sub

'Destroy the test fixture
Public Sub ITestContainer_TearDown()
    ' TODO: destruct your test fixture here
End Sub

Public Sub testNodeDispatch(tr As TestResult)
    Dim n As Object
    Set n = New CoSoapNode
    n.role = "urn:role"
    n.mustUnderstand = True
    n.relay = True
    n.Name = "name"
    n.Namespace = "urn:namespace"
    n.Value = "value"
    n.Type = "type"
    n.TypeNS = "urn:type"
    n.id = "id"
    n.ref = "ref"
    
    tr.AssertEqualsString "urn:role", n.actor
    tr.AssertEqualsString "urn:role", n.role
    tr.Assert n.mustUnderstand
    tr.Assert n.relay
    tr.AssertEqualsString "name", n.Name
    tr.AssertEqualsString "urn:namespace", n.Namespace
    tr.AssertEqualsString "value", n.Value
    tr.AssertEqualsString "type", n.Type
    tr.AssertEqualsString "urn:type", n.TypeNS
    tr.AssertEqualsString "id", n.id
    tr.AssertEqualsString "ref", n.ref
    
End Sub

Public Sub testMu0(tr As TestResult)
    Dim e As New CoEnvelope
    Set e.SerializerFactory = m_sf
    e.parse testMsg("0", "0")
    Dim h1 As CoSoapNode, h2 As CoSoapNode
    Set h1 = e.Headers.itemByName("what", "http://simon.fell.com/calc")
    Set h2 = e.Headers.itemByName("id", "http://www.pocketsoap.com/tests")
    
    tr.AssertEqualsString "10", h1.Value
    tr.Assert Not h1.mustUnderstand
    tr.AssertEqualsString "20", h2.Value
    tr.Assert Not h2.mustUnderstand
End Sub

Public Sub testMu1(tr As TestResult)
    Dim e As New CoEnvelope
    Set e.SerializerFactory = m_sf
    On Error Resume Next
    e.parse testMsg("1", "0")
    tr.AssertEqualsLong E_DONT_UNDERSTAND, Err.Number
    On Error GoTo 0
End Sub

Public Sub testKnownHeader(tr As TestResult)
    Dim e As New CoEnvelope
    Set e.SerializerFactory = m_sf
    e.parse testMsg("0", "1")
    Dim h1 As CoSoapNode, h2 As CoSoapNode
    Set h1 = e.Headers.itemByName("what", "http://simon.fell.com/calc")
    Set h2 = e.Headers.itemByName("id", "http://www.pocketsoap.com/tests")
    
    tr.AssertEqualsString "10", h1.Value
    tr.Assert Not h1.mustUnderstand
    tr.AssertEqualsString "20", h2.Value
    tr.Assert h2.mustUnderstand
End Sub

Private Function testMsg(muFlag As String, muFlagKnown As String)
    testMsg = "<S:Envelope " + _
                "S:encodingStyle=""http://schemas.xmlsoap.org/soap/encoding/"" " + _
                "xmlns:Enc=""http://schemas.xmlsoap.org/soap/encoding/"" " + _
                "xmlns:S=""http://schemas.xmlsoap.org/soap/envelope/"" " + _
                "xmlns:a=""http://simon.fell.com/calc"" " + _
                "xmlns:b=""http://www.pocketsoap.com/tests"" " + _
                "xmlns:XS=""http://www.w3.org/2001/XMLSchema"" " + _
                "xmlns:XI=""http://www.w3.org/2001/XMLSchema-instance"">" + _
                "<S:Header><a:what S:mustUnderstand='" + muFlag + "'>10</a:what>" + _
                "<b:id S:mustUnderstand='" + muFlagKnown + "'>20</b:id>" + _
                "</S:Header>" + _
                "<S:Body><a:Add /></S:Body></S:Envelope>"
End Function
