--require your plugin
require("MAPPLUGIN")

-- Define map plugin special values:

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


-- Get the physical resolution of the device
SCREEN_WIDTH = application:getDeviceWidth()
SCREEN_HEIGHT = application:getDeviceHeight()

text = "map plugin demo"

tf = TextField.new(nil, text)
tf:setPosition(10, 100);
tf:setScale(2)
stage:addChild(tf)

-- Create a map:
mp = MAPPLUGIN.new()

-- Position the map to fill the lower 60% of the screen:
mp:setPosition(0, SCREEN_HEIGHT * 0.4)
mp:setDimensions(SCREEN_WIDTH, SCREEN_HEIGHT * 0.6)

-- Switch the map to a satellite with road overlays (hybrid)
mp:setType(MAP_TYPE_HYBRID)

-- Zoom the map in
mp:setZoom(14)

-- Center the map on La Crosse, Wisconsin, USA
mp:setCenterCoordinates(43.812, -91.257)


local marker_bridge = mp:addMarker(43.809178, -91.259375, "Cass Street Bridge")
local marker_statue = mp:addMarker(43.815209, -91.256017, "Eagle Statue")

mp:setMarkerHue(marker_statue, 180.0)


stage:addEventListener(Event.ENTER_FRAME, 
	function()
	
		-- If user selects a marker, show that in the text field:
		local clicked_marker_index = mp:getClickedMarkerIndex()
		if (clicked_marker_index ~= MAP_NO_MARKER_CLICKED) then
			local clicked_marker_title = mp:getMarkerTitle(clicked_marker_index)
			local text = string.format("Clicked marker %d: %s", clicked_marker_index, clicked_marker_title)
			tf:setText(text)
		end

		-- Check for mouse clicks and add more markers:
		local map_was_clicked = mp:mapClicked()
		if (map_was_clicked == 1) then
			-- See where the map was clicked:
			local lat = mp:getMapClickLatitude()
			local lon = mp:getMapClickLongitude()
			
			-- Tell the user about the click:
			tf:setText(string.format("Clicked map at %1.4f, %1.4ff", lat, lon))
			
			-- Create a new marker at that location:
			local new_marker_index = mp:addMarker(lat, lon, "title")
			
			-- Give the marker a title using its index:
			mp:setMarkerTitle(new_marker_index, string.format("Marker idx %d", new_marker_index))
			
			-- Give it a random color:
			mp:setMarkerHue(new_marker_index, math.random(1, 360))
			
			-- Make the marker semi-transparent
			mp:setMarkerAlpha(new_marker_index, 80)
		end
	end
, self)	
