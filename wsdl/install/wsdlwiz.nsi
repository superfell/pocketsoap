; wsdlwiz.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The instalelr simply 
; prompts the user asking them where to install, and drops of notepad.exe
; there. If your Windows directory is not C:\windows, change it below.
;

; The name of the installer
Name "WsdlWizard 2.3.0"

; The file to write
OutFile "wsdlWiz.2.3.0.exe"

; do CRC check
CRCCheck on

; The default installation directory
InstallDir $PROGRAMFILES\SimonFell\WsdlWizard
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\pocketsoap.com\WsdlWiz" ""
DirShow show ; 
; The text to prompt the user to enter a directory
DirText "This will install the WSDL Wizard for PocketSOAP on your computer. Choose a directory"

; install look&feel
InstProgressFlags smooth colored
AutoCloseWindow false
ShowInstDetails show

; The stuff to install
Section ""
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\pocketsoap.com\WsdlWiz" "" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\WsdlWiz" "DisplayName" "WSDL Wizard for PocketSOAP (remove only)"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\WsdlWiz" "UninstallString" '"$INSTDIR\uninstall.exe"'
  ; Put file there
  File "wsdlWizard.exe"
  File "wsdl-cl.exe"
  File "soap11_encoding.xsd"
  File "wsdlCodeGen.dll"
  File "wsdlParser.dll"
  File "map.dll"
  RegDLL "$INSTDIR\wsdlParser.dll"
  RegDLL "$INSTDIR\wsdlCodeGen.dll"
  RegDLL "$INSTDIR\map.dll"
  CreateShortCut "$SMPROGRAMS\PocketSOAP WSDL Wizard.lnk" "$INSTDIR\wsdlWizard.exe" 
  WriteUninstaller "uninstall.exe"  
SectionEnd ; end the section

; begin uninstall settings/section
UninstallText "This will uninstall the WSDL Wizard for PocketSOAP from your system"
ShowUninstDetails show

Section "Uninstall"
  UnRegDLL "$INSTDIR\wsdlCodeGen.dll"
  UnRegDLL "$INSTDIR\wsdlParser.dll"
  UnRegDLL "$INSTDIR\map.dll"
  Delete "$INSTDIR\uninstall.exe" ; delete self (see explanation below why this works)
  Delete "$INSTDIR\wsdlWizard.exe"
  Delete "$INSTDIR\wsdl-cl.exe"
  Delete "$INSTDIR\map.dll"
  Delete "$INSTDIR\wsdlParser.dll"
  Delete "$INSTDIR\wsdlCodeGen.dll"
  Delete "$INSTDIR\soap11_encoding.xsd"
  Delete "$SMPROGRAMS\PocketSOAP WSDL Wizard.lnk"
  RMDir "$INSTDIR"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\pocketsoap.com\WsdlWiz"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WsdlWiz"
SectionEnd

; eof
