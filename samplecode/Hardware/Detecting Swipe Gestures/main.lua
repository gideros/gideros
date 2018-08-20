--[[
	'gestureStack.lua' class provided by @SinisterSoft: http://giderosmobile.com/forum/discussion/comment/55123/#Comment_55123
]]

local gestureInfo = TextField.new(nil, "GESTURE_NAME")
gestureInfo:setScale(2); gestureInfo:setPosition(10,25);
stage:addChild(gestureInfo); gestureInfo:setVisible(false)

local function showGesture(name)
	gestureInfo:setText(name)
	gestureInfo:setVisible(true)

	local timerDelayed = Timer.delayedCall(300, function() gestureInfo:setVisible(false) end)
end

stage:addEventListener(Event.ENTER_FRAME, function()
	local gesture,gestureName=getGesture()
	if gesture then
		print(gesture[1],gestureName)
		removeGesture()
		showGesture(gestureName)
	end
end)