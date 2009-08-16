<HTML>
<!-- Sitemap 1.0 -->
<OBJECT type="text/site properties">
	<param name="WindowName" value="$pocketSOAP">
	<param name="Window Styles" value="0x800624">
</OBJECT>
<UL>
<%
function writeTocEntry(sName,sLink,nImage)
  {
%>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="<%= sName %>">
		<param name="Local" value="<%= sLink %>">
		<param name="ImageNumber" value="<%= nImage %>">
		</OBJECT>
<%
  }

function writeTocInterfaceSubentry(sIfcName)
  {
%>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="<%= sIfcName %>">
		<param name="Local" value="<%= "interface_"+sIfcName+".html" %>">
		</OBJECT>
  <UL>
  <% for_each(clcMethodItems, function(x) { if (x.idlParent.Name == sIfcName) writeTocEntry(x.idlSelf.Name,"interface_"+x.idlParent.Name+"_method_"+x.idlSelf.Name+".html",11); }); %>
  </UL>
<%
  }

function isCoClassInAltTOC(sName)
{
  if (itmThisAltTOC == input.projectName) return true;

  var xmlNodes = xmlMunger.documentElement.selectNodes("./AltTOCs/AltTOC[@name='"+itmThisAltTOC+"']/CoClasses/CoClass[@name='"+sName+"']");
  return (xmlNodes.length > 0) ? true : false;
}

function isInterfaceInAltTOC(sName)
{
  if (itmThisAltTOC == input.projectName) return true;

  var xmlNodes = xmlMunger.documentElement.selectNodes("./AltTOCs/AltTOC[@name='"+itmThisAltTOC+"']/Interfaces/Interface[@name='"+sName+"']");
  return (xmlNodes.length > 0) ? true : false;
}


writeTocEntry("Overview", "library_"+idlMunger.Library.Name+".html", 11);

%>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="CoClasses">
		</OBJECT>
  <UL>
  <% for_each(clcCoClassItems, function(x) { if (isCoClassInAltTOC(x.idlSelf.Name)) writeTocEntry(x.idlSelf.Name,"coclass_"+x.idlSelf.Name+".html",11); }); %>
  </UL>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="Interfaces">
		</OBJECT>
  <UL>
  <% for_each(clcInterfaceItems, function(x) { if (isInterfaceInAltTOC(x.idlSelf.Name)) writeTocInterfaceSubentry(x.idlSelf.Name); }); %>
  </UL>
</UL>
</HTML>
