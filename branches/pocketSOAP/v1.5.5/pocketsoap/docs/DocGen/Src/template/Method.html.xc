<%@ import="_common.html.xc" %>
<%
/*
This XC file is the documentation-template for a single COM method or property.
Aside from a basic description, it is comprised of the following:
- the syntax of the method call
- a list of parameters (name, type, direction) and description of each
- a discussion of possible return values
- sample code, for implementation and/or calling, as appropriate
- links to related stuff

Multiple copies of this file will be processed by the wizard -- the input 
source var is "itmThisMethod", and the collection is "clcMethodItems".
*/

var xmlThisMethod = xmlMunger.documentElement.selectSingleNode("./Interfaces/Interface[@name='"+itmThisMethod.idlParent.Name+"']/Methods/Method[@name='"+itmThisMethod.idlSelf.Name+"']");
  //note: assume failure handled in interface.html.xc

var sDescription = selectOrCreateChildElement(xmlThisMethod,"Description",itmThisMethod.idlSelf.Name,null,"write a description for this item",input.bAutoGen).text;
var sReturnValue = selectOrCreateChildElement(xmlThisMethod,"ReturnValue",itmThisMethod.idlSelf.Name,null,"discuss the possible return values",input.bAutoGen).text;
var sSampleCode = selectOrCreateChildElement(xmlThisMethod,"SampleCode",itmThisMethod.idlSelf.Name,null,"write some sample code",input.bAutoGen).text;

// Is this method, in fact, a property?
var bIsProperty;
var bIsReadOnly;

var idlAttributes = itmThisMethod.idlSelf.Attributes;

bIsProperty = false;
for (var i=0; i < idlAttributes.Count; ++i)
  if (idlAttributes(i).Name.substring(0,4) == "prop")
    bIsProperty = true;

// Ok, so it's a property -- but is it readonly?
if (bIsProperty)
  {
  var idlMethods = itmThisMethod.idlParent.Methods;

  bIsReadOnly = true;
  for (var i=0; i < idlMethods.Count; ++i)
    for (var j=0; j < idlMethods(i).Attributes.Count; ++j)
      if (idlMethods(i).Name == itmThisMethod.idlSelf.Name && 
          idlMethods(i).Attributes(j).Name.substring(0,7) == "propput")
        bIsReadOnly = false;
  }

function writeAttribute(idlAttribute,i,n)
  {
  xcode.write(idlAttribute.Name);
  if (i < n-1) xcode.write(", ");
  }

function writeArgPre(idlArg,i,n)
  {
  var sType = idlArg.Type;
  if (sType.charAt(0) == 'I' && sType.indexOf("*") > -1) // interface pointer! construct a link
    sType = constructLinkToInterface(idlArg.Type);

  xcode.write("[");
  for_each_x(idlArg.Attributes,writeAttribute);
  xcode.write("] " + sType + " " + idlArg.Name);
  if (i < n-1) xcode.write(", ");
  }

function writeArgD(idlArg)
  {
  var xmlNode = xmlThisMethod.selectSingleNode("./Parameters/Parameter[@name='"+idlArg.Name+"']");
  if (xmlNode == null)
    {
    var xmlContainer = selectOrCreateChildElement(xmlThisMethod,"Parameters",itmThisMethod.idlSelf.Name,null,null,input.bAutoGen);
    xmlNode = selectOrCreateChildElement(xmlContainer,"Parameter",itmThisMethod.idlSelf.Name,idlArg.Name,"describe this parameter",input.bAutoGen);
    }
  
  var sType = idlArg.Type;
  if (sType.charAt(0) == 'I' && sType.indexOf("*") > -1) // interface pointer! construct a link
    sType = constructLinkToInterface(idlArg.Type);

  // Write the name, type, attributes, and short desc
  %><dt><i><%= idlArg.Name %></i></dt><dd><%= sType %>, [<% for_each_x(idlArg.Attributes,writeAttribute); %>]: <%= xmlNode.text %></dd><% 
  }
%>

<html>

<% // Write title in the form of "IFoo::Bar" %>
<% writeHtmlHeader(itmThisMethod.idlParent.Name + "::" + itmThisMethod.idlSelf.Name); %>

<body>
  
  <h1><%= itmThisMethod.idlParent.Name + "::" + itmThisMethod.idlSelf.Name + ((bIsProperty)?((bIsReadOnly)?(" Property (read-only)"):(" Property")):(" Method")) %></h1>

<%
  if (bIsProperty)
    {
%>
  <pre class="syntax">[propget] <%= itmThisMethod.idlSelf.ReturnType %> <%= itmThisMethod.idlSelf.Name %>(<% for_each_x(itmThisMethod.idlSelf.Args,writeArgPre); %>);</pre>
<%
    if (!bIsReadOnly)
      {
      var idlSibling;

      var idlMethods = itmThisMethod.idlParent.Methods;
      for (var i=0; i < idlMethods.Count; ++i)
        for (var j=0; j < idlMethods(i).Attributes.Count; ++j)
          if (idlMethods(i).Name == itmThisMethod.idlSelf.Name && 
              idlMethods(i).Attributes(j).Name.substring(0,7) == "propput")
            idlSibling = idlMethods(i);
%>
  <pre class="syntax">[propput] <%= idlSibling.ReturnType %> <%= idlSibling.Name %>(<% for_each_x(idlSibling.Args,writeArgPre); %>);</pre>
<%
      }
    } 
  else 
    {
%>
  <pre class="syntax"><%= itmThisMethod.idlSelf.ReturnType %> <%= itmThisMethod.idlSelf.Name %>(<% for_each_x(itmThisMethod.idlSelf.Args,writeArgPre); %>);</pre>
<%
    }
%>

  <p><%= sDescription %></p>

  <%
  if (!bIsProperty && itmThisMethod.idlSelf.Args.Count > 0) 
    {
  %>
  <p><% xcode.write("&nbsp;"); %></p>
  <h4>Parameters</h4>
  <dl><% for_each(itmThisMethod.idlSelf.Args,writeArgD); %></dl>
  <%
    }
  %>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>Return Values</h4>
  <p><%= sReturnValue %></p>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>Sample Code</h4>
  <pre class="code"><%= sSampleCode %></pre>

  <p><% xcode.write("&nbsp;"); %></p>
  <h4>See Also</h4>
  <a href="interface_<%= itmThisMethod.idlParent.Name %>.html">The <%= itmThisMethod.idlParent.Name %> Interface</a><% xcode.write("&nbsp;|&nbsp;"); %>
  <% writeLinks(xmlThisMethod); %>

  <% writeCopyrightP(); %>

</body>

</html>
