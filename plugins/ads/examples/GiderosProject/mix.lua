local amazon = Ads.new("amazon")
amazon:setKey("aa78d97a9409a870348898a13706d")
amazon:enableTesting()
amazon:showAd("auto")
amazon:setAlignment("center", "bottom")

ads = Ads.new("admob")
ads:setKey("a151c9c98a8857b")
ads:enableTesting()

--[[inmobi = Ads.new("inmobi")
inmobi:setKey("014d25c7fa4243179eb620cd501d7c07")
inmobi:enableTesting()]]

amazon:addEventListener(Event.AD_RECEIVED, function()
	print("amazon AD_RECEIVED")
end)

amazon:addEventListener(Event.AD_FAILED, function(e)
	print("amazon AD_FAILED", e.error)
	ads:showAd("auto")
	ads:setAlignment("center", "bottom")
end)

amazon:addEventListener(Event.AD_ACTION_BEGIN, function()
	print("amazon AD_ACTION_BEGIN")
end)

amazon:addEventListener(Event.AD_ACTION_END, function()
	print("amazon AD_ACTION_END")
end)

amazon:addEventListener(Event.AD_DISMISSED, function()
	print("amazon AD_DISMISSED")
end)

amazon:addEventListener(Event.AD_ERROR, function(e)
	print("amazon AD_ERROR", e.error)
end)

ads:addEventListener(Event.AD_RECEIVED, function()
	print("ads AD_RECEIVED")
end)

ads:addEventListener(Event.AD_FAILED, function(e)
	print("ads AD_FAILED", e.error)
	amazon:showAd("auto")
	amazon:setAlignment("center", "bottom")
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
--[[
inmobi:addEventListener(Event.AD_RECEIVED, function()
	print("ads AD_RECEIVED")
end)

inmobi:addEventListener(Event.AD_FAILED, function(e)
	print("ads AD_FAILED", e.error)
	ads:showAd("auto")
	ads:setAlignment("center", "bottom")
end)

inmobi:addEventListener(Event.AD_ACTION_BEGIN, function()
	print("ads AD_ACTION_BEGIN")
end)

inmobi:addEventListener(Event.AD_ACTION_END, function()
	print("ads AD_ACTION_END")
end)

inmobi:addEventListener(Event.AD_DISMISSED, function()
	print("ads AD_DISMISSED")
end)

inmobi:addEventListener(Event.AD_ERROR, function(e)
	print("ads AD_ERROR", e.error)
end)]]