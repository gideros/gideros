
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

function findKeyCode(code)
	for i, val in pairs(KeyCode) do
		if val == code then
			return i
		end
	end
	return "unknown"
end

-- simulate joypad with keyboard
function processKeys(e)
	local k=e.keyCode
	print(e.type)
	local bit=-1
	if k==KeyCode.ESC then			bit=9
	elseif k==KeyCode.ENTER then	bit=8
	elseif k==KeyCode.Q then		bit=7
	elseif k==KeyCode.W then		bit=6
	elseif k==KeyCode.E then		bit=5
	elseif k==KeyCode.SPACE then	bit=4
	elseif k==KeyCode.UP then		bit=3
	elseif k==KeyCode.DOWN then		bit=2
	elseif k==KeyCode.LEFT then		bit=1
	elseif k==KeyCode.RIGHT then	bit=0
	end
	if bit>=0 then
		if e.type=="keyDown" then
			udlr=udlr|(1<<bit)
		else
			udlr=udlr&~(1<<bit)
		end
	end
end

stage:addEventListener(Event.KEY_DOWN,processKeys)
stage:addEventListener(Event.KEY_UP,processKeys)

function processController(e)
	local k=e.keyCode
	local bit=-1
	if k==KeyCode.BUTTON_BACK then 		bit=9
	elseif k==KeyCode.BUTTON_MENU then 	bit=8
	elseif k==KeyCode.BUTTON_Y then		bit=7
	elseif k==KeyCode.BUTTON_A then		bit=6
	elseif k==KeyCode.BUTTON_X then		bit=5
	elseif k==KeyCode.BUTTON_B then		bit=4
	elseif k==KeyCode.DPAD_UP then		bit=3
	elseif k==KeyCode.DPAD_DOWN then	bit=2
	elseif k==KeyCode.DPAD_LEFT then	bit=1
	elseif k==KeyCode.DPAD_RIGHT then	bit=0
	elseif k==KeyCode.BUTTON_L3 then	bit=11 -- thumbs
	elseif k==KeyCode.BUTTON_R3 then	bit=10
	elseif k==KeyCode.BUTTON_L1 then	bit=13 -- shoulders
	elseif k==KeyCode.BUTTON_R1 then	bit=12
	end
	if bit>=0 then
		if e:getType()=="keyDown" then
			udlr=udlr|(1<<bit)
		else
			udlr=udlr&~(1<<bit)
		end
	else
		print("Button Up/Down ", k, findKeyCode(k))
	end
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

	controller:addEventListener(Event.KEY_DOWN,processController)
	controller:addEventListener(Event.KEY_UP,processController)
	
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

