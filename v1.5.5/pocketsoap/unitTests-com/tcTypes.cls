VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcTypes"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/tcTypes.cls,v 1.5 2005/08/18 05:12:47 simon Exp $
'
' local type mappings

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testLocalMapping", "testLocalMappingWithHref", "testXsdLong", "testXsdLongObj")
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

Public Sub testXsdLong(tr As TestResult)
    Dim e As New CoEnvelope
    e.parse getXsdLong(1)
    Const VT_I8 = 20
    tr.AssertEqualsLong VT_I8, VarType(e.Parameters.Item(0).Value), "xsd:long for 1999 schema has wrong mapping"
    tr.AssertEqualsLong VT_I8, VarType(e.Parameters.Item(1).Value), "xsd:long for 2001 schema has wrong mapping"
End Sub

Public Sub testXsdLongObj(tr As TestResult)
    Dim e As New CoEnvelope
    Dim xsdl As New CoXsdLong
    xsdl.Register e.SerializerFactory
    
    ' check when parsing xsd:longs
    e.parse getXsdLong("1")
    tr.AssertEqualsLong vbObject, VarType(e.Parameters.Item(0).Value)
    tr.AssertEqualsLong vbObject, VarType(e.Parameters.Item(1).Value)
    verifyXsdlongObject tr, e.Parameters.Item(0).Value, 1, 0, "1"
    verifyXsdlongObject tr, e.Parameters.Item(1).Value, 1, 0, "1"

    e.parse getXsdLong("4294967297")
    tr.AssertEqualsLong vbObject, VarType(e.Parameters.Item(0).Value)
    tr.AssertEqualsLong vbObject, VarType(e.Parameters.Item(1).Value)
    verifyXsdlongObject tr, e.Parameters.Item(0).Value, 1, 1, "4294967297"
    verifyXsdlongObject tr, e.Parameters.Item(1).Value, 1, 1, "4294967297"
    
    ' check property access
    Set xsdl = New CoXsdLong
    assertXsdLongValues tr, xsdl, 0, 0, "0"
    xsdl.LoDWord = 1
    assertXsdLongValues tr, xsdl, 1, 0, "1"
    xsdl.HiDWord = 1
    assertXsdLongValues tr, xsdl, 1, 1, "4294967297"
    xsdl.String = "4294967298"
    assertXsdLongValues tr, xsdl, 2, 1, "4294967298"
    
    ' check serialization
    xsdl.String = "3"
    e.Parameters.Clear
    e.Parameters.Create "myval", xsdl
    e.SetMethod "foo", "urn:bar"
    Dim env As String
    env = e.Serialize
    tr.Assert InStr(env, "XS:long") > 0
    
    e.parse env
    tr.AssertEqualsLong vbObject, VarType(e.Parameters.Item(0).Value)
    verifyXsdlongObject tr, e.Parameters.Item(0).Value, 3, 0, "3"
    
    
End Sub

Private Sub verifyXsdlongObject(tr As TestResult, o As Object, lo As Long, hi As Long, str As String)
    Dim xsdl As CoXsdLong
    tr.Assert TypeOf o Is CoXsdLong
    Set xsdl = o
    assertXsdLongValues tr, xsdl, lo, hi, str
End Sub

Private Sub assertXsdLongValues(tr As TestResult, xsdl As CoXsdLong, lo As Long, hi As Long, str As String)
    tr.AssertEqualsLong lo, xsdl.LoDWord
    tr.AssertEqualsLong hi, xsdl.HiDWord
    tr.AssertEqualsString str, xsdl.String
End Sub

Public Sub testLocalMapping(oTestResult As TestResult)
    Dim e As New CoEnvelope
    Dim sf As ISerializerFactoryConfig2
    Set sf = e.SerializerFactory
    sf.ElementMapping "getCurrentDate", "http://tempuri.org/DateService", "getCurrentDate", "http://tempuri.org/DateService"
    sf.ElementMapping "getCurrentDateResponse", "http://tempuri.org/DateService", "getCurrentDateResponse", "http://tempuri.org/DateService"
    sf.LocalTypeMapping "getCurrentDateResponse", "http://tempuri.org/DateService", "Result", "", "dateTime", "http://www.w3.org/2001/XMLSchema"
    e.parse getDateTime
    oTestResult.AssertEqualsLong e.Parameters.Count, 1
    oTestResult.AssertEqualsLong VarType(e.Parameters.Item(0).Value), vbDate
    oTestResult.AssertEqualsVariant e.Parameters.Item(0).Value, CDate("2 jan 2003 10:56:37")
End Sub

Public Sub testLocalMappingWithHref(oTestResult As TestResult)
    ' TODO: add your test code here
    Dim e As New CoEnvelope
    Dim sf As ISerializerFactoryConfig2
    Set sf = e.SerializerFactory
    sf.ElementMapping "getCurrentDate", "http://tempuri.org/DateService", "getCurrentDate", "http://tempuri.org/DateService"
    sf.ElementMapping "getCurrentDateResponse", "http://tempuri.org/DateService", "getCurrentDateResponse", "http://tempuri.org/DateService"
    sf.LocalTypeMapping "getCurrentDateResponse", "http://tempuri.org/DateService", "Result", "", "dateTime", "http://www.w3.org/2001/XMLSchema"
    e.parse getDateTimeHref
    oTestResult.AssertEqualsLong e.Parameters.Count, 1
    oTestResult.AssertEqualsLong VarType(e.Parameters.Item(0).Value), vbDate
    oTestResult.AssertEqualsVariant e.Parameters.Item(0).Value, CDate("2 jan 2003 10:56:37")
End Sub

Private Function getDateTimeHref() As String
    getDateTimeHref = "<soap:Envelope xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' " + _
               "xmlns:xsd='http://www.w3.org/2001/XMLSchema' " + _
               "xmlns:soap='http://schemas.xmlsoap.org/soap/envelope/' " + _
               "xmlns:soapenc='http://schemas.xmlsoap.org/soap/encoding/' " + _
               "soap:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'>" + _
            "<soap:Body> " + _
            "<n:getCurrentDateResponse xmlns:n='http://tempuri.org/DateService'>" + _
            "<Result href='#id0' />" + _
            "</n:getCurrentDateResponse>" + _
            "<id0 id='id0' soapenc:root='0'>2003-01-02T10:56:37.000Z</id0>" + _
            "</soap:Body>" + _
            "</soap:Envelope>"
End Function

Private Function getDateTime() As String
    getDateTime = "<soap:Envelope xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' " + _
               "xmlns:xsd='http://www.w3.org/2001/XMLSchema' " + _
               "xmlns:soap='http://schemas.xmlsoap.org/soap/envelope/' " + _
               "xmlns:soapenc='http://schemas.xmlsoap.org/soap/encoding/' " + _
               "soap:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'>" + _
            "<soap:Body> " + _
            "<n:getCurrentDateResponse xmlns:n='http://tempuri.org/DateService'>" + _
            "<Result>2003-01-02T10:56:37.000Z</Result>" + _
            "</n:getCurrentDateResponse>" + _
            "</soap:Body>" + _
            "</soap:Envelope>"
End Function

Private Function getXsdLong(v As String) As String
    getXsdLong = "<soap:Envelope  " + _
                "xmlns:xsi01='http://www.w3.org/2001/XMLSchema-instance' " + _
                "xmlns:xsd01='http://www.w3.org/2001/XMLSchema' " + _
                "xmlns:xsi99='http://www.w3.org/1999/XMLSchema-instance' " + _
                "xmlns:xsd99='http://www.w3.org/1999/XMLSchema' " + _
                "xmlns:soap='http://schemas.xmlsoap.org/soap/envelope/' " + _
                "xmlns:soapenc='http://schemas.xmlsoap.org/soap/encoding/' " + _
                "soap:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'>" + _
                "<soap:Body> " + _
                "<n:longs xmlns:n='http://tempuri.org/DateService'>" + _
                "<one xsi99:type='xsd99:long'>" + v + "</one>" + _
                "<two xsi01:type='xsd01:long'>" + v + "</two>" + _
                "</n:longs>" + _
                "</soap:Body>" + _
                "</soap:Envelope>"
End Function

