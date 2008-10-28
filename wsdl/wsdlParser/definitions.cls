VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "definitions"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/wsdlParser/definitions.cls,v 1.2 2003/12/31 04:20:27 simon Exp $
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

Public bindings As Collection
Public ports As Collection
Public portTypes As Collection
Public name As String
Public messages As Collection

Implements IVBSAXContentHandler
Implements IParseContext

Private m_props As MapCollection
Private m_stack As Collection 'parserStackItem
Private bRootChecked As Boolean
Private m_targetNSStack As Collection 'string
Private m_nsstack As Collection 'nsItem
Private m_baseURI As Collection ' string
Private m_schemas As MSXML2.XMLSchemaCache40
' this keeps track of all the schema's we've seen, we need to do a little
' processing of the order that we add them to the schema collection, to get
' around some MSXML4.0 issues
Private m_schemaList As Collection

Private m_imported As Collection    ' this keeps track of the documents we've already imported

' both these types really should be private, but VB sucks !
' and won't let you store them in a collection unless they're public
Public Type nsItem
    uri As String
    prefix As String
End Type

Public Type parserStackItem
    ch As IVBSAXContentHandler
    name As String
    uri As String
End Type

Private Sub Class_Initialize()
    Set bindings = New Collection
    Set ports = New Collection
    Set portTypes = New Collection
    Set messages = New Collection
    
    Set m_targetNSStack = New Collection
    Set m_stack = New Collection
    Set m_nsstack = New Collection
    bRootChecked = False
    Set m_baseURI = New Collection
    Set m_imported = New Collection
    Set m_schemaList = New Collection
End Sub

Public Sub Init(ByVal baseURL As String, ByVal props As MapCollection)
    Set m_props = props
    pushBaseUrl baseURL
End Sub

Public Function findBinding(ByVal localname As String, ByVal nsURI As String) As binding
    Dim b As binding
    For Each b In bindings
        If b.name.localname = localname And b.name.namespace = nsURI Then
            Set findBinding = b
            Exit Function
        End If
    Next
    Err.Raise vbObjectError + 7788, , "Unable to find binding {" + nsURI + "}" + localname
End Function

Public Function findPortType(ByVal localname As String, ByVal nsURI As String) As portType
    Dim p As portType
    For Each p In portTypes
        If p.name.localname = localname And p.name.namespace = nsURI Then
            Set findPortType = p
            Exit Function
        End If
    Next
    Err.Raise vbObjectError + 7789, , "Unable to find portType {" + nsURI + "}" + localname
End Function

Public Function findMessage(ByVal localname As String, ByVal nsURI As String) As message
    Dim m As message
    For Each m In messages
        If m.name.localname = localname And m.name.namespace = nsURI Then
            Set findMessage = m
            Exit Function
        End If
    Next
    Err.Raise vbObjectError + 7790, , "Unable to find message {" + nsURI + "}" + localname
End Function

Private Sub CreateSchemaCache()
    Debug.Print "CreateSchemaCache()"
    Set m_schemas = New XMLSchemaCache40
    m_schemas.validateOnLoad = False
    
    m_schemas.Add "http://schemas.xmlsoap.org/soap/encoding/", App.Path + "\soap11_encoding.xsd"

    Dim idx As Long
    Dim d As DOMDocument40
    Dim ns As String
    
    Dim done() As Boolean
    ReDim done(m_schemaList.Count) As Boolean
    Dim pre As String
    
    ' first up, do all the one's with no imports
    For idx = 1 To m_schemaList.Count
        Set d = m_schemaList.Item(idx).Schema
        d.setProperty "SelectionLanguage", "XPath"
        d.setProperty "SelectionNamespaces", "xmlns:sfsf='" + URI_SCHEMA_01 + "'"
        If d.selectNodes("//sfsf:import").length = 0 Then
            ns = m_schemaList.Item(idx).namespace
            FudgeImports d
            m_schemas.Add ns, d
            done(idx) = True
        End If
'       d.save "c:\soap" & idx & ".xsd"
    Next
    
    ' now do the rest
    For idx = 1 To m_schemaList.Count
        If Not done(idx) Then
            Set d = m_schemaList.Item(idx).Schema
            ns = m_schemaList.Item(idx).namespace
            FudgeImports d
            m_schemas.Add ns, d
            done(idx) = True
        End If
    Next
    
    m_schemas.Validate
    
    Set m_schemaList = Nothing
End Sub

' adds an import for the soap-enc namespace, if its not there
' this fixes a common WSDL authoring problem
' note this needs to be specifically enabled via the fudge-imports property
Private Sub FudgeImports(ByVal d As DOMDocument40)
    On Error GoTo notExists
    'If True Then
    If m_props.Value("fudge-imports") Then
        On Error GoTo 0
        If d.selectNodes("//sfsf:import[@namespace='" + URI_SOAP_ENC + "']").length = 0 Then
            Dim import As IXMLDOMNode
            Dim ns As IXMLDOMAttribute
            Set import = d.createNode(NODE_ELEMENT, "import", URI_SCHEMA_01)
            Set ns = d.createAttribute("namespace")
            ns.Value = URI_SOAP_ENC
            import.Attributes.setNamedItem ns
            
            d.documentElement.insertBefore import, d.documentElement.firstChild
        End If
    End If
    
notExists:
End Sub

Public Property Get Schemas() As XMLSchemaCache40
    Set Schemas = m_schemas
End Property

Private Sub pushBaseUrl(ByVal baseURL As String)
    ' strip the fileName part
    Dim p As Integer, q As Integer
    p = InStrRev(baseURL, "/")
    q = InStrRev(baseURL, "\")
    If q > p Then p = q
    If p > 0 Then
        baseURL = Left$(baseURL, p)
    End If
    m_baseURI.Add baseURL
End Sub

Private Sub popBaseurl()
    m_baseURI.Remove m_baseURI.Count
End Sub

Private Sub StartImportMagic(ByVal oAttributes As MSXML2.IVBSAXAttributes)
    Dim sax As SAXXMLReader
    
    Set sax = New SAXXMLReader
    Set sax.contentHandler = Me
    
    Dim sLoc As String
    sLoc = oAttributes.getValueFromName("", "location")
    sLoc = resolveUrl(sLoc)
    If Not AlreadyImported(sLoc) Then
        pushBaseUrl sLoc
        m_imported.Add sLoc
        sax.parseURL sLoc
        popBaseurl
    End If
End Sub

' returns true if we've already imported this file
Private Function AlreadyImported(ByVal sFile As String) As Boolean
    Dim x
    AlreadyImported = True
    For Each x In m_imported
        If x = sFile Then Exit Function
    Next
    AlreadyImported = False
End Function

' convert a potentially relative URL to an absolute URL
Private Function resolveUrl(ByVal url As String) As String
    If LCase$(Left$(url, 4)) = "http" Then
        resolveUrl = url
        Exit Function
    End If
    If Left$(url, 1) <> "/" Then
        resolveUrl = m_baseURI.Item(m_baseURI.Count) + url
        Exit Function
    End If
    Dim p As Integer
    Dim baseURL As String
    baseURL = m_baseURI.Item(m_baseURI.Count)
    p = InStr(8, baseURL, "/")
    resolveUrl = Left$(baseURL, p - 1) + url
End Function

Private Property Get IParseContext_Property(ByVal key As String) As Variant
    IParseContext_Property = m_props.Value(key)
End Property

' set bForTarget, if you want the "" prefix to map to the current targetNamespace, rather than the current global NS
Private Function IParseContext_ResolveNSPrefix(ByVal bForTarget As Boolean, ByVal sPrefix As String) As String
    Dim idx As Long
    If bForTarget And sPrefix = "" Then
        IParseContext_ResolveNSPrefix = m_targetNSStack.Item(m_targetNSStack.Count)
        Exit Function
    End If
    
    For idx = m_nsstack.Count To 1 Step -1
        If sPrefix = m_nsstack.Item(idx).prefix Then
            IParseContext_ResolveNSPrefix = m_nsstack.Item(idx).uri
            Exit Function
        End If
    Next
    ' should never get here !
    Err.Raise 4322, , "Unknown prefix '" + sPrefix + "'"
End Function

Private Property Get IParseContext_targetNamespace() As String
    IParseContext_targetNamespace = m_targetNSStack.Item(m_targetNSStack.Count)
End Property

Private Sub IVBSAXContentHandler_startElement(strNamespaceURI As String, strLocalName As String, strQName As String, ByVal oAttributes As MSXML2.IVBSAXAttributes)
    Dim s As parserStackItem
    Dim tnsIdx As Integer
    tnsIdx = getIndexFromName("", "targetNamespace", oAttributes)
    If tnsIdx >= 0 Then
        m_targetNSStack.Add oAttributes.getValue(tnsIdx)
    Else
        If m_targetNSStack.Count = 0 Then
            m_targetNSStack.Add ""
        Else
            m_targetNSStack.Add m_targetNSStack.Item(m_targetNSStack.Count)
        End If
    End If
    
    ' import support
    If strNamespaceURI = URI_WSDL And strLocalName = "import" Then
        StartImportMagic oAttributes
    End If
    
    If m_stack.Count = 0 Then
        ' root'ish
        s.name = strLocalName
        s.uri = strNamespaceURI
        If strNamespaceURI = URI_SCHEMA_99 Or strNamespaceURI = URI_SCHEMA_01 Then
            If strLocalName = WSDL_SCHEMA Then
                Set s.ch = New Schema
                SetHandlerCtx s.ch, Me
                ' we also need to add attributes for xmlns:xx decl's, so they get preserved into the DOM
                Dim atr As SAXAttributes40
                Set atr = New SAXAttributes40
                Dim nsi As Variant 'nsitem
                For Each nsi In m_nsstack
                    If Len(nsi.prefix) > 0 Then
                        s.ch.startPrefixMapping nsi.prefix, nsi.uri
                        atr.addAttribute "", "xmlns:" + nsi.prefix, "xmlns:" + nsi.prefix, "", nsi.uri
                    End If
                Next
                ' remember to copy the original attributes over as well
                For tnsIdx = 0 To oAttributes.length - 1
                    atr.addAttribute oAttributes.getURI(tnsIdx), oAttributes.getLocalName(tnsIdx), oAttributes.getQName(tnsIdx), oAttributes.getType(tnsIdx), oAttributes.getValue(tnsIdx)
                Next
                s.ch.startElement strNamespaceURI, strLocalName, strQName, atr
                m_stack.Add s
            End If
        ElseIf strNamespaceURI = URI_WSDL Then
            If Not bRootChecked And strLocalName = WSDL_ROOT Then
                On Error Resume Next
                name = oAttributes.getValueFromName("", "name")
                On Error GoTo 0
                bRootChecked = True
            End If
             
            If strLocalName = WSDL_PORTTYPE Then
                Dim pt As portType
                Set pt = New portType
                Set s.ch = pt
                InitNewHandler s.ch, strNamespaceURI, strLocalName, strQName, oAttributes, Me
                m_stack.Add s
                portTypes.Add pt, pt.name.localname
                
            ElseIf strLocalName = WSDL_BINDING Then
                Dim b As binding
                Set b = New binding
                Set s.ch = b
                InitNewHandler s.ch, strNamespaceURI, strLocalName, strQName, oAttributes, Me
                m_stack.Add s
                bindings.Add b, b.name.localname
                
            ElseIf strLocalName = WSDL_PORT Then
                Dim p As port
                Set p = New port
                Set s.ch = p
                InitNewHandler s.ch, strNamespaceURI, strLocalName, strQName, oAttributes, Me
                m_stack.Add s
                ports.Add p, p.name.localname
            
            ElseIf strLocalName = WSDL_MESSAGE Then
                Dim m As message
                Set m = New message
                Set s.ch = m
                InitNewHandler s.ch, strNamespaceURI, strLocalName, strQName, oAttributes, Me
                m_stack.Add s
                messages.Add m, m.name.localname
            End If
        End If
    Else
        m_stack.Item(m_stack.Count).ch.startElement strNamespaceURI, strLocalName, strQName, oAttributes
    End If
    
    If Not bRootChecked Then
        Err.Raise vbObjectError + 1234, , "Root element should be {" + URI_WSDL + "}" + WSDL_ROOT
    End If
End Sub

Private Sub IVBSAXContentHandler_characters(strChars As String)
    If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.characters strChars
End Sub

Private Sub IVBSAXContentHandler_endElement(strNamespaceURI As String, strLocalName As String, strQName As String)
    Dim s As parserStackItem
    
    ' pop the target Namespace stack
    m_targetNSStack.Remove m_targetNSStack.Count
    
    If m_stack.Count > 0 Then
        s = m_stack.Item(m_stack.Count)
        s.ch.endElement strNamespaceURI, strLocalName, strQName
        ' this looks a little suspect, to review
        If s.uri = strNamespaceURI And s.name = strLocalName Then
            If TypeOf m_stack.Item(m_stack.Count).ch Is Schema Then
                Dim nsi As Variant 'nsItem
                For Each nsi In m_nsstack
                    If Len(nsi.prefix) > 0 Then s.ch.endPrefixMapping nsi.prefix
                Next
                Dim si As Schema
                Set si = s.ch
                m_schemaList.Add si
                'Dim d As DOMDocument40
                'Set d = si.Schema
                ' m_schemas.Add si.namespace, d
            End If
            m_stack.Remove m_stack.Count
        End If
    End If
End Sub

Private Sub IVBSAXContentHandler_startPrefixMapping(strPrefix As String, strURI As String)
    Dim ns As nsItem
    ns.prefix = strPrefix
    ns.uri = strURI
    m_nsstack.Add ns
    If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.startPrefixMapping strPrefix, strURI
End Sub

Private Sub IVBSAXContentHandler_endPrefixMapping(strPrefix As String)
    Dim idx As Long
    For idx = m_nsstack.Count To 1 Step -1
        If m_nsstack.Item(idx).prefix = strPrefix Then
            m_nsstack.Remove (idx)
            If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.endPrefixMapping strPrefix
            Exit Sub
        End If
    Next
    ' should never get here !
    Err.Raise 4321, , "endPrefixMapping called, and couldn't find prefix !"
End Sub

Private Sub IVBSAXContentHandler_ignorableWhitespace(strChars As String)
    If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.ignorableWhitespace strChars
End Sub

Private Sub IVBSAXContentHandler_processingInstruction(strTarget As String, strData As String)
    If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.processingInstruction strTarget, strData
End Sub

Private Sub IVBSAXContentHandler_skippedEntity(strName As String)
    If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.skippedEntity strName
End Sub

Private Sub IVBSAXContentHandler_startDocument()
    If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.startDocument
End Sub

Private Property Set IVBSAXContentHandler_documentLocator(ByVal RHS As MSXML2.IVBSAXLocator)
    If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.documentLocation RHS
End Property

Private Sub IVBSAXContentHandler_endDocument()
    Debug.Print "defintions::IVBSAXContentHandler_endDocument"
    If m_stack.Count > 0 Then m_stack.Item(m_stack.Count).ch.endDocument
    If m_baseURI.Count = 1 Then CreateSchemaCache
End Sub

