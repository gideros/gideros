
local clown=Bitmap.new(Texture.new("clown.png"))
local w,h=clown:getWidth(),clown:getHeight()

local render=RenderTarget.new(w,h)
render:clear(0,0)
render:draw(clown)

local colours=RenderTarget.new(256,256)
local colourTable={}

local size=12

pixelClown=TileMap.new(w,h,colours,size,size,0,0,0,0,size+1,size+1)

for y=0,h-1 do
	for x=0,w-1 do
		local c,a=render:getPixel(x,y)
		if a>0 then
			local found=0
			for loop=1,#colourTable do
				if c==colourTable[loop] then
					found=loop
					break
				end
			end
			if found==0 then
				colourTable[#colourTable+1]=c
				found=#colourTable
				colours:clear(c,a,found*size,0,size,size)
				for w=0,size-1 do
					colours:clear(0x000000,1,found*size+w,0,1,1)
					colours:clear(0x000000,1,found*size+w,size-1,1,1)
				end
				for h=0,size-1 do
					colours:clear(0x000000,1,found*size,h,1,1)
					colours:clear(0x000000,1,found*size+size-1,h,1,1)
				end
				print("no",found,"colour",c,"alpha",a)
			end
			pixelClown:setTile(x+1,y+1,found+1,1,0)
		end
	end
end

pixelClown:setPosition(200,200)
pixelClown:setScale(2)

-- put my palette on screen!
colourTest=Bitmap.new(colours)
colourTest:setScale(4)
stage:addChild(colourTest)

colours:clear(0xff00ff,1,4,1,1,1)

-- put the fast pixel sprite on screen...
stage:addChild(pixelClown)

function gameLoop(e)
	pixelClown:setRotation(pixelClown:getRotation()+0.5)
	colours:clear(math.random(0xffffff),1,4*size+1,1,size-2,size-2)
end

stage:addEventListener(Event.ENTER_FRAME,gameLoop)