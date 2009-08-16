# Microsoft Developer Studio Project File - Name="pSOAP32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=pSOAP32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pSOAP32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pSOAP32.mak" CFG="pSOAP32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pSOAP32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pSOAP32 - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pSOAP32 - Win32 Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pSOAP32 - Win32 Unicode Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pSOAP32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "..\..\common" /I "..\..\3rdparty" /I "..\..\3rdparty\expat\xmlparse" /I "..\..\3rdparty\expat\xmltok" /I "..\psoap_w32" /I "..\..\3rdparty\STLport-4.5\stlport" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x20100000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"msvcrtd.lib" /pdbtype:sept /libpath:"..\..\3rdparty\expat\dbgbin"
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=.\Debug\pSOAP32.dll
InputPath=.\Debug\pSOAP32.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pSOAP32 - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugU"
# PROP BASE Intermediate_Dir "DebugU"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugU"
# PROP Intermediate_Dir "DebugU"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_UNICODE" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "..\..\common" /I "..\..\3rdparty" /I "..\..\3rdparty\expat\xmlparse" /I "..\..\3rdparty\expat\xmltok" /I "..\psoap_w32" /I "..\..\3rdparty\STLport-4.5\stlport" /I "..\..\3rdparty\zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x20100000" /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\3rdparty\expat\bin" /libpath:"..\..\3rdparty\zlib\release"
# Begin Custom Build - Performing registration
OutDir=.\DebugU
TargetPath=.\DebugU\pSOAP32.dll
InputPath=.\DebugU\pSOAP32.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	if "%OS%"=="" goto NOTNT 
	if not "%OS%"=="Windows_NT" goto NOTNT 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	goto end 
	:NOTNT 
	echo Warning : Cannot register Unicode DLL on Windows 95 
	:end 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pSOAP32 - Win32 Release MinDependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseMinDependency"
# PROP BASE Intermediate_Dir "ReleaseMinDependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseMinDependency"
# PROP Intermediate_Dir "ReleaseMinDependency"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /O1 /I "..\..\common" /I "..\..\3rdparty" /I "..\..\3rdparty\expat\xmlparse" /I "..\..\3rdparty\expat\xmltok" /I "..\psoap_w32" /I "..\..\3rdparty\STLport-4.5\stlport" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x20100000" /subsystem:windows /dll /machine:I386 /libpath:"..\..\3rdparty\expat\bin"
# Begin Custom Build - Performing registration
OutDir=.\ReleaseMinDependency
TargetPath=.\ReleaseMinDependency\pSOAP32.dll
InputPath=.\ReleaseMinDependency\pSOAP32.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pSOAP32 - Win32 Unicode Release MinDependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseUMinDependency"
# PROP BASE Intermediate_Dir "ReleaseUMinDependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseUMinDependency"
# PROP Intermediate_Dir "ReleaseUMinDependency"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_UNICODE" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /O1 /I "..\..\common" /I "..\..\3rdparty" /I "..\..\3rdparty\expat\xmlparse" /I "..\..\3rdparty\expat\xmltok" /I "..\psoap_w32" /I "..\..\3rdparty\STLport-4.5\stlport" /I "..\..\3rdparty\zlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_UNICODE" /D "_ATL_STATIC_REGISTRY" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x20100000" /subsystem:windows /dll /machine:I386 /libpath:"..\..\3rdparty\expat\bin" /libpath:"..\..\3rdparty\zlib\release"
# Begin Custom Build - Performing registration
OutDir=.\ReleaseUMinDependency
TargetPath=.\ReleaseUMinDependency\pSOAP32.dll
InputPath=.\ReleaseUMinDependency\pSOAP32.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	if "%OS%"=="" goto NOTNT 
	if not "%OS%"=="Windows_NT" goto NOTNT 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	goto end 
	:NOTNT 
	echo Warning : Cannot register Unicode DLL on Windows 95 
	:end 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "pSOAP32 - Win32 Debug"
# Name "pSOAP32 - Win32 Unicode Debug"
# Name "pSOAP32 - Win32 Release MinDependency"
# Name "pSOAP32 - Win32 Unicode Release MinDependency"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Common\Base64.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\Common\ConvertUTF.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\copyHelpers.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\Envelope.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\EnvWriter.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\expatpp.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\fixupMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\HTTPTransport.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\InterfaceFinder.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\Namespaces.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\ParseHelpers.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\pSOAP.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\pSOAP.def
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\pSOAP.idl
# ADD MTL /tlb "pSOAP32.tlb" /Oicf
# End Source File
# Begin Source File

SOURCE=.\pSOAP.rc
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "RC_NOTCE"
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\psParser.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\QName.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\rawtcp.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerArray.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\serializerb64.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerDate.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerHexBin.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerNode.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerNull.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerPB.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerQName.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerSimple.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SOAPNode.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SOAPNodes.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\..\common\stringHelpers.cpp
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\xsdLong.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\_BldNum.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Base64.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ConvertUTF.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\copyHelpers.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\Envelope.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\EnvWriter.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\expatpp.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\fixupMgr.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\headerHelpers.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\HTTPTransport.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\idMgr.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\InterfaceFinder.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\Namespaces.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\ParseHelpers.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\psParser.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\QName.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\rawtcp.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\reportErrorImpl.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerArray.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\serializerb64.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerDate.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerFactory.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerHexBin.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerNode.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerNull.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerPB.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerQName.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerSimple.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\sfDispImpl.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SOAPNode.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SOAPNodes.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\stringBuff.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\stringHelpers.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\tags.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\TransportBase.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\variantSourceCracker.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\xsdLong.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\pSOAP_Core\DeserializerArray.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\DeserializerArray12.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\DeserializerXsdLong.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\Envelope.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\HTTPTransport.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\InterfaceFinder.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\QName.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\rawtcp.rgs
# End Source File
# Begin Source File

SOURCE=.\registry.bin
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerArray.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerArray12.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerB64.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerBoolean.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerDate.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerFactory.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerHexBin.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerNode.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerNull.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerPB.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerQName.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SerializerSimple.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\SOAPNode.rgs
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\xsdLong.rgs
# End Source File
# End Group
# Begin Group "Tests"

# PROP Default_Filter "vbs"
# Begin Source File

SOURCE=..\samples\WSH\guid.vbs
# End Source File
# Begin Source File

SOURCE=..\samples\WSH\interop.vbs
# End Source File
# Begin Source File

SOURCE=..\samples\WSH\quote.vbs
# End Source File
# End Group
# Begin Source File

SOURCE=.\todo.txt
# End Source File
# End Target
# End Project
