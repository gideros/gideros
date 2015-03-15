ads = Ads.new("iad")

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

ads:addEventListener(Event.AD_RECEIVED, function()
	print("ads AD_RECEIVED")
	local width = ads:getWidth()
	local height = ads:getHeight()
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
			ads:setAlignment(unpack(pos))
			ads:showAd("banner")
			ads:setAlignment(unpack(pos))
		end)
	end)
end)

ads:addEventListener(Event.AD_FAILED, function(e)
	print("ads AD_FAILED", e.error)
end)

ads:addEventListener(Event.AD_ACTION_BEGIN, function()
	print("ads AD_ACTION_BEGIN")
end)

ads:addEventListener(Event.AD_ACTION_END, function()
	print("ads AD_ACTION_END")
end)

ads:addEventListener(Event.AD_DISMISSED, function()
	print("ads AD_DISMISSED")
end)

ads:addEventListener(Event.AD_ERROR, function(e)
	print("ads AD_ERROR", e.error)
end)