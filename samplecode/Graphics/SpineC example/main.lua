--[[ Currently supported API calls:
	setMix(globalMixDuration)
	setMix(fromAnimation,toAnimation,mixDuration)
	addAnimation(track,animation,mixDuration,loop,delay)
	setAnimation(track,animation,mixDuration,loop)
]]

application:setBackgroundColor(0)
require("spine")

local rooster=SpineSprite.new("rooster.json","rooster.atlas")
rooster:setAnimation(0,"rooster_walk_anim",nil,true)
rooster:setMix(0.3)
rooster:setScale(.6); rooster:setPosition(170,200)
stage:addChild(rooster)

rooster.isRunning=false

stage:addEventListener(Event.MOUSE_DOWN, function(event)

	if rooster.isRunning then
		rooster:setAnimation(0, "rooster_walk_anim", nil, true)
	else
		rooster:setAnimation(0, "rooster_run_anim", nil, true)
	end
	
	rooster.isRunning=not rooster.isRunning
	
end)