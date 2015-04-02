--require plugin
require "media"

--just a function to create text button
function createText(str, y, callback)
	local text = TextField.new(nil, str)
	text:setScale(3)
	text:setPosition(40, y)
	stage:addChild(text)
	
	text:addEventListener(Event.MOUSE_DOWN, function(self, e)
		if self:hitTestPoint(e.x, e.y) then
			print("call")
			callback()
		end
	end, text)
end

--here we will store Bitmap isntance
local bmp

local shouldResize = true

--when some picture was selected by user
mediamanager:addEventListener(Event.MEDIA_RECEIVE, function(e)
	--print path
	print(e.path)
	local path = ""
	
	if shouldResize then
		--shouldResize = false
		local media = Media.new(e.path)
		print("media sizes", media:getWidth(), media:getHeight())
		media:flipVertical()
		media:flipHorizontal()
		media:trim(0xffffff)
		--media:drawText(50, 50, "Test Text", 0xff0000, 60)
		--media:drawLine(20, 20, 100, 70, 0xff0000)
		media:resize(200, 200, false)
		print(media:getPixel(1, 1))
		for x = 50, 100 do
			for y = 50, 100 do
				media:setPixel(x, y, 255, 0, 0, 0.5)
			end
		end
		--media:drawImage(100, 100, "ball.png")
		media:save()
		path = media:getPath()
		local mreal = Media.new(200, 200)
		mreal:drawImage(0,0, media)
		mreal:drawImage(100, 100, "ball.png")
		mreal:save()
		path = mreal:getPath()
	end
	
	--remove previous Bitmap
	if bmp then
		bmp:removeFromParent()
		bmp = nil
	end
	
	--add selected image to the stage
	bmp = Bitmap.new(Texture.new(path, true))
	bmp:setPosition(10, 10)
	stage:addChildAt(bmp, 1)
	application:setBackgroundColor(0xff0000)
end)

--user canceled selecting image
mediamanager:addEventListener(Event.MEDIA_CANCEL, function()
	print("User canceled media input")
end)

mediamanager:addEventListener(Event.VIDEO_COMPLETE, function()
	print("Video completed")
end)

createText("Check camera", 150, function()
	print(mediamanager:isCameraAvailable())
end)

createText("Take thumbnail", 200, function()
	shouldResize = true
	mediamanager:takePicture()
end)

createText("Take picture", 250, function()
	mediamanager:takePicture()
end)

createText("Take screenshot", 300, function()
	mediamanager:takeScreenshot()
end)

createText("Get picture", 350, function()
	shouldResize = true
	mediamanager:getPicture()
end)

createText("Post picture", 400, function()
	mediamanager:postPicture("ball.png")
end)

createText("Play Video", 450, function()
	mediamanager:playVideo("test.mp4")
end)

--delete previos copy
local media = Media.new("ball.png")
mediamanager:deleteFile(media:getPath())

local media = Media.new("ball.png")
local bmp = Bitmap.new(Texture.new(media:getPath(), true))
stage:addChild(bmp)

stage:addEventListener(Event.MOUSE_DOWN, function(e)
	if bmp:hitTestPoint(e.x, e.y) then
		print("flood")
		media:floodFill(e.x, e.y, 0xff0000, 0.5, 100, true)
		media:save()
		bmp:removeFromParent()
		bmp = Bitmap.new(Texture.new(media:getPath(), true))
		stage:addChild(bmp)
	end
end)

local text = TextField.new(nil, "Click on ball to floodFill it")
text:setScale(2)
text:setPosition(100, 50)
stage:addChild(text)
