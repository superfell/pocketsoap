VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "IntArraySerializer"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
'The contents of this file are subject to the Mozilla Public License
'Version 1.1 (the "License"); you may not use this file except in
'compliance with the License. You may obtain a copy of the License at
'http://www.mozilla.org/MPL/
'
'Software distributed under the License is distributed on an "AS IS"
'basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
'License for the specific language governing rights and limitations
'under the License.
'
'The Original Code is pocketSOAP Collections Sample.

'The Initial Developer of the Original Code is Simon Fell.
'Portions created by Simon Fell are Copyright (C) 2004
'Simon Fell. All Rights Reserved.
'
'Contributor (s):

Option Explicit

Implements PocketSOAP.CoSerializerArray
Implements PocketSOAP.CoDeserializerArray

Private Const soap_enc = "http://schemas.xmlsoap.org/soap/encoding/"
Private Const xsd = "http://www.w3.org/2001/XMLSchema"

Private m_col As Collection
Private m_node As CoSoapNode

Private Sub CoDeserializerArray_Start(ByVal node As PocketSOAP.ISOAPNode, ByVal ElementName As String, ByVal Attributes As PocketSOAP.ISoapDeSerializerAttributes, ByVal ns As PocketSOAP.ISOAPNamespaces)
    Set m_col = New Collection
    Set m_node = node
End Sub

Private Sub CoDeserializerArray_Characters(ByVal charData As String)
    ' don't care
End Sub

Private Sub CoDeserializerArray_Child(ByVal id As Long, ByVal ready As Boolean, ByVal childNode As PocketSOAP.ISOAPNode)
    ' to make the potential defered references easier to handle, we drop the nodes into the collection
    ' then at the end, we'll walk the collection and grab all the values
    m_col.Add childNode
End Sub

Private Sub CoDeserializerArray_ChildRef(ByVal href As String, ByVal hrefNode As PocketSOAP.ISOAPNode)
    m_col.Add hrefNode
End Sub

Private Sub CoDeserializerArray_ChildReady(ByVal id As Long, ByVal childNode As PocketSOAP.ISOAPNode)
End Sub

Private Sub CoDeserializerArray_ref(ByVal id As String, ByVal idNode As PocketSOAP.ISOAPNode)
End Sub

Private Sub CoDeserializerArray_End()
    Dim nc As New Collection
    Dim i As Integer
    Dim n As CoSoapNode
    For i = 1 To m_col.Count
        Set n = m_col.item(i)
        nc.Add n.Value
    Next
    m_node.Value = nc
    Set m_node = Nothing
    Set m_col = Nothing
End Sub

Private Sub CoSerializerArray_Serialize(Val As Variant, ByVal ctx As PocketSOAP.ISerializerContext, ByVal dest As PocketSOAP.ISerializerOutput)
    ' write out the soap-enc:arrayType attribute
    dest.QNameAttribute "arrayType", soap_enc, "int[]", xsd
    
    ' val is a variant containing a standard vb collection
    Dim col As Collection
    Set col = Val
    Dim item As Variant
    For Each item In col
        dest.SerializeValue item, "item", ""
    Next
End Sub
