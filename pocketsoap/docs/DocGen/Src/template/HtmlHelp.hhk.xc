<HTML>
<!-- Sitemap 1.0 -->
<OBJECT type="text/site properties">
	<param name="WindowName" value="$pocketSOAP">
</OBJECT>
<UL>
<%
function writeIndexEntry(sName,sLink)
  {
%>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="<%= sName %>">
		<param name="Name" value="<%= sName %>">
		<param name="Local" value="<%= sLink %>">
		</OBJECT>
<%
  }

writeIndexEntry(idlMunger.Library.Name, "library_"+idlMunger.Library.Name+".html");

for_each(clcCoClassItems, function(x) { writeIndexEntry(x.idlSelf.Name,"coclass_"+x.idlSelf.Name+".html"); });
for_each(clcInterfaceItems, function(x) { writeIndexEntry(x.idlSelf.Name,"interface_"+x.idlSelf.Name+".html"); });
for_each(clcMethodItems, function(x) { writeIndexEntry(x.idlParent.Name+"::"+x.idlSelf.Name,"interface_"+x.idlParent.Name+"_method_"+x.idlSelf.Name+".html"); });
%>
</UL>
</HTML>
