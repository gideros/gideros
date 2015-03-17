local ads = Ads.new("amazon")
ads:setKey("aa78d97b17a9409a870348898a13706d")
ads:enableTesting()

ads:showAd("auto")

local pos = {"center", "bottom"}

ads:setAlignment(unpack(pos))

local shape = Shape.new()
stage:addChild(shape)

local text = TextField.new(nil, "Ad goes here")
text:setScale(3)
text:setTextColor(0xffffff)
text:setPosition(50, 35)
shape:addChild(text)
ads:showAd("auto")

local isVisible = true
stage:addEventListener(Event.MOUSE_DOWN, function()
	if isVisible then
		isVisible = false
		ads:hideAd("auto")
	else
		isVisible = true
		ads:showAd("auto")
	end
end)

ads:addEventListener(Event.AD_RECEIVED, function(e)
	--ads:showAd("auto")
	print("ads AD_RECEIVED", e.type)
	local width = ads:getWidth()
	local height = ads:getHeight()
	print(width, height)
	print("position", ads:getPosition())
	print("position", ads:getPosition())
	local x = math.floor((application:getContentWidth() - width)/2)
	local y = math.floor((application:getContentHeight() - height)/2)
	
	shape:clear()
	shape:setFillStyle(Shape.SOLID, 0xff0000)
	shape:beginPath()
	shape:moveTo(0,0)
	shape:lineTo(0,height)
	shape:lineTo(width,height)
	shape:lineTo(width,0)
	shape:closePath()
	shape:endPath()
	shape:setPosition(x, y)
	Timer.delayedCall(2000, function()
		local tween = GTween.new(ads, 1, {x = x, y = y}, {ease = easing.outBack, dispatchEvents = true})
		tween:addEventListener("complete", function()
			print("reached:", ads:getPosition())
			ads:showAd("auto")
			ads:setAlignment(unpack(pos))
		end)
	end)
end)

ads:addEventListener(Event.AD_FAILED, function(e)
	print("ads AD_FAILED", e.type, e.error)
end)

ads:addEventListener(Event.AD_ACTION_BEGIN, function(e)
	print("ads AD_ACTION_BEGIN", e.type)
end)

ads:addEventListener(Event.AD_ACTION_END, function(e)
	print("ads AD_ACTION_END", e.type)
end)

ads:addEventListener(Event.AD_DISMISSED, function(e)
	print("ads AD_DISMISSED", e.type)
end)

ads:addEventListener(Event.AD_ERROR, function(e)
	print("ads AD_ERROR", e.error)
end)