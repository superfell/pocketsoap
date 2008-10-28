VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "vb6CodeGen"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
' $Header: c:/cvs/pocketsoap/wsdl/codegen/vb6CodeGen.cls,v 1.8 2004/06/22 04:25:08 simon Exp $
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
' Portions created by Simon Fell are Copyright (C) 2002-2004
' Simon Fell. All Rights Reserved.
'
' Contributor (s):
' Anant Verma
'
Option Explicit

Implements ICodeGenTarget

Private m_def As wsdlParser.definitions
Private m_portType As wsdlParser.portType
Private m_binding As wsdlParser.binding
Private m_dir As String
Private m_wsdlUrl As String
Private m_ctx As MAPLib.MapCollection

Private m_sf As ISerializerFactory2
Private m_pf As Object  ' the current proxy file
Private m_classes As Collection ' collection of classNames that we generated
Private m_types As Collection ' cached set of xsd -> COM Types
Private m_factorySetup As Collection

Private m_prjName As String ' based on definitions -> name
Private m_rpcEncType As Boolean

Public Enum TypeStyle
    tPrimative
    tObject
End Enum
    
Public Enum ArrayType
    atNotArray
    atEncoded
    atLiteral
End Enum

Public Type comType
    name As String
    style As TypeStyle
    array As ArrayType
    xmlType As qname
    opt As Boolean
End Type

' Standard context properties we look for
' proxyserver : Generate code in the proxy to use a proxy server with this name
' proxyport   : Generate code int he proxy to use a proxy server at this port
' project     : Override the project name
' class       : Override the proxy class name

Private Sub ICodeGenTarget_Initialize(ByVal genDirectory As String, ByVal wsdlUrl As String, ByVal wsdlDef As wsdlParser.definitions, ByVal ctx As MAPLib.MapCollection)
    m_dir = genDirectory
    m_wsdlUrl = wsdlUrl
    Set m_def = wsdlDef
    Set m_ctx = ctx
    Set m_sf = New PocketSOAP.CoSerializerFactory
    Set m_classes = New Collection
    Set m_types = New Collection
    Set m_factorySetup = New Collection
    Dim newName As String
    On Error Resume Next
    newName = m_ctx.Value("project")
    On Error GoTo 0
    If Len(m_def.name) > 0 Then
        m_prjName = m_def.name
    ElseIf Len(newName) > 0 Then
        m_prjName = newName
    Else
        m_prjName = "SoapProxy"
    End If
    If Len(m_prjName) > 15 Then m_prjName = Left$(m_prjName, 15)
End Sub

Private Sub ICodeGenTarget_Finalize()
    genProjectFile
    Set m_def = Nothing
End Sub

Private Sub ICodeGenTarget_StartProxy(ByVal port As wsdlParser.port, _
                                        ByVal binding As wsdlParser.binding, _
                                        ByVal portType As wsdlParser.portType)
    Set m_portType = portType
    Set m_binding = binding
    
    Dim fso As Object, fn As String, clsName As String, tmpPortType As String
    tmpPortType = portType.name.localname
    On Error Resume Next
    portType.name.localname = m_ctx.Value("class")
    On Error GoTo 0
    clsName = VbNameBuilder(m_prjName, portType.name)
    portType.name.localname = tmpPortType
    
    m_classes.Add clsName
    fn = m_dir & clsName & ".cls"
    Set fso = CreateObject("Scripting.FileSystemObject")
    Set m_pf = fso.createTextFile(fn)
    Dim f As Object
    Set f = m_pf
    
    WriteClassHeader f, clsName, m_wsdlUrl, False
    f.writeline "private m_url as string"
    f.writeline "Private m_transport As ISOAPTransport"
    f.writeline "Private m_sf As ISerializerFactoryConfig2"
    f.writeline ""
    f.writeline "' this is a common type that the complexType custom serializers use"
    f.writeline "Public Type refItem"
    f.writeline vbTab + "href As String"
    f.writeline vbTab + "hrefNode As ISOAPNode"
    f.writeline "End Type"
    f.writeline ""
    f.writeline "Private Sub Class_Initialize()"
    f.writeline "   m_url = """ + port.SoapURL + """"
    f.writeline "   Set m_transport = New HTTPTransport"
    f.writeline "   ' add calls to setProxy / authentication, etc here"
    f.writeline "   dim t as IHttpTransportAdv"
    f.writeline "   set t= m_transport"
    Dim ps As String, pp As Integer
    pp = 8080
    On Error Resume Next
    ps = m_ctx.Value("proxyserver")
    pp = m_ctx.Value("proxyport")
    On Error GoTo 0
    If Len(ps) > 0 Then
        f.writeline "   t.SetProxy """ + ps + """, " & pp
    End If
    f.writeline ""
    f.writeline "   configureSerializerFactory"
    f.writeline "End Sub"
    f.writeline ""
    f.writeline "public property get Url() as string"
    f.writeline "    Url = m_url"
    f.writeline "end property"
    f.writeline ""
    f.writeline "public property let Url(byval newURL as string)"
    f.writeline "    m_url = newURL"
    f.writeline "end property"
    f.writeline ""
    f.writeline "Public Property Get Transport() As ISOAPTransport"
    f.writeline "   Set Transport = m_transport"
    f.writeline "End Property"
    f.writeline ""
    f.writeline "Public Property Set Transport(ByVal newTransport As ISOAPTransport)"
    f.writeline "   Set m_transport = newTransport"
    f.writeline "End Property"
    f.writeline ""
End Sub

Private Sub ICodeGenTarget_FinalizeProxy()
    m_pf.writeline ""
    m_pf.writeline "private sub configureSerializerFactory()"
    m_pf.writeline "    Set m_sf = New CoSerializerFactory"
    m_pf.writeline "    dim i as new CoInterfaceFinder"
    m_pf.writeline "   ' add calls to here alter the serializerFactory configuration"
    Dim s As Variant
    For Each s In m_factorySetup
        m_pf.writeline "    " + s
    Next
    m_pf.writeline "end sub"
    m_pf.writeline ""
    m_pf.writeline "Private Sub SendRecv(ByVal env As CoEnvelope, ByVal soapAction As String)"
    m_pf.writeline "   Dim ht As IHTTPTransport"
    m_pf.writeline "   If TypeOf m_transport Is IHTTPTransport Then"
    m_pf.writeline "       Set ht = m_transport"
    m_pf.writeline "       ht.soapAction = soapAction"
    m_pf.writeline "   End If"
    m_pf.writeline "   m_transport.Send m_url, env.Serialize"
    m_pf.writeline "   env.Parse m_transport"
    m_pf.writeline "End Sub"
    m_pf.Close
    Set m_portType = Nothing
    Set m_binding = Nothing
    Set m_pf = Nothing
End Sub

Private Sub ICodeGenTarget_Operation(ByVal opName As String)
    
    Dim o As portTypeOperation, bo As Operation
    For Each o In m_portType.operations
        If o.name = opName Then Exit For
    Next
    For Each bo In m_binding.operations
        If bo.name = opName Then Exit For
    Next
    
    ' check for doc/literal or rpc/enc
    Dim isEncoded As Boolean, isRpc As Boolean
    isEncoded = (bo.inputBody.use <> "literal")
    ' remember that rpc/document can be set at the binding level, but overriden at the operation level
    If Len(bo.style) > 0 Then
        isRpc = (bo.style = "rpc")
    Else
        isRpc = (m_binding.bindingStyle = "rpc")
    End If
    
    If isRpc And Not isEncoded Then
        Err.Raise vbObjectError + 666, , "Sorry only encoded is supported for RPC style operations"
    End If
    If Not isRpc And isEncoded Then
        Err.Raise vbObjectError + 666, , "Sorry only literal messages are support for document style operations"
    End If
    
    If isRpc Then
        GenerateRpcEncOperation opName, o, bo
    Else
        GenerateDocLitOperation opName, o, bo
    End If
    
End Sub

' generate an operation proxy for a doc/literal operation
' nothing fancy here yet, just map the input message to a complex type, and the return message to a complexType
Private Sub GenerateDocLitOperation(ByVal opName As String, ByVal o As portTypeOperation, ByVal bo As Operation)
    m_rpcEncType = False
    Dim msgIn As message, msgOut As message
    Set msgIn = m_def.findMessage(o.inputMessage.msg.localname, o.inputMessage.msg.Namespace)
    If Len(o.outputMessage.name) > 0 Then
        Set msgOut = m_def.findMessage(o.outputMessage.msg.localname, o.outputMessage.msg.Namespace)
    End If
    
    Dim reqElem As qname, resElem As qname, hasReq As Boolean, hasRes As Boolean
    hasReq = msgIn.parts.Count > 0
    hasRes = msgOut.parts.Count > 0
    
    If hasReq Then
        Set reqElem = msgIn.parts.Item(1).element
        If reqElem.localname = "" Then Err.Raise vbObjectError + 678, , "Message parts for doc/literal must use elements not types"
    End If
    If hasRes Then
        Set resElem = msgOut.parts.Item(1).element
        If resElem.localname = "" Then Err.Raise vbObjectError + 678, , "Message parts for doc/literal must use elements not types"
    End If
    
    Dim reqType As comType, resType As comType
    
    If hasReq Then reqType = GenDocLiteralType(reqElem)
    If hasRes Then resType = GenDocLiteralType(resElem)
    
    Dim f As Object
    Dim functionType As String
    Dim paramName As String
    Dim bodyparamName As String
    functionType = IIf(hasRes, "function", "sub")
    Set f = m_pf
    f.write "public " + functionType + " " + opName + "("
    If hasReq Then
        paramName = reqElem.localname
        If paramName = reqType.name Then paramName = paramName + "_"
        ' fix by averma to change paramName if it is same as opName - 10/07/2003
        If paramName = opName Then paramName = paramName + "_"
        f.write "byval " + paramName + " as " + reqType.name
        bodyparamName = paramName
    End If
    Dim hdr ' As soapHeader
    Dim hdrType As comType
    Dim hdrMsg As message
    Dim hdrPart As part
    Dim pPart As part
    Dim bWriteComma As Boolean
    Dim addHeaders As String
    bWriteComma = hasReq
    For Each hdr In bo.inputHeaders
        Set hdrMsg = m_def.findMessage(hdr.message.localname, hdr.message.Namespace)
        For Each hdrPart In hdrMsg.parts
            If hdrPart.name = hdr.part Then
                Set pPart = hdrPart
                hdrType = mapHeaderType(pPart)
                paramName = safeVBVarName(hdrPart.name)
                If bWriteComma Then
                    f.write ", "
                Else
                    bWriteComma = True
                End If
                f.write "byval " + paramName + " as " + hdrType.name
                addHeaders = addHeaders + "    if not " + paramName + " is nothing then" + vbCrLf
                If hdr.use = "encoded" Then
                    addHeaders = addHeaders + "        e.headers.create """ + hdr.part + """, " + paramName + vbCrLf
                Else
                    addHeaders = addHeaders + "        e.headers.create """ + pPart.element.localname + """, " + paramName + ", """ + pPart.element.Namespace + """" + vbCrLf
                End If
                addHeaders = addHeaders + "    end if" + vbCrLf
            End If
        Next
    Next
    f.write ")"
    If hasRes Then f.write " as " + resType.name
    f.write vbCrLf
    
    f.writeline "    dim e as coEnvelope"
    f.writeline "    set e = CreateObject(""pocketSOAP.Envelope.2"")"
    f.writeline "    e.encodingStyle = """""
    f.writeline "    set e.serializerFactory = m_sf"
    If Len(addHeaders) > 0 Then
        f.write addHeaders
    End If
    If hasReq Then
        f.writeline "    e.body.create """ + reqElem.localname + """, " + bodyparamName + ", """ + reqElem.Namespace + """"
    End If
    f.writeline "    SendRecv e, """ + bo.SOAPAction + """"
    
    ' we know the return value is an object
    If hasRes Then
        f.write "    "
        If resType.style = tObject Then f.write "set "
        f.writeline opName + " = e.body.item(0).value"
    End If
    
    f.writeline "end " + functionType
    f.writeline ""

End Sub

Private Function mapHeaderType(ByRef part As wsdlParser.part) As comType
    ' either the part has a type attribute, in which case its an rpc/enc type
    If Len(part.xmlType.localname) > 0 Then
        mapHeaderType = mapType(part.xmlType)
    Else
        ' otherwise its doc lit
        mapHeaderType = GenDocLiteralType(part.element)
    End If
End Function

Private Function GenDocLiteralType(ByVal oElem As qname) As comType
    Dim t As ISchemaType
    Dim schema As ISchema, sc As IXMLDOMSchemaCollection2, element As ISchemaElement, eType As ISchemaType
    Set sc = m_def.schemas
    Set schema = sc.getSchema(oElem.Namespace)
    Set element = schema.elements.itemByQName(oElem.localname, oElem.Namespace)
    Set eType = element.Type
    Dim qn As New qname
    ' check for an anonymous type
    If eType.namespaceURI = "" And eType.name = "" Then
        qn.Namespace = "http://anontypes.wsdl.pocketsoap.com/" + element.namespaceURI
        qn.localname = element.name
        ' look in the cached mappings collection first
        GenDocLiteralType = findCachedType(qn)
        If GenDocLiteralType.name <> "" Then Exit Function
        GenDocLiteralType = CreateComplexType(qn, eType)
    Else
        qn.Namespace = eType.namespaceURI
        qn.localname = eType.name
        GenDocLiteralType = mapType(qn)
    End If
    Dim ln As String
    ln = "m_sf.ElementMapping """ + oElem.localname + """, """ + oElem.Namespace + """, """ + qn.localname + """, """ + qn.Namespace + """"
    m_factorySetup.Add ln
End Function

Private Function isSoapEncArray(t As ISchemaType) As Boolean
    Dim bt As ISchemaType
    For Each bt In t.baseTypes
        If bt.name = "Array" And bt.namespaceURI = SOAP_11_ENCODING Then
            isSoapEncArray = True
            Exit Function
        End If
        isSoapEncArray = isSoapEncArray(bt)
        If isSoapEncArray Then Exit Function
    Next
    isSoapEncArray = False
End Function

' for the given set of parts, generate a complex type
' todo, refactor into typeBuilder and serializerBuilder classes
Private Function CreateComplexType(ByVal tQName As qname, eType As ISchemaType) As comType
       
    Dim tSerQName As New qname
    tSerQName.localname = "s_" + tQName.localname
    tSerQName.Namespace = "http://serializers.schemas.pocketsoap.com/"
    Set CreateComplexType.xmlType = tQName
    
    Dim ln As String
    
    ' is it a SOAP Section 5 array ?
    Dim isArray As Boolean
    isArray = isSoapEncArray(eType)
    
    ' ok, for arrays, we need to find the arrayType, and ensure that that type exists, then
    ' return a type of [arrayType](), we don't need to generate a wrapper class
    If isArray Then
        CreateComplexType = HandleArrayType(tQName, eType)
        Exit Function
    End If
    
    CreateComplexType.array = atNotArray
    CreateComplexType.name = VbNameBuilder(m_prjName, tQName)
    CreateComplexType.style = tObject

    ' register a cached copy of the type mapping now,
    ' this stops a boat load of issues, if there are recursive
    ' types defined.
    m_types.Add CreateComplexType, tQName.ExpandedName

    ' the complex Type writer
    Dim complexTW As complexTypeWriter
    Set complexTW = New complexTypeWriter
    complexTW.Init m_dir, CreateComplexType.name, m_wsdlUrl
    
    Dim serCls As String
    serCls = VbNameBuilder(m_prjName, tSerQName)
    
    ' the serializer Writer
    Dim serTW As serializerWriter
    Set serTW = New serializerWriter
    serTW.Init m_dir, CreateComplexType.name, serCls, tQName, m_rpcEncType, m_wsdlUrl
    
    Dim complexType As ISchemaComplexType
    Set complexType = eType
    Dim smg As ISchemaModelGroup
    Set smg = complexType.contentModel
    
    ' walk each contained particle
    Dim atr As ISchemaAttribute
    Dim ptype As qname
    Set ptype = New qname
    Dim itemType As comType
    Dim itemName As String
    
    ' Attributes
    For Each atr In complexType.Attributes
        ptype.Namespace = atr.Type.namespaceURI
        ptype.localname = atr.Type.name
        itemType = mapType(ptype)
        itemName = safeVBVarName(atr.name)
        
        complexTW.AddPropertyField itemName, itemType
        serTW.AddAttribute itemName, atr, ptype
    Next
    
    ' elements
    AddParticlesToContainer tQName, smg, complexTW, serTW
    
    If complexType.contentType = SCHEMACONTENTTYPE_TEXTONLY Then
        ' todo, think about the case where this isn't based on xsd:string
        ptype.localname = eType.baseTypes.Item(0).name
        ptype.Namespace = eType.baseTypes.Item(0).namespaceURI
        itemType = mapType(ptype)
        
        complexTW.AddPropertyField "Value", itemType
        serTW.AddContent "Value"
    End If

    ' add base type info
    AddBaseTypeInfo tQName, complexType, complexTW, serTW
    
    ' finish generating the class & serializer
    serTW.Complete
    complexTW.Complete
    
    ' register the class/serializer mappings
    Dim SerProgID As String, clsProgID As String
    SerProgID = m_prjName + "." + serCls
    clsProgID = m_prjName + "." + CreateComplexType.name
    ln = "m_sf.Serializer i.DefaultIID(""" + clsProgID + """), """ + eType.name + """, """ + eType.namespaceURI + """, """ + SerProgID + """"
    m_factorySetup.Add ln
    ln = "m_sf.Deserializer """ + tQName.localname + """, """ + tQName.Namespace + """, False, """ + clsProgID + """, """ + SerProgID + """"
    m_factorySetup.Add ln
    
    ' add the generated classes to the project
    m_classes.Add CreateComplexType.name
    m_classes.Add serCls
    
    ' create the type hierarchy
    CheckAndCreateTypeHierarchy tQName, eType
End Function

' this function calls the complexTypeWriter telling it about all the base types and fields that this type is inheriting from
Function AddBaseTypeInfo(tQName As qname, complexType As ISchemaComplexType, complexTW As complexTypeWriter, serTW As serializerWriter)

    Dim baseType As ISchemaType
    Dim baseCT As ISchemaComplexType
    Dim particle As ISchemaParticle
    Dim attr As ISchemaAttribute
    Dim baseQName As qname, cQName As qname
    Dim baseComType As comType, cComType As comType
    Dim itemName As String
    
    Set baseQName = New qname
    Set cQName = New qname
    
    For Each baseType In complexType.baseTypes
        If TypeOf baseType Is ISchemaComplexType Then
            Set baseCT = baseType
            baseQName.localname = baseCT.name
            baseQName.Namespace = baseCT.namespaceURI
            baseComType = mapType(baseQName)
            complexTW.AddBaseType baseQName, baseComType
            serTW.AddBaseType baseQName, baseComType
            
            AddBaseTypeParticles baseCT.contentModel, complexTW, baseComType
            
            For Each attr In baseCT.Attributes
                cQName.localname = attr.Type.name
                cQName.Namespace = attr.Type.namespaceURI
                cComType = mapType(cQName)
                itemName = safeVBVarName(attr.name)
                complexTW.AddBaseTypeProperty baseComType, itemName, cComType
            Next
            
            ' add nested base types
            AddBaseTypeInfo baseQName, baseCT, complexTW, serTW
        End If
    Next
End Function

Function AddBaseTypeParticles(smg As ISchemaModelGroup, complexTW As complexTypeWriter, baseComType As comType)
    Dim particle As ISchemaParticle
    Dim cComType As comType
    Dim itemName As String
    For Each particle In smg.particles
        If particle.itemType = SOMITEM_ALL Or particle.itemType = SOMITEM_SEQUENCE Then
            AddBaseTypeParticles particle, complexTW, baseComType
            
        ElseIf particle.itemType = SOMITEM_ELEMENT Then
            cComType = BuildComTypeFromElementParticle(particle)
            itemName = safeVBVarName(particle.name)
            complexTW.AddBaseTypeProperty baseComType, itemName, cComType
        End If
    Next
End Function

' this processes all the particles in the modelgroup and adds them to the type and serializer
' tQName, qName of containing XML type
' smg the ModelGroup containing the particles to add
' complexTW the destination complex Type
' setTW the destination serializer class for the complex type
Function AddParticlesToContainer(tQName As qname, smg As ISchemaModelGroup, complexTW As complexTypeWriter, serTW As serializerWriter)
    Dim e As ISchemaElement
    Dim particle As ISchemaParticle
    Dim itemType As comType
    Dim itemName As String
    Dim ln As String
    For Each particle In smg.particles
        If particle.itemType = SOMITEM_ALL Or particle.itemType = SOMITEM_SEQUENCE Then
            Dim mg As ISchemaModelGroup
            Set mg = particle
            AddParticlesToContainer tQName, mg, complexTW, serTW
            
        ElseIf particle.itemType = SOMITEM_ELEMENT Then
            Set e = particle
            itemType = BuildComTypeFromElementParticle(particle)
            itemName = safeVBVarName(particle.name)
            
            complexTW.AddPropertyField itemName, itemType
            ' serialization code
            serTW.AddElement itemName, itemType, particle
            ' type mappings
            ln = "m_sf.LocalTypeMapping """ + tQName.localname + """, """ + tQName.Namespace + """, """ + e.name + """, """ + e.namespaceURI + """, """ + e.Type.name + """, """ + e.Type.namespaceURI + """"
            m_factorySetup.Add ln
        End If
    Next
End Function

Function BuildComTypeFromElementParticle(particle As ISchemaParticle) As comType
    Dim e As ISchemaElement
    Set e = particle
    Dim ptype As New qname
    ptype.localname = e.Type.name
    ptype.Namespace = e.Type.namespaceURI
    Dim itemType As comType
    itemType = mapType(ptype)
    If particle.maxOccurs = 1 And particle.minOccurs = 0 Then
        itemType.opt = True
    ElseIf particle.maxOccurs <> 1 Or particle.minOccurs <> 1 Then
        itemType.array = atLiteral
    ' fix to support nillable clause - averma - 10/07/2003
    ElseIf particle.itemType = SOMITEM_ELEMENT Then
        If e.isNillable Then
            itemType.opt = True
        End If
    End If
    BuildComTypeFromElementParticle = itemType
End Function

' given a particular complex type (tQName/eType) see if there are any types derived from it
' and generate classes/serializers for the derived types as well
Function CheckAndCreateTypeHierarchy(tQName As qname, eType As ISchemaType)
    ' for each schema we know about, see if there are any types derived from us
    Dim idx As Integer
    Dim sc As IXMLDOMSchemaCollection2
    Set sc = m_def.schemas
    For idx = 0 To sc.length - 1
        ' we know that none of the types in the soap encoding schema derive from any custom type
        ' so we can always skip that one
        If sc.namespaceURI(idx) <> SOAP_11_ENCODING Then
            CheckAndCreateTypeHierarchyForSchema tQName, eType, sc.getSchema(sc.namespaceURI(idx))
        End If
    Next
End Function

' look at all the complex types in this schema, and generate new types for any that derive from the tQName type
Function CheckAndCreateTypeHierarchyForSchema(tQName As qname, eType As ISchemaType, schema As ISchema)
    Dim si As ISchemaItem
    Dim ct As ISchemaComplexType
    Dim bt As ISchemaItem
    Dim idx As Integer
    For Each si In schema.types
        If si.itemType = SOMITEM_COMPLEXTYPE Then
            Set ct = si
            For idx = 0 To ct.baseTypes.length - 1
                Set bt = ct.baseTypes.Item(idx)
                If bt.name = tQName.localname And bt.namespaceURI = tQName.Namespace Then
                    Dim qn As New qname
                    qn.localname = si.name
                    qn.Namespace = si.namespaceURI
                    mapType qn
                End If
            Next
        End If
    Next
End Function


' generate an operation proxy for a RPC/Encoded operation
Private Sub GenerateRpcEncOperation(ByVal opName As String, ByVal o As portTypeOperation, ByVal bo As Operation)
    m_rpcEncType = True
    Dim msgIn As message, msgOut As message
    Set msgIn = m_def.findMessage(o.inputMessage.msg.localname, o.inputMessage.msg.Namespace)
    If Len(o.outputMessage.name) > 0 Then
        Set msgOut = m_def.findMessage(o.outputMessage.msg.localname, o.outputMessage.msg.Namespace)
    End If
    
    ' do we generate a sub or a function ?
    Dim functionType As String
    functionType = "sub"
    If Not msgOut Is Nothing Then
        If msgOut.parts.Count > 0 Then functionType = "function"
    End If
    'functionType = IIf(msgOut.parts.Count > 0, "function", "sub")
    
    ' serializerFactory setup goo
    Dim ln As String
    ln = "m_sf.ElementMapping """ + opName + """, """ + bo.inputBody.Namespace + """, """ + opName + """, """ + bo.inputBody.Namespace + """"
    m_factorySetup.Add ln
    ln = "m_sf.ElementMapping """ + opName + "Response"", """ + bo.outputBody.Namespace + """, """ + opName + "Response"", """ + bo.outputBody.Namespace + """"
    m_factorySetup.Add ln
    
    Dim f As Object
    Set f = m_pf
    f.write "public " + functionType + " " + opName + "("
    Dim p As wsdlParser.part, bFirst As Boolean, vbType As comType, retType As comType
    bFirst = True
    ' todo: need to cope with out & in/out params
    For Each p In msgIn.parts
        If Not bFirst Then f.write ", " Else bFirst = False
        vbType = mapType(p.xmlType)
        If vbType.array Then
            f.write "byref " & safeVBVarName(p.name) & "()"
        Else
            f.write "byval " & safeVBVarName(p.name)
        End If
        If Len(vbType.name) > 0 Then f.write " as " & vbType.name
        ' add type mappings
        ln = "m_sf.LocalTypeMapping """ + opName + """, """ + bo.inputBody.Namespace + """,""" + p.name + """, """", """ + vbType.xmlType.localname + """,""" + vbType.xmlType.Namespace + """"
        m_factorySetup.Add ln
    Next
    ' headers
    Dim hdr ' As soapHeader
    Dim hdrType As comType
    Dim hdrMsg As message
    Dim hdrPart 'As part
    Dim pPart As part
    Dim bWriteComma As Boolean
    Dim addHeaders As String
    Dim paramName As String
    bWriteComma = Not bFirst
    For Each hdr In bo.inputHeaders
        Set hdrMsg = m_def.findMessage(hdr.message.localname, hdr.message.Namespace)
        For Each hdrPart In hdrMsg.parts
            If hdrPart.name = hdr.part Then
                Set pPart = hdrPart
                hdrType = mapHeaderType(pPart)
                paramName = safeVBVarName(hdrPart.name)
                If bWriteComma Then
                    f.write ", "
                Else
                    bWriteComma = True
                End If
                f.write "byval " + paramName + " as " + hdrType.name
                addHeaders = addHeaders + "    if not " + paramName + " is nothing then" + vbCrLf
                If hdr.use = "encoded" Then
                    addHeaders = addHeaders + "        e.headers.create """ + hdr.part + """, " + paramName + vbCrLf
                Else
                    addHeaders = addHeaders + "        e.headers.create """ + pPart.element.localname + """, " + paramName + ", """ + pPart.element.Namespace + """" + vbCrLf
                End If
                addHeaders = addHeaders + "    end if" + vbCrLf
            End If
        Next
    Next

    f.write ")"
    If Not msgOut Is Nothing Then
        If msgOut.parts.Count > 0 Then
            Set p = msgOut.parts.Item(1)
            retType = mapType(p.xmlType)
            If Len(retType.name) > 0 Then f.write " as " & retType.name
            If retType.array Then f.write "()"
            ln = "m_sf.LocalTypeMapping """ + opName + "Response"", """ + bo.outputBody.Namespace + """,""" + p.name + ""","""",""" + p.xmlType.localname + """,""" + p.xmlType.Namespace + """"
            m_factorySetup.Add ln
        End If
    End If
    f.write vbCrLf
    
    f.writeline "    dim e as coEnvelope"
    f.writeline "    set e = CreateObject(""pocketSOAP.Envelope.2"")"
    f.writeline "    set e.serializerFactory = m_sf"
    If Len(addHeaders) > 0 Then
        f.write addHeaders
    End If
    f.writeline "    e.SetMethod """ + opName + """,""" + bo.inputBody.Namespace & """"
    For Each p In msgIn.parts
        f.writeline "    e.Parameters.Create """ & p.name & """, " + safeVBVarName(p.name)
    Next
    f.writeline "    SendRecv e, """ + bo.SOAPAction + """"
    ' build the return parameter/value stuff
    If Len(retType.name) > 0 Then
        ' we know the return value is an object
        f.write "    "
        If retType.style = tObject Then f.write "set "
        ' we know the return value is a simple type
        Dim rt As qname
        Set rt = msgOut.parts.Item(1).xmlType
        If retType.array = atNotArray Then
            f.writeline opName + " = e.parameters.item(0).valueas(""" + rt.localname + """,""" + rt.Namespace + """)"
        Else
            f.writeline opName + " = e.parameters.item(0).value"
        End If
    End If
    f.writeline "end " + functionType
    f.writeline ""
End Sub

Private Function vbNameFromVT(ByVal t As Long) As String
    Dim r As String
    r = ""
    Select Case t
        Case vbString: r = "string"
        Case vbByte: r = "byte"
        Case vbInteger: r = "Integer"
        Case vbLong: r = "Long"
        Case vbSingle: r = "Single"
        Case vbDouble: r = "Double"
        Case vbCurrency: r = "Currency"
        Case vbDate: r = "Date"
        Case vbBoolean: r = "Boolean"
        Case vbDecimal: r = "Variant"   ' VB only supports decimal in a variant
    End Select
    vbNameFromVT = r
End Function

Private Function mapType(ByVal xsdType As wsdlParser.qname) As comType
    Dim ct
    mapType.array = False
    Set mapType.xmlType = xsdType
    On Error GoTo notfound
    ' fix by averma for supporting xsd:anyType - 10/07/2003
    If m_sf.IsAnyType(xsdType.localname, xsdType.Namespace) Then
        mapType.name = "Variant"
        mapType.style = tPrimative
        Exit Function
    End If
    ct = m_sf.FindComType(xsdType.localname, xsdType.Namespace)
    If IsNumeric(ct) Then
        If ((ct And vbArray) > 0) Then
            mapType.array = atEncoded
            mapType.name = vbNameFromVT(ct And (Not vbArray))
        Else
            mapType.name = vbNameFromVT(ct)
        End If
        mapType.style = tPrimative
        Exit Function
    End If
    
notfound:
    ' look in the cached mappings collection first
    mapType = findCachedType(xsdType)
    If mapType.name <> "" Then Exit Function
    
    ' look in the schema cache for a schema defn
    Dim schema As ISchema
    Err.Clear
    Set schema = SchemaForType(xsdType)
    
    ' find the type in the schema
    Dim oType As ISchemaType
    Set oType = schema.types.itemByQName(xsdType.localname, xsdType.Namespace)
    If TypeOf oType Is ISchemaComplexType Then
        mapType = CreateComplexType(xsdType, oType)
    ElseIf oType.enumeration.length > 0 Then
        mapType = CreateEnumType(xsdType, oType)
    ElseIf oType.itemType = SOMITEM_SIMPLETYPE And oType.derivedBy = SCHEMADERIVATIONMETHOD_RESTRICTION Then
        mapType = CreateRestrictedSimpleType(xsdType, oType)
    Else
        Err.Raise vbObjectError + 6543, , "Sorry, this type " + xsdType.ExpandedName + " is not yet supported"
    End If
End Function

' note that sc.getSchema(someNS) fails when xs:import is used, as imported types will appear in the
' schema that imported them, not in the original schmea (which isn't in the schema collection)
Private Function SchemaForType(qn As qname) As ISchema
    Dim sc As IXMLDOMSchemaCollection2
    Dim s As ISchema
    Dim t As ISchemaType
    Set sc = m_def.schemas
    Dim idx As Integer
    For idx = 0 To sc.length - 1
        Set s = sc.getSchema(sc.namespaceURI(idx))
        For Each t In s.types
            If t.name = qn.localname And t.namespaceURI = qn.Namespace Then
                Set SchemaForType = s
                Exit Function
            End If
        Next
    Next
    Err.Raise 987, , "Unable to find a schmea definition for " + qn.ExpandedName
End Function

Private Function CreateRestrictedSimpleType(ByVal xsdType As qname, ByVal oType As ISchemaType) As comType
    ' just thunk to whatever type we're derivered from
    Dim derivedType As ISchemaType
    Set derivedType = oType.baseTypes.Item(0)
    Dim dtQN As New qname
    dtQN.Namespace = derivedType.namespaceURI
    dtQN.localname = derivedType.name
    Dim ct As comType
    ct = mapType(dtQN)
    Set ct.xmlType = xsdType
    
    ' add the (de)/serializer reference
    Dim vt
    vt = m_sf.FindComType(dtQN.localname, dtQN.Namespace)
    m_factorySetup.Add "m_sf.serializer " & vt & ",""" + xsdType.localname + """, """ + xsdType.Namespace + """, """ + simpleSerName(vt) + """"
    m_factorySetup.Add "m_sf.deserializer " + """" + xsdType.localname + """, """ + xsdType.Namespace + """, false, " & vt & ", """ + simpleSerName(vt) + """"
    
    m_types.Add ct, xsdType.ExpandedName
    CreateRestrictedSimpleType = ct
End Function

' given a VT of a simple type, return the progId of the serializer for that type
' this really should be calling the instance of the serializerfactory to get this
' info, but although it knows this info, there's no call available to return it.
' will rev in the next release of pocketsoap
Private Function simpleSerName(vt As Variant) As String
    Dim r As String
    r = "PocketSOAP.SimpleSerializer"
    Select Case vt
        Case vbDate: r = "PocketSOAP.SerializerDate"
        Case vbBoolean: r = "PocketSOAP.SerializerBoolean"
    End Select
    simpleSerName = r
End Function

Private Function CreateEnumType(ByVal xsdType As qname, ByVal oType As ISchemaType) As comType
    ' create the type defn' and the serializer
    Dim esw As EnumSerializerWriter
    Set esw = New EnumSerializerWriter
    Dim enumName As String
    Dim serName As String
    enumName = VbNameBuilder(m_prjName, xsdType)
    serName = "s_" + enumName
    esw.Init m_dir, serName, False, m_wsdlUrl
    esw.AddEnum m_prjName, xsdType, oType
    esw.Complete
    m_classes.Add serName
    
    ' add the serializer/de-serializer mappings
    m_factorySetup.Add "m_sf.Serializer vbLong, """ + xsdType.localname + """, """ + xsdType.Namespace + """, """ + m_prjName + "." + serName + """"
    m_factorySetup.Add "m_sf.Deserializer """ + xsdType.localname + """, """ + xsdType.Namespace + """, False, vbLong, """ + m_prjName + "." + serName + """"
    
    CreateEnumType.array = atNotArray
    CreateEnumType.name = enumName
    CreateEnumType.style = tPrimative
    Set CreateEnumType.xmlType = xsdType
    
    m_types.Add CreateEnumType, xsdType.ExpandedName
End Function

' arrays
Private Function HandleArrayType(ByVal xsdType As qname, oType As ISchemaType) As comType
    Dim retType As comType
    Dim smg As ISchemaModelGroup
    Dim ct As ISchemaComplexType
    Set ct = oType
    ' look for the arrayType attribute
    Dim encArrayType As ISchemaItem
    Set encArrayType = ct.Attributes.itemByQName("arrayType", SOAP_11_ENCODING)
    Dim wsdlArrayType As String
    Dim idx As Integer
    For idx = 0 To encArrayType.unhandledAttributes.length - 1
        If encArrayType.unhandledAttributes.getLocalName(idx) = "arrayType" Then
            If encArrayType.unhandledAttributes.getURI(idx) = WSDL_URI Then
                wsdlArrayType = encArrayType.unhandledAttributes.getValue(idx)
                Exit For
            End If
        End If
    Next
    If wsdlArrayType = "" Then Err.Raise vbObjectError + 4433, , "SOAP-ENC:Array derivation must include wsdl:arrayType attribute"
    
    Dim itemType As comType
    Dim qn As New qname
    Dim p As Integer
    p = InStrRev(wsdlArrayType, ":")
    qn.Namespace = Left$(wsdlArrayType, p - 1)
    Dim q As Integer
    q = InStr(p, wsdlArrayType, "[")
    qn.localname = Mid$(wsdlArrayType, p + 1, q - p - 1)
    itemType = mapType(qn)
    
    retType.array = True
    retType.name = itemType.name
    retType.style = tPrimative
    Set retType.xmlType = qn
    
    m_types.Add retType, xsdType.ExpandedName
    Dim sfc As String
    If itemType.style = tObject Then
        sfc = "m_sf.Deserializer """ + qn.localname + """, """ + qn.Namespace + """, true, "
        sfc = sfc + "vbObject, "
        sfc = sfc + """pocketSOAP.ArrayDeserializer.1"""
        m_factorySetup.Add sfc
        sfc = "m_sf.Serializer vbObject + vbArray,""" + qn.localname + """,""" + qn.Namespace + """,""PocketSOAP.ArraySerializer.1"""
        m_factorySetup.Add sfc
    End If
    HandleArrayType = retType
End Function

Private Function findCachedType(ByVal xsdType As qname) As comType
    On Error GoTo notfound
    findCachedType = m_types.Item(xsdType.ExpandedName)
    Exit Function
    
notfound:
    findCachedType.name = ""
End Function

Private Sub genProjectFile()
    Dim fso As Object, p As Object
    Set fso = CreateObject("Scripting.FileSystemObject")
    Set p = fso.createTextFile(m_dir & m_prjName & ".vbp")

    p.writeline "Type=OleDll"
    p.writeline "Reference=*\G{00020430-0000-0000-C000-000000000046}#2.0#0#C:\WINNT\System32\stdole2.tlb#OLE Automation"
    p.writeline "Reference=*\G{1259E267-74E3-4D78-B08E-DB9F9F785E04}#1.0#0#C:\Program Files\SimonFell\PocketSOAP1.3\pSOAP32.dll#PocketSOAP 1.2.9.1 Type Library"
    p.writeline "Reference=*\G{87DFB82A-6F3E-4A15-803B-73A1154D1A7C}#1.0#0#C:\Program Files\SimonFell\PocketSOAP1.3\psProxy.dll#PocketSOAP SOAPProxy 1.2 Type Library"
    
    Dim c As Variant
    For Each c In m_classes
        p.writeline "Class=" & c & "; " & c & ".cls"
    Next
    p.writeline "Module=modCommon; modCommon.bas"
    
    p.writeline "Startup=""(None)"""
    p.writeline "Command32="""""
    p.writeline "Name=""" + m_prjName + """"
    p.writeline "NoControlUpgrade=1"
    p.writeline "HelpContextID=""0"""
    p.writeline "Description=""" + IIf(Len(m_def.name) > 0, m_def.name, m_prjName) + " Proxy Class. Generated by the PocketSOAP WsdlWizard"""
    p.writeline "CompatibleMode=""1"""
    p.writeline "MajorVer=1"
    p.writeline "MinorVer=0"
    p.writeline "RevisionVer=0"
    p.writeline "AutoIncrementVer=1"
    p.writeline "ServerSupportFiles=0"
    p.writeline "DllBaseAddress=&H51000000"
    p.writeline "CompilationType=0"
    p.writeline "OptimizationType=1"
    p.writeline "FavorPentiumPro(tm)=-1"
    p.writeline "CodeViewDebugInfo=0"
    p.writeline "NoAliasing=0"
    p.writeline "BoundsCheck=0"
    p.writeline "OverflowCheck=0"
    p.writeline "FlPointCheck=0"
    p.writeline "FDIVCheck=0"
    p.writeline "UnroundedFP=0"
    p.writeline "StartMode=1"
    p.writeline "Unattended=-1"
    p.writeline "Retained=1"
    p.writeline "ThreadPerObject=0"
    p.writeline "MaxNumberOfThreads=1"
    p.writeline "ThreadingModel=1"
    p.writeline ""
    p.writeline "[MS Transaction Server]"
    p.writeline "AutoRefresh=1"
    p.Close
    
    genHelperModule
End Sub

Private Sub genHelperModule()
    Dim fso As Object, p As Object
    Set fso = CreateObject("scripting.filesystemObject")
    Set p = fso.createTextFile(m_dir & "modCommon.bas")
    
    p.writeline "Attribute VB_Name = ""modCommon"""
    p.writeline "Option Explicit"
    p.writeline ""
    p.writeline "Public Function ArrayIsValid(v) As Boolean"
    p.writeline "   On Error GoTo noBounds"
    p.writeline "   Dim ix As Long"
    p.writeline "   ix = LBound(v)"
    p.writeline "   ArrayIsValid = True"
    p.writeline "   Exit Function"
    p.writeline ""
    p.writeline "noBounds:"
    p.writeline "   ArrayIsValid = False"
    p.writeline "End Function"
    p.Close
End Sub
