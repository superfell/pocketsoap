; $Header: c:/cvs/pocketsoap/pocketsoap/Install_W32/pocketSOAP_w32.nsi,v 1.14 2006/05/02 04:00:06 simon Exp $
;
; pocketSOAP_w32.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The instalelr simply 
; prompts the user asking them where to install, and drops of notepad.exe
; there. If your Windows directory is not C:\windows, change it below.
;

!define PS_BUILD  		"1.5.5" 	; Build/Release #.
!define PS_BUILD_NAME 	"1.5.5"		; Build/Release friendly name

!include "MUI.nsh"

; The name of the installer
Name "PocketSOAP ${PS_BUILD_NAME}"

; The file to write
OutFile "PocketSOAP.${PS_BUILD}.exe"

; do CRC check
CRCCheck on

; The default installation directory
InstallDir $PROGRAMFILES\SimonFell\PocketSOAP
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketSOAP" ""

!define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

  !insertmacro MUI_LANGUAGE "English"

; The stuff to install
Section ""
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketSOAP" "" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketSOAP" "DisplayName" "PocketSOAP ${PS_BUILD_NAME} (remove only)"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketSOAP" "UninstallString" '"$INSTDIR\uninstall.exe"'
  ; Put file there
  Delete "$INSTDIR\xmltok.dll"
  Delete "$INSTDIR\xmlparse.dll"
  Delete "$INSTDIR\pszlib.dll"
  Delete "$INSTDIR\zlib.dll"
  File "..\pSOAP_W32\ReleaseMinDependency\pSOAP32.dll"
  File "..\psProxy_W32\ReleaseMinDependency\psProxy.dll"
  File "..\Attachments_w32\Release\psDime.dll"
  File "..\..\pocketHTTP\win32\ReleaseMinDependency\pocketHTTP.dll"
  ; new for 1.5, we ship the .IDL & .h file, makes life way easier for the C++ WSDL wizard
  File "..\pSOAP_Core\pSOAP.IDL"
  File "..\pSOAP_w32\pSOAP.h"
  File "..\Attachments_core\Attachments.idl"
  File "..\Attachments_w32\Attachments.h"
  
  SetOutPath "$INSTDIR\docs"
  File "..\docs\redist\attachments.chm"
  File "..\docs\redist\master.chm"
  File "..\docs\redist\proxydocs.chm"
  File "..\docs\redist\psdocs.chm"
  RegDLL "$INSTDIR\pocketHTTP.dll"
  RegDLL "$INSTDIR\pSOAP32.dll"
  RegDLL "$INSTDIR\psProxy.dll"
  RegDLL "$INSTDIR\psDime.dll"
  CreateDirectory "$SMPROGRAMS\PocketSOAP"
  CreateShortCut "$SMPROGRAMS\PocketSOAP\Documentation.lnk" "$INSTDIR\docs\master.chm" 
  WriteUninstaller "uninstall.exe"  
SectionEnd ; end the section

Section "Uninstall"
  UnRegDLL "$INSTDIR\psDime.dll"
  UnRegDLL "$INSTDIR\psProxy.dll"
  UnRegDLL "$INSTDIR\pSOAP32.dll"
  UnRegDLL "$INSTDIR\pocketHTTP.dll"
  Delete "$INSTDIR\uninstall.exe" ; delete self (see explanation below why this works)
  Delete "$INSTDIR\pocketHTTP.dll"
  Delete "$INSTDIR\psDime.dll"
  Delete "$INSTDIR\psProxy.dll"
  Delete "$INSTDIR\pSOAP32.dll"
  Delete "$INSTDIR\pSOAP.IDL"
  Delete "$INSTDIR\pSOAP.h"
  Delete "$INSTDIR\Attachments.h"
  Delete "$INSTDIR\Attachments.idl"
  Delete "$INSTDIR\docs\attachments.chm"
  Delete "$INSTDIR\docs\master.chm"
  Delete "$INSTDIR\docs\proxydocs.chm"
  Delete "$INSTDIR\docs\psdocs.chm"
  Delete "$SMPROGRAMS\PocketSOAP\Documentation.lnk"
  RMDir "$SMPROGRAMS\PocketSOAP"
  RMDir "$INSTDIR\docs"
  RMDir "$INSTDIR"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketSOAP"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PocketSOAP"
SectionEnd

; eof
