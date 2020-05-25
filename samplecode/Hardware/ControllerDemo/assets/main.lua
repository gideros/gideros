
-- first get the joypad image on screen, with all controls independent
shoulderTexture=Texture.new("images/shoulder.png",true)
triggerTexture=Texture.new("images/trigger.png",true)
thumbTexture=Texture.new("images/thumb.png",true)
dpadTexture=Texture.new("images/dpad.png",true)
buttonTexture=Texture.new("images/button.png",true)

shellSprite=Bitmap.new(Texture.new("images/shell.png",true))
shellSprite:setAnchorPoint(0.5,0.5)

triggers={}
buttons={}
thumbOrigins={Sprite.new(),Sprite.new()}
dpadOrigin=Sprite.new()
buttonsOrigin=Sprite.new()

do
	local rlduA={90,270,180,0}
	local rlduX={1,-1,0,0}
	local rlduY={0,0,1,-1}
	for loop=1,14 do
		if loop<=2 then
			triggers[loop]=Bitmap.new(triggerTexture)
			triggers[loop]:setAnchorPoint(0.5,0.5)
			triggers[loop]:setPosition(rlduX[loop]*140,-350)
		end
		if loop<=4 then
			buttons[loop]=Bitmap.new(dpadTexture)
			buttons[loop]:setPosition(rlduX[loop]*29,rlduY[loop]*29)
			buttons[loop]:setRotation(rlduA[loop])
			dpadOrigin:addChild(buttons[loop])
		elseif loop<=10 then
			buttons[loop]=Bitmap.new(buttonTexture)
			if loop<=8 then
				buttons[loop]:setPosition(rlduX[loop-4]*35,rlduY[loop-4]*35)
				buttonsOrigin:addChild(buttons[loop])
			else
				buttons[loop]:setPosition(rlduX[loop-8]*60,-127)
			end
		elseif loop<=12 then
			buttons[loop]=Bitmap.new(thumbTexture)
			thumbOrigins[loop-10]:addChild(buttons[loop])
		elseif loop<=14 then
			buttons[loop]=Bitmap.new(shoulderTexture)
			buttons[loop]:setPosition(rlduX[loop-12]*150,-280)
		end
		buttons[loop]:setAnchorPoint(0.5,0.5)
	end
	thumbOrigins[1]:setPosition(98,5)
	thumbOrigins[2]:setPosition(-183,-127)
	dpadOrigin:setPosition(-92,5)
	buttonsOrigin:setPosition(183,-127)
end

joypad=Sprite.new()
joypad:setScale(0.5)
joypad:setPosition(300,300)

joypad:addChild(shellSprite)
for loop=1,2 do
	joypad:addChild(buttons[loop+8]) -- back and start
	joypad:addChild(buttons[loop+12]) -- shoulders
	joypad:addChild(triggers[loop])
	joypad:addChild(thumbOrigins[loop])
end

joypad:addChild(dpadOrigin)
joypad:addChild(buttonsOrigin)

stage:addChild(joypad)

local udlr,debouncedUdlr,oldUdlr=0,0,0

-- simulate joypad with keyboard
stage:addEventListener(Event.KEY_DOWN,function(e)
	local k=e.keyCode
	if k==KeyCode.ESC then			udlr=udlr|0b00001000000000
	elseif k==KeyCode.ENTER then	udlr=udlr|0b00000100000000
	elseif k==KeyCode.Q then		udlr=udlr|0b00000010000000
	elseif k==KeyCode.W then		udlr=udlr|0b00000001000000
	elseif k==KeyCode.E then		udlr=udlr|0b00000000100000
	elseif k==KeyCode.SPACE then	udlr=udlr|0b00000000010000
	elseif k==KeyCode.UP then		udlr=udlr|0b00000000001000
	elseif k==KeyCode.DOWN then		udlr=udlr|0b00000000000100
	elseif k==KeyCode.LEFT then		udlr=udlr|0b00000000000010
	elseif k==KeyCode.RIGHT then	udlr=udlr|0b00000000000001
	end
end)

stage:addEventListener(Event.KEY_UP,function(e)
	local k=e.keyCode
	if k==KeyCode.ESC then			udlr=udlr&0b11110111111111
	elseif k==KeyCode.ENTER then	udlr=udlr&0b11111011111111
	elseif k==KeyCode.Q then		udlr=udlr&0b11111101111111
	elseif k==KeyCode.W then		udlr=udlr&0b11111110111111
	elseif k==KeyCode.E then		udlr=udlr&0b11111111011111
	elseif k==KeyCode.SPACE then	udlr=udlr&0b11111111101111
	elseif k==KeyCode.UP then		udlr=udlr&0b11111111110111
	elseif k==KeyCode.DOWN then		udlr=udlr&0b11111111111011
	elseif k==KeyCode.LEFT then		udlr=udlr&0b11111111111101
	elseif k==KeyCode.RIGHT then	udlr=udlr&0b11111111111110
	end
end)

function findKeyCode(code)
	for i, val in pairs(KeyCode) do
		if val == code then
			return i
		end
	end
	return "unknown"
end

print("Probing")
pcall(function() require "controller" end)
if controller then
	print("CONTROLLER PLUGIN PRESENT")
	
--print("any controllers", controller:isAnyAvailable())
--print("controller count", controller:getPlayerCount())
--print("controller 1", controller:getControllerName(1))
--print("players", controller:getPlayers()[1])
--print("controller 1 vibrate", controller:vibrate(1, 1000))

	controller:addEventListener(Event.KEY_DOWN, function(e)
		local k=e.keyCode
		if k==KeyCode.BUTTON_BACK then 		udlr=udlr|0b00001000000000
		elseif k==KeyCode.BUTTON_MENU then 	udlr=udlr|0b00000100000000
		elseif k==KeyCode.BUTTON_Y then		udlr=udlr|0b00000010000000
		elseif k==KeyCode.BUTTON_A then		udlr=udlr|0b00000001000000
		elseif k==KeyCode.BUTTON_X then		udlr=udlr|0b00000000100000
		elseif k==KeyCode.BUTTON_B then		udlr=udlr|0b00000000010000
		elseif k==KeyCode.DPAD_UP then		udlr=udlr|0b00000000001000
		elseif k==KeyCode.DPAD_DOWN then	udlr=udlr|0b00000000000100
		elseif k==KeyCode.DPAD_LEFT then	udlr=udlr|0b00000000000010
		elseif k==KeyCode.DPAD_RIGHT then	udlr=udlr|0b00000000000001
		elseif k==KeyCode.BUTTON_L3 then	udlr=udlr|0b00100000000000 -- thumbs
		elseif k==KeyCode.BUTTON_R3 then	udlr=udlr|0b00010000000000
		elseif k==KeyCode.BUTTON_L1 then	udlr=udlr|0b10000000000000 -- shoulders
		elseif k==KeyCode.BUTTON_R1 then	udlr=udlr|0b01000000000000
		else
		print("Button Down ", k, findKeyCode(k))
		end
	end)
	
	controller:addEventListener(Event.KEY_UP, function(e)
		local k=e.keyCode
		if k==KeyCode.BUTTON_BACK then 		udlr=udlr&0b11110111111111
		elseif k==KeyCode.BUTTON_MENU then 	udlr=udlr&0b11111011111111
		elseif k==KeyCode.BUTTON_Y then		udlr=udlr&0b11111101111111
		elseif k==KeyCode.BUTTON_A then		udlr=udlr&0b11111110111111
		elseif k==KeyCode.BUTTON_X then		udlr=udlr&0b11111111011111
		elseif k==KeyCode.BUTTON_B then		udlr=udlr&0b11111111101111
		elseif k==KeyCode.DPAD_UP then		udlr=udlr&0b11111111110111
		elseif k==KeyCode.DPAD_DOWN then	udlr=udlr&0b11111111111011
		elseif k==KeyCode.DPAD_LEFT then	udlr=udlr&0b11111111111101
		elseif k==KeyCode.DPAD_RIGHT then	udlr=udlr&0b11111111111110
		elseif k==KeyCode.BUTTON_L3 then	udlr=udlr&0b11011111111111 -- thumbs
		elseif k==KeyCode.BUTTON_R3 then	udlr=udlr&0b11101111111111
		elseif k==KeyCode.BUTTON_L1 then	udlr=udlr&0b01111111111111 -- shoulders
		elseif k==KeyCode.BUTTON_R1 then	udlr=udlr&0b10111111111111
		end
	end)
	
	controller:addEventListener(Event.LEFT_JOYSTICK, function(e)
		buttons[12]:setPosition(30*e.x,30*e.y)
		print("LEFT_JOYSTICK:", "x:"..e.x, "y:"..e.y, "angle:"..e.angle, "strength:"..e.strength)
	end)
	
	controller:addEventListener(Event.RIGHT_JOYSTICK, function(e)
		buttons[11]:setPosition(30*e.x,30*e.y)
		print("RIGHT_JOYSTICK:", "x:"..e.x, "y:"..e.y, "angle:"..e.angle, "strength:"..e.strength)
	end)
	
	controller:addEventListener(Event.LEFT_TRIGGER, function(e)
		local s=e.strength
		triggers[2]:setColorTransform(1-(0.7*s),1-(0.5*s),1)
		print("LEFT_TRIGGER:", "strength:"..e.strength)
	end)
	
	controller:addEventListener(Event.RIGHT_TRIGGER, function(e)
		local s=e.strength
		triggers[1]:setColorTransform(1-(0.7*s),1-(0.5*s),1)
		print("RIGHT_TRIGGER:", "strength:"..e.strength)
	end)
	
	controller:addEventListener(Event.CONNECTED, function(e)
	print("Player: ", e.playerId, "connected ["..controller:getControllerName(e.playerId).."]")
end)

controller:addEventListener(Event.DISCONNECTED, function(e)
	print("Player: ", e.playerId, "disconnected")
end)
else
	print("No controller plugin")
end

function gameLoop(e)
	-- nice way to get debounced button bitmap
	debouncedUdlr=(udlr~oldUdlr)&udlr
	if debouncedUdlr~=0 then print("Debounced: "..debouncedUdlr) end
	
	for loop=1,#buttons do
		if udlr&(1<<(loop-1))==0 then
			buttons[loop]:setColorTransform(1,1,1)
		else
			buttons[loop]:setColorTransform(0.5,0.7,1)
		end
	end
	
	oldUdlr=udlr
end

stage:addEventListener(Event.ENTER_FRAME,gameLoop)

