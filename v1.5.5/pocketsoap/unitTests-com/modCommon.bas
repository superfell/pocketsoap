Attribute VB_Name = "modCommon"
' $Header: c:/cvs/pocketsoap/pocketsoap/unitTests-com/modCommon.bas,v 1.7 2004/08/21 19:10:13 simon Exp $

Option Explicit

Public Const XSD = "http://www.w3.org/2001/XMLSchema"
Public Const XSI = "http://www.w3.org/2001/XMLSchema-instance"
Public Const SOAP_11_ENV = "http://schemas.xmlsoap.org/soap/envelope/"
Public Const SOAP_11_ENC = "http://schemas.xmlsoap.org/soap/encoding/"

Public Const SOAP_12_ENV = "http://www.w3.org/2003/05/soap-envelope"
Public Const SOAP_12_ENC = "http://www.w3.org/2003/05/soap-encoding"

Private Declare Function CreateFile Lib "kernel32" Alias "CreateFileA" (ByVal lpFileName As String, ByVal dwDesiredAccess As Long, ByVal dwShareMode As Long, lpSecurityAttributes As Long, ByVal dwCreationDisposition As Long, ByVal dwFlagsAndAttributes As Long, ByVal hTemplateFile As Long) As Long
Private Declare Function GetFileSize Lib "kernel32" (ByVal hFile As Long, lpFileSizeHigh As Long) As Long
Private Declare Function ReadFile Lib "kernel32" (ByVal hFile As Long, lpBuffer As Byte, ByVal nNumberOfBytesToRead As Long, lpNumberOfBytesRead As Long, ByVal lpOverlapped As Long) As Long
Private Declare Function CloseHandle Lib "kernel32" (ByVal hObject As Long) As Long

Public Function getNewDom() As DOMDocument40
    Dim d As DOMDocument40
    Set d = New DOMDocument40
    d.validateOnParse = False
    d.async = False
    d.setProperty "SelectionLanguage", "XPath"
    d.setProperty "SelectionNamespaces", "xmlns:a='urn:tests' xmlns:s12='" + SOAP_12_ENV + "' xmlns:s11='" + SOAP_11_ENV + "'"
    Set getNewDom = d
End Function

Public Function getMessage(fileName As String) As Byte()
    fileName = App.Path + "\msgs\" + fileName + ".xml"
    
    Dim fileHandle As Long
    Dim res() As Byte
    Dim rc As Long
    Dim cb As Long
    Dim fs As Long
    fileHandle = CreateFile(fileName, &H80000000, 0, 0, 3, 0, 0)
    If fileHandle <> -1 Then
        fs = GetFileSize(fileHandle, 0)
        ReDim res(fs - 1) As Byte
        rc = ReadFile(fileHandle, res(0), fs, cb, 0)
        CloseHandle fileHandle
    End If
    
    getMessage = res
End Function

