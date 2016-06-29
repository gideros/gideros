
local clown=Bitmap.new(Texture.new("clown.png"))
local w,h=clown:getWidth(),clown:getHeight()

local render=RenderTarget.new(w,h)
render:clear(0,0)
render:draw(clown)

local pixelClown=Sprite.new()
pixels={}

for y=0,h-1 do
	for x=0,w-1 do
		local c,a=render:getPixel(x,y)
		if a>0 then
			pixels[#pixels+1]=Pixel.new(c,math.random(),9,9)
			pixels[#pixels]:setPosition((-w*5)+x*10,(-h*5)+y*10)
			pixels[#pixels]:setAnchorPosition(4.5,4.5)
			pixelClown:addChild(pixels[#pixels])
		end
	end
end

pixelClown:setPosition(200,200)

stage:addChild(pixelClown)

function gameLoop(e)
	pixelClown:setRotation(pixelClown:getRotation()+1)
	for loop=1,#pixels do
		local c,a=pixels[loop]:getColor()
		a=a+0.03
		if a>1 then a=a-1 end
		pixels[loop]:setColor(c,a)
	end
end

stage:addEventListener(Event.ENTER_FRAME,gameLoop)