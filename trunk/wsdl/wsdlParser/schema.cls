VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "schema"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/schema.cls,v 1.1 2003/10/21 03:47:16 simon Exp $
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
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/schema.cls,v 1.1 2003/10/21 03:47:16 simon Exp $
Option Explicit

' this is the schema handler. we basically accumulate the events into a DOM
' then load the DOM into the schema cache.

Implements IVBSAXContentHandler
Implements IParseContextInit

Private m_ctx As IParseContext
Private m_schemaNS As String
Private m_tns As String

Private m_dom As DOMDocument40
Private m_w As MXXMLWriter40
Private m_ch As IVBSAXContentHandler
Private endDocDone As Boolean

Private Sub Class_Initialize()
    m_tns = ""
    Set m_dom = New DOMDocument40
    Set m_w = New MXXMLWriter40
    m_w.output = m_dom
    m_w.indent = True
    Set m_ch = m_w
    endDocDone = False
End Sub

Public Property Get Schema() As DOMDocument40
    If Not endDocDone Then
        IVBSAXContentHandler_endDocument
        endDocDone = True
    End If
    Set Schema = m_dom
End Property

Public Property Get namespace() As String
    namespace = m_tns
End Property

Private Sub IParseContextInit_SetParseContext(ByVal ctx As IParseContext)
    Set m_ctx = ctx
    IVBSAXContentHandler_startDocument
End Sub

Private Sub IVBSAXContentHandler_startElement(strNamespaceURI As String, strLocalName As String, strQName As String, ByVal oAttributes As MSXML2.IVBSAXAttributes)
    Debug.Print "StartElement " & strQName
    If (strNamespaceURI = URI_SCHEMA_99 Or strNamespaceURI = URI_SCHEMA_01) And strLocalName = "schema" Then
        m_schemaNS = strNamespaceURI
        Dim tnsIdx As Integer
        tnsIdx = getIndexFromName("", "targetNamespace", oAttributes)
        If tnsIdx >= 0 Then
            m_tns = oAttributes.getValue(tnsIdx)
        End If
    End If
    ' ok, this is a fairly evil hack, but no apparent "correct" way to do this so far
    If strLocalName = "attribute" Then
        Dim idx As Integer, i As Integer
        idx = getIndexFromName(URI_WSDL, "arrayType", oAttributes)
        If idx >= 0 Then
            Dim A As SAXAttributes40
            Set A = New SAXAttributes40
            For i = 0 To oAttributes.length - 1
                If i <> idx Then
                    A.addAttribute oAttributes.getURI(i), oAttributes.getLocalName(i), oAttributes.getQName(i), oAttributes.getType(i), oAttributes.getValue(i)
                Else
                    Dim qn As New QName
                    qn.extractQNameFromAttriubteValueWithTarget m_ctx, oAttributes, URI_WSDL, "arrayType", False
                    A.addAttribute oAttributes.getURI(i), oAttributes.getLocalName(i), oAttributes.getQName(i), oAttributes.getType(i), qn.namespace + ":" + qn.localname
                End If
            Next
            Set oAttributes = A
        End If
    End If
    m_ch.startElement strNamespaceURI, strLocalName, strQName, oAttributes
End Sub

Private Sub IVBSAXContentHandler_characters(strChars As String)
End Sub

Private Sub IVBSAXContentHandler_endElement(strNamespaceURI As String, strLocalName As String, strQName As String)
    Debug.Print "endElement " & strQName
    m_ch.endElement strNamespaceURI, strLocalName, strQName
End Sub

Private Property Set IVBSAXContentHandler_documentLocator(ByVal RHS As MSXML2.IVBSAXLocator)
    Set m_ch.documentLocator = RHS
End Property

Private Sub IVBSAXContentHandler_endDocument()
    Debug.Print "EndDoc"
    m_ch.endDocument
End Sub

Private Sub IVBSAXContentHandler_endPrefixMapping(strPrefix As String)
    Debug.Print "endPrefixMapping " & strPrefix
    m_ch.endPrefixMapping strPrefix
End Sub

Private Sub IVBSAXContentHandler_ignorableWhitespace(strChars As String)
    Debug.Print "ignorableWhitespace"
    m_ch.ignorableWhitespace strChars
End Sub

Private Sub IVBSAXContentHandler_processingInstruction(strTarget As String, strData As String)
    Debug.Print "PI"
    m_ch.processingInstruction strTarget, strData
End Sub

Private Sub IVBSAXContentHandler_skippedEntity(strName As String)
    Debug.Print "Skipped entity"
    m_ch.skippedEntity strName
End Sub

Private Sub IVBSAXContentHandler_startDocument()
    Debug.Print "StartDoc"
    m_ch.startDocument
End Sub

Private Sub IVBSAXContentHandler_startPrefixMapping(strPrefix As String, strURI As String)
    Debug.Print "prefixMapping " & strPrefix & " -> " & strURI
    m_ch.startPrefixMapping strPrefix, strURI
End Sub

