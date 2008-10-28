VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcEchotests"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/tcEchotests.cls,v 1.4 2004/10/15 04:14:10 simon Exp $
'
' some payload size based tests run against the echo service
'

Option Explicit

Const ECHO_TEST = "echo.asp"
Const ECHO_GZIP = "gzipecho.ashx"

' Interface declaration
Implements ITestContainer

Private http_url As String
Private https_url As String

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testEchoUnicode", "testEchoLargeString", "testEchoStringSSL", "testEchoLargeStringSSL", "testGzip", "testDeflate")
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
    http_url = TEST_SERVER_HTTP + ECHO_TEST
    https_url = TEST_SERVER_HTTPS + ECHO_TEST
End Sub

'Destroy the test fixture
Public Sub ITestContainer_TearDown()
    ' TODO: destruct your test fixture here
End Sub

Public Sub testEchoUnicode(ByVal tr As TestResult)
    Dim s As String, i As Long, r As String
    For i = 200 To 300
        s = s & ChrW(i)
    Next
    r = echoString(http_url, s)
    tr.AssertEqualsString s, r, "unicode string mismatch"
End Sub

Public Sub testEchoLargeString(ByVal tr As TestResult)
    Dim s As String, i As Long, r As String
    s = String(66000, "q")
    r = echoString(http_url, s)
    tr.AssertEqualsString s, r, "long string"
End Sub

Public Sub testEchoStringSSL(ByVal tr As TestResult)
    Dim s As String, i As Long, r As String
    s = "Hello world over SSL"
    r = echoString(https_url, s)
    tr.AssertEqualsString s, r, "long string"
End Sub

Public Sub testEchoLargeStringSSL(ByVal tr As TestResult)
    Dim s As String, i As Long, r As String
    s = String(19000, "q")
    r = echoString(https_url, s)
    tr.AssertEqualsString s, r, "long string"
End Sub

Public Sub testGzip(ByVal tr As TestResult)
    Dim s As String, r As String
    s = String(133000, "c")
    r = echoString(TEST_SERVER_HTTP + ECHO_GZIP, s)
    tr.AssertEqualsString s, r, "compressed string"
End Sub

Public Sub testDeflate(ByVal tr As TestResult)
    Dim s As String, r As String
    s = String(133000, "c")
    r = echoString(TEST_SERVER_HTTP + ECHO_GZIP, s)
    tr.AssertEqualsString s, r, "compressed string"
End Sub

Private Function echoString(ByVal sURL As String, ByVal s As String) As String
    Dim t As IHTTPTransportAdv2
    Set t = CreateObject("pocketSOAP.hTTPTransport.2")
    echoString = echoStringWT(sURL, s, t)
End Function

Private Function echoStringWT(ByVal sURL As String, ByVal s As String, ByVal t As IHTTPTransportAdv2) As String
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.SetMethod "echoString", "http://soapinterop.org/"

    e.Parameters.Create "inputString", s
    t.SOAPAction = "http://soapinterop.org/"
    t.send sURL, e.Serialize

    e.parse t
    echoStringWT = e.Parameters.Item(0).Value
End Function

