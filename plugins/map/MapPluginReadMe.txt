These files implement native mapping with annoations (marked locations) for Gideros projects on Android and iOS.

The AndroidPlugin folder contains instructions and files for adding the plugin to a project exported for Android using Eclipse.
The IOSPlugin folder contains instructions and files for adding the plugin to a project exported for iOS using Xcode.
The source folder contains the source code for building the libraries for the Android plugin.
The GiderosProject folder contains a sample project using the map plugin.

Note that map overlaps this portion of the screen and any other graphical content the app draws on that region of the screen will be obscured.  You can show or hide the map through the Lua code, but when shown, the map overlaps all other graphics, so it is not possible to render anything over the top of the map.

This interface design users polling rather than callbacks to simplify the implementation of the plugin code connecting the Lua interface with the native code.  That is, rather than using event listeners or other callback functions to handle events, you simply check for such events every frame.  For example, to see what map marker (if any) the user has clicked, you would call Map:getClickedMarkerIndex() within onEnterFrame(), and do whatever you want to do in response to a clicked marker if a valid index is returned.


Lua interface:

require("MAPPLUGIN"): Loads the plugin - must be included before using any of the map plugin methods.

Map:new(): Constructor

Map:show(): Display the map (map is shown by default)

Map:hide(): Hide the map

Map:setDimensions(int width, int height): Set size of map in pixels in device pixels*

Map:setPosition(int x, int y): Set position of map on screen in device pixels from top left*

Map:setCenterCoordinates(double latitude, double longitude): Center the map around the specified coordinates

Map:setZoom(float zoom): Set zoom level from 0 (very large area) to 19 (zoomed in close)

Map:setType(int map_type): Switch between MAP_TYPE_NORMAL, MAP_TYPE_SATELLITE, MAP_TYPE_HYBRID

Map:setMyLocationEnabled(boolean enabled): Turn on/off the “My location” button

Map:mapClicked(): Return true if the map was clicked since last call

Map:getMapClickLatitude(): Return latitude of last click on the map

Map:getMapClickLongitude(): Return longitude of last click on the map

Map:getCenterLatitude(): Returns latitude of center of area shown

Map:getCenterLongitude(): Returns longitude of center of area shown

Map:clear(): Remove all markers

Map:addMarker(double latitude, double longitude, String title): Adds a marker, and returns an index that can be used to access it. Due to the asynchronous processing, adding multiple markers at once may cause incorrect indexes to be returned.  See addMarkerAtIndex below.

Map:addMarkerAtIndex(double latitude, double longitude, String title, int index): Adds a marker at the given index.  Indexes should be used in order, starting at 0.

Map:setMarkerTitle(int marker_idx, String title): Sets the title of the specified marker

Map:setMarkerHue(int marker_idx, float hue): Sets the color of the specified marker in degrees around a color wheel, from red at 0 through the colors and back to red at 360. Defined for convenience: MAP_MARKER_RED, MAP_MARKER_ORANGE, MAP_MARKER_YELLOW, MAP_MARKER_GREEN, MAP_MARKER_BLUE, MAP_MARKER_PURPLE

Map:setMarkerAlpha(int marker_idx, float alpha): Set the alpha of the specified marker from 0 (fully transparent) to 100 (fully opaque)

Map:setMarkerCoordinates(int marker_idx, double latitude, double longitude): Moves the specified marker to the given coordinates

Map:String getMarkerTitle(int marker_idx): Sets the title of the specified marker

Map:getMarkerLatitude(int marker_idx): Returns the latitude of the specified marker

Map:getMarkerLongitude(int marker_idx): Returns the longitude of the specified marker

Map:getClickedMarkerIndex(): Return the index of the last marker clicked, if any since last call, or MAP_NO_MARKER_CLICKED

* Note that the map size and position on the screen are specified in device pixels.  You'll want to use application:getDeviceWidth() and application:getDeviceHeight() to get the size of the screen on the actual device, and size and position your map relative to that width and height.

Special values (may be copied to your Lua source):

	MAP_TYPE_NORMAL = 0
	MAP_TYPE_SATELLITE = 1
	MAP_TYPE_HYBRID = 2 -- (Satellite with roads, etc. overlaid)

	MAP_NO_MARKER_CLICKED = -1 -- (return value from getClickedMarkerIndex() if no marker has been clicked since the previous call)
	MAP_INVALID_COORDINATE = 1000.0 -- (return value from getMapClickedLatitude() and getMapCLickedLongitude() if the map has not been clicked since the previous call)
	
	MAP_MARKER_RED = 0.0
	MAP_MARKER_ORANGE = 22.5
	MAP_MARKER_YELLOW = 45.0
	MAP_MARKER_GREEN = 90.0
	MAP_MARKER_BLUE = 180.0
	MAP_MARKER_PURPLE = 270.0
