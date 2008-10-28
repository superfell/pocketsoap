Attribute VB_Name = "helpers"
' $Header: c:/cvs/pocketsoap/wsdl/wsdl_gui/helpers.bas,v 1.1 2003/10/21 03:47:16 simon Exp $
Option Explicit

Public Const SOAP_11_ENCODING = "http://schemas.xmlsoap.org/soap/encoding/"
Public Const WSDL_URI = "http://schemas.xmlsoap.org/wsdl/"

Public Function findMessage(ByVal defs As wsdlParser.definitions, msgName As String, ByVal msgNS As String) As wsdlParser.message
    Dim m As wsdlParser.message
    For Each m In defs.messages
        Debug.Print m.name.localname & ":" & m.name.Namespace
        If m.name.localname = msgName And m.name.Namespace = msgNS Then
            Set findMessage = m
            Exit Function
        End If
    Next
    Err.Raise 43211 + vbObjectError, , "Couldn't find message"
End Function
