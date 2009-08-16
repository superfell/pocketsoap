<%
var sGenX = "Gen&lt;X&gt;"; // useful, to avoid writing Gen&lt;X&gt; everywhere  :/

function writeHtmlHeader(sTitle) 
  { 
%>
<head>
   <title><%= sTitle %></title>
   <link rel="stylesheet" href="styles.css">
   <meta name="generator" content="DevelopMentor Gen<X> v1.0">
</head>
<%
  }

function writeLinks(xmlThis) 
  {
  var xmlLinks = xmlThis.selectNodes("./Links/*");
  //note: missing links is not an error

  for (var xml=xmlLinks.nextNode(); xml != null; xml=xmlLinks.nextNode()) {
    %><a href="<%= xml.getAttribute("href") %>"><%= xml.text %></a><% xcode.write("&nbsp;|&nbsp;"); %><% 
    }
  }

function writeCopyrightP()
  {
%>
<p><% xcode.write("&nbsp;"); %></p>
<p><% xcode.write("&nbsp;"); %></p>
<h3>Copyright</h3>
<p>Copyright <% xcode.write("&copy;"); %> Simon Fell, 2000-2004. All rights reserved.<br>

<p><% xcode.write("&nbsp;"); %></p>
<%
  }

function constructLinkToInterface(sInterfaceName) // return string
  {
  // Remove asterisks (there shouldn't be more than two)
  var sCleanInterfaceName = sInterfaceName.replace("*","").replace("*","");
  while ( sCleanInterfaceName.charAt(sCleanInterfaceName.length-1) == " " )
          sCleanInterfaceName = sCleanInterfaceName.substr(0,sCleanInterfaceName.length-1) ;
      
  if (sCleanInterfaceName == "IUnknown")
    return '<a href="http://msdn.microsoft.com/library/en-us/com/cmi_q2z_9dwu.asp?frame=true" target="_blank">' + sInterfaceName + '</a>';
  else if (sCleanInterfaceName == "IDispatch")
    return '<a href="http://msdn.microsoft.com/library/default.asp?url=/library/en-us/automat/htm_hh2/chap5_78v9.asp" target="_blank">' + sInterfaceName + '</a>';
  else if (sCleanInterfaceName == "IDispatchEx")
    return '<a href="http://msdn.microsoft.com/library/default.asp?url=/library/en-us/script56/html/jslrfidispatchex.asp" target="_blank">' + sInterfaceName + '</a>';
          
  return '<a href="interface_' + sCleanInterfaceName + '.html">' + sInterfaceName + '</a>';
  }
%>
