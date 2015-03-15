--[[admob = Ads.new("admob")
admob:setKey("a151c9c98a8857b")
admob:showAd("banner", "545D828B0002576098659942B35646D7")]]

--[[admob = Ads.new("inmobi")
admob:setKey("014d25c7fa4243179eb620cd501d7c07")
admob:showAd("300x250")]]

--[[admob = Ads.new("tapjoy")
admob:setKey("4cc74714-108f-495f-ac9c-ad7a9ef61af7", "fkMLsN3FSw7F4p1xGiFT")
admob:showAd("640x100")]]

--[[admob = Ads.new("amazon")
admob:setKey("aa78d97b17a9409a870348898a13706d")
admob:showAd("300x50")]]

--admob:setAlignment("center", "bottom")
	
--[[adcolony = Ads.new("adcolony")
adcolony:setKey("app3eefea4fed0e448e93d1ee", "vza612000e0bdc4bce817e30")]]

--[[admob = Ads.new("leadbolt")
admob:setKey("123119241")
admob:showAd()]]

--[[admob = Ads.new("heyzap")
admob:setKey("123119241")
admob:showAd("banner")]]
	
--[[stage:addEventListener(Event.MOUSE_DOWN, function()
	--admob:hideAd()
	--admob:showAd("banner", "545D828B0002576098659942B35646D7")
	--adcolony:showAd("video")
	--admob:hideAd()
	--admob:showAd("320x50")
	--adcolony:showAd("v4vc", "vza612000e0bdc4bce817e30", "true")
end)]]
--[[iad = Ads.new("inmobi")
iad:setKey("4d70a8ce12e54f54a1c578c42ebc87db")
iad:showAd("320x50")]]
--iad:showAd("banner", "cfbb21950ba71a1ab926f67a895c9065")
--iad:showAd("video", "vza612000e0bdc4bce817e30")
--[[isAd = false
stage:addEventListener(Event.MOUSE_DOWN, function()
	if isAd then
		iad:hideAd()
		iad:showAd("banner")
	else
		iad:hideAd()
		iad:showAd("iab_banner")
	end
	isAd = not isAd
end)]]



--[[
adwhirl = Ads.new("adwhirl")
adwhirl:setKey("b9c9b13ab8cc42cc85020656376f97a1") --adwhirl
]]

--[[
--Admob events

admob:addEventListener(Event.AD_RECEIVED, function()
	print("Admob AD_RECEIVED")
end)

admob:addEventListener(Event.AD_FAILED, function(e)
	print("Admob AD_FAILED", e.error)
end)

admob:addEventListener(Event.AD_ACTION_BEGIN, function()
	print("Admob AD_ACTION_BEGIN")
end)

admob:addEventListener(Event.AD_ACTION_END, function()
	print("Admob AD_ACTION_END")
end)

admob:addEventListener(Event.AD_DISMISSED, function()
	print("Admob AD_DISMISSED")
end)

admob:addEventListener(Event.AD_ERROR, function(e)
	print("Admob AD_ERROR", e.error)
end)]]


--Acolony events

--[[
adcolony:addEventListener(Event.AD_RECEIVED, function()
	print("Adcolony AD_RECEIVED")
end)

adcolony:addEventListener(Event.AD_FAILED, function(e)
	print("Adcolony AD_FAILED", e.error)
end)

adcolony:addEventListener(Event.AD_ACTION_BEGIN, function()
	print("Adcolony AD_ACTION_BEGIN")
end)

adcolony:addEventListener(Event.AD_ACTION_END, function()
	print("Adcolony AD_ACTION_END")
end)

adcolony:addEventListener(Event.AD_DISMISSED, function()
	print("Adcolony AD_DISMISSED")
end)

adcolony:addEventListener(Event.AD_ERROR, function(e)
	print("Adcolony AD_ERROR", e.error)
end)]]


--Iad events

--[[
iad:addEventListener(Event.AD_RECEIVED, function()
	print("Iad AD_RECEIVED")
end)

iad:addEventListener(Event.AD_FAILED, function(e)
	print("Iad AD_FAILED", e.error)
end)

iad:addEventListener(Event.AD_ACTION_BEGIN, function()
	print("Iad AD_ACTION_BEGIN")
end)

iad:addEventListener(Event.AD_ACTION_END, function()
	print("Iad AD_ACTION_END")
end)

iad:addEventListener(Event.AD_DISMISSED, function()
	print("Iad AD_DISMISSED")
end)

iad:addEventListener(Event.AD_ERROR, function(e)
	print("Iad AD_ERROR", e.error)
end)]]
