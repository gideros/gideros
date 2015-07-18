application:setBackgroundColor(0xa0b0ff)

stage:addChild(Game.new())

local fps = TextField.new(nil, "")
stage:addChild(fps)
fps:setPosition(2, 20)

fps:setScale(2)

local frame = 0
local timer = os.timer()
local function displayFps()
	frame = frame + 1
	if frame == 20 then
		local currentTimer = os.timer()
		fps:setText(math.floor(20 / (currentTimer - timer) + 0.5))
		frame = 0
		timer = currentTimer	
	end
end
 
stage:addEventListener(Event.ENTER_FRAME, displayFps)

collectgarbage()
