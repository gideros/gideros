require "ads"
admob = Ads.new("admob")
admob:setKey("a1514202b662eb6")
admob:enableTesting()
admob:showAd("auto")

inmobi = Ads.new("inmobi")
inmobi:setKey("c4a7260ee6ca41ef84c1d949db48e0c6")
inmobi:addEventListener(Event.AD_RECEIVED, function()
	Timer.delayedCall(1000, function()
		admob:showAd("auto")
	end)
end)

admob:addEventListener(Event.AD_RECEIVED, function()
	Timer.delayedCall(1000, function()
		inmobi:showAd("auto")
	end)
end)