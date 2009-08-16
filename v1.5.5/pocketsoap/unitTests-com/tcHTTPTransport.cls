VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcHTTPTransport"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' COMUnit 1.1 - TestContainer Class
'
' HTTP transport related tests

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables
Private test_url As String
Private test_url_ssl As String
Private test_url_auth As String

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testDirect", "testDirectAuth2", "testDirectAuth", "testHttpProxy", "testHttpsProxy", "testHttpProxyServerAuth", "testHttpProxyBothAuth", "testReuseObject", "testTracing")
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
    test_url = TEST_SERVER_HTTP + "echo.asp"
    test_url_ssl = TEST_SERVER_HTTPS + "echo.asp"
    test_url_auth = TEST_SERVER_HTTP + "echo_auth.asp"
End Sub

'Destroy the test fixture
Public Sub ITestContainer_TearDown()
    ' TODO: destruct your test fixture here
End Sub

Public Sub testTracing(ByVal tr As TestResult)
    Dim h As PocketSOAP.IHTTPTransportAdv2
    Set h = New PocketSOAP.HTTPTransport
    h.Option("tracing.file") = TRACE_FILE
    h.Option("compression.accept") = False
    Dim req() As Byte
    req = getMessage("invalid")
    
    Dim e As New CoEnvelope
    Dim st As ISOAPTransport2
    Set st = h
    st.send TEST_SERVER_HTTP + "gzipecho.ashx", req
    On Error GoTo bang
    e.parse h
    tr.AddFailure "call to Parse should of failed on invalid XML"
    Exit Sub
    
bang:
    ' todo, somehow check the log file is correct
End Sub

Public Sub testReuseObject(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    Dim f As New CoEnvelope
    e.SetMethod "echoString", "http://soapinterop.org/"
    e.Parameters.Create "inputString", "hello"
    Dim t As New HTTPTransport
    t.SOAPAction = "http://soapinterop.org/"
    t.setProxy PROXY_SERVER, PROXY_PORT
    t.send test_url, e.Serialize
    f.parse t
    tr.AssertEqualsString "hello", e.Parameters.Item(0).Value
    t.send test_url, e.Serialize
    f.parse t
    tr.AssertEqualsString "hello", e.Parameters.Item(0).Value
End Sub

Public Sub testDirect(ByVal tr As TestResult)
    Const msgLen = 20000
    Dim e As Object
    Set e = getEnv(msgLen)
    Dim t As Object
    Set t = CreateObject("PocketSOAP.HTTPTransport.2")
    t.send test_url, e.Serialize
    e.parse t
    tr.AssertEqualsLong 200, t.StatusCode, "last HTTP status code"
    Dim swa As ISwATransport
    Set swa = t
    tr.Assert InStr(1, swa.contentType, "text/xml", vbTextCompare) > 0, "content-type header"
    checkResEnv tr, e, msgLen
End Sub

Public Sub testDirectAuth(ByVal tr As TestResult)
    Const msgLen = 5
    Dim e As Object
    Set e = getEnv(msgLen)
    Dim t As Object
    Set t = CreateObject("PocketSOAP.HTTPTransport.2")
    t.Authentication HTTP_AUTH_USER, HTTP_AUTH_PWD
    t.send test_url_auth, e.Serialize
    e.parse t
    tr.AssertEqualsLong 200, t.StatusCode, "last HTTP status code"
    Dim swa As ISwATransport
    Set swa = t
    tr.Assert InStr(1, swa.contentType, "text/xml", vbTextCompare) > 0, "content-type header"
    checkResEnv tr, e, msgLen
End Sub

Public Sub testDirectAuth2(ByVal tr As TestResult)
    Const msgLen = 5
    Dim e As Object
    Set e = getEnv(msgLen)
    Dim t As Object
    Set t = CreateObject("PocketSOAP.HTTPTransport.2")
    On Error Resume Next
    t.send test_url_auth, e.Serialize
    e.parse t
    tr.Assert Err.Number <> 0, "should get authentication failure"
    On Error GoTo 0
End Sub

Public Sub testHttpProxy(ByVal tr As TestResult)
    Const msgLen = 20000
    Dim e As Object
    Set e = getEnv(msgLen)
    Dim t As Object
    Set t = CreateObject("PocketSOAP.HTTPTransport.2")
    t.setProxy PROXY_SERVER, PROXY_PORT
    t.send test_url, e.Serialize
    e.parse t
    tr.AssertEqualsLong 200, t.StatusCode, "last HTTP status code"
    Dim swa As ISwATransport
    Set swa = t
    tr.Assert InStr(1, swa.contentType, "text/xml", vbTextCompare) > 0, "content-type header"
    checkResEnv tr, e, msgLen
End Sub

Public Sub testHttpsProxy(ByVal tr As TestResult)
    Const msgLen = 10000
    Dim e As Object
    Set e = getEnv(msgLen)
    Dim t As Object
    Set t = CreateObject("PocketSOAP.HTTPTransport.2")
    t.setProxy PROXY_SERVER, PROXY_PORT
    t.send test_url_ssl, e.Serialize
    e.parse t
    tr.AssertEqualsLong 200, t.StatusCode, "last HTTP status code"
    Dim swa As ISwATransport
    Set swa = t
    tr.Assert InStr(1, swa.contentType, "text/xml", vbTextCompare) > 0, "content-type header"
    checkResEnv tr, e, msgLen
End Sub

Public Sub testHttpProxyServerAuth(ByVal tr As TestResult)
    Const msgLen = 20000
    Dim e As Object
    Set e = getEnv(msgLen)
    Dim t As Object
    Set t = CreateObject("PocketSOAP.HTTPTransport.2")
    t.setProxy PROXY_SERVER, PROXY_PORT
    t.Authentication HTTP_AUTH_USER, HTTP_AUTH_PWD
    t.send test_url_auth, e.Serialize
    e.parse t
    tr.AssertEqualsLong 200, t.StatusCode, "last HTTP status code"
    Dim swa As ISwATransport
    Set swa = t
    tr.Assert InStr(1, swa.contentType, "text/xml", vbTextCompare) > 0, "content-type header"
    checkResEnv tr, e, msgLen
End Sub

Public Sub testHttpProxyBothAuth(ByVal tr As TestResult)
    Const msgLen = 20000
    Dim e As Object
    Set e = getEnv(msgLen)
    Dim t As Object
    Set t = CreateObject("PocketSOAP.HTTPTransport.2")
    t.setProxy PROXY_SERVER, PROXY_PORT
    t.Authentication HTTP_AUTH_USER, HTTP_AUTH_PWD
    t.ProxyAuthentication PROXY_AUTH_USER, PROXY_AUTH_PWD
    t.send test_url_auth, e.Serialize
    e.parse t
    tr.AssertEqualsLong 200, t.StatusCode, "last HTTP status code"
    Dim swa As ISwATransport
    Set swa = t
    tr.Assert InStr(1, swa.contentType, "text/xml", vbTextCompare) > 0, "content-type header"
    checkResEnv tr, e, msgLen
End Sub

Private Sub checkResEnv(ByVal tr As TestResult, e As Object, ByVal msgLen As Long)
    tr.AssertEqualsString String(msgLen, "x"), e.Parameters.Item(0).Value, "wrong string returned"
End Sub

Private Function getEnv(ByVal msgLen As Long) As Object
    Dim e As Object
    Set e = CreateObject("pocketSOAP.Envelope.2")
    e.SetMethod "echoString", "http://soapinterop.org/"
    e.Parameters.Create "inputString", String(msgLen, "x")
    Set getEnv = e
End Function

