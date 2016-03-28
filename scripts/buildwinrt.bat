REM consider /clp:Verbosity=minimal

cd ..\plugins\luasocket\source\winrt\luasocket
msbuild luasocket.sln /p:Configuration=Debug   /p:Platform=Win32 /clp:ErrorsOnly
msbuild luasocket.sln /p:Configuration=Release /p:Platform=Win32 /clp:ErrorsOnly
msbuild luasocket.sln /p:Configuration=Debug   /p:Platform=ARM   /clp:ErrorsOnly
msbuild luasocket.sln /p:Configuration=Release /p:Platform=ARM   /clp:ErrorsOnly

cd ..\..\..\..\..\2dsg\gfxbackends\dx11

call compileshaders.bat

cd ..\..\..\winrt
msbuild gideros.sln /p:Configuration=Debug   /p:Platform=Win32 /clp:ErrorsOnly
msbuild gideros.sln /p:Configuration=Release /p:Platform=Win32 /clp:ErrorsOnly
msbuild gideros.sln /p:Configuration=Debug   /p:Platform=ARM   /clp:ErrorsOnly
msbuild gideros.sln /p:Configuration=Release /p:Platform=ARM   /clp:ErrorsOnly

cd ..\winrt_example
REM msbuild giderosgame.sln /t:publish /p:Configuration=Debug   /p:Platform=Win32 /clp:ErrorsOnly /p:AppxBundle=Always
REM msbuild giderosgame.sln /t:publish /p:Configuration=Debug   /p:Platform=ARM   /clp:ErrorsOnly /p:AppxBundle=Always

msbuild giderosgame.sln /t:publish /p:Configuration=Release /p:Platform=Win32 /clp:ErrorsOnly /p:AppxBundle=Always
msbuild giderosgame.sln /t:publish /p:Configuration=Release /p:Platform=ARM   /clp:ErrorsOnly /p:AppxBundle=Always

cd ..\scripts
