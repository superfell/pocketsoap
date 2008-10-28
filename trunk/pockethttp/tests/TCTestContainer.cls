VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "BasicTests"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pockethttp/tests/TCTestContainer.cls,v 1.8 2005/03/10 05:22:47 simon Exp $
' COMUnit 1.1 - TestContainer Class

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables
' TODO: specify your TestContainer test fixture member variables here

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ' TODO: add the names of your test methods as a parameter into the Array() function
    ITestContainer_TestCaseNames = Array("test_proxy_resolution", "test_utf8_bytes", "test_utf8_string", "test_long_utf8_string", "test_utf16_string", "test_ascii_string", "test_no_charset", "test_short_to", "test_long_to", "test_tracing_option")
End Property

' Run the specified test case methods in this test container
Public Sub ITestContainer_RunTestCase(oTestCase As ITestCase, oTestResult As TestResult)
    On Error GoTo ErrorHandler
    InvokeHook Me, oTestCase.Name, INVOKE_FUNC, oTestResult
    ' CallByName Me, oTestCase.Name, VbMethod, oTestResult
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

' tests that when going through a proxy, it doesn't do local name resolution
Public Sub test_proxy_resolution(tr As TestResult)
    Dim req As New PocketHTTP.CoPocketHTTP
    req.SetProxy "localhost", 5049
    Dim res As PocketHTTP.IHttpResponse
    On Error Resume Next
    Set res = req.GetResponse("http://unknown.pocketsoap.com", "")
    tr.Assert Err.Number <> -2147013892, "Got unexpected failed to resolve address error"
End Sub

Public Sub test_no_charset(tr As TestResult)
    testEchoAscii tr, "", "hellow world from PocketHTTP."
End Sub

Public Sub test_ascii_string(tr As TestResult)
    testEchoAscii tr, "us-ascii", "hellow world from PocketHTTP."
End Sub

Private Sub testEchoAscii(tr As TestResult, charSetToSend As String, stringToEcho As String)
    Dim b() As Byte
    ReDim b(Len(stringToEcho) - 1)
    Dim i As Integer
    For i = 1 To Len(stringToEcho)
        b(i - 1) = Asc(Mid$(stringToEcho, i, 1))
    Next
    Dim h As New CoPocketHTTP
    Dim res As IHttpResponse
    Dim hdr As PocketHTTP.IHeader
    h.Method = "POST"
    Set hdr = h.Headers.Create("Content-Type", "text/plain")
    If (charSetToSend <> "") Then
        hdr.Attribute("charset") = charSetToSend
    End If
    Set res = h.GetResponse(ECHO_TEST_URL, b)
    tr.AssertEqualsString stringToEcho, res.String, "ascii string not correct"
End Sub

Public Sub test_utf8_string(tr As TestResult)
    Dim str As String
    str = ChrW(&H431) + ChrW(&H424) + ChrW(&H437)
    testEchoString tr, str
End Sub

Public Sub test_long_utf8_string(tr As TestResult)
    testEchoString tr, getLongTestString
End Sub

Private Function getLongTestString() As String
    Dim str As String
    str = ChrW(&H2190) + ChrW(&H2191) + ChrW(&H2192)
    While Len(str) < 33000
        str = str + str
    Wend
    getLongTestString = str
End Function

Public Sub test_utf16_string(tr As TestResult)
    Dim str As String
    str = getLongTestString
    Dim b() As Byte
    ReDim b(Len(str) * 2 - 1)
    Dim i As Long
    For i = 1 To Len(str)
        b((i - 1) * 2) = AscW(Mid$(str, i, 1)) Mod 256
        b((i - 1) * 2 + 1) = AscW(Mid$(str, i, 1)) \ 256
    Next
    Dim h As New CoPocketHTTP
    Dim res As IHttpResponse
    h.Method = "POST"
    h.Headers.Create "Content-Type", "text/plain; charset=utf16"
    Set res = h.GetResponse(ECHO_TEST_URL, b)
    tr.AssertEqualsString str, res.String, "utf16 string response is wrong"
End Sub

Private Sub testEchoString(tr As TestResult, str As String)
    Dim h As New CoPocketHTTP
    Dim res As IHttpResponse
    h.Method = "POST"
    h.Headers.Create "Content-Type", "text/plain; charset=utf8"
    Set res = h.GetResponse(ECHO_TEST_URL, str)
    tr.AssertEqualsString str, res.String, "utf8 encoded response string is not correct"
End Sub

Public Sub test_utf8_bytes(tr As TestResult)
    Dim h As New CoPocketHTTP
    Dim res As IHttpResponse
    ' some cryillic characters encoded into utf8
    ' 0x0431, 0x0424, 0x0437
    Dim UTF8(5) As Byte
    UTF8(0) = &HD0
    UTF8(1) = &HB1
    UTF8(2) = &HD0
    UTF8(3) = &HA4
    UTF8(4) = &HD0
    UTF8(5) = &HB7
    
    h.Method = "POST"
    h.Headers.Create "Content-Type", "text/plain; charset=utf8"
    Set res = h.GetResponse(ECHO_TEST_URL, UTF8)
    
    Dim rb() As Byte
    Dim stm As PocketHTTP.IReadableStreamVB
    Set stm = res.Stream
    rb = stm.ReadBytes(6)
    
    assertArraysEqual tr, UTF8, rb, "utf8 response bytes"
End Sub

Public Sub test_tracing_option(tr As TestResult)
    Dim h As New CoPocketHTTP
    h.Option("tracing.file") = TRACE_FILE
    tr.AssertEqualsString TRACE_FILE, h.Option("tracing.file")
    h.Method = "POST"
    h.Headers.Create "Content-Type", "text/plain;"
    h.Option("compression.enabled") = True
    Dim res As IHttpResponse
    Set res = h.GetResponse(ECHO_TEST_URL, "Some Test String")
    tr.AssertEqualsString "Some Test String", res.String
    tr.AssertEqualsLong 200, res.StatusCode
    Set res = h.GetResponse(ECHO_TEST_URL, "Some Test String")
    tr.AssertEqualsString "Some Test String", res.String
    tr.AssertEqualsLong 200, res.StatusCode
    Set res = h.GetResponse(ECHO_TEST_URL, "Some Test String")
    tr.AssertEqualsString "Some Test String", res.String
    tr.AssertEqualsLong 200, res.StatusCode
End Sub

Public Sub test_short_to(tr As TestResult)
    test_to tr, 5000
End Sub

Public Sub test_long_to(tr As TestResult)
    test_to tr, 45000
End Sub

Private Sub test_to(tr As TestResult, ByVal timeoutValue As Long)
    Const test_string = "Some String"
    Dim h As New CoPocketHTTP
    Dim res As IHttpResponse
    h.Timeout = timeoutValue
    h.Method = "POST"
    h.Headers.Create "Content-Type", "text/plain; charset=utf8"
    Set res = h.GetResponse(TIMEOUT_TEST_URL(timeoutValue - 1000), test_string)
    tr.AssertEqualsString test_string, res.String
    
    On Error GoTo timedout
    Set res = h.GetResponse(TIMEOUT_TEST_URL(timeoutValue + 1000), test_string)
    tr.AddFailure "should of gotten a timeout"
    Exit Sub
    
timedout:
    tr.AssertEqualsLong &H8007274C, Err.Number
End Sub

Private Sub assertArraysEqual(tr As TestResult, ByRef aExp, ByRef aAct, msg As String)
    tr.AssertEqualsLong UBound(aExp), UBound(aAct), msg + " array sizes different"
    Dim b1 As Byte, b2 As Byte
    Dim i As Integer
    For i = 0 To UBound(aExp)
        b1 = aExp(i)
        b2 = aAct(i)
        tr.AssertEqualsVariant b1, b2, " arrays different at offset " & i
    Next
End Sub
