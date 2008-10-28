Attribute VB_Name = "helpers"
' $Header: c:/cvs/pocketsoap/wsdl/codegen/helpers.bas,v 1.3 2004/01/06 04:07:08 simon Exp $
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

Public Const SOAP_11_ENCODING = "http://schemas.xmlsoap.org/soap/encoding/"
Public Const WSDL_URI = "http://schemas.xmlsoap.org/wsdl/"
Public Const ANONYMOUS_TYPENS = "http://anontypes.wsdl.pocketsoap.com/"
Public Const OPT_PROPERTY_SUFIX = "Occurs"

Public Sub WriteClassHeader(ByVal f As Object, ByVal className As String, ByVal wsdlUrl As String, ByVal persitable As Boolean)
    f.writeline "VERSION 1.0 CLASS"
    f.writeline "BEGIN"
    f.writeline "MultiUse = -1  'True"
    f.writeline "Persistable = " & IIf(persitable, "-1  ' ", "0  ' Not") & "Persistable"
    f.writeline "DataBindingBehavior = 0  'vbNone"
    f.writeline "DataSourceBehavior = 0   'vbNone"
    f.writeline "MTSTransactionMode = 0   'NotAnMTSObject"
    f.writeline "End"
    f.writeline "Attribute VB_Name = """ + className + """"
    f.writeline "Attribute VB_GlobalNameSpace = False"
    f.writeline "Attribute VB_Creatable = True"
    f.writeline "Attribute VB_PredeclaredId = False"
    f.writeline "Attribute VB_Exposed = True"
    f.writeline "' --------------------------------------------------------------------------------------------"
    f.writeline "' $Head" + "er: $"
    f.writeline "'"
    f.writeline "' generated at " & Now & " from"
    f.writeline "' " & wsdlUrl
    f.writeline "' by the PocketSOAP WSDL Wizard v" & App.Major & "." & App.Minor & "." & App.Revision
    f.writeline "' --------------------------------------------------------------------------------------------"
    f.writeline "Option Explicit"
    f.writeline ""
End Sub

' this returns the current value of lv and increments lv to the next valid value
Public Function incNextSpareLocal(ByRef lv As String) As String
    incNextSpareLocal = lv
    Dim c As String
    c = Right$(lv, 1)
    If c < "z" Then
        lv = Left$(lv, Len(lv) - 1) + Chr(Asc(c) + 1)
    Else
        lv = Left$(lv, Len(lv) - 1) + "aa"
    End If
    Dim sx As String
    sx = safeVBVarName(lv)
    If sx <> lv Then incNextSpareLocal lv
End Function

' returns a version of strName that is safe to use as a VB variable/property name
Public Function safeVBVarName(ByVal strName As String) As String
    Dim kn()
    Dim lcName As String
    lcName = LCase$(strName)
    ' todo, make this more efficient !
    ' todo, get all the keywords
    kn = array("string", "int", "single", "double", "boolean", "byte", "object", "variant", "return", "type", "as", "array", "currency", "call", "select", "scale", "case", "event", "private", "public", "dim")
    If strName = "Name" Then
        safeVBVarName = "name"
        Exit Function
    End If
    strName = Replace(strName, "-", "")
    strName = Replace(strName, ":", "")
    strName = Replace(strName, " ", "")
    strName = Replace(strName, "_", "")
    Dim i As Integer
    For i = LBound(kn) To UBound(kn)
        If kn(i) = lcName Then
            safeVBVarName = strName & "0"
            Exit Function
        End If
    
    Next
    safeVBVarName = strName
End Function

' generate a VB class name from an xsdTYpe QName
' applies VB max progID length of 39 rules
' todo: ensure that the vbNames don't clash
Public Function VbNameBuilder(ByVal prjName As String, ByVal xsdType As qname) As String
    Dim n As String
    n = xsdType.localname
    n = Replace(n, ".", "")
    n = Replace(n, "-", "")
    
    ' modified by averma - 10/24/2003
    n = safeVBVarName(n)
        
    Dim maxLen As Integer
    maxLen = 39 - Len(prjName) - 1
    If Len(n) > maxLen Then n = Left$(n, maxLen)
    VbNameBuilder = n
End Function
