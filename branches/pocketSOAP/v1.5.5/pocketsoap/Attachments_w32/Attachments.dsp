# Microsoft Developer Studio Project File - Name="Attachments" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Attachments - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Attachments.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Attachments.mak" CFG="Attachments - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Attachments - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Attachments - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/soap/pocketSOAP/Attachments", TYDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Attachments - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Zi /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_MIN_CRT" /FD /c
# ADD CPP /nologo /MT /W3 /Zi /O1 /I "..\psoap_w32" /I "..\..\3rdparty\expat\xmltok" /I "..\..\3rdparty\STLport-4.5\stlport" /I "..\..\3rdparty" /I "..\Attachments_w32" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /opt:ref
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /base:"0x20160000" /subsystem:windows /dll /debug /machine:I386 /out:"Release/psDime.dll" /libpath:"..\3rdparty\expat\lib" /libpath:"..\..\3rdparty\expat\bin" /opt:ref
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=.\Release\psDime.dll
InputPath=.\Release\psDime.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Attachments - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\psoap_w32" /I "..\..\3rdparty\expat\xmltok" /I "..\..\3rdparty\STLport-4.5\stlport" /I "..\..\3rdparty" /I "..\Attachments_w32" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /base:"0x20160000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"msvcrtd.lib" /out:"Debug/psDime.dll" /pdbtype:sept /libpath:"..\..\3rdparty\expat\dbgbin"
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=.\Debug\psDime.dll
InputPath=.\Debug\psDime.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "Attachments - Win32 Release"
# Name "Attachments - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Attachments_core\Attachment.cpp
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Attachments.cpp
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Attachments.def
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Attachments.idl
# ADD MTL /tlb ".\Attachments.tlb" /h "Attachments.h" /iid "Attachments_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Attachments.rc
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\AttachmentsCollection.cpp
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\contentId.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ConvertUTF.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\dime.cpp
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\DimePackager.cpp
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Manager.cpp
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\MimePackager.cpp
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\PartPayload.cpp
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\..\Common\stringHelpers.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\_BldNum.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Attachment.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\AttachmentBuilder.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\AttachmentsCollection.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\contentId.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ConvertUTF.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Dime.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\DimePackager.h
# End Source File
# Begin Source File

SOURCE=..\pSOAP_Core\headerHelpers.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\IPackager.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Manager.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\MimePackager.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\PackagerStreamerBase.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\PartPayload.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Resource.h
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\stringBuff.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\stringHelpers.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\Attachments_core\Attachment.rgs
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\Manager.rgs
# End Source File
# End Group
# Begin Source File

SOURCE=..\Attachments_core\dime.vbs
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\mstk.vbs
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\test_apache.vbs
# End Source File
# Begin Source File

SOURCE=..\Attachments_core\test_glue.vbs
# End Source File
# End Target
# End Project
