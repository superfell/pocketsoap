VERSION 5.00
Begin VB.Form demoForm 
   Caption         =   "Form1"
   ClientHeight    =   9270
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   6570
   LinkTopic       =   "Form1"
   ScaleHeight     =   9270
   ScaleWidth      =   6570
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "DeSerialize"
      Height          =   375
      Left            =   1800
      TabIndex        =   2
      Top             =   240
      Width           =   1575
   End
   Begin VB.TextBox msg 
      Height          =   8055
      Left            =   120
      MultiLine       =   -1  'True
      TabIndex        =   1
      Top             =   1080
      Width           =   6375
   End
   Begin VB.CommandButton cmdSerialize 
      Caption         =   "Serialize"
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   240
      Width           =   1575
   End
End
Attribute VB_Name = "demoForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
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

Private Const xsd = "http://www.w3.org/2001/XMLSchema"

Private Sub cmdSerialize_Click()
    ' create and populate a collection
    Dim col As New Collection
    Dim i As Integer
    For i = 0 To 10
        col.Add CLng(i * 10)
    Next
    
    ' create an envelope, and add the collection as a parameter
    Dim e As New CoEnvelope
    e.SetMethod "collection", "http://samples.pocketsoap.com/collections/"
    e.Parameters.Create "numbers", col
    
    ' tell the serializer factory about our custom serializer
    ' {A4C46780-499F-101B-BB78-00AA00383CBB} is the interface IID of the standard VB collection class
    e.SerializerFactory.Serializer "{A4C46780-499F-101B-BB78-00AA00383CBB}", "", "", "colDemo.IntArraySerializer"
    msg.Text = e.Serialize
End Sub

Private Sub Command1_Click()
    Dim e As New CoEnvelope
    ' tell the serializer factory about our custom deserializer
    e.SerializerFactory.Deserializer "int", xsd, True, vbLong, "colDemo.IntArraySerializer"
    
    ' parse the message
    e.Parse msg.Text

    ' dump the contents of the collection
    msg.Text = "parameter type is " & TypeName(e.Parameters.Item(0).Value) & vbCrLf
    Dim c As Collection
    Set c = e.Parameters.Item(0).Value
    Dim i As Integer
    For i = 1 To c.Count
        msg.Text = msg.Text & "item " & i & " = " & c.Item(i) & vbCrLf
    Next
End Sub

