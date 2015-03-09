pushd \
call C:\Qt\Qt5.3.2\5.3\mingw482_32\bin\qtenv2.bat
popd

cd ..
cd plugins

for /D %%s in (*) do (

cd %%s
cd source
if exist %%s.pro (
qmake %%s.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
)
cd ..\..

)

cd ..
cd scripts
