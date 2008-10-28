@echo off
@echo Cab Builder script for Pocket PC Installs
@echo.
@echo Updating DLL's from build tree

set WCE_TOOLS=%WPCEmulDir%\..

mkdir .\arm
mkdir .\sh3
mkdir .\mips
mkdir .\x86
mkdir .\ix86

copy ARMRelMinDependency\pocketXmlRpc.dll							.\arm
copy ..\..\pocketHTTP\pocketpc\ARMRelMinDependency\pocketHTTP.dll 	.\arm

copy SH3RelMinDependency\pocketXmlRpc.dll							.\sh3
copy ..\..\pocketHTTP\pocketpc\SH3RelMinDependency\pocketHTTP.dll	.\sh3

copy MIPSRelMinDependency\pocketXmlRpc.dll							.\mips
copy ..\..\pocketHTTP\pocketpc\MIPSRelMinDependency\pocketHTTP.dll	.\mips

copy X86EMRelMinDependency\pocketXmlRpc.dll							.\x86
copy ..\..\pocketHTTP\pocketpc\X86EMRelMinDependency\pocketHTTP.dll	.\x86

copy X86RelMinDependency\pocketXmlRpc.dll							.\ix86
copy ..\..\pocketHTTP\pocketpc\X86RelMinDependency\pocketHTTP.dll	.\ix86

@echo running cabwiz

SET WCE_CABWIZ=%WCE_TOOLS%\support\ActiveSync\windows ce application installation\cabwiz
"%WCE_CABWIZ%\cabwiz.exe" pocketXMLRPC.inf /err err.log /cpu arm mips sh3

@echo done.!
