VERSION 5.00
Begin VB.Form frmTests 
   Caption         =   "Xml-Rpc tests"
   ClientHeight    =   8220
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   8535
   LinkTopic       =   "Form1"
   ScaleHeight     =   8220
   ScaleWidth      =   8535
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdGo 
      Caption         =   "&Run"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   1
      Top             =   120
      Width           =   1455
   End
   Begin VB.ListBox List1 
      Height          =   7470
      Left            =   120
      TabIndex        =   0
      Top             =   600
      Width           =   8295
   End
   Begin VB.Label lres 
      BackColor       =   &H00000000&
      Caption         =   "Label1"
      BeginProperty Font 
         Name            =   "Courier New"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1800
      TabIndex        =   2
      Top             =   120
      Width           =   6540
   End
End
Attribute VB_Name = "frmTests"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private tc As Integer
Private fc As Integer

Private Sub log(ByVal txt As String)
    List1.AddItem txt
    DoEvents
End Sub

Private Sub cmdGo_Click()
    tc = 0
    fc = 0
    List1.Clear
    lres.ForeColor = vbWhite
    lres.Caption = "Running ... "
    Dim f
    Set f = CreateObject("pocketXMLRPC.Factory")

    'test_radio f
    test_dotnet f
    
    If fc > 0 Then
        lres.ForeColor = vbRed
    Else
        lres.ForeColor = vbGreen
    End If
    lres.Caption = fc & " failed out of " & tc & " tests"
End Sub

Function test_dotnet(ByVal f)
    log "Testing against XML-RPC.NET"
    
    Dim rpc As Object
    Set rpc = f.Proxy("http://localhost/xmlrpc/echo.ashx", "interopEchoTests.", , , "localhost", 7070)
    test_dotnet_proxy rpc
    
    ' now use a pre-configured transport instead
    log "Testing preconfigured transport"
    Dim h As Object
    Set h = CreateObject("pocket.http")
    h.setProxy "localhost", 7070
    h.proxyAuthentication "fred", "ginger"
    Set rpc = f.ProxyForTransport("http://localhost/xmlrpc/echo.ashx", "interopEchoTests.", h)
    test_dotnet_proxy rpc
End Function

Function test_dotnet_proxy(rpc As Object)
    testS rpc, "hello World !"
    testS rpc, ""
    testS rpc, ChrW$(&H401) & ChrW$(&H402) & ChrW$(&H403)
    testS rpc, "<g>"
    testI rpc, -1
    testI rpc, 0
    testI rpc, 42
    testF rpc, 42.42
    testB rpc, False
    testB rpc, True
    testD rpc, Now
    testD rpc, DateAdd("m", -7, Now)
    testSA rpc, Array("a", "b", "c", "d")
    testIA rpc, Array(1, 2, 3, 4)
    testFA rpc, Array(42.42, 43.43, 44.44)
    testBA rpc, Array(False, True, False)
    testDA rpc, Array(Now, DateAdd("m", -7, Now), Now)
    Dim bytes(300) As Byte
    Dim i As Integer
    For i = LBound(bytes) To UBound(bytes)
        bytes(i) = i Mod 255
    Next
    testBin rpc, bytes
    testStr rpc, makeStruct(1, 42.42, "foo")
    testStrA rpc, Array(makeStruct(1, 42.42, "foo"), makeStruct(-1, 0#, "bar"))
End Function

Function test_radio(f)
    log "Testing against Radio"
    
    Dim rpc As Object
    Set rpc = f.Proxy("http://localhost:5335/RPC2", "interopEchoTests.", , , "localhost", 7070)

    testS rpc, "hello World !"
    testS rpc, ""
    testS rpc, "<g>"
    testI rpc, -1
    testI rpc, 0
    testI rpc, 42
    testF rpc, 42.42
    'testB rpc, False
    'testB rpc, True
    'testD rpc, Now
    'testD rpc, DateAdd("m", -7, Now)
    testSA rpc, Array("a", "b", "c", "d")
    testIA rpc, Array(1, 2, 3, 4)
    testFA rpc, Array(42.42, 43.43, 44.44)
    'testBA rpc, Array(False, True, False)
    'testDA rpc, Array(Now, DateAdd("m", -7, Now), Now)
    'Dim bytes(300) As Byte
    'Dim i As Integer
    'For i = LBound(bytes) To UBound(bytes)
    '    bytes(i) = i Mod 255
    'Next
    'testBin rpc, bytes
    testStr rpc, makeStruct(1, 42.42, "foo")
    testStrA rpc, Array(makeStruct(1, 42.42, "foo"), makeStruct(-1, 0#, "bar"))
End Function

Function makeStruct(ByVal i As Long, ByVal f As Double, ByVal s As String)
    Dim xs As Object
    Set xs = CreateObject("pocketXMLRPC.Struct")
    xs.varInt = i
    xs.varFloat = f
    xs.varString = s
    Set makeStruct = xs
End Function

Function testStr(rpc, s)
    tc = tc + 1
    Dim r As Object
    Set r = rpc.echoStruct(s)
    If strCheck("echoStruct", r, s) Then
        log "echoStruct passed"
    Else
        fc = fc + 1
    End If
End Function

Function strCheck(tn, r, s) As Boolean
    If (r.varInt <> s.varInt) Or (r.varFloat <> s.varFloat) Or (r.varString <> s.varString) Then
        log tn & " failed"
        strCheck = False
    Else
        strCheck = True
    End If
End Function

Function testStrA(rpc, s)
    tc = tc + 1
    Dim r, i
    r = rpc.echoStructArray(s)
    For i = LBound(r) To UBound(r)
        If Not strCheck("echoStructArray", r(i), s(i)) Then
            fc = fc + 1
            Exit Function
        End If
    Next
    log "echoStructArray passed"
End Function

Function testS(rpc, s)
    check "echoString", rpc.echoString(s), s
End Function

Function testI(rpc, s)
    check "echoInteger", rpc.echoInteger(s), s
End Function

Function testF(rpc, s)
    check "echoFloat", rpc.echoFloat(s), s
End Function

Function testB(rpc, s)
    check "echoBoolean", rpc.echoBoolean(s), s
End Function

Function testD(rpc, s)
    check "echoDate", rpc.echoDate(s), s
End Function

Function testSA(rpc, s)
    checkArray "echoStringArray", rpc.echoStringArray(s), s
End Function

Function testIA(rpc, s)
    checkArray "echoIntegerArray", rpc.echoIntegerArray(s), s
End Function

Function testFA(rpc, s)
    checkArray "echoFloatArray", rpc.echoFloatArray(s), s
End Function

Function testBA(rpc, s)
    checkArray "echoBoolArray", rpc.echoBooleanArray(s), s
End Function

Function testDA(rpc, s)
    checkArray "echoDateArray", rpc.echoDateArray(s), s
End Function

Function testBin(rpc, s)
    checkArray "echoBase64", rpc.echoBase64(s), s
End Function

Function check(tn, r, s)
    tc = tc + 1
    If r <> s Then
        log tn & " failed got '" & r & "' expecting '" & r & "'"
        fc = fc + 1
    Else
        log tn & " passed"
    End If
End Function

Function checkArray(tn, r, s)
    Dim i
    tc = tc + 1
    If UBound(r) <> UBound(s) Then
        log tn & " failed : array bounds mismatch"
        fc = fc + 1
    Else
        For i = LBound(r) To UBound(r)
            If r(i) <> s(i) Then
                log tn & " failed : mismatch at index " & i & " got '" & r(i) & "' expecting '" & s(i) & "'"
                fc = fc + 1
                Exit Function
            End If
        Next
    End If
    log tn & " passed"
End Function

Private Sub Form_Load()
    lres.Caption = ""
End Sub
