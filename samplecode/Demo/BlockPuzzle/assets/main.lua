--app:setBackgroundColor(0x323232)
local SCR = Game.Screen

local manager

function GoToScene(name, duration, transition)
	duration = duration or 1
	transition = transition or SceneManager.fade
	
	Game.prevScene = Game.curScene
	Game.curScene = name
	manager:changeScene(name, duration, transition)
end

function GoBack(duration, transition)
	duration = duration or 1
	transition = transition or SceneManager.fade
	
	Game.prevScene = Game.curScene
	Game.curScene = name
	GoToScene(Game.prevScene, duration, transition)
end

function ShowLoadingScreen()
	local obj = Game.loading
	
	if (obj) then 
		obj.bg:setDimensions(SCR.W, SCR.H)
		obj.bg:setPosition(SCR.CX, SCR.CY)
		stage:addChild(obj.bg)
		
		obj:setPosition(SCR.CX - obj:getWidth() / 2, SCR.CY - obj:getHeight() / 2)
		stage:addChild(obj)
	else
		obj = Label.new{w = 200, h = 200, text = "Loading..."}
		obj:setTextColor(0xffffff)
		obj:setPosition(SCR.CX - obj:getWidth() / 2, SCR.CY - obj:getHeight() / 2)
		local bg = Pixel.new(0x101010, 1, SCR.W, SCR.H)
		bg:setAnchorPoint(.5,.5)
		bg:setPosition(SCR.CX, SCR.CY)
		stage:addChild(bg)
		stage:addChild(obj)
		
		obj.bg = bg
		Game.loading = obj
	end
end

function HideLoadingScreen()
	local obj = Game.loading
	assert(obj, "NO loading resource found!")
	
	stage:removeChild(obj.bg)
	stage:removeChild(obj)
end

function GetFont(name) return Game.FONTS[name] end

local function OnResize()
	local minX, minY, maxX, maxY = app:getLogicalBounds()
	local w = -minX + maxX
	local h = -minY + maxY
	
	local SCR = Game.Screen
	
	SCR.Left = minX
	SCR.Top = minY
	SCR.Right = maxX
	SCR.Bottom = maxY
	SCR.W = w
	SCR.H = h
	SCR.CX = minX + w / 2
	SCR.CY = minY + h / 2
end

local function LoadFonts(t)
	local f = {}
	for name,data in pairs(t) do 
		f[name] = TTFont.new(data.path, data.size)
	end
	Game.FONTS = f
end

local function LoadApp()
	local ts = os.clock()
	
	stage:addEventListener(Event.APPLICATION_RESIZE, OnResize)
	
	require "SceneManager"
	require "Easing"
	
	LoadFonts {
		["main_48"] = {path = "data/Amazonas.ttf", size = 48}
	}
	
	manager = SceneManager.new{
		["Game"] = GameScene,
	}
	manager:changeScene(Game.curScene)
	stage:addChild(manager)
	
	HideLoadingScreen()
	print("Loading time", os.clock() - ts)
end
ShowLoadingScreen()
Core.asyncCall(LoadApp)