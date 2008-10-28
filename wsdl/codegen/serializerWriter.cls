VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "serializerWriter"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/codegen/serializerWriter.cls,v 1.5 2004/05/10 01:49:05 simon Exp $
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
' Portions created by Simon Fell are Copyright (C) 2002,2004
' Simon Fell. All Rights Reserved.
'
' Contributor (s):
'

Option Explicit

Private fs As Object

Private m_top As Collection
Private m_ser As Collection
Private m_des As Collection
Private m_desElem As Collection
Private m_desChar As Collection
Private m_propCode As Collection
Private m_deserArrayTypes As MapCollection
    
Private m_className As String
Private m_tQName As qname
Private m_bIncXsiType As Boolean
    
Private m_bfirstAttribute As Boolean
Private m_bSerIdxDefined As Boolean
Private m_nextSpareLocal As String
Private m_incChildSupport As Boolean

' dir           is the path to the directory where we're generating code
' className     is the name of the object we're serializing to/from
' serClassName  is the name of the generated serializer class
' tQName        is the XML Type QName that we're mapping to/from
' wsdlUrl       is the URL of the WSDL file we're generaing from
' bIncXsiType   should we attempt to write out the xsi:type on the wire ?

Public Sub Init(ByVal dir As String, ByVal className As String, ByVal serClassName As String, ByVal tQName As qname, ByVal bIncXsiType As Boolean, ByVal wsdlUrl As String)
    Dim fso As Object
    Set fso = CreateObject("Scripting.FileSystemObject")
    Set fs = fso.createTextFile(dir & serClassName & ".cls")
    WriteClassHeader fs, serClassName, wsdlUrl, False
    m_className = className
    Set m_tQName = tQName
    m_bIncXsiType = bIncXsiType
    WriteSerializerTop
    
    Set m_top = New Collection
    Set m_ser = New Collection
    Set m_des = New Collection
    Set m_desChar = New Collection
    Set m_desElem = New Collection
    Set m_deserArrayTypes = New MapCollection
    m_incChildSupport = False
    m_bfirstAttribute = True
    m_bSerIdxDefined = False
    m_nextSpareLocal = "a"
    
End Sub

' propName  the name of the property on the object
' atr       the attribute to serialize/de-serialize to propName
' xmlType   the QName of the type of the attribute

Public Sub AddAttribute(ByVal propName As String, ByVal atr As ISchemaAttribute, ByVal xmlType As qname)
    ' serialization
    If m_bfirstAttribute Then
        m_ser.Add "dim d2 as ISerializerOutput2" + vbCrLf + vbTab + "set d2 = dest" + vbCrLf
    End If
    m_ser.Add "d2.SerializeAttribute val." & propName & ", """ & atr.name & """, """ & atr.namespaceURI & """" + vbCrLf
    
    ' deserialization
    If m_bfirstAttribute Then
        m_des.Add "Dim a As ISoapDeSerializerAttributes2" + vbCrLf + vbTab + "Set a = Attributes" + vbCrLf
        m_bfirstAttribute = False
    End If
    m_des.Add "m_obj." + propName + " = a.ValueAs ( """ + atr.name + """, """ + atr.namespaceURI + """,""" + xmlType.localname + """,""" + xmlType.Namespace + """)" + vbCrLf
End Sub

' propName      the name of the property on the object
' itemType      the COM type of the item
' xmlElem       the XML Element we're [de]serializing to

Public Sub AddElement(ByVal propName As String, itemType As comType, ByVal xmlElem As ISchemaElement)
    ' serializer
    If itemType.array = atLiteral Then
        If Not m_bSerIdxDefined Then
            m_ser.Add "dim idx as long" + vbCrLf
            m_bSerIdxDefined = True
        End If
        m_ser.Add "if ArrayIsValid(val." + propName + ") then" + vbCrLf
        m_ser.Add vbTab + "for idx = lbound(val." + propName + ") to ubound(val." + propName + ")" + vbCrLf
        m_ser.Add vbTab + vbTab + "dest.SerializeValue val." + propName + "()(idx), """ + xmlElem.name + """, """ + xmlElem.namespaceURI + """" + vbCrLf
        m_ser.Add vbTab + "next" + vbCrLf
        m_ser.Add "end if" + vbCrLf
    Else
        If itemType.opt Then
            m_ser.Add "if val." + propName + OPT_PROPERTY_SUFIX + " then" + vbCrLf + vbTab
        End If
        m_ser.Add "dest.SerializeValue val." + propName + ", """ + xmlElem.name + """, """ + xmlElem.namespaceURI + """" + vbCrLf
        If itemType.opt Then
            m_ser.Add "end if" + vbCrLf
        End If
    End If
    
    ' de-serialization code
    Dim localVar As String
    
    m_desElem.Add "if name=""" + xmlElem.name + """ and Namespace=""" + xmlElem.namespaceURI + """ then" + vbCrLf
    If itemType.array <> atLiteral Then
        ' during deserialization XSD primatives can be null that can't be NULL in COM, so we end up
        ' always having to check for NULL first (submitted by averma 10/7/2003)
        m_desElem.Add vbTab + "If Not IsNull(childNode.Value) Then" + vbCrLf + vbTab

        m_desElem.Add vbTab + IIf(itemType.style = tObject, "set ", "") + "m_obj." + propName + " = childNode.value" + vbCrLf
        m_desElem.Add vbTab + "end if" + vbCrLf
    Else
        localVar = incNextSpareLocal(m_nextSpareLocal)
        m_desElem.Add vbTab + "dim " + localVar + "() as " + itemType.name + vbCrLf
        m_desElem.Add vbTab + localVar + " = m_obj." + propName + vbCrLf
        m_desElem.Add vbTab + "m_obj." + propName + " = AddTo" + itemType.name + "Array(" + localVar + ", childNode)" + vbCrLf
        m_deserArrayTypes.Value(itemType.name) = itemType
    End If
    m_desElem.Add vbTab + "exit sub" + vbCrLf
    m_desElem.Add "end if" + vbCrLf
    m_incChildSupport = True
End Sub

' propName      the name of the property that maps the to the element content
Public Sub AddContent(ByVal propName As String)
    m_ser.Add "dest.writeText val." + propName + vbCrLf
    m_desChar.Add "m_obj.Value = charData" + vbCrLf
End Sub


' if the type has base types, then its part of a type heiracrhy and we should send the xsi:type on the wire
Public Sub AddBaseType(ByVal baseQName As qname, baseComType As comType)
    m_bIncXsiType = True
End Sub


' finished adding Attributes/Elements/Content finish up generating the proxy class
Public Sub Complete()
    Dim s
    For Each s In m_top
        fs.write s
    Next
    fs.writeline ""
    fs.writeline "Private Sub ITypesInit_Initialize(ByVal xmlType As String, ByVal XmlTypeNamespace As String, ByVal comType As Variant)"
    fs.writeline vbTab + "m_xmlType = xmlType"
    fs.writeline vbTab + "m_xmlTypeNS = XmlTypeNamespace"
    fs.writeline vbTab + "m_comType = comType"
    fs.writeline "End Sub"
    fs.writeline ""
    fs.writeline "Private Sub ISoapSerializer_Serialize(theVal As Variant, ByVal ctx As PocketSOAP.ISerializerContext, ByVal dest As PocketSOAP.ISerializerOutput)"
    fs.writeline vbTab + "dim val as " + m_className
    fs.writeline vbTab + "set val = theVal"
    
    If Len(m_tQName.Namespace) >= Len(ANONYMOUS_TYPENS) Then
        If Left$(m_tQName.Namespace, Len(ANONYMOUS_TYPENS)) = ANONYMOUS_TYPENS Then
            m_bIncXsiType = False
        End If
    End If
    If m_bIncXsiType Then
        fs.writeline vbTab + "dest.QNameAttribute ""type"", ctx.SerializerFactory.XsiForPrimaryNS, """ + m_tQName.localname + """, """ + m_tQName.Namespace + """"
    End If
    
    For Each s In m_ser
        fs.write vbTab + s
    Next
    fs.writeline "End Sub"
    fs.writeline ""

    ' de-serializer start
    fs.writeline "Private Sub ISoapDeSerializer_Start(ByVal node As PocketSOAP.ISOAPNode, ByVal ElementName As String, ByVal Attributes As PocketSOAP.ISoapDeSerializerAttributes, ByVal ns As PocketSOAP.ISOAPNamespaces)"
    fs.writeline vbTab + "set m_obj = createObject(m_comtype)"
    fs.writeline vbTab + "node.value = m_obj"
    If m_incChildSupport Then
        fs.writeline vbTab + "set m_refs = new collection"
    End If
    For Each s In m_des
        fs.write vbTab + s
    Next
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
    If m_incChildSupport Then
        fs.writeline vbTab + "If ready Then newNode childNode.name, childNode.namespace, childNode"
    End If
    fs.writeline "End Sub"
    fs.writeline ""
    fs.writeline "Private Sub ISoapDeSerializer_ChildReady(ByVal id As Long, ByVal childNode As PocketSOAP.ISOAPNode)"
    If m_incChildSupport Then
        fs.writeline vbTab + "newNode childNode.name, childNode.namespace, childNode"
    End If
    fs.writeline "End Sub"
    fs.writeline ""
    fs.writeline "Private Sub ISoapDeSerializer_ChildRef(ByVal href As String, ByVal hrefNode As PocketSOAP.ISOAPNode)"
    If m_incChildSupport Then
        fs.writeline vbTab + "Dim r As refItem"
        fs.writeline vbTab + "r.href = href"
        fs.writeline vbTab + "Set r.hrefNode = hrefNode"
        fs.writeline vbTab + "m_refs.Add r"
    End If
    fs.writeline "End Sub"
    fs.writeline ""
    fs.writeline "Private Sub ISoapDeSerializer_Ref(ByVal id As String, ByVal idNode As PocketSOAP.ISOAPNode)"
    If m_incChildSupport Then
        fs.writeline vbTab + "Dim i As Long"
        fs.writeline vbTab + "Dim r As refItem"
        fs.writeline vbTab + "For i = m_refs.Count To 1 Step -1"
        fs.writeline vbTab + "    r = m_refs.Item(i)"
        fs.writeline vbTab + "    If r.href = id Then"
        fs.writeline vbTab + "        newNode r.hrefNode.name, r.hrefNode.Namespace, idNode"
        fs.writeline vbTab + "        m_refs.Remove i"
        fs.writeline vbTab + "    End If"
        fs.writeline vbTab + "Next"
    End If
    fs.writeline "End Sub"
    fs.writeline ""
    If m_incChildSupport Then
        fs.writeline "private sub newNode(byval name as string, byval namespace as string, byval childNode as ISOAPNode)"
        For Each s In m_desElem
            fs.write vbTab + s
        Next
        fs.writeline "End Sub"
        fs.writeline ""
    End If
    
    ' serializer class tidy up
    fs.writeline "Private Sub ISoapDeSerializer_End()"
    fs.writeline vbTab + "set m_obj = nothing"
    fs.writeline vbTab + "set m_refs = nothing"
    fs.writeline "End Sub"
    fs.writeline ""
    
    ' write the array de-serializer helpers
    Dim arrType, itemType As comType
    For Each arrType In m_deserArrayTypes
        fs.writeline "private function AddTo" + arrType + "Array(a() as " + arrType + ", byval childNode as PocketSOAP.ISOAPNode) as " + arrType + "()"
        fs.writeline vbTab + "Dim ub As Long"
        fs.writeline vbTab + "On Error Resume Next"
        fs.writeline vbTab + "ub = UBound(a) + 1"
        fs.writeline vbTab + "On Error GoTo 0"
        fs.writeline vbTab + "ReDim Preserve a(ub)"
        itemType = m_deserArrayTypes.Value(arrType)
        
        ' fix to check for null child nodes - averma - 10/02/03
        fs.writeline vbTab + "If Not IsNull(childNode.Value) Then"
        fs.writeline vbTab + "    " + IIf(itemType.style = tObject, "set ", "") + "a(ub) = childNode.Value"
        fs.writeline vbTab + "end if"
        
        fs.writeline vbTab + "AddTo" + arrType + "Array = a"
        fs.writeline "end function" + vbCrLf
    Next
    
    fs.Close
    
End Sub

' className is the name of the object we're serializing to/from
Private Sub WriteSerializerTop()
    
    fs.writeline "Implements ISoapSerializer"
    fs.writeline "Implements ISoapDeSerializer"
    fs.writeline "Implements ITypesInit"
    fs.writeline ""
    fs.writeline "Private m_xmlType As String"
    fs.writeline "Private m_xmlTypeNS As String"
    fs.writeline "Private m_comtype As variant"
    fs.writeline "private m_obj as object"
    fs.writeline "Private m_refs As Collection ' of refItem"
    fs.writeline ""
    

End Sub
