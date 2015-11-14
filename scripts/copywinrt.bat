:: After compiling the WinRT version of Gideros, we need to copy library and project files to the "release"
:: folder ready for packaging with NSIS (or into a DMG file on Mac OS X)
:: This batch file is located in "scripts" directory

rmdir /s /q ..\..\release\Templates\VisualStudio
xcopy /S /E /I ..\ui\Templates\VisualStudio ..\..\release\Templates\VisualStudio

:: x86 release version for Windows
copy ..\winrt\Release\gideros.Windows\gideros.Windows.lib "..\..\release\Templates\VisualStudio\WinRT Template"
copy ..\winrt\Release\luawinrt.Windows\luawinrt.Windows.lib "..\..\release\Templates\VisualStudio\WinRT Template"
copy ..\winrt\Release\libgvfswinrt.Windows\libgvfswinrt.Windows.lib "..\..\release\Templates\VisualStudio\WinRT Template"
copy ..\Release\AllPlugins\WinRT\Release\Win32\luasocket.Windows.lib "..\..\release\Templates\VisualStudio\WinRT Template"

:: ARM release version for WinPhone
copy ..\winrt\ARM\Release\gideros.WindowsPhone\gideros.WindowsPhone.lib "..\..\release\Templates\VisualStudio\WinRT Template"
copy ..\winrt\ARM\Release\luawinrt.WindowsPhone\luawinrt.WindowsPhone.lib "..\..\release\Templates\VisualStudio\WinRT Template"
copy ..\winrt\ARM\Release\libgvfswinrt.WindowsPhone\libgvfswinrt.WindowsPhone.lib "..\..\release\Templates\VisualStudio\WinRT Template"
copy ..\Release\AllPlugins\WinRT\Release\ARM\luasocket.WindowsPhone.lib "..\..\release\Templates\VisualStudio\WinRT Template"
