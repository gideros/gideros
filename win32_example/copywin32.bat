rmdir /s /q ..\..\release\Templates\win32
mkdir ..\..\release\Templates\win32
mkdir ..\..\release\Templates\win32\WindowsDesktopTemplate
xcopy /s /e /i ..\..\release\plugins ..\..\release\templates\win32\WindowsDesktopTemplate\plugins

copy win32.exe ..\..\release\Templates\win32\WindowsDesktopTemplate
copy win32noconsole.exe ..\..\release\Templates\win32\WindowsDesktopTemplate
copy *.dll ..\..\release\Templates\win32\WindowsDesktopTemplate
