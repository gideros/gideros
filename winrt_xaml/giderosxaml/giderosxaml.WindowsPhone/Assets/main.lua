--[[

A frame by frame bird animation example
The old frame is removed by Sprite:removeChild and the new frame is added by Sprite:addChild

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]
require ("ads")
ad = Ads.new("pubcenter")
ad:enableTesting()

ad:addEventListener(Event.AD_RECEIVED, function(e)
    print("ads AD_RECEIVED", e.type)
end)

ad:addEventListener(Event.AD_FAILED, function(e)
    print("ads AD_FAILED", e.type, e.error)
end)

ad:addEventListener(Event.AD_ACTION_BEGIN, function(e)
    print("ads AD_ACTION_BEGIN", e.type)
end)

ad:addEventListener(Event.AD_ACTION_END, function(e)
    print("ads AD_ACTION_END", e.type)
end)

ad:addEventListener(Event.AD_DISPLAYED, function(e)
    print("ads AD_DISPLAYED", e.type)
end)

ad:addEventListener(Event.AD_DISMISSED, function(e)
    print("ads AD_DISMISSED", e.type)
end)

ad:addEventListener(Event.AD_ERROR, function(e)
    print("ads AD_ERROR", e.error)
end)


ad:showAd("160x600")
ad:setPosition(0,100)

