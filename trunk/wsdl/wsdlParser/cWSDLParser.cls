VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Engine"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/cWSDLParser.cls,v 1.2 2004/01/16 03:31:41 simon Exp $
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

Private m_defs As definitions
Private m_props As MapCollection
Private m_err As String
Private m_ern As Long

Implements IVBSAXErrorHandler

Const INET_E_DOWNLOAD_FAILURE = &H800C0008
Const INET_E_OBJECT_NOT_FOUND = &H800C0006

' used properties are
'   fudge-imports add missing import elements for soap-enc namespace to schema docs
Public Property Get Property(ByVal key As String) As Variant
    Property = m_props.Value(key)
End Property

Public Property Let Property(ByVal key As String, ByVal val As Variant)
    m_props.Value(key) = val
End Property

Public Function ParseWSDLFile(ByVal sFileName As String, Optional properties) As definitions
     Dim sax As SAXXMLReader40
     m_err = ""
     m_ern = 0
     On Error GoTo bang
    
11   Set sax = New SAXXMLReader40
21   Set m_defs = New definitions
31   If (Not IsMissing(properties)) And (Not IsNull(properties)) Then Set m_props = properties
41   m_defs.Init sFileName, m_props
51   Set sax.contentHandler = m_defs
61   sax.putFeature "server-http-request", False
62   sax.putFeature "exhaustive-errors", True
71   Set sax.errorHandler = Me
81   sax.parseURL sFileName
91   Set ParseWSDLFile = m_defs
     Exit Function
    
bang:
     App.LogEvent "Error in ParseWSDLFile 0x" & Hex(Err.Number) & " " & Err.Description & " at line " & Erl & vbCrLf & m_err & " " & m_ern
     If m_ern = 0 Then m_ern = Err.Number
     If m_err = "" Then
        If m_ern = INET_E_DOWNLOAD_FAILURE Then
            m_err = "Unable to load WSDL, please make sure it exists!"
        ElseIf m_ern = INET_E_OBJECT_NOT_FOUND Then
            m_err = "specified WSDL does not exist"
        Else
            m_err = Err.Description
        End If
     End If
     Err.Raise m_ern, Err.Source, m_err
End Function

Private Sub Class_Initialize()
    Set m_props = New MapCollection
    m_props.Value("fudge-imports") = False
End Sub

Private Sub IVBSAXErrorHandler_error(ByVal oLocator As MSXML2.IVBSAXLocator, strErrorMessage As String, ByVal nErrorCode As Long)
    Debug.Print "error " & strErrorMessage & " " & nErrorCode
    handleError oLocator, strErrorMessage, nErrorCode
End Sub

Private Sub IVBSAXErrorHandler_fatalError(ByVal oLocator As MSXML2.IVBSAXLocator, strErrorMessage As String, ByVal nErrorCode As Long)
    Debug.Print "fatalError " & strErrorMessage & " " & nErrorCode
    handleError oLocator, strErrorMessage, nErrorCode
End Sub

Private Sub IVBSAXErrorHandler_ignorableWarning(ByVal oLocator As MSXML2.IVBSAXLocator, strErrorMessage As String, ByVal nErrorCode As Long)
    Debug.Print "IVBSAXErrorHandler_ignorableWarning"
End Sub

Private Sub handleError(ByVal oLocator As MSXML2.IVBSAXLocator, strErrorMessage As String, ByVal nErrorCode As Long)
    If Len(Err.Description) > 0 And (Left$(strErrorMessage, 17) = "Unspecified error" Or Left$(strErrorMessage, "6") = "System") Then
        m_err = Err.Description
    Else
        m_err = strErrorMessage & " [line=" & oLocator.lineNumber & " col=" & oLocator.columnNumber & "]"
    End If
    m_ern = nErrorCode
End Sub
