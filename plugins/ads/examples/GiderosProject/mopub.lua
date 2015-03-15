ads = Ads.new("mopub")
--ads:setKey("b00117b17d4c4d409db17d549eca7d52")
--ads:setKey("f561bfcecf98411d8dad0a48b43f95b1")

if application:getDeviceInfo() == "iOS" then
	ads:showAd("banner", "8e4b972b7bfc4ec58706e832c1fd713c")
	--ads:showAd("interstitial", "1ac6a77cac574d9a89f59924420f6f51")
else
	ads:showAd("banner", "b00117b17d4c4d409db17d549eca7d52")
	--ads:showAd("interstitial", "f561bfcecf98411d8dad0a48b43f95b1")
end

ads:addEventListener(Event.AD_RECEIVED, function()
	ads:setAlignment("center", "bottom")
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