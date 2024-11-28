application:setBackgroundColor(0xa0b0ff)

stage:addChild(Game.new())

local fps = TextField.new(nil, "")
fps:setScale(2)
fps:setPosition(2, 20)
stage:addChild(fps)

local frame = 0
local timer, currentTimer = os.timer(), 0
local function displayFps()
	frame += 1
	if frame == 20 then
		currentTimer = os.timer()
		fps:setText((20/(currentTimer-timer)+0.5)//1)
		frame = 0
		timer = currentTimer	
	end
end
stage:addEventListener(Event.ENTER_FRAME, displayFps)

collectgarbage()
