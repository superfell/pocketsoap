VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "tcQName"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/tcQName.cls,v 1.2 2003/05/17 23:30:36 simon Exp $
'
' XMLQname tests

Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testSet", "testProps")
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

Public Sub testSet(tr As TestResult)
    Dim qn As Object
    Set qn = CreateObject("pocketSOAP.QName")
    Const ln = "localElementName"
    Const ns = "http://tests.pocketsoap.com/"
    qn.Set ln, ns
    
    tr.AssertEqualsString ln, qn.Name, "localname doesn't match"
    tr.AssertEqualsString ns, qn.Namespace, "namespace doesn't match"
End Sub

Public Sub testProps(tr As TestResult)
    Dim qn As Object
    Set qn = CreateObject("pocketSOAP.QName")
    Const ln = "localElementName"
    Const ns = "http://tests.pocketsoap.com/"
    
    tr.AssertEqualsString "", qn.Name
    tr.AssertEqualsString "", qn.Namespace
    
    qn.Name = ln
    tr.AssertEqualsString ln, qn.Name
    tr.AssertEqualsString "", qn.Namespace
    
    qn.Namespace = ns
    tr.AssertEqualsString ln, qn.Name
    tr.AssertEqualsString ns, qn.Namespace
    
    Set qn = CreateObject("pocketSOAP.QName")
    tr.AssertEqualsString "", qn.Name
    tr.AssertEqualsString "", qn.Namespace
    
    qn.Namespace = ns
    tr.AssertEqualsString "", qn.Name
    tr.AssertEqualsString ns, qn.Namespace
    
    qn.Name = ln
    tr.AssertEqualsString ln, qn.Name
    tr.AssertEqualsString ns, qn.Namespace
End Sub
