VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "EnumSerializerWriter"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/codegen/EnumSerializerWriter.cls,v 1.1 2003/10/21 03:47:15 simon Exp $
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

Private fs As Object

Private m_top As Collection
Private m_ser As Collection
Private m_desChar As Collection
    
' dir           is the path to the directory where we're generating code
' className     is the name of the object we're serializing to/from
' serClassName  is the name of the generated serializer class
' tQName        is the XML Type QName that we're mapping to/from
' wsdlUrl       is the URL of the WSDL file we're generaing from
' bIncXsiType   should we attempt to write out the xsi:type on the wire ?

Public Sub Init(ByVal dir As String, ByVal serClassName As String, ByVal bIncXsiType As Boolean, ByVal wsdlUrl As String)
    Dim fso As Object
    Set fso = CreateObject("Scripting.FileSystemObject")
    Set fs = fso.createTextFile(dir & serClassName & ".cls")
    WriteClassHeader fs, serClassName, wsdlUrl, False
    WriteSerializerTop
    
    Set m_top = New Collection
    Set m_ser = New Collection
    Set m_desChar = New Collection
End Sub

Public Sub AddEnum(ByVal prjName As String, ByVal xsdType As qname, ByVal sType As ISchemaType)
    Dim i As Long
    
    ' create a definition of the enum type
    Dim enumType As String
    enumType = VbNameBuilder(prjName, xsdType)
    m_top.Add "public enum " + enumType + vbCrLf
    For i = 0 To sType.enumeration.length - 1
        m_top.Add vbTab + safeVBVarName(sType.enumeration.Item(i)) + vbCrLf
    Next
    m_top.Add "end enum" + vbCrLf + vbCrLf
    
    ' serialization code
    m_ser.Add "Select Case theVal" + vbCrLf
    For i = 0 To sType.enumeration.length - 1
        m_ser.Add vbTab + "case " + safeVBVarName(sType.enumeration.Item(i)) + " : dest.WriteText """ + sType.enumeration.Item(i) + """" + vbCrLf
    Next
    m_ser.Add vbTab + "case else : err.raise vbObjectError + 4343,,""Cannot serialize invalid value for enum " + enumType + """" + vbCrLf
    m_ser.Add "end select" + vbCrLf
    
    ' deserialization code
    m_desChar.Add "select case charData" + vbCrLf
    For i = 0 To sType.enumeration.length - 1
        m_desChar.Add vbTab + "case """ + sType.enumeration.Item(i) + """ : m_node.Value = " + safeVBVarName(sType.enumeration.Item(i)) + vbCrLf
    Next
    m_desChar.Add vbTab + "case else : err.raise vbObjectError + 4344,,""Cannot de-serialize invalid value ["" + charData + ""] for enum " + enumType + """" + vbCrLf
    m_desChar.Add "end select" + vbCrLf
    
End Sub

' finished adding Attributes/Elements/Content finish up generating the proxy class
Public Sub Complete()
    Dim s
    For Each s In m_top
        fs.write s
    Next
    fs.writeline ""
    fs.writeline "Private Sub ISoapSerializer_Serialize(theVal As Variant, ByVal ctx As PocketSOAP.ISerializerContext, ByVal dest As PocketSOAP.ISerializerOutput)"
    
    For Each s In m_ser
        fs.write vbTab + s
    Next
    fs.writeline "End Sub"
    fs.writeline ""

    ' de-serializer start
    fs.writeline "Private Sub ISoapDeSerializer_Start(ByVal node As PocketSOAP.ISOAPNode, ByVal ElementName As String, ByVal Attributes As PocketSOAP.ISoapDeSerializerAttributes, ByVal ns As PocketSOAP.ISOAPNamespaces)"
    fs.writeline vbTab + "set m_node = node"
    fs.writeline "End Sub"
    fs.writeline ""

    ' de-serialize char data
    fs.writeline "Private Sub ISoapDeSerializer_Characters(ByVal charData As String)"
    For Each s In m_desChar
        fs.write vbTab + s
    Next
    fs.writeline "End Sub"
    fs.writeline ""
        
    ' child nodes
    fs.writeline "Private Sub ISoapDeSerializer_Child(ByVal id As Long, ByVal ready As Boolean, ByVal childNode As PocketSOAP.ISOAPNode)"
    fs.writeline "End Sub"
    fs.writeline ""
    fs.writeline "Private Sub ISoapDeSerializer_ChildReady(ByVal id As Long, ByVal childNode As PocketSOAP.ISOAPNode)"
    fs.writeline "End Sub"
    fs.writeline ""
    fs.writeline "Private Sub ISoapDeSerializer_ChildRef(ByVal href As String, ByVal hrefNode As PocketSOAP.ISOAPNode)"
    fs.writeline "End Sub"
    fs.writeline ""
    fs.writeline "Private Sub ISoapDeSerializer_Ref(ByVal id As String, ByVal idNode As PocketSOAP.ISOAPNode)"
    fs.writeline "End Sub"
    fs.writeline ""
    
    ' serializer class tidy up
    fs.writeline "Private Sub ISoapDeSerializer_End()"
    fs.writeline vbTab + "set m_node = nothing"
    fs.writeline "End Sub"
    fs.writeline ""
    
    fs.Close
End Sub

Private Sub WriteSerializerTop()
    fs.writeline "Implements ISoapSerializer"
    fs.writeline "Implements ISoapDeSerializer"
    fs.writeline ""
    fs.writeline "private m_node as ISOAPNode2"
    fs.writeline ""
End Sub

