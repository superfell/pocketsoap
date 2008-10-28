VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "complexTypeWriter"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/codegen/complexTypeWriter.cls,v 1.2 2003/10/22 02:39:54 simon Exp $
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

Private fc As Object

Private m_vars As Collection
Private m_propCode As Collection
Private m_bases As Collection
Private m_impl_shims As Collection

Const PROP_PREFIX = "m_p"
Const OPT_PREFIX = "m_o"

Public Sub Init(ByVal dir As String, ByVal className As String, ByVal wsdlUrl As String)
    Dim fso As Object
    Set fso = CreateObject("Scripting.FileSystemObject")
    Set fc = fso.createTextFile(dir & className & ".cls")
    
    WriteClassHeader fc, className, wsdlUrl, False
    Set m_vars = New Collection
    Set m_propCode = New Collection
    Set m_bases = New Collection
    Set m_impl_shims = New Collection
End Sub

Public Sub Complete()
    Dim s As Variant
    For Each s In m_bases
        fc.writeline s
    Next
    fc.writeline
    For Each s In m_vars
        fc.writeline s
    Next
    fc.writeline
    For Each s In m_propCode
        fc.write s
    Next
    fc.writeline
    fc.Close
End Sub

Public Sub AddPropertyField(ByVal fieldName, ByRef itemType As comType)
    m_vars.Add "private m_p" & fieldName & IIf(itemType.array <> atNotArray, "()", "") & " as " & itemType.name
    m_propCode.Add generatorPropertyAccessors(fieldName, PROP_PREFIX & fieldName, OPT_PREFIX & fieldName, itemType, True)
    If itemType.opt Then
        m_vars.Add "private " + OPT_PREFIX & fieldName & " as boolean"
        m_propCode.Add generateOptionalHandlers("", fieldName, itemType, True)
    End If
End Sub

Public Sub AddBaseType(ByVal baseQName As qname, baseComType As comType)
    m_bases.Add "implements " + baseComType.name + "' " + baseQName.ExpandedName
End Sub

Public Sub AddBaseTypeProperty(baseComType As comType, ByVal itemName As String, childComType As comType)
    m_propCode.Add generatorPropertyAccessors(baseComType.name + "_" + itemName, itemName, OPT_PREFIX + itemName, childComType, False)
    If childComType.opt Then
        m_propCode.Add generateOptionalHandlers(baseComType.name, itemName, childComType, False)
    End If
End Sub

' generates a pair of property accessors
' itemName is the external name of the property
' privateName is the name of the variable its mapped to
' privateOptFlag is the name of the variable that tracks if this property has been set or not (only used if itemType.opt is true)
' itemType is the comType of the property
' IsPublicProperty when true generates a public property accessor, when false generates a private one (for interface implementations)
'
Private Function generatorPropertyAccessors(ByVal itemName As String, _
                                                ByVal privateName As String, _
                                                ByVal privateOptFlag As String, _
                                                ByRef itemType As comType, _
                                                ByVal IsPublicProperty As Boolean) As String
    Dim ln As String
    Dim opt As String
    Dim useSet As Boolean
    Dim pubprivate As String
    useSet = (itemType.style = tObject) And (itemType.array = atNotArray)
    pubprivate = IIf(IsPublicProperty, "public", "private")
    ln = pubprivate + " property get " & itemName & "() as " & itemType.name & IIf(itemType.array <> atNotArray, "()", "") + vbCrLf
    ln = ln + vbTab
    If useSet Then ln = ln + "set "
    ln = ln + itemName + " = " + privateName + vbCrLf
    ln = ln + "end property" + vbCrLf + vbCrLf
    ln = ln + pubprivate + " property "
    ln = ln + IIf(useSet, "set ", "let ")
    ln = ln + itemName & "("
    If itemType.array = atNotArray Then ln = ln + "byval "
    ln = ln + "newVal" + IIf(itemType.array <> atNotArray, "()", "") + " as " + itemType.name + ")" + vbCrLf
    ln = ln + vbTab + IIf(useSet, "set ", "") + privateName + " = newVal" + vbCrLf
    If itemType.opt Then
        ln = ln + vbTab + privateOptFlag + " = true" + vbCrLf
    End If
    ln = ln + "end property" + vbCrLf + vbCrLf
    
    generatorPropertyAccessors = ln
End Function

' fields that are optional have additional properies
Private Function generateOptionalHandlers(ByVal itfName As String, _
                                            ByVal itemName As String, _
                                            ByRef itemType As comType, _
                                            ByVal IsPublicProperty As Boolean) As String
    Dim ct As comType
    ct.opt = False
    ct.array = atNotArray
    ct.style = tPrimative
    ct.name = "boolean"
    Dim propName As String
    If Len(itfName) > 0 Then
        propName = itfName + "_" + itemName + OPT_PROPERTY_SUFIX
    Else
        propName = itemName + OPT_PROPERTY_SUFIX
    End If
    generateOptionalHandlers = generatorPropertyAccessors(propName, OPT_PREFIX + itemName, "", ct, IsPublicProperty)
End Function
