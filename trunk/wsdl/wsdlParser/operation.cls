VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "operation"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/operation.cls,v 1.1 2003/10/21 03:47:16 simon Exp $
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
Public soapAction As String
Public style As String

Public Type soapBody
    use As String
    namespace As String
    encodingStyle As String
    parts As String
End Type

Public Type soapHeader
    use As String
    namespace As String
    encodingStyle As String
    message As QName
    part As String
End Type

Private m_inputHeaders As Collection
Private m_outputHeaders As Collection
Private iSB As soapBody
Private oSB As soapBody
Private bInput As Boolean
Private bOutput As Boolean
Private m_ctx As IParseContext

Private Sub IParseContextInit_SetParseContext(ByVal ctx As IParseContext)
    Set m_ctx = ctx
    Set m_inputHeaders = New Collection
    Set m_outputHeaders = New Collection
End Sub

Public Property Get inputHeaders() As Collection
    Set inputHeaders = m_inputHeaders
End Property

Public Property Get outputHeaders() As Collection
    Set outputHeaders = m_outputHeaders
End Property

Public Property Get inputBody() As soapBody
    inputBody = iSB
End Property

Public Property Get outputBody() As soapBody
    outputBody = oSB
End Property

Private Sub IVBSAXContentHandler_startElement(strNamespaceURI As String, strLocalName As String, strQName As String, ByVal oAttributes As MSXML2.IVBSAXAttributes)
    If strNamespaceURI = URI_WSDL Then
        If strLocalName = "operation" Then
            name = oAttributes.getValueFromName("", "name")
            
        ElseIf strLocalName = "input" Then
            bInput = True
            
        ElseIf strLocalName = "output" Then
            bOutput = True
        
        End If
        
    ElseIf strNamespaceURI = URI_WSDL_SOAP Then
        If strLocalName = "operation" Then
            soapAction = oAttributes.getValueFromName("", "soapAction")
            On Error Resume Next
            style = oAttributes.getValueFromName("", "style")
            On Error GoTo 0
            
        ElseIf strLocalName = "body" And bInput Then
            iSB.use = oAttributes.getValueFromName("", "use")
            On Error Resume Next
            iSB.namespace = oAttributes.getValueFromName("", "namespace")
            iSB.encodingStyle = oAttributes.getValueFromName("", "encodingStyle")
            iSB.parts = oAttributes.getValueFromName("", "parts")
            On Error GoTo 0
            
        ElseIf strLocalName = "body" And bOutput Then
            oSB.use = oAttributes.getValueFromName("", "use")
            On Error Resume Next
            oSB.namespace = oAttributes.getValueFromName("", "namespace")
            oSB.encodingStyle = oAttributes.getValueFromName("", "encodingStyle")
            oSB.parts = oAttributes.getValueFromName("", "parts")
            On Error GoTo 0
        
        ElseIf strLocalName = "header" Then
            Dim h As soapHeader
            h.use = oAttributes.getValueFromName("", "use")
            Set h.message = New QName
            h.message.extractQNameFromAttriubteValue m_ctx, oAttributes, "", "message"
            On Error Resume Next
            h.encodingStyle = oAttributes.getValueFromName("", "encodingStyle")
            h.namespace = oAttributes.getValueFromName("", "namespace")
            h.part = oAttributes.getValueFromName("", "part")
            On Error GoTo 0
            If bInput Then m_inputHeaders.Add h
            If bOutput Then m_outputHeaders.Add h
            
        End If
    End If
End Sub

Private Sub IVBSAXContentHandler_characters(strChars As String)
End Sub

Private Sub IVBSAXContentHandler_endElement(strNamespaceURI As String, strLocalName As String, strQName As String)
    If strNamespaceURI = URI_WSDL Then
        If strLocalName = "input" Then bInput = False
        If strLocalName = "output" Then bOutput = False
    End If
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

