; PocketHTTP.nsi
;

!include "MUI.nsh"
!define PHTTP_BUILD  		"1.3.3" 

; The name of the installer
Name "PocketHTTP for Windows"

; The file to write
OutFile "PocketHTTP_w32_133.exe"

; do CRC check
CRCCheck on

; The default installation directory
InstallDir $PROGRAMFILES\SimonFell\PocketHTTP
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketHTTP" ""

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
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketHTTP" "" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketHTTP" "DisplayName" "PocketHTTP For Windows ${PHTTP_BUILD} (remove only)"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketHTTP" "UninstallString" '"$INSTDIR\uninstall.exe"'
  
  ; Put file there
  File "ReleaseMinDependency\pocketHTTP.dll"
  File "..\license.txt"
  ; delete unneeded files if we're installed over the top of a previous install
  Delete "$INSTDIR\xmltok.dll"
  Delete "$INSTDIR\pszlib.dll"
  Delete "$INSTDIR\zlib.dll"
  RegDLL "$INSTDIR\pocketHTTP.dll"
  WriteUninstaller "uninstall.exe"
SectionEnd ; end the section

Section "Uninstall"
  UnRegDLL "$INSTDIR\pocketHTTP.dll"
  Delete "$INSTDIR\uninstall.exe" ; delete self (see explanation below why this works)
  Delete "$INSTDIR\pocketHTTP.dll"
  Delete "$INSTDIR\license.txt"
  RMDir "$INSTDIR"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketHTTP"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PocketHTTP"
SectionEnd

; eof
