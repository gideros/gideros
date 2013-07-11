;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Gideros"
  OutFile "gideros_2013.06.1.exe"

  ;Default installation folder
  InstallDir $PROGRAMFILES\Gideros
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Gideros" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

;  !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;
 
Section "Remove previous install"
  RMDir /r "$INSTDIR\All Plugins"
  RMDir /r "$INSTDIR\Documentation"
  RMDir /r "$INSTDIR\Examples"
  RMDir /r "$INSTDIR\imageformats"
  RMDir /r "$INSTDIR\platforms"
  RMDir /r "$INSTDIR\Plugins"
  RMDir /r "$INSTDIR\Resources"
  RMDir /r "$INSTDIR\Sdk"
  RMDir /r "$INSTDIR\Templates"
  RMDir /r "$INSTDIR\Tools"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.exe"
SectionEnd

;--------------------------------
;Installer Sections

Section "Gideros (required)"
  SectionIn RO

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...
  File /r /x gideros_mui2.nsi *.*
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Gideros" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Gideros"
  CreateShortCut "$SMPROGRAMS\Gideros\Gideros Studio.lnk" "$INSTDIR\GiderosStudio.exe" "" "$INSTDIR\GiderosStudio.exe" 0
  CreateShortCut "$SMPROGRAMS\Gideros\Gideros Player.lnk" "$INSTDIR\GiderosPlayer.exe" "" "$INSTDIR\GiderosPlayer.exe" 0
  CreateShortCut "$SMPROGRAMS\Gideros\Gideros Font Creator.lnk" "$INSTDIR\GiderosFontCreator.exe" "" "$INSTDIR\GiderosFontCreator.exe" 0
  CreateShortCut "$SMPROGRAMS\Gideros\Gideros Texture Packer.lnk" "$INSTDIR\GiderosTexturePacker.exe" "" "$INSTDIR\GiderosTexturePacker.exe" 0
  CreateShortCut "$SMPROGRAMS\Gideros\Gideros License Manager.lnk" "$INSTDIR\GiderosLicenseManager.exe" "" "$INSTDIR\GiderosLicenseManager.exe" 0
  CreateShortCut "$SMPROGRAMS\Gideros\Examples.lnk" "$INSTDIR\Examples" "" "$INSTDIR\Examples" 0
  CreateShortCut "$SMPROGRAMS\Gideros\Reference Manual.lnk" "$INSTDIR\Documentation\reference_manual.html" "" "$INSTDIR\Documentation\reference_manual.html" 0
  CreateShortCut "$SMPROGRAMS\Gideros\Getting Started.lnk" "$INSTDIR\Documentation\getting_started.html" "" "$INSTDIR\Documentation\getting_started.html" 0
  CreateShortCut "$SMPROGRAMS\Gideros\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd


;--------------------------------
;Descriptions

  ;Language strings
;  LangString DESC_SecDummy ${LANG_ENGLISH} "A test section."

  ;Assign language strings to sections
;  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
;  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...
  RMDir /r $INSTDIR

;  Delete "$INSTDIR\Uninstall.exe"

;  RMDir "$INSTDIR"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Gideros\*.*"
  ; Remove shortcut directories used
  RMDir "$SMPROGRAMS\Gideros"

  DeleteRegKey HKCU "Software\Gideros"

SectionEnd