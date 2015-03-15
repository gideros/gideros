ads = Ads.new("adcolony")
local test
ads:setKey("app3eefea4fed0e448e93d1ee", "vza612000e0bdc4bce817e30")

ads:showAd("video", "vza612000e0bdc4bce817e30")
--ads:showAd("v4vc", "vza612000e0bdc4bce817e30", "true")

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