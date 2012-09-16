VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "compressionTests"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' COMUnit 1.1 - TestContainer Class

Option Explicit

' Interface declaration
Implements ITestContainer

Private Const SMALL_TEST_DATA = "Hello World"

' Fixture Member Variables
' TODO: specify your TestContainer test fixture member variables here

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ' TODO: add the names of your test methods as a parameter into the Array() function
    ITestContainer_TestCaseNames = Array("test_response_gzip", "test_gzip_post", "test_no_gzip")
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


Public Sub test_response_gzip(oTestResult As TestResult)
    Dim t As New PocketHTTP.CoPocketHTTP
    Dim res As PocketHTTP.IHttpResponse
    t.Method = "POST"
    Set res = t.GetResponse(ECHO_TEST_URL, SMALL_TEST_DATA)
    oTestResult.AssertEqualsString SMALL_TEST_DATA, res.String, "unexpected response data"
    oTestResult.AssertEqualsString "gzip", res.Headers.Find("Content-Encoding").Value, "response was not compressed and should be"
End Sub

Public Sub test_gzip_post(oTR As TestResult)
    Dim t As New CoPocketHTTP
    Dim res As IHttpResponse
    t.Method = "POST"
    t.Option("compression.enabled") = True ' enable outbound compression
    Set res = t.GetResponse(HEADERS_TEST_URL, SMALL_TEST_DATA)
    Dim d As New DOMDocument40
    d.async = False
    d.validateOnParse = False
    d.loadXML res.String
    oTR.AssertEqualsString "deflate", d.selectSingleNode("//Content-Encoding").Text
    
    t.Option("compression.method") = "gzip"
    Set res = t.GetResponse(HEADERS_TEST_URL, SMALL_TEST_DATA)
    d.loadXML res.String
    oTR.AssertEqualsString "gzip", d.selectSingleNode("//Content-Encoding").Text
    
    Set res = t.GetResponse(ECHO_TEST_URL, SMALL_TEST_DATA)
    oTR.AssertEqualsString SMALL_TEST_DATA, res.String, "response data not echo'd correctly"
End Sub

Public Sub test_no_gzip(oTR As TestResult)
    Dim t As New CoPocketHTTP
    Dim res As IHttpResponse
    t.Option("compression.accept") = False
    Set res = t.GetResponse(HEADERS_TEST_URL, "")
    On Error GoTo done
    res.Headers.Find ("Content-Encoding")
    oTR.AddFailure "Found a Content-Encoding and we shouldn't get one"
    Exit Sub
done:
End Sub
