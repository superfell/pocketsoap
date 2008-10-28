VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3090
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3090
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.ListBox List1 
      Height          =   2205
      Left            =   120
      TabIndex        =   1
      Top             =   720
      Width           =   4455
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Echo Struct Array"
      Height          =   495
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   1455
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

' This sample shows how to manually handle arrays of complex types using PocketSOAP
' alternatively you could use the WSDL wizard to generate a proxy class, including
' a class that strongly types the SOAPStruct object for you
' the WSDL for this service is available at http://www.pocketsoap.com/services/ilab.wsdl
'

Private Sub Command1_Click()
    ' create the soap envelope and set the methodName
    Dim e As New CoEnvelope
    e.SetMethod "echoStructArray", "http://soapinterop.org/"
    
    ' create the array, and populate the array items
    Dim arr(2) As Object
    Set arr(0) = makeSoapStruct("String one", 1, 1.1)
    Set arr(1) = makeSoapStruct("String two", 2, 2.2)
    Set arr(2) = makeSoapStruct("String three", 3, 3.3)
    ' create the parameter
    e.Parameters.Create "inputStructArray", arr
    
    ' call the service
    Dim http As New HTTPTransport
    http.SOAPAction = "http://soapinterop.org/"
    http.Send "http://soap.4s4c.com/ilab/soap.asp", e.Serialize
    
    ' parse the response
    e.Parse http
    
    'get the returned array
    Dim res()
    res = e.Parameters.Item(0).Value
    ' iterate over it
    Dim idx As Integer
    Dim n As CoSoapNode
    For idx = LBound(res) To UBound(res)
        ' each item in the array is a soapnode, with child nodes for varString, varInt, varFloat
        Set n = res(idx)
        List1.AddItem n.Nodes.ItemByName("varString").Value & " " & n.Nodes.ItemByName("varInt").Value & " " & n.Nodes.ItemByName("varFloat").Value
    Next
End Sub

' helper function to create a SOAPStruct complex type
Private Function makeSoapStruct(varString As String, varInt As Long, varFloat As Single) As CoSoapNode
    Dim ct As New CoSoapNode
    ct.Nodes.Create "varString", varString
    ct.Nodes.Create "varInt", varInt
    ct.Nodes.Create "varFloat", varFloat
    Set makeSoapStruct = ct
End Function
