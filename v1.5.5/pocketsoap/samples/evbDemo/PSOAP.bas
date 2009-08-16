Attribute VB_Name = "basPSOAP"
'******************************************************************************
' Implements the Pocket SOAP declarations and functions
'******************************************************************************
' FileName:  PSOAP.bas
' Creator:   Christian Forsberg
' Created:   2001-03-27
'******************************************************************************
' Version   Date   Who Comment
' 00.00.000 010327 CFO Created
' 00.01.000 011112 SZF Modified for PocketSOAP 1.1
'******************************************************************************
Option Explicit

Private pEnvelope As PocketSOAP.CoEnvelope
Private pHTTP As PocketSOAP.HTTPTransport

Public Sub SOAPInit()
  
' Create Pocket SOAP objects.
' Known bugs:
' Version   Date   Who Comment
' 00.00.000 010327 CFO Created
'******************************************************************************
  
  Dim retVal As Integer
  
  Set pEnvelope = CreateObject("PocketSOAP.Envelope.2")
  Set pHTTP = CreateObject("PocketSOAP.HTTPTransport.2")
  
End Sub
Public Function SOAPCall(ByVal Listener As String, ByVal MethodName As String, _
                         ByVal Arguments As Variant, ByVal ShowXML As Boolean, _
                         ByVal URI As String) As String

' Call Web Service (SOAP).
' IN:  Listener, listener URL
'      MethodName, name of method to call
'      Arguments, arguments array (x,0)=name, (x,1)=value (optional, none=0)
'      ShowXML, show sent and received XML (SOAP)?
'      URI, URI of service (optional, none="")
' OUT: SOAPCall, return value
' Known bugs:
' Version   Date   Who Comment
' 00.00.000 010327 CFO Created
' 00.01.000 011112 SZF Modified for PocketSOAP 1.1
'******************************************************************************
  Dim lsResponse As String
  Dim lsRequest As String
  Dim ls As String
  Dim i As Integer
  
  ' Init Envelope
  pEnvelope.SetMethod MethodName, URI
  
  ' Set parameters
  pEnvelope.Parameters.Clear
  If Not VarType(Arguments) = vbInteger Then
    For i = 0 To UBound(Arguments, 1) - 1
      pEnvelope.Parameters.Create Arguments(i, 0), Arguments(i, 1)
    Next 'i
  End If
  
  ' Set request
  lsRequest = pEnvelope.Serialize
  
  If ShowXML Then MsgBox lsRequest, , "Request"
  
  ' Make SOAP call
  pHTTP.send Listener, lsRequest
  
  ' Parse response
  pEnvelope.parse pHTTP
  
  ' Get and show result
  'pDumpParams 0, pEnvelope.Parameters
  SOAPCall = pEnvelope.Parameters.Item(0).Value

End Function
Private Function pDumpParams(ByVal Indent As Integer, ByVal Parameters As ISOAPParams)
  
' Dump parameter values.
' IN:  Indent, indent
'      Parameters, parameter collection
' Known bugs:
' Version   Date   Who Comment
' 00.00.000 010327 CFO Created
'******************************************************************************
  Dim lParameter
  
  For Each lParameter In Parameters
    MsgBox Space(Indent) & lParameter.Name & " : " & lParameter.Value
    pDumpParams Indent + 2, lParameter.Parameters
  Next

End Function

