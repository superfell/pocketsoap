VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "message"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/message.cls,v 1.2 2004/05/27 05:31:02 simon Exp $
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

Public Name As QName
Public parts As Collection

Implements IVBSAXContentHandler
Implements IParseContextInit

Private m_ctx As IParseContext

Private Sub IParseContextInit_SetParseContext(ByVal ctx As IParseContext)
    Set m_ctx = ctx
End Sub


Private Sub IVBSAXContentHandler_startElement(strNamespaceURI As String, strLocalName As String, strQName As String, ByVal oAttributes As MSXML2.IVBSAXAttributes)
    If strNamespaceURI = URI_WSDL Then
        If strLocalName = WSDL_MESSAGE Then
            Set parts = New Collection
            Set Name = New QName
            Name.extractQNameFromAttriubteValue m_ctx, oAttributes, "", "name"
        
        ElseIf strLocalName = "part" Then
            Dim p As New part
            p.Name = oAttributes.getValueFromName("", "name")
            Set p.XmlType = New QName
            Set p.Element = New QName
            p.XmlType.extractQNameFromAttriubteValue m_ctx, oAttributes, "", "type"
            p.Element.extractQNameFromAttriubteValue m_ctx, oAttributes, "", "element"
            parts.Add p
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


