ads = Ads.new("applovin")
ads:setKey("foTTHR80e36XxXglVHsIhmTAJfaBwTPZKu7cJGdyJkUFG9-mRr37abIvFt49QUkmMsNg6_dQi6gzFekhTnq3M6")
ads:enableTesting()

local cur = 1

--ads:showAd(banners[cur])

local pos = {"center", "bottom"}

ads:setAlignment(unpack(pos))
--ads:setPosition(0,50)
visible = false
stage:addEventListener(Event.MOUSE_DOWN, function()
	print("me")
	if visible then
		ads:hideAd("auto")
	else
		ads:showAd("auto")
	end
	visible = not visible
	--ads:showAd("interstitial")
end)

local shape = Shape.new()
stage:addChild(shape)

local text = TextField.new(nil, "Ad goes here")
text:setScale(3)
text:setTextColor(0xffffff)
text:setPosition(50, 35)
shape:addChild(text)

ads:addEventListener(Event.AD_RECEIVED, function(e)
	print("ads AD_RECEIVED", e.type)
	--ads:showAd("auto")
	--ads:showAd("interstitial")
	local width = ads:getWidth()
	local height = ads:getHeight()
	local x = math.floor((application:getContentWidth() - width)/2)
	local y = math.floor((application:getContentHeight() - height)/2)
	--local x = 0
	--local y = 0
	
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
			--[[ads:hideAd(banners[cur])
			cur = cur + 1
			if cur > #banners then
				cur = 1
			end
			ads:showAd(banners[cur])
			ads:setAlignment(unpack(pos))
			--ads:setPosition(0,50)]]
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

ads:addEventListener(Event.AD_DISPLAYED, function(e)
	print("ads AD_DISPLAYED", e.type)
end)

ads:addEventListener(Event.AD_DISMISSED, function(e)
	print("ads AD_DISMISSED", e.type)
end)

ads:addEventListener(Event.AD_ERROR, function(e)
	print("ads AD_ERROR", e.error)
end)