ouyaScene = Core.class(OUYALayer)

function ouyaScene:init()
	--let's create couple of simple TextFields
	local startText = TextField.new(nil, "Start")
	startText:setScale(4)
	startText:setPosition(500, 300)
	startText:addEventListener(Event.MOUSE_UP, function()
		print("starting")
	end)
	self:addChild(startText)
	
	local optionsText = TextField.new(nil, "Options")
	optionsText:setScale(4)
	optionsText:setPosition(500, 400)
	optionsText:addEventListener(Event.MOUSE_UP, function()
		print("options")
	end)
	self:addChild(optionsText)
	
	local aboutText = TextField.new(nil, "About")
	aboutText:setScale(4)
	aboutText:setPosition(500, 500)
	aboutText:addEventListener(Event.MOUSE_UP, function()
		print("about")
	end)
	self:addChild(aboutText)
	
	--now ad them all as ouya selectable items
	self:addObject(startText)
	self:addObject(optionsText)
	self:addObject(aboutText)
	
	--add some additional ouya events only to this specific scene
	self:addEventListener(Event.RIGHT_JOYSTICK, function(e)
		print("RIGHT_JOYSTICK:", "x:"..e.x, "y:"..e.y, "angle:"..e.angle, "strength:"..e.strength)
	end)
end

function ouyaScene:back()
	--automatically executed when pressing OUYA_A button
	print("going back")
end

--launch our test scene
stage:addChild(ouyaScene.new())