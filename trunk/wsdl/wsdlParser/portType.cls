VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "portType"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/portType.cls,v 1.1 2003/10/21 03:47:16 simon Exp $
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
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/portType.cls,v 1.1 2003/10/21 03:47:16 simon Exp $
Option Explicit

Implements IVBSAXContentHandler
Implements IParseContextInit

Public name As QName
Public operations As Collection

Private m_ch As IVBSAXContentHandler
Private m_ctx As IParseContext

Private Sub IParseContextInit_SetParseContext(ByVal ctx As IParseContext)
    Set m_ctx = ctx
End Sub


Private Sub IVBSAXContentHandler_startElement(strNamespaceURI As String, strLocalName As String, strQName As String, ByVal oAttributes As MSXML2.IVBSAXAttributes)
    
    If Not m_ch Is Nothing Then
        m_ch.startElement strNamespaceURI, strLocalName, strQName, oAttributes
        Exit Sub
    End If
    
    If strNamespaceURI = URI_WSDL Then
        If strLocalName = "portType" Then
            Set name = New QName
            name.extractQNameFromAttriubteValue m_ctx, oAttributes, "", "name"
            Set operations = New Collection
        
        ElseIf strLocalName = "operation" Then
            Dim o As portTypeOperation
            Set o = New portTypeOperation
            Set m_ch = o
            InitNewHandler m_ch, strNamespaceURI, strLocalName, strQName, oAttributes, m_ctx
            operations.Add o, o.name
        End If
    End If
End Sub

Private Sub IVBSAXContentHandler_characters(strChars As String)
    If Not m_ch Is Nothing Then m_ch.characters strChars
End Sub

Private Sub IVBSAXContentHandler_endElement(strNamespaceURI As String, strLocalName As String, strQName As String)
    If strNamespaceURI = URI_WSDL And strLocalName = "operation" Then Set m_ch = Nothing
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

