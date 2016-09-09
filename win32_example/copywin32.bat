rmdir /s /q ..\..\release\Templates\win32
mkdir ..\..\release\Templates\win32
mkdir ..\..\release\Templates\win32\WindowsDesktopTemplate
mkdir ..\..\release\Templates\win32\WindowsDesktopTemplate\plugins

copy plugins\*.dll ..\..\release\templates\win32\WindowsDesktopTemplate\plugins

copy win32.exe ..\..\release\Templates\win32\WindowsDesktopTemplate
copy win32noconsole.exe ..\..\release\Templates\win32\WindowsDesktopTemplate
copy *.dll ..\..\release\Templates\win32\WindowsDesktopTemplate
