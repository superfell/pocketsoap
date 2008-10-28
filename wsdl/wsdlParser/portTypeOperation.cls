VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "portTypeOperation"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/portTypeOperation.cls,v 1.2 2003/12/31 04:20:27 simon Exp $
'
' The contents of this file are subject to the Mozilla Public License
' Version 1.1 (the "License"); you may not use this file except in
' compliance with the License. You may obtain a copy of the License at
' http://www.mozilla.org/MPL/
'
' Software distributed under the License is distributed on an "AS IS"
' basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
' License for the specific language governing rights and limitations
' under the License.
'
' The Original Code is pocketSOAP WSDL Wizard.
'
' The Initial Developer of the Original Code is Simon Fell.
' Portions created by Simon Fell are Copyright (C) 2002
' Simon Fell. All Rights Reserved.
'
' Contributor (s):
'
Option Explicit

Implements IVBSAXContentHandler
Implements IParseContextInit

Public name As String

Public Type msgInfo
    msg As QName
    name As String
End Type

Private iMsg As msgInfo
Private oMsg As msgInfo

' added support for SOAP fault by averma - 10/13/2002
Private fMsgs As New Collection ' there could be multiple fault messages associated with a single operation

Private m_ctx As IParseContext
'

Private Sub IParseContextInit_SetParseContext(ByVal ctx As IParseContext)
    Set m_ctx = ctx
End Sub

Public Property Get inputMessage() As msgInfo
    inputMessage = iMsg
End Property

Public Property Get outputMessage() As msgInfo
    outputMessage = oMsg
End Property

' added support for SOAP fault by averma - 10/13/2002
Public Property Get faultMessages() As Collection ' of msgInfo
    Set faultMessages = fMsgs
End Property


Private Sub IVBSAXContentHandler_startElement(strNamespaceURI As String, strLocalName As String, strQName As String, ByVal oAttributes As MSXML2.IVBSAXAttributes)
    If strNamespaceURI = URI_WSDL Then
        If strLocalName = "operation" Then
            name = oAttributes.getValueFromName("", "name")
            
        ElseIf strLocalName = "input" Then
            Set iMsg.msg = New QName
            iMsg.msg.extractQNameFromAttriubteValue m_ctx, oAttributes, "", "message"
            On Error Resume Next
            iMsg.name = oAttributes.getValueFromName("", "name")
            If Err <> 0 Then iMsg.name = iMsg.msg.localname
            On Error GoTo 0
            
        ElseIf strLocalName = "output" Then
            Set oMsg.msg = New QName
            oMsg.msg.extractQNameFromAttriubteValue m_ctx, oAttributes, "", "message"
            On Error Resume Next
            oMsg.name = oAttributes.getValueFromName("", "name")
            If Err <> 0 Then oMsg.name = oMsg.msg.localname
            On Error GoTo 0
            
        ' added support for SOAP fault by averma - 10/13/2002
        ElseIf strLocalName = "fault" Then
            Dim fMsg As msgInfo
            Set fMsg.msg = New QName
            fMsg.msg.extractQNameFromAttriubteValue m_ctx, oAttributes, "", "message"
            On Error Resume Next
            fMsg.name = oAttributes.getValueFromName("", "name")
            If Err <> 0 Then fMsg.name = fMsg.msg.localname
            fMsgs.Add fMsg, fMsg.name
            On Error GoTo 0
        
        End If
    End If
End Sub

Private Sub IVBSAXContentHandler_characters(strChars As String)
End Sub

Private Sub IVBSAXContentHandler_endElement(strNamespaceURI As String, strLocalName As String, strQName As String)
End Sub

Private Property Set IVBSAXContentHandler_documentLocator(ByVal RHS As MSXML2.IVBSAXLocator)
End Property

Private Sub IVBSAXContentHandler_endDocument()
End Sub

Private Sub IVBSAXContentHandler_endPrefixMapping(strPrefix As String)
End Sub

Private Sub IVBSAXContentHandler_ignorableWhitespace(strChars As String)
End Sub

Private Sub IVBSAXContentHandler_processingInstruction(strTarget As String, strData As String)
End Sub

Private Sub IVBSAXContentHandler_skippedEntity(strName As String)
End Sub

Private Sub IVBSAXContentHandler_startDocument()
End Sub

Private Sub IVBSAXContentHandler_startPrefixMapping(strPrefix As String, strURI As String)
End Sub
