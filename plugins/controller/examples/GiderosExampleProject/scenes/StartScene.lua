
local function randomize(elem)
	elem:setPosition(math.random(-1000, 1000), math.random(-1000, 1000))
	elem:setRotation(math.random(0, 360))
	elem:setAlpha(math.random())
	elem:setScale(math.random())
end
StartScene = Core.class(ControllerLayer)

function StartScene:init()
	--here we'd probably want to set up a background picture
	local screen = Bitmap.new(Texture.new("images/gideros_mobile.png", true))
	self:addChild(screen)
	screen:setPosition((conf.width-screen:getWidth())/2, (conf.height-screen:getHeight())/2)
	
	local bmp_up = Bitmap.new(Texture.new("images/start_up.png", true))
	local bmp_down = Bitmap.new(Texture.new("images/start_down.png", true))
	local button = Button.new(bmp_up, bmp_down)
	self:addObject(button)
	button:addEventListener("click", function()
		--go to select scene
		sceneManager:changeScene("select", 0.1, conf.transition, conf.easing)
	end)
	self:addChild(button)
	button:setPosition((conf.width-button:getWidth())/2, 100)
	
	local bmp_up = Bitmap.new(Texture.new("images/options_up.png", true))
	local bmp_down = Bitmap.new(Texture.new("images/options_down.png", true))
	local button = Button.new(bmp_up, bmp_down)
	self:addObject(button)
	button:addEventListener("click", function()
		print("options")
	end)
	self:addChild(button)
	button:setPosition((conf.width-button:getWidth())/2, 200)
	
	local bmp_up = Bitmap.new(Texture.new("images/help_up.png", true))
	local bmp_down = Bitmap.new(Texture.new("images/help_down.png", true))
	local button = Button.new(bmp_up, bmp_down)
	self:addObject(button)
	button:addEventListener("click", function()
		print("help")
	end)
	self:addChild(button)
	button:setPosition((conf.width-button:getWidth())/2, 300)
	
end