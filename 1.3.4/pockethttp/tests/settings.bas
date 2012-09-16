Attribute VB_Name = "Module1"
Option Explicit

Public Const TEST_SERVER = "http://localhost/ph-tests/"
Public Const PROXY_HOST = "localhost"
Public Const PROXY_PORT = 5049

Public Const ECHO_TEST_URL = TEST_SERVER + "echo.aspx"
Public Const HEADERS_TEST_URL = TEST_SERVER + "headers.aspx"
Public Const TIMEOUT_QS = "to"

Public Const TRACE_FILE = "c:\temp\phttp.log"

Public Function TIMEOUT_TEST_URL(ByVal timeoutInMs As Long) As String
    TIMEOUT_TEST_URL = ECHO_TEST_URL + "?" + TIMEOUT_QS + "=" & (timeoutInMs / 1000)
End Function

