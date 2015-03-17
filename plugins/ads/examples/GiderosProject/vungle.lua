ads = Ads.new("vungle")
local test

if application:getDeviceInfo() == "Android" then
	ads:setKey("com.jenots.mashballs")
elseif application:getDeviceInfo() == "iOS" then
	ads:setKey("52976a8d6829a7000a00000b")
end

ads:showAd("video")
--ads:showAd("v4vc")

ads:addEventListener(Event.AD_RECEIVED, function()
	print("ads AD_RECEIVED")
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