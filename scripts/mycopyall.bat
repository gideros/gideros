rem rm -rf => rmdir /S /Q
rem cp -R => xcopy /S /I
rem replaced hg commands with xcopy

rmdir /S /Q ..\..\release
mkdir ..\..\release

copy ..\libgid\release\gid.dll ..\..\release
copy ..\libgvfs\release\gvfs.dll ..\..\release
copy ..\lua\release\lua.dll ..\..\release
copy ..\libgideros\release\gideros.dll ..\..\release
copy ..\libpystring\release\pystring.dll ..\..\release

copy ..\ui\release\GiderosStudio.exe ..\..\release
copy ..\player\release\GiderosPlayer.exe ..\..\release
copy ..\texturepacker\release\GiderosTexturePacker.exe ..\..\release
copy ..\fontcreator\release\GiderosFontCreator.exe ..\..\release
copy ..\licensemanager\release\GiderosLicenseManager.exe ..\..\release

xcopy /S /I ..\ui\Resources ..\..\release\Resources
:: rem svn export ..\ui\Resources ..\..\release\Resources
:: rm -rf ..\..\temp
:: hg archive -I ..\ui\Resources ..\..\temp
:: mv ..\..\temp\ui\Resources ..\..\release

xcopy /S /I ..\ui\Tools ..\..\release\Tools
:: rem svn export ..\ui\Tools ..\..\release\Tools
:: rm -rf ..\..\temp
:: hg archive -I ..\ui\Tools ..\..\temp
:: mv ..\..\temp\ui\Tools ..\..\release

call mycopyqt56.bat

mkdir ..\..\release\Templates

xcopy /S /I ..\ui\Templates\Eclipse ..\..\release\Templates\Eclipse
:: rem svn export ..\ui\Templates\Eclipse ..\..\release\Templates\Eclipse
:: rm -rf ..\..\temp
:: hg archive -I ..\ui\Templates\Eclipse ..\..\temp
:: mv ..\..\temp\ui\Templates\Eclipse ..\..\release\Templates

xcopy /S /I ..\ui\Templates\Xcode4 ..\..\release\Templates\Xcode4
:: rem svn export ..\ui\Templates\Xcode4 ..\..\release\Templates\Xcode4
:: rm -rf ..\..\temp
:: hg archive -I ..\ui\Templates\Xcode4 ..\..\temp
:: mv ..\..\temp\ui\Templates\Xcode4 ..\..\release\Templates

mkdir "..\..\release\Templates\Eclipse\Android Template\assets"
mkdir "..\..\release\Templates\Eclipse\Android Template\gen"
mkdir "..\..\release\Templates\Eclipse\Android Template\res\layout"
mkdir "..\..\release\Templates\Xcode4\iOS Template\iOS Template\assets"

xcopy /S /I ..\samplecode ..\..\release\Examples
:: rem svn export ..\samplecode ..\..\release\Examples
:: rm -rf ..\..\temp
:: hg archive -I ..\samplecode ..\..\temp
:: mv ..\..\temp\samplecode ..\..\release\Examples

xcopy /S /I ..\ios\GiderosiOSPlayer ..\..\release\GiderosiOSPlayer
:: rem svn export ..\ios\GiderosiOSPlayer ..\..\release\GiderosiOSPlayer
:: rm -rf ..\..\temp
:: hg archive -I ..\ios\GiderosiOSPlayer ..\..\temp
:: mv ..\..\temp\ios\GiderosiOSPlayer ..\..\release

copy ..\gdrdeamon\release\gdrdeamon.exe ..\..\release\Tools
copy ..\gdrbridge\release\gdrbridge.exe ..\..\release\Tools
copy ..\gdrexport\release\gdrexport.exe ..\..\release\Tools


copy "..\ui\Templates\Xcode4\iOS Template\iOS Template\giderosapi.h" "..\..\release\Templates\Xcode4\iOS Template\iOS Template"
copy "..\ui\Templates\Xcode4\iOS Template\iOS Template\*.a"          "..\..\release\Templates\Xcode4\iOS Template\iOS Template"
copy "..\ui\Templates\Xcode4\iOS Template\iOS Template\giderosapi.h" "..\..\release\GiderosiOSPlayer\GiderosiOSPlayer"
copy "..\ui\Templates\Xcode4\iOS Template\iOS Template\*.a"          "..\..\release\GiderosiOSPlayer\GiderosiOSPlayer"

mkdir ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\Sdk\include\*.h                                                        ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy "..\plugins\Game Kit\source\iOS\gamekit.mm"                               ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy "..\plugins\Store Kit\source\iOS\storekit.mm"                             ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy "..\plugins\iAd\source\iOS\iad.mm"                                        ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\LuaSQLite3\source\lsqlite3.c                                   ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\LuaSQLite3\source\lsqlite3_stub.cpp                            ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\LuaSocket\source\luasocket_stub.cpp                            ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy "..\ui\Templates\Xcode4\iOS Template\iOS Template\Plugins\libluasocket.a" ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\LuaFileSystem\source\lfs.h                                     ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\LuaFileSystem\source\lfs.c                                     ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\LuaFileSystem\source\lfs_stub.cpp                              ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\BitOp\source\bit.c                                             ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\BitOp\source\bit_stub.cpp                                      ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\JSON\source\fpconv.c                                           ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\JSON\source\fpconv.h                                           ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\JSON\source\strbuf.c                                           ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\JSON\source\strbuf.h                                           ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\JSON\source\lua_cjson.c                                        ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
copy ..\plugins\JSON\source\lua_cjson_stub.cpp                                 ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins
mkdir "..\..\release\Templates\Xcode4\iOS Template\iOS Template\Plugins"
copy ..\..\release\GiderosiOSPlayer\GiderosiOSPlayer\Plugins\* "..\..\release\Templates\Xcode4\iOS Template\iOS Template\Plugins"

copy ..\android\GiderosAndroidPlayer\gideros.jar "..\..\release\Templates\Eclipse\Android Template"
mkdir "..\..\release\Templates\Eclipse\Android Template\jni"
copy ..\android\lib\jni\Application.mk "..\..\release\Templates\Eclipse\Android Template\jni"
xcopy /S /I ..\android\build\libs "..\..\release\Templates\Eclipse\Android Template"

copy ..\plugins\LuaSocket\source\libs\armeabi\libluasocket.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi"
copy ..\plugins\LuaSocket\source\libs\armeabi-v7a\libluasocket.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi-v7a"
copy ..\plugins\LuaSocket\source\libs\x86\libluasocket.so "..\..\release\Templates\Eclipse\Android Template\libs\x86"

copy ..\plugins\LuaFileSystem\source\libs\armeabi\liblfs.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi"
copy ..\plugins\LuaFileSystem\source\libs\armeabi-v7a\liblfs.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi-v7a"
copy ..\plugins\LuaFileSystem\source\libs\x86\liblfs.so "..\..\release\Templates\Eclipse\Android Template\libs\x86"

copy "..\plugins\Google Billing\source\Android\libs\armeabi\libggooglebilling.so" "..\..\release\Templates\Eclipse\Android Template\libs\armeabi"
copy "..\plugins\Google Billing\source\Android\libs\armeabi-v7a\libggooglebilling.so" "..\..\release\Templates\Eclipse\Android Template\libs\armeabi-v7a"
copy "..\plugins\Google Billing\source\Android\libs\x86\libggooglebilling.so" "..\..\release\Templates\Eclipse\Android Template\libs\x86"

copy ..\plugins\LuaSQLite3\source\libs\armeabi\liblsqlite3.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi"
copy ..\plugins\LuaSQLite3\source\libs\armeabi-v7a\liblsqlite3.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi-v7a"
copy ..\plugins\LuaSQLite3\source\libs\x86\liblsqlite3.so "..\..\release\Templates\Eclipse\Android Template\libs\x86"

copy ..\plugins\BitOp\source\libs\armeabi\libbitop.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi"
copy ..\plugins\BitOp\source\libs\armeabi-v7a\libbitop.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi-v7a"
copy ..\plugins\BitOp\source\libs\x86\libbitop.so "..\..\release\Templates\Eclipse\Android Template\libs\x86"

copy ..\plugins\JSON\source\libs\armeabi\libjson.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi"
copy ..\plugins\JSON\source\libs\armeabi-v7a\libjson.so "..\..\release\Templates\Eclipse\Android Template\libs\armeabi-v7a"
copy ..\plugins\JSON\source\libs\x86\libjson.so "..\..\release\Templates\Eclipse\Android Template\libs\x86"

:: rm -rf ..\..\temp
:: rem mkdir ..\..\temp
:: hg archive -I "..\plugins\Google Billing\source\Android\com" ..\..\temp
xcopy /S /I "..\plugins\Google Billing\source\Android\com" "..\..\release\Templates\Eclipse\Android Template\src"

REM SEEMS TO BE COPYING IN THE WRONG DIRECTION! (commented out)
:: xcopy /S /I "..\plugins\Google Billing\source\Android\com" ..\android\GiderosAndroidPlayer\src

:: rmdir /S /Q ..\android\GiderosAndroidPlayer\libs
:: xcopy /S /I "..\..\release\Templates\Eclipse\Android Template\libs" ..\android\GiderosAndroidPlayer

REM CREATE APK FOR ANDROID PLAYER
cd ..\android\GiderosAndroidPlayer
call ant debug
move bin\GiderosAndroidPlayer-debug.apk ..\..\..\release\GiderosAndroidPlayer.apk
cd ..\..\scripts

REM ZIP UP IOS PLAYER DIRECTORY (AND REMOVE DIR) (commented out)
:: cd ..\..\release
:: zip -r GiderosiOSPlayer.zip GiderosiOSPlayer
:: rmdir /S /Q GiderosiOSPlayer
:: cd ..\gideros\scripts

xcopy /S /I ..\doc ..\..\release\Documentation
:: rem svn export ..\doc ..\..\release\Documentation
:: rm -rf ..\..\temp
:: hg archive -I  ..\doc ..\..\temp
:: mv ..\..\temp\doc ..\..\release\Documentation

copy ..\licenses.txt ..\..\release\licenses.txt

xcopy /S /I ..\Sdk ..\..\release

xcopy /S /I ..\plugins "..\..\release\All Plugins"
:: rem svn export ..\plugins "..\..\release\All Plugins"
:: rm -rf ..\..\temp
:: hg archive -I ..\plugins ..\..\temp
:: mv ..\..\temp\plugins "..\..\release\All Plugins"

mkdir "..\..\release\All Plugins\BitOp\bin"
mkdir "..\..\release\All Plugins\Facebook\bin"
mkdir "..\..\release\All Plugins\Flurry\bin"
mkdir "..\..\release\All Plugins\Game Kit\bin"
mkdir "..\..\release\All Plugins\Google Billing\bin"
mkdir "..\..\release\All Plugins\iAd\bin"
mkdir "..\..\release\All Plugins\JSON\bin"
mkdir "..\..\release\All Plugins\LPeg\bin"
mkdir "..\..\release\All Plugins\LuaFileSystem\bin"
mkdir "..\..\release\All Plugins\LuaSocket\bin"
mkdir "..\..\release\All Plugins\LuaSQLite3\bin"
mkdir "..\..\release\All Plugins\Microphone\bin"
mkdir "..\..\release\All Plugins\Store Kit\bin"
mkdir "..\..\release\All Plugins\BitOp\bin\Windows"
copy ..\plugins\BitOp\source\release\bitop.dll "..\..\release\All Plugins\BitOp\bin\Windows"
mkdir "..\..\release\All Plugins\LuaSQLite3\bin\Windows"
copy "..\plugins\LuaSQLite3\source\release\lsqlite3.dll" "..\..\release\All Plugins\LuaSQLite3\bin\Windows"
mkdir "..\..\release\All Plugins\LuaSocket\bin\Windows"
copy "..\plugins\LuaSocket\source\release\luasocket.dll" "..\..\release\All Plugins\LuaSocket\bin\Windows"
mkdir "..\..\release\All Plugins\LuaFileSystem\bin\Windows"
copy "..\plugins\LuaFileSystem\source\release\lfs.dll" "..\..\release\All Plugins\LuaFileSystem\bin\Windows"
mkdir "..\..\release\All Plugins\LPeg\bin\Windows"
copy "..\plugins\LPeg\source\release\lpeg.dll" "..\..\release\All Plugins\LPeg\bin\Windows"
mkdir "..\..\release\All Plugins\Microphone\bin\Windows"
copy "..\plugins\Microphone\source\Desktop\release\microphone.dll" "..\..\release\All Plugins\Microphone\bin\Windows"
mkdir "..\..\release\All Plugins\JSON\bin\Windows"
copy "..\plugins\JSON\source\release\json.dll" "..\..\release\All Plugins\JSON\bin\Windows"


mkdir "..\..\release\All Plugins\BitOp\bin\Android"
mkdir "..\..\release\All Plugins\BitOp\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\BitOp\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\BitOp\bin\Android\x86"
copy ..\plugins\BitOp\source\libs\armeabi\libbitop.so "..\..\release\All Plugins\BitOp\bin\Android\armeabi" 
copy ..\plugins\BitOp\source\libs\armeabi-v7a\libbitop.so "..\..\release\All Plugins\BitOp\bin\Android\armeabi-v7a" 
copy ..\plugins\BitOp\source\libs\x86\libbitop.so "..\..\release\All Plugins\BitOp\bin\Android\x86" 

mkdir "..\..\release\All Plugins\LuaSocket\bin\Android"
mkdir "..\..\release\All Plugins\LuaSocket\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\LuaSocket\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\LuaSocket\bin\Android\x86"
copy ..\plugins\LuaSocket\source\libs\armeabi\libluasocket.so "..\..\release\All Plugins\LuaSocket\bin\Android\armeabi" 
copy ..\plugins\LuaSocket\source\libs\armeabi-v7a\libluasocket.so "..\..\release\All Plugins\LuaSocket\bin\Android\armeabi-v7a" 
copy ..\plugins\LuaSocket\source\libs\x86\libluasocket.so "..\..\release\All Plugins\LuaSocket\bin\Android\x86" 

mkdir "..\..\release\All Plugins\LPeg\bin\Android"
mkdir "..\..\release\All Plugins\LPeg\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\LPeg\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\LPeg\bin\Android\x86"
copy ..\plugins\LPeg\source\libs\armeabi\liblpeg.so "..\..\release\All Plugins\LPeg\bin\Android\armeabi" 
copy ..\plugins\LPeg\source\libs\armeabi-v7a\liblpeg.so "..\..\release\All Plugins\LPeg\bin\Android\armeabi-v7a" 
copy ..\plugins\LPeg\source\libs\x86\liblpeg.so "..\..\release\All Plugins\LPeg\bin\Android\x86" 

mkdir "..\..\release\All Plugins\LuaFileSystem\bin\Android"
mkdir "..\..\release\All Plugins\LuaFileSystem\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\LuaFileSystem\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\LuaFileSystem\bin\Android\x86"
copy ..\plugins\LuaFileSystem\source\libs\armeabi\liblfs.so "..\..\release\All Plugins\LuaFileSystem\bin\Android\armeabi" 
copy ..\plugins\LuaFileSystem\source\libs\armeabi-v7a\liblfs.so "..\..\release\All Plugins\LuaFileSystem\bin\Android\armeabi-v7a" 
copy ..\plugins\LuaFileSystem\source\libs\x86\liblfs.so "..\..\release\All Plugins\LuaFileSystem\bin\Android\x86" 

mkdir "..\..\release\All Plugins\LuaSQLite3\bin\Android"
mkdir "..\..\release\All Plugins\LuaSQLite3\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\LuaSQLite3\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\LuaSQLite3\bin\Android\x86"
copy ..\plugins\LuaSQLite3\source\libs\armeabi\liblsqlite3.so "..\..\release\All Plugins\LuaSQLite3\bin\Android\armeabi" 
copy ..\plugins\LuaSQLite3\source\libs\armeabi-v7a\liblsqlite3.so "..\..\release\All Plugins\LuaSQLite3\bin\Android\armeabi-v7a"
copy ..\plugins\LuaSQLite3\source\libs\x86\liblsqlite3.so "..\..\release\All Plugins\LuaSQLite3\bin\Android\x86"

mkdir "..\..\release\All Plugins\Microphone\bin\Android"
mkdir "..\..\release\All Plugins\Microphone\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\Microphone\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\Microphone\bin\Android\x86"
copy ..\plugins\Microphone\source\Android\libs\armeabi\libmicrophone.so "..\..\release\All Plugins\Microphone\bin\Android\armeabi" 
copy ..\plugins\Microphone\source\Android\libs\armeabi-v7a\libmicrophone.so "..\..\release\All Plugins\Microphone\bin\Android\armeabi-v7a"
copy ..\plugins\Microphone\source\Android\libs\x86\libmicrophone.so "..\..\release\All Plugins\Microphone\bin\Android\x86"

mkdir "..\..\release\All Plugins\JSON\bin\Android"
mkdir "..\..\release\All Plugins\JSON\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\JSON\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\JSON\bin\Android\x86"
copy ..\plugins\JSON\source\libs\armeabi\libjson.so "..\..\release\All Plugins\JSON\bin\Android\armeabi" 
copy ..\plugins\JSON\source\libs\armeabi-v7a\libjson.so "..\..\release\All Plugins\JSON\bin\Android\armeabi-v7a"
copy ..\plugins\JSON\source\libs\x86\libjson.so "..\..\release\All Plugins\JSON\bin\Android\x86"

mkdir "..\..\release\All Plugins\Flurry\bin\Android"
mkdir "..\..\release\All Plugins\Flurry\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\Flurry\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\Flurry\bin\Android\x86"
copy ..\plugins\Flurry\source\Android\libs\armeabi\libflurry.so "..\..\release\All Plugins\Flurry\bin\Android\armeabi" 
copy ..\plugins\Flurry\source\Android\libs\armeabi-v7a\libflurry.so "..\..\release\All Plugins\Flurry\bin\Android\armeabi-v7a"
copy ..\plugins\Flurry\source\Android\libs\x86\libflurry.so "..\..\release\All Plugins\Flurry\bin\Android\x86"

mkdir "..\..\release\All Plugins\Facebook\bin\Android"
mkdir "..\..\release\All Plugins\Facebook\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\Facebook\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\Facebook\bin\Android\x86"
copy ..\plugins\Facebook\source\Android\libs\armeabi\libfacebook.so "..\..\release\All Plugins\Facebook\bin\Android\armeabi" 
copy ..\plugins\Facebook\source\Android\libs\armeabi-v7a\libfacebook.so "..\..\release\All Plugins\Facebook\bin\Android\armeabi-v7a"
copy ..\plugins\Facebook\source\Android\libs\x86\libfacebook.so "..\..\release\All Plugins\Facebook\bin\Android\x86"

mkdir "..\..\release\All Plugins\Google Billing\bin\Android"
mkdir "..\..\release\All Plugins\Google Billing\bin\Android\armeabi"
mkdir "..\..\release\All Plugins\Google Billing\bin\Android\armeabi-v7a"
mkdir "..\..\release\All Plugins\Google Billing\bin\Android\x86"
copy "..\plugins\Google Billing\source\Android\libs\armeabi\libggooglebilling.so" "..\..\release\All Plugins\Google Billing\bin\Android\armeabi" 
copy "..\plugins\Google Billing\source\Android\libs\armeabi-v7a\libggooglebilling.so" "..\..\release\All Plugins\Google Billing\bin\Android\armeabi-v7a"
copy "..\plugins\Google Billing\source\Android\libs\x86\libggooglebilling.so" "..\..\release\All Plugins\Google Billing\bin\Android\x86"


mkdir ..\..\release\Plugins
copy ..\plugins\lsqlite3\source\release\lsqlite3.dll ..\..\release\Plugins
copy ..\plugins\LuaSocket\source\release\luasocket.dll ..\..\release\Plugins
copy ..\plugins\lfs\source\release\lfs.dll ..\..\release\Plugins
copy ..\plugins\Microphone\source\Desktop\release\microphone.dll ..\..\release\Plugins
copy ..\plugins\BitOp\source\release\bitop.dll ..\..\release\Plugins
copy ..\plugins\JSON\source\release\json.dll ..\..\release\Plugins
