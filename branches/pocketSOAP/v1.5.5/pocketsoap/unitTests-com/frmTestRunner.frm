VERSION 5.00
Object = "{2F53C057-4650-4B9F-A149-F990D158A360}#7.3#0"; "COMUnitRunner.ocx"
Begin VB.Form frmTestRunner 
   Caption         =   "Form1"
   ClientHeight    =   6045
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   10620
   LinkTopic       =   "Form1"
   ScaleHeight     =   6045
   ScaleWidth      =   10620
   StartUpPosition =   3  'Windows Default
   Begin COMUnitRunner.UnitRunner UnitRunner1 
      Height          =   5415
      Left            =   0
      TabIndex        =   2
      Top             =   0
      Width           =   10575
      _ExtentX        =   18653
      _ExtentY        =   9551
   End
   Begin VB.CommandButton btnRun 
      Caption         =   "Run Tests"
      Default         =   -1  'True
      Height          =   495
      Left            =   8040
      TabIndex        =   0
      Top             =   5520
      Width           =   1215
   End
   Begin VB.CommandButton btnClose 
      Caption         =   "Close"
      Height          =   495
      Left            =   9360
      TabIndex        =   1
      Top             =   5520
      Width           =   1215
   End
End
Attribute VB_Name = "frmTestRunner"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/frmTestRunner.frm,v 1.6 2003/06/21 04:34:07 simon Exp $

' COMUnit 1.1 - TestRunner form

Option Explicit

' Initialize the TestRunner control
Private Sub Form_Load()
    ' TODO: add instances of your TestContainer classes to the UnitRunner control
    ' e.g. UnitRunner1.AddTestContainer New TCTestContainer
    UnitRunner1.AddTestContainer New tcSoap12Serialization
    UnitRunner1.AddTestContainer New tcSOAP12Deser
    UnitRunner1.AddTestContainer New tcSOAP11Serialization
    UnitRunner1.AddTestContainer New tcMultiref11
    UnitRunner1.AddTestContainer New tcMultiref12
    UnitRunner1.AddTestContainer New tcSerialization
    UnitRunner1.AddTestContainer New tcEchotests
    UnitRunner1.AddTestContainer New tcQName
    UnitRunner1.AddTestContainer New tcHTTPTransport
    UnitRunner1.AddTestContainer New tcDateSerializer
    UnitRunner1.AddTestContainer New tcTypes
    UnitRunner1.AddTestContainer New tcHeaders
    
End Sub

' Run the tests selected in the UnitRunner
Private Sub btnRun_Click()
    UnitRunner1.Run
End Sub

' Close the form
Private Sub btnClose_Click()
    Unload Me
End Sub

' Resize the UnitRunner control and the buttons on the form
Private Sub Form_Resize()
    UnitRunner1.Move 0, 0, ScaleWidth, PosInt(ScaleHeight - btnClose.Height - 50)
    btnClose.Move PosInt(ScaleWidth - btnClose.Width), PosInt(ScaleHeight - btnClose.Height)
    btnRun.Move PosInt(ScaleWidth - btnClose.Width - btnRun.Width - 100), PosInt(ScaleHeight - btnRun.Height)
End Sub

Private Function PosInt(iValue) As Integer
    PosInt = IIf(iValue > 0, iValue, 0)
End Function
