; $Header: c:/cvs/pocketsoap/pocketsoap/Install_PPC/pocketSOAP_ppc.nsi,v 1.14 2006/05/03 04:19:02 simon Exp $
;
; pocketSOAP_ppc.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The instalelr simply 
; prompts the user asking them where to install, and drops of notepad.exe
; there. If your Windows directory is not C:\windows, change it below.
;

!define PS_BUILD  		"1.5.5"    		; Build/Release #.
!define PS_BUILD_NAME 	"1.5"			; Build/Release friendly name

!include "MUI.nsh"


; The name of the installer
Name "PocketSOAP for PocketPC ${PS_BUILD_NAME}"

; The file to write
OutFile "PocketSOAP.ppc.${PS_BUILD}.exe"

; do CRC check
CRCCheck on

; The default installation directory
InstallDir "$PROGRAMFILES\SimonFell\PocketSOAP for Pocket PC"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketSOAPPPC" ""

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
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketSOAPPPC" "" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketSOAPPPC" "DisplayName" "PocketSOAP for PocketPC ${PS_BUILD_NAME} (remove only)"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketSOAPPPC" "UninstallString" '"$INSTDIR\uninstall.exe"'
  ; Put file there
  File "pocketSOAP.mips.CAB"
  File "pocketSOAP.sh3.CAB"
  File "pocketSOAP.arm.CAB"
  File "pocketSOAP.ini"
  ; new for 1.5, we ship the .IDL & .h file, makes life way easier for the C++ WSDL wizard and other C++ coders
  File "..\pSOAP_Core\pSOAP.IDL"
  File "..\pSOAP_ppc\pSOAP.h"
  File "..\Attachments_core\Attachments.idl"
  File "..\Attachments_ppc\Attachments.h"
  SetOutPath "$INSTDIR\docs"
  File "..\docs\redist\attachments.chm"
  File "..\docs\redist\master.chm"
  File "..\docs\redist\proxydocs.chm"
  File "..\docs\redist\psdocs.chm"
  CreateDirectory "$SMPROGRAMS\PocketSOAP"
  CreateShortCut  "$SMPROGRAMS\PocketSOAP\Documentation.lnk" "$INSTDIR\docs\master.chm" 
  WriteUninstaller "uninstall.exe"  
  ReadRegStr $4 HKLM "software\Microsoft\Windows\CurrentVersion\App Paths\CEAppMgr.exe" ""
  Exec '"$4" "$INSTDIR\pocketSOAP.ini"'
SectionEnd ; end the section

Section "Uninstall"
  Delete "$INSTDIR\uninstall.exe" ; delete self (see explanation below why this works)
  Delete "$INSTDIR\pocketSOAP.mips.CAB"
  Delete "$INSTDIR\pocketSOAP.sh3.CAB"
  Delete "$INSTDIR\pocketSOAP.arm.CAB"
  Delete "$INSTDIR\pocketSOAP.ini"
  Delete "$INSTDIR\pSOAP.IDL"
  Delete "$INSTDIR\pSOAP.h"
  Delete "$INSTDIR\Attachments.IDL"
  Delete "$INSTDIR\Attachments.h"
  Delete "$INSTDIR\docs\attachments.chm"
  Delete "$INSTDIR\docs\master.chm"
  Delete "$INSTDIR\docs\proxydocs.chm"
  Delete "$INSTDIR\docs\psdocs.chm"
  Delete "$SMPROGRAMS\PocketSOAP\Documentation.lnk"
  RMDir  "$SMPROGRAMS\PocketSOAP"
  RMDir "$INSTDIR\docs"
  RMDir "$INSTDIR"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketSOAPPPC"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PocketSOAPPPC"
SectionEnd

; eof
