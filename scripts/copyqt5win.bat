set "qt=D:\Qt\5.4\mingw491_32"

xcopy %qt%\bin\icudt53.dll ..\player\release
xcopy %qt%\bin\icuin53.dll ..\player\release
xcopy %qt%\bin\icuuc53.dll ..\player\release
xcopy %qt%\bin\libgcc_s_dw2-1.dll ..\player\release
xcopy %qt%\bin\libstdc++-6.dll ..\player\release
xcopy %qt%\bin\libwinpthread-1.dll ..\player\release
xcopy %qt%\bin\Qt5Core.dll ..\player\release
xcopy %qt%\bin\Qt5Gui.dll ..\player\release
xcopy %qt%\bin\Qt5Network.dll ..\player\release
xcopy %qt%\bin\Qt5OpenGL.dll ..\player\release
xcopy %qt%\bin\Qt5PrintSupport.dll ..\player\release
xcopy %qt%\bin\Qt5Widgets.dll ..\player\release
xcopy %qt%\bin\Qt5Xml.dll ..\player\release

mkdir ..\player\release\imageformats
xcopy %qt%\plugins\imageformats\qjpeg.dll ..\player\release\imageformats

mkdir ..\player\release\platforms
xcopy %qt%\plugins\platforms\qminimal.dll ..\player\release\platforms
xcopy %qt%\plugins\platforms\qoffscreen.dll ..\player\release\platforms
xcopy %qt%\plugins\platforms\qwindows.dll ..\player\release\platforms

xcopy %qt%\lib\qscintilla2.dll ..\player\release

xcopy ..\libgid\external\glew-1.10.0\lib\mingw48_32\glew32.dll ..\player\release
xcopy ..\libgid\external\openal-soft-1.13\build\mingw48_32\OpenAL32.dll ..\player\release

xcopy %qt%\bin\icudt53.dll ..\player\release\Tools
xcopy %qt%\bin\icuin53.dll ..\player\release\Tools
xcopy %qt%\bin\icuuc53.dll ..\player\release\Tools
xcopy %qt%\bin\libgcc_s_dw2-1.dll ..\player\release\Tools
xcopy %qt%\bin\libstdc++-6.dll ..\player\release\Tools
xcopy %qt%\bin\libwinpthread-1.dll ..\player\release\Tools
xcopy %qt%\bin\Qt5Core.dll ..\player\release\Tools
xcopy %qt%\bin\Qt5Network.dll ..\player\release\Tools
xcopy %qt%\bin\Qt5Xml.dll ..\player\release\Tools
