@echo off
@echo Cab Builder script for Pocket PC Installs
@echo.
@echo Updating DLL's from build tree

set WCE_TOOLS=%WPCEmulDir%\..

mkdir .\arm
mkdir .\sh3
mkdir .\mips
mkdir .\x86
mkdir .\IX86

copy ..\pSOAP_PPC\ARMRelMinDependency\psoap.dll						.\arm
copy ..\attachments_ppc\ARMRelMinDependency\psDime.dll				.\arm
copy ..\..\pocketHTTP\pocketpc\ARMRelMinDependency\PocketHTTP.dll 	.\arm
copy ..\samples\psQuote2\ARMRel\psQuote2.exe						.\arm
copy "%WCE_TOOLS%\mfc\lib\arm\mfcce300.dll"							.\arm

copy ..\pSOAP_PPC\SH3RelMinDependency\psoap.dll						.\sh3
copy ..\Attachments_ppc\SH3RelMinDependency\psDime.dll	 			.\sh3
copy ..\..\pocketHTTP\pocketpc\SH3RelMinDependency\PocketHTTP.dll 	.\sh3
copy ..\samples\psQuote2\SH3Rel\psQuote2.exe						.\sh3
copy "%WCE_TOOLS%\mfc\lib\sh3\mfcce300.dll"							.\sh3

copy ..\pSOAP_PPC\MIPSRelMinDependency\psoap.dll					.\mips
copy ..\Attachments_ppc\MIPSRelMinDependency\psDime.dll				.\mips
copy ..\..\pocketHTTP\pocketpc\MIPSRelMinDependency\PocketHTTP.dll 	.\mips
copy ..\samples\psQuote2\MIPSRel\psQuote2.exe						.\mips
copy "%WCE_TOOLS%\mfc\lib\mips\mfcce300.dll"						.\mips

copy ..\pSOAP_PPC\X86EMRelMinDependency\psoap.dll					.\x86
copy ..\Attachments_ppc\X86EMRelMinDependency\psDime.dll			.\x86
copy ..\..\pocketHTTP\pocketpc\X86EMRelMinDependency\PocketHTTP.dll .\x86
copy ..\samples\psQuote2\X86EMRel\psQuote2.exe						.\x86
copy "%WCE_TOOLS%\mfc\lib\x86em\mfcce300.dll"						.\x86

copy ..\pSOAP_PPC\X86Rel\psoap.dll									.\IX86
copy ..\Attachments_ppc\X86Rel\psDime.dll							.\IX86
copy ..\..\pocketHTTP\pocketpc\X86RelMinDependency\PocketHTTP.dll 	.\IX86
copy ..\samples\psQuote2\X86Rel\psQuote2.exe						.\IX86
copy "%WCE_TOOLS%\mfc\lib\x86em\mfcce300.dll"						.\IX86

@echo running cabwiz

SET WCE_CABWIZ=%WCE_TOOLS%\support\ActiveSync\windows ce application installation\cabwiz
rem SET WCE_CABWIZ=C:\Windows CE Tools\wce300\Pocket PC 2002\support\ActiveSync\windows ce application installation\cabwiz

"%WCE_CABWIZ%\cabwiz.exe" pocketSOAP.inf /err err.log /cpu arm mips sh3 ix86

@echo done.!

