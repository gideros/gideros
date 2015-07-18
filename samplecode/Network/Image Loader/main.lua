--[[

An example showing downloading and displaying an image

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]


local info = TextField.new(nil, "loading...")
info:setPosition(10, 10)
stage:addChild(info)

local function onComplete(event)
	info:setText("done")

	local out = io.open("|D|image.png", "wb")
    out:write(event.data)
    out:close()

    local b = Bitmap.new(Texture.new("|D|image.png"))
	b:setAnchorPoint(0.5, 0.5)
	b:setPosition(160, 240)
    stage:addChild(b)
end

local function onError()
	info:setText("error")
end

local function onProgress(event)
    info:setText("progress: " .. event.bytesLoaded .. " of " .. event.bytesTotal)
end

local loader = UrlLoader.new("http://giderosmobile.com/giderosmobile.png")

loader:addEventListener(Event.COMPLETE, onComplete)
loader:addEventListener(Event.ERROR, onError)
loader:addEventListener(Event.PROGRESS, onProgress)
