VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmWiz 
   Caption         =   "pocketSOAP WSDL Client Wizard"
   ClientHeight    =   5355
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5985
   Icon            =   "frmWiz.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   5355
   ScaleWidth      =   5985
   StartUpPosition =   2  'CenterScreen
   WhatsThisHelp   =   -1  'True
   Begin VB.Frame fPanes 
      Caption         =   "Welcome"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   3735
      Index           =   0
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   5775
      Begin VB.TextBox txtProjectName 
         Height          =   285
         Left            =   120
         TabIndex        =   25
         Text            =   "TestSfdcWsdl"
         Top             =   3120
         Width           =   5175
      End
      Begin VB.CommandButton cmdBrowse 
         Caption         =   "..."
         Height          =   255
         Left            =   5280
         TabIndex        =   21
         Top             =   2400
         Width           =   255
      End
      Begin VB.TextBox txtDest 
         Height          =   285
         Left            =   120
         TabIndex        =   17
         Text            =   "g:\"
         Top             =   2400
         Width           =   5175
      End
      Begin VB.TextBox txtWSDLURL 
         Height          =   285
         Left            =   120
         TabIndex        =   8
         Text            =   "http://www.pocketsoap.com/registration/service.wsdl"
         Top             =   1680
         Width           =   5175
      End
      Begin VB.Label Label1 
         AutoSize        =   -1  'True
         Caption         =   "Enter name of VB Project"
         Height          =   195
         Index           =   6
         Left            =   120
         TabIndex        =   24
         Top             =   2880
         Width           =   1785
      End
      Begin VB.Label Label1 
         AutoSize        =   -1  'True
         Caption         =   "Enter the directory to save the proxy class to"
         Height          =   195
         Index           =   5
         Left            =   120
         TabIndex        =   16
         Top             =   2160
         Width           =   3135
      End
      Begin VB.Label Label1 
         AutoSize        =   -1  'True
         Caption         =   "Enter the URL of the WSDL file"
         Height          =   195
         Index           =   2
         Left            =   120
         TabIndex        =   7
         Top             =   1440
         Width           =   2235
      End
      Begin VB.Label Label1 
         AutoSize        =   -1  'True
         Caption         =   "This wizard will generate a SOAP proxy class from a WSDL description of a web service"
         Height          =   390
         Index           =   1
         Left            =   120
         TabIndex        =   6
         Top             =   720
         Width           =   4275
         WordWrap        =   -1  'True
      End
      Begin VB.Label Label1 
         AutoSize        =   -1  'True
         Caption         =   "Welcome to the pocketSOAP WSDL client generator"
         Height          =   195
         Index           =   0
         Left            =   120
         TabIndex        =   5
         Top             =   360
         Width           =   3765
      End
   End
   Begin VB.CommandButton cmdAbout 
      Caption         =   "&About"
      Height          =   375
      Left            =   1080
      TabIndex        =   23
      Top             =   4920
      Width           =   975
   End
   Begin VB.Frame fPanes 
      Caption         =   "Generating Code"
      Height          =   3375
      Index           =   3
      Left            =   0
      TabIndex        =   15
      Top             =   840
      Width           =   5775
      Begin MSComctlLib.ProgressBar pbCodeGen 
         Height          =   375
         Left            =   120
         TabIndex        =   18
         Top             =   1080
         Width           =   5535
         _ExtentX        =   9763
         _ExtentY        =   661
         _Version        =   393216
         Appearance      =   1
      End
      Begin VB.Label lDone 
         Caption         =   "xxxxx"
         Height          =   1515
         Left            =   120
         TabIndex        =   22
         Top             =   1680
         Width           =   5415
         WordWrap        =   -1  'True
      End
      Begin VB.Label lWait 
         AutoSize        =   -1  'True
         Caption         =   "Please wait ....."
         Height          =   195
         Left            =   120
         TabIndex        =   20
         Top             =   720
         Width           =   1080
      End
      Begin VB.Label Label2 
         AutoSize        =   -1  'True
         Caption         =   "The pocketSOAP WSDL Client Wizard is generating the proxy code"
         Height          =   195
         Index           =   0
         Left            =   120
         TabIndex        =   19
         Top             =   360
         Width           =   4800
      End
   End
   Begin VB.Frame fPanes 
      Caption         =   "Select Operations"
      Height          =   3135
      Index           =   2
      Left            =   0
      TabIndex        =   12
      Top             =   480
      Width           =   5895
      Begin VB.ListBox lstOperations 
         Height          =   2400
         Left            =   120
         MultiSelect     =   1  'Simple
         TabIndex        =   13
         Top             =   600
         Width           =   5535
      End
      Begin VB.Label Label1 
         AutoSize        =   -1  'True
         Caption         =   "Select the opreations you wish to include in the proxy"
         Height          =   195
         Index           =   4
         Left            =   120
         TabIndex        =   14
         Top             =   360
         Width           =   3750
      End
   End
   Begin VB.Frame fPanes 
      Caption         =   "Select Service"
      Height          =   3135
      Index           =   1
      Left            =   0
      TabIndex        =   9
      Top             =   240
      Width           =   5895
      Begin VB.ListBox lstServices 
         Height          =   2400
         Left            =   120
         TabIndex        =   11
         Top             =   600
         Width           =   5535
      End
      Begin VB.Label Label1 
         AutoSize        =   -1  'True
         Caption         =   "Select the service and port to generate a proxy for"
         Height          =   195
         Index           =   3
         Left            =   120
         TabIndex        =   10
         Top             =   360
         Width           =   3540
      End
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   0
      TabIndex        =   4
      Top             =   4920
      Width           =   975
   End
   Begin VB.CommandButton cmdNext 
      Caption         =   "&Next"
      Default         =   -1  'True
      Height          =   375
      Left            =   3240
      TabIndex        =   3
      Top             =   4920
      Width           =   975
   End
   Begin VB.CommandButton cmdPrev 
      Caption         =   "&Previous"
      Height          =   375
      Left            =   2160
      TabIndex        =   2
      Top             =   4920
      Width           =   975
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "&OK"
      Enabled         =   0   'False
      Height          =   375
      Left            =   4920
      TabIndex        =   1
      Top             =   4920
      Width           =   975
   End
End
Attribute VB_Name = "frmWiz"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/wsdl/wsdl_gui/frmWiz.frm,v 1.3 2004/05/10 01:49:05 simon Exp $
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
' Portions created by Simon Fell are Copyright (C) 2002-2003
' Simon Fell. All Rights Reserved.
'
' Contributor (s):
' Anant Verma
'
Option Explicit

Private m_currentPane As Long
Private m_wsdl As wsdlParser.definitions
Private m_binding As wsdlParser.binding
Private m_port As wsdlParser.Port

Const tab_WELCOME = 0
Const tab_SERVICE = 1
Const tab_OPERATIONS = 2
Const tab_CODEGEN = 3

Private Sub Fire_OnTab()
    DoEvents
    If m_currentPane = tab_WELCOME Then OnWelcomeSelected
    If m_currentPane = tab_SERVICE Then OnServiceSelected
    If m_currentPane = tab_OPERATIONS Then OnOperationsSelected
    If m_currentPane = tab_CODEGEN Then OnCodeGen
End Sub

Private Sub OnCodeGen()
    pbCodeGen.Max = lstOperations.ListCount + 1
    pbCodeGen.Value = 1
    
    Dim pt As wsdlParser.portType
    For Each pt In m_wsdl.portTypes
        If pt.Name.localname = m_binding.bindingType.localname And pt.Name.namespace = m_binding.bindingType.namespace Then
            Exit For
        End If
    Next
    
    Dim codegen As ICodeGenTarget
    Set codegen = New vb6CodeGen
    Dim ctx As MapCollection
    Set ctx = New MapCollection
    ctx.Value("project") = txtProjectName.Text
    codegen.Initialize txtDest.Text, txtWSDLURL.Text, m_wsdl, ctx
    
    codegen.StartProxy m_port, m_binding, pt
       
    Dim i As Long
    For i = 0 To lstOperations.ListCount - 1
        pbCodeGen.Value = pbCodeGen.Value + 1
        If lstOperations.Selected(i) Then
            codegen.operation lstOperations.List(i)
        End If
    Next
    codegen.FinalizeProxy
    codegen.Finalize
    
    lWait.Caption = ""
    lDone.Caption = "Finished generating proxy class for " & m_port.Name.localname & ", click OK to close the wizard"
    cmdOK.Enabled = True
End Sub



Private Sub OnWelcomeSelected()
   Set m_wsdl = Nothing
End Sub

Private Sub OnServiceSelected()
    On Error GoTo handleErr
    If Right$(txtDest.Text, 1) <> "\" Then txtDest.Text = txtDest.Text + "\"
    If m_wsdl Is Nothing Then
        lstServices.Clear
        Dim we As wsdlParser.Engine
        Set we = New wsdlParser.Engine
        Set m_wsdl = we.ParseWSDLFile(txtWSDLURL.Text)
    
        Dim p As wsdlParser.Port
        For Each p In m_wsdl.ports
            lstServices.AddItem p.Name.localname & " -> " & p.binding.localname
        Next
        lstServices.Selected(0) = True
    End If
    Exit Sub
    
handleErr:
    If Err.Number = -2146697208 Then
        MsgBox "Unable to load WSDL, make sure its a valid URL", vbOKOnly + vbCritical
        cmdPrev_Click
    Else
        MsgBox "Unexpected error whilst parsing WSDL" & vbCrLf & Err.Description & vbCrLf & "err=" & Err.Number, vbOKOnly + vbCritical
    End If
End Sub

Private Sub OnOperationsSelected()
    lstOperations.Clear
    Set m_port = m_wsdl.ports.Item(lstServices.ListIndex + 1)
    Dim b As binding
    For Each b In m_wsdl.bindings
        If b.Name.localname = m_port.binding.localname And b.Name.namespace = m_port.Name.namespace Then
            Set m_binding = b
            Dim o As operation
            Dim X As ListItem
            For Each o In b.operations
                lstOperations.AddItem o.Name
                lstOperations.Selected(lstOperations.ListCount - 1) = True
            Next
            Exit For
        End If
    Next
End Sub

Private Sub cmdAbout_Click()
    frmAbout.Show 1, Me
End Sub

Private Sub cmdBrowse_Click()
    txtDest.Text = BrowseForFolder(Me.hWnd, "Select directory")
    If Right$(txtDest.Text, 1) <> "\" Then txtDest.Text = txtDest.Text + "\"
End Sub

Private Sub cmdNext_Click()
    fPanes(m_currentPane).Visible = False
    m_currentPane = m_currentPane + 1
    fPanes(m_currentPane).Visible = True
    SetPrevNext
    Fire_OnTab
End Sub

Private Sub cmdPrev_Click()
    fPanes(m_currentPane).Visible = False
    m_currentPane = m_currentPane - 1
    fPanes(m_currentPane).Visible = True
    SetPrevNext
    Fire_OnTab
End Sub

Private Sub cmdOK_Click()
    saveVals
    Unload Me
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub Form_Load()
    InitFrames
    lstOperations.Width = fPanes.Item(0).Width - (lstOperations.Left * 2)
    lstServices.Width = fPanes.Item(0).Width - (lstServices.Left * 2)
    pbCodeGen.Width = fPanes.Item(0).Width - (pbCodeGen.Left * 2)
    LoadLastVals
    lDone.Caption = ""
End Sub

Private Sub LoadLastVals()
    txtWSDLURL.Text = GetSetting(App.Title, "wizard", "url", "http://www.pocketsoap.com/services/ilab.wsdl")
    txtDest.Text = GetSetting(App.Title, "wizard", "dest", "c:\temp\")
    txtProjectName.Text = GetSetting(App.Title, "wizard", "prj", "")
End Sub

Private Sub saveVals()
    SaveSetting App.Title, "wizard", "url", txtWSDLURL.Text
    SaveSetting App.Title, "wizard", "dest", txtDest.Text
    SaveSetting App.Title, "wizard", "prj", txtProjectName.Text
End Sub

Private Sub InitFrames()
    Dim i As Long
    For i = fPanes.LBound To fPanes.UBound
        fPanes(i).BorderStyle = 1
        fPanes(i).Top = 0
        fPanes(i).Left = 0
        fPanes(i).Width = Width - 120
        fPanes(i).Height = Height - 1000
        If i = fPanes.LBound Then
            fPanes(i).Visible = True
        Else
            fPanes(i).Visible = False
        End If
    Next
    m_currentPane = fPanes.LBound
    SetPrevNext
End Sub

Private Sub SetPrevNext()
    cmdPrev.Enabled = m_currentPane > fPanes.LBound
    cmdNext.Enabled = m_currentPane < fPanes.UBound
End Sub

