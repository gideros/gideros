ads = Ads.new("samsung")
local test

if application:getDeviceInfo() == "Android" then
	ads:setKey("xv0d00000002jy")
elseif application:getDeviceInfo() == "iOS" then
	ads:setKey("xv0d0000000300")
end

ads:showAd("small_banner")
Timer.delayedCall(1000, function()
	ads:showAd("interstitial")
end)

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