<x:stylesheet version="1.0"
	xmlns:x="http://www.w3.org/1999/XSL/Transform"
	xmlns:w="http://schemas.xmlsoap.org/wsdl/"
	xmlns="http://www.w3.org/1999/xhtml" 
	xmlns:xlink="http://www.w3.org/1999/xlink"
	xmlns:rddl="http://www.rddl.org/">
<x:output 
	method="xml" 
	omit-xml-declaration="yes"
	indent="yes"
	/>
<x:template match="w:definitions">
	<html>
	<head>
	<title><x:value-of select="@name" /> Service Description</title>
	<link rel="stylesheet" href="wsdl.css" type="text/css" />
	</head>
	<body>
	<h1><x:value-of select="@name" /> Service Description</h1>
	<rddl:resource
		id="WSDL"
		xlink:title="WSDL Service Description"
		xlink:role="http://schemas.xmlsoap.org/wsdl/"
		xlink:arcrole="http://www.rddl.org/purposes#module"
		xlink:href="soap.asp?WSDL">
		<div class="subtext">
			click here for the <a href="soap.asp?WSDL">WSDL Service Description</a>
		</div>
	</rddl:resource>
	
	<x:apply-templates select="w:portType" />
	
	<hr class="footer"/>
	<div class="footertext">Powered by 4s4c</div>
	</body>
	</html>
</x:template>

<x:template match="w:portType">
	<h2>Operations for <x:value-of select="@name" /></h2>
	<x:apply-templates select="w:operation"/>
</x:template>

<x:template match="w:operation">
	<div class="op">
		<x:variable name="msgout" select="substring-after(w:output/@message,':')"/>
		<x:for-each select="//w:message[@name=$msgout]/w:part">
			<x:value-of select="@name"/><x:text> </x:text><span class="ptype">[<x:value-of select="@type"/>]</span>
			<x:choose>
				<x:when test="position() = last()" />
				<x:otherwise>, </x:otherwise>
			</x:choose>
		</x:for-each>
		

		<span class="opname"><x:text> </x:text><x:value-of select="@name"/></span> (


		<x:variable name="msgin" select="substring-after(w:input/@message,':')"/>
		<x:for-each select="//w:message[@name=$msgin]/w:part">
			<x:value-of select="@name"/><x:text> </x:text><span class="ptype">[<x:value-of select="@type"/>]</span>
			<x:choose>
				<x:when test="position() = last()" />
				<x:otherwise>, </x:otherwise>
			</x:choose>
		</x:for-each>
		)
	</div>
</x:template>
</x:stylesheet>