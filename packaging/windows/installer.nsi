; Windows installer (NSIS 3). Built by packaging/make_windows.sh, which
; passes: VERSION, OUTFILE, EXE, ICON, WELCOME_BMP, HEADER_BMP and either
; DATA_ISO (a disc image) or DATA_DIR (a folder with the disc's files).
Unicode true
SetCompressor /SOLID lzma
ManifestDPIAware true

!include "MUI2.nsh"

!define APPNAME "JumpStart 4th Grade"
!define UNINSTKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\JumpStart4thGrade"

Name "${APPNAME}: Haunted Island"
OutFile "${OUTFILE}"
InstallDir "$PROGRAMFILES64\${APPNAME}"
InstallDirRegKey HKLM "${UNINSTKEY}" "InstallLocation"
RequestExecutionLevel admin
BrandingText "${APPNAME} ${VERSION}"

VIProductVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "${APPNAME}: Haunted Island"
VIAddVersionKey "FileDescription" "${APPNAME} Setup"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "LegalCopyright" "Game content (c) Knowledge Adventure"

!define MUI_ICON "${ICON}"
!define MUI_UNICON "${ICON}"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${WELCOME_BMP}"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${WELCOME_BMP}"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "${HEADER_BMP}"
!define MUI_ABORTWARNING

!define MUI_WELCOMEPAGE_TITLE "Welcome to ${APPNAME}: Haunted Island"
!define MUI_WELCOMEPAGE_TEXT "This will install ${APPNAME} on your computer.$\r$\n$\r$\nClick Next to continue."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\JumpStart4.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Play ${APPNAME} now"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Install"
    SetShellVarContext all
    SetOutPath "$INSTDIR"
    File "/oname=JumpStart4.exe" "${EXE}"
!ifdef DATA_ISO
    File "/oname=Game.iso" "${DATA_ISO}"
!else
    SetOutPath "$INSTDIR\Disc"
    File /r "${DATA_DIR}/*"
    SetOutPath "$INSTDIR"
!endif
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    CreateShortcut "$SMPROGRAMS\${APPNAME}.lnk" "$INSTDIR\JumpStart4.exe"
    CreateShortcut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\JumpStart4.exe"

    WriteRegStr HKLM "${UNINSTKEY}" "DisplayName" "${APPNAME}: Haunted Island"
    WriteRegStr HKLM "${UNINSTKEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "${UNINSTKEY}" "Publisher" "Knowledge Adventure (native port)"
    WriteRegStr HKLM "${UNINSTKEY}" "DisplayIcon" "$INSTDIR\JumpStart4.exe"
    WriteRegStr HKLM "${UNINSTKEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "${UNINSTKEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegDWORD HKLM "${UNINSTKEY}" "NoModify" 1
    WriteRegDWORD HKLM "${UNINSTKEY}" "NoRepair" 1
    SectionGetSize 0 $0
    WriteRegDWORD HKLM "${UNINSTKEY}" "EstimatedSize" $0
SectionEnd

Section "Uninstall"
    SetShellVarContext all
    Delete "$SMPROGRAMS\${APPNAME}.lnk"
    Delete "$DESKTOP\${APPNAME}.lnk"
    Delete "$INSTDIR\JumpStart4.exe"
    Delete "$INSTDIR\Game.iso"
    RMDir /r "$INSTDIR\Disc"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"
    DeleteRegKey HKLM "${UNINSTKEY}"
    ; Saved players (in each user's AppData) are kept.
SectionEnd
