print(os.getenv("GIDEROS_STUDIO_DATA"))
pcall (function ()
    GIDEROS_STUDIO_DATA=loadstring("return "..os.getenv("GIDEROS_STUDIO_DATA"))()
    print(GIDEROS_STUDIO_DATA.editFile)
end)

function ReloadFile()
    data=nil
    pcall(function ()
        local f=io.open(GIDEROS_STUDIO_DATA.editFile,"rb")
        data=f:read("*a")
        f:close()
    end)
    if not data or #data~=204 then 
        data=string.char(0,1).."XF"..(string.char(0):rep(200)) 
        print(#data)
        FileReset()
    end
end

require("json")

function loadMap(filename)
	local file=io.open(filename..".map","r")
	if file then
		local text=file:read()
		file:close()
		if text then
			local map=json.decode(text)
			if map then
				map.name=filename
				return map
			end
		end
	end
end

local filename=string.sub(GIDEROS_STUDIO_DATA.editFile,1,-5)
local map=loadMap(filename)

if map then
	for i=1, #map.tilesets do
		local tileset = map.tilesets[i]
		
		tileset.sizex = math.floor((tileset.imagewidth - tileset.margin + tileset.spacing) / (tileset.tilewidth + tileset.spacing))
		tileset.sizey = math.floor((tileset.imageheight - tileset.margin + tileset.spacing) / (tileset.tileheight + tileset.spacing))
		tileset.lastgid = tileset.firstgid + (tileset.sizex * tileset.sizey) - 1

		tileset.texture = Texture.new(map.name..".png",true)
	end

	local function gid2tileset(gid)
		for i=1, #map.tilesets do
			local tileset = map.tilesets[i]
		
			if tileset.firstgid <= gid and gid <= tileset.lastgid then
				return tileset
			end
		end
	end

	local allLayers = Sprite.new()

	for i=1, #map.layers do
		local layer = map.layers[i]

		local tilemaps = {}
		local group = Sprite.new()

		for y=1,layer.height do
			for x=1,layer.width do
				local i = x + (y - 1) * layer.width
				local gid = layer.data[i]
				local tileset = gid2tileset(gid)
				
				if tileset then
					local tilemap = nil
					if tilemaps[tileset] then
						tilemap = tilemaps[tileset]
					else
						tilemap = TileMap.new(layer.width, 
											  layer.height,
											  tileset.texture,
											  tileset.tilewidth,
											  tileset.tileheight,
											  tileset.spacing,
											  tileset.spacing,
											  tileset.margin,
											  tileset.margin,
											  map.tilewidth,
											  map.tileheight)
						tilemaps[tileset] = tilemap
						group:addChild(tilemap)
					end
					
					local tx = (gid - tileset.firstgid) % tileset.sizex + 1
					local ty = math.floor((gid - tileset.firstgid) / tileset.sizex) + 1
					
					tilemap:setTile(x, y, tx, ty)
				end
			end
		end

		group:setAlpha(layer.opacity)
		
		allLayers:addChild(group)
	end

	stage:addChild(allLayers)

	local dragging, startx, starty

	local function onMouseDown(event)
		dragging = true
		startx = event.x
		starty = event.y
	end

	local function onMouseMove(event)
		if dragging then
			local dx = event.x - startx
			local dy = event.y - starty
			allLayers:setX(allLayers:getX() + dx)
			allLayers:setY(allLayers:getY() + dy)
			startx = event.x
			starty = event.y
		end
	end

	local function onMouseUp(event)
		dragging = false
	end

	stage:addEventListener(Event.MOUSE_DOWN, onMouseDown)
	stage:addEventListener(Event.MOUSE_MOVE, onMouseMove)
	stage:addEventListener(Event.MOUSE_UP, onMouseUp)
end