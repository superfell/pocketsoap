; PocketXMLRPC.nsi
;

!include "MUI.nsh"


; The name of the installer
Name "PocketXmlRpc for PocketPC"

; The file to write
OutFile "PocketXmlRpc_ppc_121.exe"


; do CRC check
CRCCheck on

; The default installation directory
InstallDir $PROGRAMFILES\SimonFell\PocketXMLRPC_PocketPC
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketXMLRPC_PPC" ""

!define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\license.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

  !insertmacro MUI_LANGUAGE "English"

; The stuff to install
Section ""
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketXMLRPC_PPC" "" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketXMLRPC_PPC" "DisplayName" "PocketXMLRPC For PocketPC v1.2.1 (remove only)"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketXMLRPC_PPC" "UninstallString" '"$INSTDIR\uninstall.exe"'
  
  ; Put file there  
  File "pocketXMLRPC.arm.CAB"
  File "pocketXMLRPC.sh3.CAB"
  File "pocketXMLRPC.mips.CAB"
  File "pocketXMLRPC.ini"
  File "..\license.txt"
  WriteUninstaller "uninstall.exe"
  ReadRegStr $1 HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\CEAPPMGR.EXE" ""
  ExecWait '"$1" "$INSTDIR\pocketXMLRPC.ini"'
SectionEnd ; end the section

Section "Uninstall"
  Delete "$INSTDIR\uninstall.exe" ; delete self (see explanation below why this works)
  Delete "$INSTDIR\pocketXMLRPC.arm.cab"
  Delete "$INSTDIR\pocketXMLRPC.sh3.cab"
  Delete "$INSTDIR\pocketXMLRPC.mips.cab"
  Delete "pocketXMLRPC.ini"
  Delete "$INSTDIR\license.txt"
  RMDir "$INSTDIR"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketXMLRPC_PPC"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PocketXMLRPC_PPC"
SectionEnd

; eof
