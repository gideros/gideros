
pcall(function() require "json" end)

pcall(function() require "chroma" end)
if Chroma then
	print("Chroma successfully loaded")
	
	chroma=Chroma.new()

	chroma:clear("keyboard")
	local h=chroma:getHeight("keyboard")
	for y=1,h do
		for x=1,0xf do
			chroma:setColor("keyboard",x,y,(x*16)<<16)
			chroma:setColor("keyboard",31-x,y,(x*16)<<16)
		end
	end
	for loop=1,12 do
		chroma:setKey("f"..loop,0x100-(loop*0x10))
	end
	chroma:setKey({"w","up"},0xffffff)
	chroma:setKey({"z","down"},0xffffff)
	chroma:setKey({"a","left"},0xffffff)
	chroma:setKey({"s","right"},0xffffff)
end

frameCounter=0
test1=false

function gameLoop(e)
	frameCounter+=1
	if chroma and chroma:isReady() then
		if not test1 then
			chroma:effect("mouse","static",255<<8)
			chroma:effect("mousepad","static",255<<16)
			test1=true
		end
		chroma:colorScroll("keyboard",1,0,true)
	end
end

stage:addEventListener(Event.ENTER_FRAME,gameLoop)
