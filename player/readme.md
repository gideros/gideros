# Gideros Player
### This is the player simulator to Gideros Studio run on desktop.

Features added:

* Zoom In and Out option, controlled by Alt + Arrow Up or Alt + Arrow Down
* Fit to Window by zooming
* Hide Menu bar option
* Auto Scale option, the canvas occupies all the player screen (but the player dont resize anymore when resolution are changed
* Draw Infos option, to show the player informations when the app is running
* Some resolutions added
* Configurations dialog added, to control some commom options and aditional color options (Player window, canvas background and info text color)
* Settings/GiderosPlayer.ini automatically created to store some configurations


##### Important

Don't forget to build these dependencies, in this order, before build this project:
* libpystring
* libgvfs
* liblua
* libgid
* libgideros


To make a easy workflow, add this custom steps (Add Build Step -> Custom Process Step) to Qt Creator:


##### Release (Windows)

To copy generated files from build player process to root\release folder:

* Command: cmd
* Arguments: /C %{buildDir}\scripts\copy-generated.bat
* Working directory: %{buildDir}

To copy required player files previously builded to root\release folder:

* Command: cmd
* Arguments: /C %{buildDir}\scripts\copy-required.bat
* Working directory: %{buildDir}

To start the player from root\release folder:

* Command: cmd
* Arguments: /C %{buildDir}\scripts\start-player.bat
* Working directory: %{buildDir}