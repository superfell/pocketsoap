VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "qname"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/qname.cls,v 1.1 2003/10/21 03:47:16 simon Exp $
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

Public localname As String
Public QName As String
Public namespace As String
Public prefix As String

Public Property Get ExpandedName() As String
    ExpandedName = namespace + "#" + localname
End Property

Public Function extractQNameFromAttriubteValue(ByVal ctx As IParseContext, _
                                                ByVal oAttributes As IVBSAXAttributes, _
                                                ByVal ns As String, _
                                                ByVal ln As String)
    extractQNameFromAttriubteValue = extractQNameFromAttriubteValueWithTarget(ctx, oAttributes, ns, ln, True)
End Function

Public Function extractQNameFromAttriubteValueWithTarget(ByVal ctx As IParseContext, _
                                                            ByVal oAttributes As IVBSAXAttributes, _
                                                            ByVal ns As String, _
                                                            ByVal ln As String, _
                                                            ByVal bUnprefixedIsTNS As Boolean)
    Dim av As String
    On Error Resume Next
    av = oAttributes.getValueFromName(ns, ln)
    On Error GoTo 0
    
    Dim p As Integer
    p = InStr(1, av, ":")
    QName = av
    If (p > 0) Then
        localname = Right$(av, Len(av) - p)
        prefix = Left$(av, p - 1)
    Else
        prefix = ""
        localname = av
    End If
    
    ' todo resolve Namespace
    namespace = ctx.ResolveNSPrefix(bUnprefixedIsTNS, prefix)
End Function

