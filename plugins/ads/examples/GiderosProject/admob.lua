ads = Ads.new("admob")
ads:setKey("a151c9c98a8857b")
ads:enableTesting()

local banners = {
	"smart_banner",
	"banner",
	"iab_banner",
	"iab_mrect"
}

local cur = 1

--[[ads:showAd(banners[cur])

local pos = {"center", "bottom"}

ads:setAlignment(unpack(pos))
--ads:setPosition(0,50)]]

ads:showAd("auto")

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
			--[[cur = cur + 1
			if cur > #banners then
				cur = 1
			end
			ads:showAd(banners[cur])
			ads:setAlignment(unpack(pos))]]
			--ads:setPosition(0,50)
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