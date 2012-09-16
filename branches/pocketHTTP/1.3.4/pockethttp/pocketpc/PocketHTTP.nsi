; PocketHTTP.nsi
;

!include "MUI.nsh"

!define PHTTP_BUILD  		"1.3.4"

; The name of the installer
Name "PocketHTTP for PocketPC"

; The file to write
OutFile "PocketHTTP_ppc_134.exe"

; do CRC check
CRCCheck on

; The default installation directory
InstallDir $PROGRAMFILES\SimonFell\PocketHTTP_PocketPC
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketHTTP_PPC" ""

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
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketHTTP_PPC" "" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketHTTP_PPC" "DisplayName" "PocketHTTP For PocketPC v${PHTTP_BUILD} (remove only)"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketHTTP_PPC" "UninstallString" '"$INSTDIR\uninstall.exe"'
  
  ; Put file there  
  File "pocketHTTP.arm.CAB"
  File "pocketHTTP.sh3.CAB"
  File "pocketHTTP.mips.CAB"
  File "pocketHTTP.ini"
  File "..\license.txt"
  WriteUninstaller "uninstall.exe"
  ReadRegStr $1 HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\CEAPPMGR.EXE" ""
  ExecWait '"$1" "$INSTDIR\pocketHTTP.ini"'
SectionEnd ; end the section

Section "Uninstall"
  Delete "$INSTDIR\uninstall.exe" ; delete self (see explanation below why this works)
  Delete "$INSTDIR\pocketHTTP.arm.cab"
  Delete "$INSTDIR\pocketHTTP.sh3.cab"
  Delete "$INSTDIR\pocketHTTP.mips.cab"
  Delete "pocketHTTP.ini"
  Delete "$INSTDIR\license.txt"
  RMDir "$INSTDIR"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SimonFell\PocketHTTP_PPC"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PocketHTTP_PPC"
SectionEnd

; eof
