; PocketXMLRPC.nsi
;

!include "MUI.nsh"

; The name of the installer
Name "PocketXmlRpc for Windows"

; The file to write
OutFile "PocketXmlRpc_w32_121.exe"

; do CRC check
CRCCheck on

; The default installation directory
InstallDir $PROGRAMFILES\SimonFell\PocketXMLRPC
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketXMLRPC" ""

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
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketXMLRPC" "" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketXMLRPC" "DisplayName" "PocketXMLRPC For Windows v1.2.1 (remove only)"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketXMLRPC" "UninstallString" '"$INSTDIR\uninstall.exe"'
  
  ; Put file there
  Delete "$INSTDIR\xmltok.dll"
  Delete "$INSTDIR\xmlparse.dll"
  Delete "$INSTDIR\zlib.dll"
  File "..\..\PocketHTTP\Win32\ReleaseMinDependency\pocketHTTP.dll"
  File "ReleaseMinDependency\pocketXMLRPC.dll"
  File "..\license.txt"
  RegDLL "$INSTDIR\pocketHTTP.dll"
  RegDLL "$INSTDIR\pocketXMLRPC.dll"
  WriteUninstaller "uninstall.exe"
SectionEnd ; end the section


Section "Uninstall"
  UnRegDLL "$INSTDIR\pocketXMLRPC.dll"
  UnRegDLL "$INSTDIR\pocketHTTP.dll"
  Delete "$INSTDIR\uninstall.exe" ; delete self (see explanation below why this works)
  Delete "$INSTDIR\pocketXMLRPC.dll"
  Delete "$INSTDIR\pocketHTTP.dll"
  Delete "$INSTDIR\license.txt"
  RMDir "$INSTDIR"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketXMLRPC"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PocketXMLRPC"
SectionEnd

; eof
