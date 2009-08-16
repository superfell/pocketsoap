VERSION 5.00
Begin VB.Form frmSoakTest 
   Caption         =   "Soak Test"
   ClientHeight    =   1905
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   6480
   LinkTopic       =   "Form1"
   ScaleHeight     =   1905
   ScaleWidth      =   6480
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command3 
      Caption         =   "Reset"
      Height          =   495
      Left            =   5160
      TabIndex        =   4
      Top             =   960
      Width           =   1095
   End
   Begin VB.Timer Timer1 
      Interval        =   10
      Left            =   2520
      Top             =   1200
   End
   Begin VB.CommandButton Command2 
      Caption         =   "Stop"
      Height          =   495
      Left            =   3240
      TabIndex        =   3
      Top             =   960
      Width           =   1455
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Start"
      Height          =   495
      Left            =   360
      TabIndex        =   0
      Top             =   960
      Width           =   1455
   End
   Begin VB.Label Label2 
      Caption         =   "0 / 0 (failures/runs)"
      BeginProperty Font 
         Name            =   "@Arial Unicode MS"
         Size            =   18
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   2520
      TabIndex        =   2
      Top             =   120
      Width           =   3855
   End
   Begin VB.Label Label1 
      Caption         =   "Soak Test"
      BeginProperty Font 
         Name            =   "@Arial Unicode MS"
         Size            =   18
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   240
      TabIndex        =   1
      Top             =   120
      Width           =   2055
   End
End
Attribute VB_Name = "frmSoakTest"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private run_tests As Boolean
Private runs As Integer
Private failures As Integer

Private Sub Command1_Click()
    run_tests = True
End Sub

Private Sub Command2_Click()
    run_tests = False
End Sub

Private Sub Command3_Click()
    runs = 0
    failures = 0
End Sub

Private Sub Timer1_Timer()
    If run_tests Then
        Timer1.Enabled = False
        runTests
        updateDisplay
        Timer1.Enabled = True
    End If
End Sub

Private Sub runTests()
    Dim et As tcEchotests
    Set et = New tcEchotests
    Dim tr As TestResult
    
    Set tr = New TestResult
    et.testGzip tr
    updateStats tr
    
    Set tr = New TestResult
    et.testDeflate tr
    updateStats tr
End Sub

Private Sub updateStats(ByVal tr As TestResult)
    If Not tr.WasSuccessful Then
        failures = failures + 1
    End If
    runs = runs + 1
End Sub

Private Sub updateDisplay()
    Label2.Caption = failures & " / " & runs & " (failures/runs)"
    DoEvents
End Sub
