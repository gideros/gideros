ads = Ads.new("chartboost")

if application:getDeviceInfo() == "Android" then
	ads:setKey("5180c1ec17ba470e6d000000", "ee7f50b143cc6fedc29fd38d86ad4bfbacc5602b")
elseif application:getDeviceInfo() == "iOS" then
	ads:setKey("5209160616ba47c50d000010", "b0f7cb6f0d70bd6b65adaee26dc8c62118f77088")
end

ads:showAd("moreapps")

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