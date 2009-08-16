<%@ import="_common.html.xc" %>
<%
/*
This XC file is the documentation-template for a single COM coclass.  Aside 
from a basic description, it is comprised of the following:
- a table listing the interface(s) it supports
- a discussion of how to create instances of it
- a listing of any associated cotypes/categories
- links to related entities (interfaces, etc)

Multiple copies of this file will be processed by the wizard -- the input 
source var is "itmThisCoClass", and the collection is "clcCoClassItems".
*/

var xmlThisCoClass = xmlMunger.documentElement.selectSingleNode("./CoClasses/CoClass[@name='"+itmThisCoClass.idlSelf.Name+"']"); 
  //note: assume failure handled in library.html.xc

var sProgID = selectOrCreateChildElement(xmlThisCoClass,"ProgID",itmThisCoClass.idlSelf.Name,null,"progid?",input.bAutoGen).text;
var sDescription = selectOrCreateChildElement(xmlThisCoClass,"Description",itmThisCoClass.idlSelf.Name,null,"write a description for this item",input.bAutoGen).text;
var sHowToInstantiate = selectOrCreateChildElement(xmlThisCoClass,"HowToInstantiate",itmThisCoClass.idlSelf.Name,null,"discuss how to obtain instances of this coclass",input.bAutoGen).text;
var sSampleCode = selectOrCreateChildElement(xmlThisCoClass,"SampleCode",itmThisCoClass.idlSelf.Name,null,"write some sample code (typically, demonstrating component-instantiation)",input.bAutoGen).text;

function writeAttribute(idlAttribute)
  {
  if (idlAttribute.Name.substring(0,4) == "uuid") 
    { %><%= idlAttribute.Name.toLowerCase() %><br><% }
  }

function writeInterfaceLineTR(idlLine)
  {
  var xmlNode = xmlMunger.documentElement.selectSingleNode("./Interfaces/Interface[@name='"+idlLine.Name+"']/Short");
  var xmlNodeText = "" ;
  if  ( xmlNode )
          xmlNodeText = xmlNode.text ;
  //note: assume failure handled in library.html.xc
%>
<tr>
<td><a href="interface_<%= idlLine.Name %>.html"><%= idlLine.Name %></a><% xcode.write("&nbsp;&nbsp;"); %><%= (idlLine.Text.indexOf("[default]")>=0)?("[default]"):("") %></td>
<td><%= xmlNodeText %></td>
</tr>
<%
  }
%>

<html>

<% writeHtmlHeader(itmThisCoClass.idlSelf.Name); %>

<body>
  
  <h1><%= itmThisCoClass.idlSelf.Name %> CoClass</h1>
  
  <pre class="syntax"><% for_each(itmThisCoClass.idlSelf.Attributes,writeAttribute); %>progid(<%= sProgID %>)</pre>

  <p><%= sDescription %></p>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>Supported Interfaces</h4>
  <table>
  <tr>
    <th width=30%>Name</th>
    <th width=70%>Description</th>
  </tr>
  <% for_each(itmThisCoClass.idlSelf.Interfaces,writeInterfaceLineTR); %>
  </table>
  
  <p><% xcode.write("&nbsp;"); %></p>
  <h4>How to Instantiate</h4>
  <p><%= sHowToInstantiate %></p>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>Sample Code</h4>
  <pre class="code"><%= sSampleCode %></pre>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>See Also</h4>
  <a href="library_<%= itmThisCoClass.idlParent.Name %>.html">The <%= itmThisCoClass.idlParent.Name %> Library</a><% xcode.write("&nbsp;|&nbsp;"); %>
  <% writeLinks(xmlThisCoClass); %>

<% writeCopyrightP(); %>

</body>

</html>
