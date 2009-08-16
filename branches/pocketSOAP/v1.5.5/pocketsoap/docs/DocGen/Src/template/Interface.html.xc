<%@ import="_common.html.xc" %>
<%
/*
This XC file is the documentation-template for a single COM interface.  Aside 
from a basic description, it is comprised of the following:
- a table describing its constituent methods, in vtable order
- a discussion of when to implement, vs when to call
- a listing of any associated cotypes/categories
- links to related entities (methods, coclasses, etc)

Multiple copies of this file will be processed by the wizard -- the input 
source var is "itmThisInterface", and the collection is "clcInterfaceItems".
*/

var xmlThisInterface = xmlMunger.documentElement.selectSingleNode("./Interfaces/Interface[@name='"+itmThisInterface.idlSelf.Name+"']");
  //note: assume failure handled in library.html.xc

var sDescription = selectOrCreateChildElement(xmlThisInterface,"Description",itmThisInterface.idlSelf.Name,null,"write a description for this item",input.bAutoGen).text;
var sWhenToImpl = selectOrCreateChildElement(xmlThisInterface,"WhenToImpl",itmThisInterface.idlSelf.Name,null,"discuss when to implement this interface",input.bAutoGen).text;
var sWhenToCall = selectOrCreateChildElement(xmlThisInterface,"WhenToCall",itmThisInterface.idlSelf.Name,null,"discuss when to call this interface",input.bAutoGen).text;

function writeAttribute(idlAttribute)
  {
  if (idlAttribute.Name.substring(0,4) == "uuid") 
    { %><%= idlAttribute.Name.toLowerCase() %><br><% }
  }

function writeMethodTR(idlMethod)
  {
  var xmlShort = xmlThisInterface.selectSingleNode("./Methods/Method[@name='"+idlMethod.Name+"']/Short");
  if (xmlShort == null)
    {
    var xmlContainer = selectOrCreateChildElement(xmlThisInterface,"Methods",itmThisInterface.idlSelf.Name,null,null,input.bAutoGen);
    var xmlChild = selectOrCreateChildElement(xmlContainer,"Method",null,idlMethod.Name,null,input.bAutoGen);
    xmlShort = selectOrCreateChildElement(xmlChild,"Short",idlMethod.Name,null,"write a short description",input.bAutoGen);

    selectOrCreateChildElement(xmlChild,"Links",null,null,null,true);
    selectOrCreateChildElement(xmlChild,"Parameters",null,null,null,true);
    }
  
  // So, what is this thing, exactly?
  var sType = "[method]"; // assume method until proven property
  for (var i = 0; i < idlMethod.Attributes.Count; ++i)
    {
    if (idlMethod.Attributes(i).Name == "propget") sType = "[propget]";
    else if (idlMethod.Attributes(i).Name == "propput") sType = "[propput]";
    }

  // Write the name, type, and short desc (omit desc for propput -- it'll be same as for the propget)
  %>
  <tr>
  <td><a href="interface_<%= itmThisInterface.idlSelf.Name %>_method_<%= idlMethod.Name %>.html"><%= idlMethod.Name %></a><% xcode.write("&nbsp;&nbsp;"); %><%= (sType!="[method]")?(sType):("") %></td>
  <td><%= (sType!="[propput]")?(xmlShort.text):("") %></td>
  </tr>
  <%
  }
%>

<html>

<% writeHtmlHeader(itmThisInterface.idlSelf.Name); %>

<body>
  
  <h1><%= itmThisInterface.idlSelf.Name %> Interface</h1>

  <pre class="syntax"><% for_each(itmThisInterface.idlSelf.Attributes,writeAttribute); %></pre>
  
  <p><%= sDescription %></p>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>Base Interface</h4>
  <p><%= constructLinkToInterface(itmThisInterface.idlSelf.BaseName) %></p>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>Methods, in vtable order (after base interface)</h4>
  <table>
  <tr>
    <th width=30%>Name</th>
    <th width=70%>Description</th>
  </tr>
  <% for_each(itmThisInterface.idlSelf.Methods,writeMethodTR); %>
  </table>
  
  <p><% xcode.write("&nbsp;"); %></p>
  <h4>When to Implement</h4>
  <p><%= sWhenToImpl %></p>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>When to Call</h4>
  <p><%= sWhenToCall %></p>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>See Also</h4>
  <a href="library_<%= itmThisInterface.idlParent.Name %>.html">The <%= itmThisInterface.idlParent.Name %> Library</a><% xcode.write("&nbsp;|&nbsp;"); %>
  <% writeLinks(xmlThisInterface); %>

<% writeCopyrightP(); %>

</body>

</html>
