require "giftiz"

local button = GiftizButton.new("images")
stage:addChild(button)

stage:addEventListener(Event.MOUSE_UP, function()
	--giftiz:missionComplete()
	giftiz:purchaseMade(2.99)
end)