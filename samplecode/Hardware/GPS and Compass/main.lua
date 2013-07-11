--[[

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

geolocation = Geolocation.new()

local arial18 = Font.new("arial-18.txt", "arial-18.png")
local arial36 = Font.new("arial-36.txt", "arial-36.png")

local background = Bitmap.new(Texture.new("compass-background.png"))
local pointer = Bitmap.new(Texture.new("compass-pointer.png", true))
pointer:setAnchorPoint(0.5, 0.5)

stage:addChild(background)
stage:addChild(pointer)

background:setPosition(10, 40)
pointer:setPosition(160, 210)

local locationLabel = TextField.new(arial18)
stage:addChild(locationLabel)
locationLabel:setY(410)

local headingLabel = TextField.new(arial36)
stage:addChild(headingLabel)
headingLabel:setY(46)

local magneticButton = Button.new(Bitmap.new(Texture.new("mag-up.png")), Bitmap.new(Texture.new("mag-down.png")))
magneticButton:setPosition(45, 430)

local trueButton = Button.new(Bitmap.new(Texture.new("true-up.png")), Bitmap.new(Texture.new("true-down.png")))
trueButton:setPosition(45, 430)

magneticButton:addEventListener("click", 
	function()
		stage:removeChild(magneticButton)
		stage:addChild(trueButton)
		magnetic = false
	end)

trueButton:addEventListener("click", 
	function()
		stage:removeChild(trueButton)
		stage:addChild(magneticButton)
		magnetic = true
	end)

stage:addChild(magneticButton)
magnetic = true

local function onHeadingUpdate(event)
	local heading = nil
	
	if magnetic then
		heading = event.magneticHeading
	else
		heading = event.trueHeading
	end

	pointer:setRotation(-heading)
	headingLabel:setText(math.floor(heading).."°")
	headingLabel:setX(math.floor((320 - headingLabel:getWidth())/2))
end

local function degree2str(x)
	local sign = (x < 0) and "-" or ""
	x = math.abs(x)
	local degrees = math.floor(x)
	local minutes = math.floor((x - degrees) * 60)
	local seconds = math.floor((x - degrees - minutes / 60) * 3600)

	return sign..degrees.."°"..minutes.."'"..seconds.."''"
end

local function onLocationUpdate(event)
	locationLabel:setText(degree2str(event.latitude)..", "..degree2str(event.longitude))
	locationLabel:setX(math.floor((320 - locationLabel:getWidth())/2))
end

geolocation:addEventListener(Event.HEADING_UPDATE, onHeadingUpdate)
geolocation:addEventListener(Event.LOCATION_UPDATE, onLocationUpdate)
geolocation:start()
