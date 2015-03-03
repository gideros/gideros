set "qt=C:\Qt\Qt5.3.0\5.3\mingw482_32"

cp %qt%\bin\icudt52.dll ..\..\release
cp %qt%\bin\icuin52.dll ..\..\release
cp %qt%\bin\icuuc52.dll ..\..\release
cp %qt%\bin\libgcc_s_dw2-1.dll ..\..\release
cp %qt%\bin\libstdc++-6.dll ..\..\release
cp %qt%\bin\libwinpthread-1.dll ..\..\release
cp %qt%\bin\Qt5Core.dll ..\..\release
cp %qt%\bin\Qt5Gui.dll ..\..\release
cp %qt%\bin\Qt5Network.dll ..\..\release
cp %qt%\bin\Qt5OpenGL.dll ..\..\release
cp %qt%\bin\Qt5PrintSupport.dll ..\..\release
cp %qt%\bin\Qt5Widgets.dll ..\..\release
cp %qt%\bin\Qt5Xml.dll ..\..\release

mkdir ..\..\release\imageformats
cp %qt%\plugins\imageformats\qjpeg.dll ..\..\release\imageformats

mkdir ..\..\release\platforms
cp %qt%\plugins\platforms\qminimal.dll ..\..\release\platforms
cp %qt%\plugins\platforms\qoffscreen.dll ..\..\release\platforms
cp %qt%\plugins\platforms\qwindows.dll ..\..\release\platforms

cp %qt%\lib\qscintilla2.dll ..\..\release

cp ..\libgid\external\glew-1.10.0\lib\mingw48_32\glew32.dll ..\..\release
cp ..\libgid\external\openal-soft-1.13\build\mingw48_32\OpenAL32.dll ..\..\release

cp %qt%\bin\icudt52.dll ..\..\release\Tools
cp %qt%\bin\icuin52.dll ..\..\release\Tools
cp %qt%\bin\icuuc52.dll ..\..\release\Tools
cp %qt%\bin\libgcc_s_dw2-1.dll ..\..\release\Tools
cp %qt%\bin\libstdc++-6.dll ..\..\release\Tools
cp %qt%\bin\libwinpthread-1.dll ..\..\release\Tools
cp %qt%\bin\Qt5Core.dll ..\..\release\Tools
cp %qt%\bin\Qt5Network.dll ..\..\release\Tools
cp %qt%\bin\Qt5Xml.dll ..\..\release\Tools
