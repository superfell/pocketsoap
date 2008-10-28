<%@ import="_common.html.xc" %>
<%
/*
This XC file is the documentation-template for an entire COM TypeLib.  Aside 
from a basic description, it is comprised of links to the top-level coclasses 
and interfaces contained therein.
*/

var idlThisLibrary = idlMunger.Library;
var xmlThisLibrary = xmlMunger.documentElement;

var sDescription = selectOrCreateChildElement(xmlThisLibrary,"Description",idlThisLibrary.Name,null,"write a description for this item",input.bAutoGen).text;

var sHelpString;
for_each(idlThisLibrary.Attributes,function foo(idl) { if (idl.Name.substring(0,10) == "helpstring") sHelpString = idl.Name; })
sHelpString = sHelpString.substring(sHelpString.indexOf('"')+1,sHelpString.lastIndexOf('"'));
sHelpString = makeSafeForHtml(sHelpString);

function writeAttribute(idlAttribute)
  {
  if (idlAttribute.Name.substring(0,7) == "version" || idlAttribute.Name.substring(0,4) == "uuid") 
    { %><%= idlAttribute.Name.toLowerCase() %><br><% }
  }

function writeCoClassTR(itm)
  {
  var xmlShort = xmlThisLibrary.selectSingleNode("./CoClasses/CoClass[@name='"+itm.idlSelf.Name+"']/Short");
  if (xmlShort == null)
    {
    var xmlContainer = selectOrCreateChildElement(xmlThisLibrary,"CoClasses",idlThisLibrary.Name,null,null,input.bAutoGen);
    var xmlChild = selectOrCreateChildElement(xmlContainer,"CoClass",null,itm.idlSelf.Name,null,input.bAutoGen);
    xmlShort = selectOrCreateChildElement(xmlChild,"Short",itm.idlSelf.Name,null,"write a short description",input.bAutoGen);

    selectOrCreateChildElement(xmlChild,"Links",null,null,null,true);
    }

  %>
  <tr>
  <td><a href="coclass_<%= itm.idlSelf.Name %>.html"><%= itm.idlSelf.Name %></a></td>
  <td><%= xmlShort.text %></td>
  </tr>
  <%
  }

function writeInterfaceTR(itm)
  {
  var xmlShort = xmlThisLibrary.selectSingleNode("./Interfaces/Interface[@name='"+itm.idlSelf.Name+"']/Short");
  if (xmlShort == null)
    {
    var xmlContainer = selectOrCreateChildElement(xmlThisLibrary,"Interfaces",idlThisLibrary.Name,null,null,input.bAutoGen);
    var xmlChild = selectOrCreateChildElement(xmlContainer,"Interface",null,itm.idlSelf.Name,null,input.bAutoGen);
    xmlShort = selectOrCreateChildElement(xmlChild,"Short",itm.idlSelf.Name,null,"write a short description",input.bAutoGen);

    selectOrCreateChildElement(xmlChild,"Links",null,null,null,true);
    selectOrCreateChildElement(xmlChild,"Methods",null,null,null,true);
    }

  %>
  <tr>
  <td><a href="interface_<%= itm.idlSelf.Name %>.html"><%= itm.idlSelf.Name %></a></td>
  <td><%= xmlShort.text %></td>
  </tr>
  <%
  }
%>
<html>

<% writeHtmlHeader(sHelpString); %>

<body>

  <h1><%= sHelpString %></h1>
  
  <pre class="syntax"><%= idlThisLibrary.Name %><br><% for_each(idlThisLibrary.Attributes,writeAttribute); %></pre>

  <p><%= sDescription %></p>
  
  <% if (clcCoClassItems.Count > 0) { %>
  <p><% xcode.write("&nbsp;"); %></p>
  <h3>CoClasses</h3>
  <p>The following coclasses are defined in this library:</p>
  <table>
  <tr>
    <th width=30%>Name</th>
    <th width=70%>Description</th>
  </tr>
  <% for_each(clcCoClassItems,writeCoClassTR); %>
  </table>
  <% } %>
  
  <% if (clcInterfaceItems.Count > 0) { %>
  <p><% xcode.write("&nbsp;"); %></p>
  <h3>Interfaces</h3>
  <p>The following interfaces are defined in this library:</p>
  <table>
  <tr>
    <th width=30%>Name</th>
    <th width=70%>Description</th>
  </tr>
  <% for_each(clcInterfaceItems,writeInterfaceTR); %>
  </table>
  <% } %>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>See Also</h4>
  <% writeLinks(xmlThisLibrary); %>

  <% writeCopyrightP(); %>

</body>

</html>
