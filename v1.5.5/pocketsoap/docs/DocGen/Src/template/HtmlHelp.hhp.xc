[OPTIONS]
Compatibility=1.1 or later
Compiled file=<%= input.projectName %>.chm
Contents file=<%= input.projectName %>.hhc
Default Window=$pocketSOAP
Default topic=library_<%= idlMunger.Library.Name %>.html
Display compile progress=No
Full-text search=Yes
Index file=<%= input.projectName %>.hhk
Language=0x409 English (United States)

[WINDOWS]
$pocketSOAP="PocketSOAP 1.3 Help","<%= input.projectName %>.hhc","<%= input.projectName %>.hhk","library_<%= idlMunger.Library.Name %>.html","library_<%= idlMunger.Library.Name %>.html",,,,,0xa0520,,0x383e,[50,50,750,550],,,,,,,0

[FILES]
library_<%= idlMunger.Library.Name %>.html
<%
for_each(clcCoClassItems, function(x) { xcode.write("coclass_"+x.idlSelf.Name+".html\r\n"); });
for_each(clcInterfaceItems, function(x) { xcode.write("interface_"+x.idlSelf.Name+".html\r\n"); });
for_each(clcMethodItems, function(x) { xcode.write("interface_"+x.idlParent.Name+"_method_"+x.idlSelf.Name+".html\r\n"); });
for_each(clcAltTOCs, function(s) { if (s != input.projectName) xcode.write(s+".hhc\r\n"); });
%>
