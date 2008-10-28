VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "ICodeGenTarget"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/codegen/ICodeGenTarget.cls,v 1.1 2003/10/21 03:47:15 simon Exp $
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

' implement this to get called to codegen from the WSDL [i.e. one impl per language]

' initialize / finalize will get called once per run
Public Sub Initialize(ByVal genDirectory As String, _
                        ByVal wsdlUrl As String, _
                        ByVal wsdlDef As wsdlParser.definitions, _
                        ByVal ctxParams As MAPLib.MapCollection)
End Sub

' startProxy will get called for each portType we code gen for
Public Sub StartProxy(ByVal port As wsdlParser.port, _
                        ByVal binding As wsdlParser.binding, _
                        ByVal portType As wsdlParser.portType)
End Sub

' operation will get called 0..n times after startProxy
Public Sub Operation(ByVal opName As String)
End Sub

' FinalizeProxy will get called once all the operations for a given proxy have been generated
Public Sub FinalizeProxy()
End Sub

' Finalize will get called once all the proxies have been generated
Public Sub Finalize()
End Sub

