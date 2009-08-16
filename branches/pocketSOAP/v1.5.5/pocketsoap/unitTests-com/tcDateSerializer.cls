VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcDateSerializer"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/tcDateSerializer.cls,v 1.2 2003/05/17 23:30:36 simon Exp $
'
' tests for the xsd:dateTime (de)serialization
'

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables
' TODO: specify your TestContainer test fixture member variables here

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ' TODO: add the names of your test methods as a parameter into the Array() function
    ITestContainer_TestCaseNames = Array("serDateTime", "serDate", "serTime", "deserTime")
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

Public Sub serDateTime(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.SetMethod "foo", ""
    e.Parameters.Create "thedate", CDate("30 May 2002 10:05 PM")
    
    Dim d As New MSXML2.DOMDocument40
    d.async = False
    d.validateOnParse = False
    tr.Assert d.loadXML(e.Serialize)
    Dim n As MSXML2.IXMLDOMNode
    Set n = d.selectSingleNode("//thedate")
    tr.AssertEqualsString "XS:dateTime", n.Attributes.getQualifiedItem("type", e.SerializerFactory.PrimarySchema + "-instance").Text
    tr.AssertEqualsString "2002-05-30T22:05:00Z", n.Text
End Sub

Public Sub serDate(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.SetMethod "foo", ""
    Dim s As ISerializerFactoryConfig2
    Set s = e.SerializerFactory
    s.ElementMapping "foo", "", "foo", ""
    s.LocalTypeMapping "foo", "", "thedate", "", "date", s.PrimarySchema
    e.Parameters.Create "thedate", CDate("30 may 2002")
    
    Dim d As New DOMDocument40
    d.async = False
    d.validateOnParse = False
    tr.Assert d.loadXML(e.Serialize)
    Dim n As IXMLDOMNode
    Set n = d.selectSingleNode("//thedate")
    tr.AssertEqualsString "XS:date", n.Attributes.getQualifiedItem("type", e.SerializerFactory.PrimarySchema + "-instance").Text
    tr.AssertEqualsString "2002-05-30", n.Text
End Sub


Public Sub serTime(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.SetMethod "foo", ""
    Dim s As ISerializerFactoryConfig2
    Set s = e.SerializerFactory
    s.ElementMapping "foo", "", "foo", ""
    s.LocalTypeMapping "foo", "", "thedate", "", "time", s.PrimarySchema
    e.Parameters.Create "thedate", CDate("5:34:59 PM")
    
    Dim d As New DOMDocument40
    d.async = False
    d.validateOnParse = False
    tr.Assert d.loadXML(e.Serialize)
    Dim n As IXMLDOMNode
    Set n = d.selectSingleNode("//thedate")
    tr.AssertEqualsString "XS:time", n.Attributes.getQualifiedItem("type", e.SerializerFactory.PrimarySchema + "-instance").Text
    tr.AssertEqualsString "17:34:59Z", n.Text
End Sub


Public Sub deserTime(ByVal tr As TestResult)
    Dim e As New CoEnvelope
    e.parse getTimeEnv
    tr.Assert DateDiff("s", CDate("22:50:30"), e.Parameters.itemByName("t").Value) = 0
    tr.Assert DateDiff("s", CDate("June 29 2002"), e.Parameters.itemByName("d").Value) = 0
    tr.Assert DateDiff("s", CDate("May 22 2002 10:59:59 AM"), e.Parameters.itemByName("dt").Value) = 0
    
End Sub


Private Function getTimeEnv() As String
    getTimeEnv = "<SOAP-ENV:Envelope SOAP-ENV:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/' " + _
                    "xmlns:xsd='http://www.w3.org/2001/XMLSchema' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' " + _
                    "xmlns:SOAP-ENV='http://schemas.xmlsoap.org/soap/envelope/'>" + _
                    "<SOAP-ENV:Body><m:GotStruct xmlns:m='http://examples.pocketsoap.com/'>" + _
                    "<t xsi:type='xsd:time'>22:50:30Z</t>" + _
                    "<d xsi:type='xsd:date'>2002-06-29</d>" + _
                    "<dt xsi:type='xsd:dateTime'>2002-05-22T10:59:59Z</dt>" + _
                    "</m:GotStruct></SOAP-ENV:Body></SOAP-ENV:Envelope>"
End Function

