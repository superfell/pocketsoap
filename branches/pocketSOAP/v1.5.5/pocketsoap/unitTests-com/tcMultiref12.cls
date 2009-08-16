VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcMultiref12"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/tcMultiref12.cls,v 1.1 2003/06/15 01:06:59 simon Exp $
'
' tests for the SOAP 1.2 graph encoding

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testSerialize")
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

Public Sub testSerialize(ByVal tr As TestResult)
    Dim e As ISOAPEnvelope2
    Set e = New CoEnvelope
    e.EnvelopeVersion = SOAP_12_ENV
    e.SetMethod "foo", "urn:tests"
    Dim n As ISOAPNode2
    Set n = New CoSoapNode
    n.EncodingStyle = SOAP_12_ENC
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
    
    Dim href As String
    href = bill.Attributes.getQualifiedItem("id", "").Text
    tr.Assert bill.childNodes.length > 0
    tr.AssertEqualsString href, ship.Attributes.getQualifiedItem("ref", "").Text
    tr.AssertEqualsLong 0, ship.childNodes.length
    
    
End Sub
