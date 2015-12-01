--Conatiner
local c=Sprite.new()
stage:addChild(c)
c:setPosition(100,100)

--Moon
local p=Path2D.new()
local ms="MQQZ" --MoveTo, QuadTo, QuadTo, Close
local mp={100,0, -50,100, 100,200, 20,100, 100,0 }
p:setPath(ms,mp) --Set the path from a set of commands and coordinates
p:setLineThickness(3) -- Outline width
p:setFillColor(0xE0E0E0,0.7) --Fill color
p:setLineColor(0xC0C0C0) --Line color
p:setAnchorPosition(100,100)
c:addChild(p)

--Banana shape, SVG path format
local banana="M8.64,223.948c0,0,143.468,3.431,185.777-181.808c2.673-11.702-1.23-20.154,1.316-33.146h16.287c0,0-3.14,17.248,1.095,30.848c21.392,68.692-4.179,242.343-204.227,196.59L8.64,223.948z"
p=Path2D.new()
p:setSvgPath(banana) --Set the path from a SVG path description
p:setLineThickness(5) -- Outline width
p:setFillColor(0xFFFF80,0.7) --Fill color
p:setLineColor(0x404000) --Line color
p:setAnchorPosition(100,100)
c:addChild(p)

--Animate container
local mc = MovieClip.new{
	{1, 100, c, {scale = {0.5,2,"inOutSine"}}},	
	{101, 200, c, {scale = {2,0.5,"inOutSine"}}},	
}
mc:setGotoAction(200,1)
c.mc=mc
