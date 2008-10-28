Attribute VB_Name = "mAttrHelpers"
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/mAttrHelpers.bas,v 1.1 2003/10/21 03:47:16 simon Exp $
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

Public Function getIndexFromName(ByVal sURI As String, ByVal sName As String, ByVal oAtt As IVBSAXAttributes) As Long
    On Error GoTo noName
    getIndexFromName = oAtt.getIndexFromName(sURI, sName)
    Exit Function
    
noName:
    getIndexFromName = -1
End Function

Public Sub InitNewHandler(ByVal h As IVBSAXContentHandler, ByVal sURI As String, ByVal sName As String, ByVal sQName As String, ByVal oAtt As IVBSAXAttributes, ByVal ctx As IParseContext)
    SetHandlerCtx h, ctx
    h.startElement sURI, sName, sQName, oAtt
End Sub

Public Sub SetHandlerCtx(ByVal h As IVBSAXContentHandler, ByVal ctx As IParseContext)
    If Not h Is Nothing Then
        If TypeOf h Is IParseContextInit Then
            Dim ctxi As IParseContextInit
            Set ctxi = h
            ctxi.SetParseContext ctx
        End If
    End If
End Sub

