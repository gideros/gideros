cd ..\plugins\luasocket\source\winrt\luasocket
msbuild luasocket.sln /p:Configuration=Debug   /p:Platform=Win32 /clp:Verbosity=minimal
msbuild luasocket.sln /p:Configuration=Release /p:Platform=Win32 /clp:Verbosity=minimal
msbuild luasocket.sln /p:Configuration=Debug   /p:Platform=ARM   /clp:Verbosity=minimal
msbuild luasocket.sln /p:Configuration=Release /p:Platform=ARM   /clp:Verbosity=minimal

cd ..\..\..\..\..\2dsg\gfxbackends\dx11

call compileshaders.bat

cd ..\..\..\winrt
msbuild gideros.sln /p:Configuration=Debug   /p:Platform=Win32 /clp:Verbosity=minimal
msbuild gideros.sln /p:Configuration=Release /p:Platform=Win32 /clp:Verbosity=minimal
msbuild gideros.sln /p:Configuration=Debug   /p:Platform=ARM   /clp:Verbosity=minimal
msbuild gideros.sln /p:Configuration=Release /p:Platform=ARM   /clp:Verbosity=minimal

cd ..\winrt_example\giderosgame\giderosgame.Windows
msbuild giderosgame.Windows.vcxproj /t:publish /p:Configuration=Release /p:Platform=Win32 /clp:Verbosity=minimal /p:AppxBundle=Always

cd ..\giderosgame.WindowsPhone
msbuild giderosgame.WindowsPhone.vcxproj /t:publish /p:Configuration=Release /p:Platform=ARM /clp:Verbosity=minimal /p:AppxBundle=Always

cd ..\..\..\scripts
