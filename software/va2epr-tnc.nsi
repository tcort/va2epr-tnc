Name "VA2EPR Terminal Node Controller"
OutFile "va2epr-tnc-installer.exe"
InstallDir $PROGRAMFILES\VA2EPR-TNC
DirText "This will install the VA2EPR Terminal Node Controller software on your computer. Choose a directory"

LicenseText "GNU General Public License version 3 or later"
LicenseData "COPYING"

Section ""
SetOutPath $INSTDIR
File COPYING
File va2epr-tnc.exe
File va2epr_tnc.html

CreateDirectory "$SMPROGRAMS\VA2EPR-TNC"
CreateShortCut "$SMPROGRAMS\VA2EPR-TNC\va2epr-tnc.exe.lnk" "$INSTDIR\va2epr-tnc.exe"
CreateShortCut "$SMPROGRAMS\VA2EPR-TNC\Uninstall.exe.lnk" "$INSTDIR\Uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VA2EPR-TNC" \
"DisplayName" \
"VA2EPR Terminal Node Controller Software (remove only)"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VA2EPR-TNC" \
"UninstallString" \
"$INSTDIR\Uninstall.exe"
WriteUninstaller $INSTDIR\Uninstall.exe

MessageBox MB_OK "Installation was successful."
SectionEnd

Section "Uninstall"
Delete $INSTDIR\Uninstall.exe
Delete $INSTDIR\COPYING
Delete $INSTDIR\va2epr-tnc.exe
Delete $INSTDIR\va2epr_tnc.html
RMDir $INSTDIR


Delete "$SMPROGRAMS\VA2EPR-TNC\va2epr-tnc.exe.lnk"
Delete "$SMPROGRAMS\VA2EPR-TNC\Uninstall.exe.lnk"
RMDir "$SMPROGRAMS\VA2EPR-TNC"

DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\VA2EPR-TNC"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\VA2EPR-TNC"

MessageBox MB_OK "Uninstall was successful."
SectionEnd
