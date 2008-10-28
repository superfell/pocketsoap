@echo off
@echo "Building Installer for WSDL 2.0"

copy ..\codegen\wsdlcodegen.dll .
copy ..\wsdlParser\wsdlParser.dll .
copy ..\wsdlParser\*.xsd .
copy ..\wsdl_gui\wsdlWizard.exe .
copy ..\wsdlcl\Release\wsdl-cl.exe .
copy ..\..\pocketsoap\samples\mapex\release\map.dll .

"c:\Program Files\NSIS\makensis" wsdlWiz.nsi
