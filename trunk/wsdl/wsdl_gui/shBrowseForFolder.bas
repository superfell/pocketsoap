Attribute VB_Name = "modShBrowseForFolder"
' $Header: c:/cvs/pocketsoap/wsdl/wsdl_gui/shBrowseForFolder.bas,v 1.1 2003/10/21 03:47:16 simon Exp $
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

Private Type BrowseInfo
    hWndOwner      As Long
    pIDLRoot       As Long
    pszDisplayName As Long
    lpszTitle      As Long
    ulFlags        As Long
    lpfnCallback   As Long
    lParam         As Long
    iImage         As Long
End Type

Private Const BIF_RETURNONLYFSDIRS = 1
Private Const MAX_PATH = 260

Private Declare Sub CoTaskMemFree Lib "ole32.dll" (ByVal hMem As Long)
Private Declare Function lstrcat Lib "kernel32" Alias "lstrcatA" _
       (ByVal lpString1 As String, ByVal lpString2 As String) As Long
Private Declare Function SHBrowseForFolder Lib "shell32" _
       (lpbi As BrowseInfo) As Long
Private Declare Function SHGetPathFromIDList Lib "shell32" _
       (ByVal pidList As Long, ByVal lpBuffer As String) As Long

Public Function BrowseForFolder(hWndOwner As Long, sPrompt As String) As String

    Dim iNull As Integer
    Dim lpIDList As Long
    Dim lResult As Long
    Dim sPath As String
    Dim udtBI As BrowseInfo

    With udtBI
        .hWndOwner = hWndOwner
        .lpszTitle = lstrcat(sPrompt, "")
        .ulFlags = BIF_RETURNONLYFSDIRS
    End With

    lpIDList = SHBrowseForFolder(udtBI)
    If lpIDList Then
        sPath = String$(MAX_PATH, 0)
        lResult = SHGetPathFromIDList(lpIDList, sPath)
        Call CoTaskMemFree(lpIDList)
        iNull = InStr(sPath, vbNullChar)
        If iNull Then
            sPath = Left$(sPath, iNull - 1)
        End If
    End If

    BrowseForFolder = sPath

End Function


