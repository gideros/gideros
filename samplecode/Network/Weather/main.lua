--[[

This example displays weather forecast of San Francisco and demonstrates remote loading and parsing of XML data.

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

-- add background
stage:addChild(Bitmap.new(Texture.new("cloud_background.png")))

-- load fonts
local arial12 = Font.new("arial-12.txt", "arial-12.png")
local arial24 = Font.new("arial-24.txt", "arial-24.png")

-- helper function to get a child of xml node by name
local function getChild(xml, name)
	for i=1,#xml.ChildNodes do
		if xml.ChildNodes[i].Name == name then
			return xml.ChildNodes[i]
		end
	end
end

-- helper function to get all children of xml node by name
local function getChildren(xml, name)
	local result = {}
	for i=1,#xml.ChildNodes do
		if xml.ChildNodes[i].Name == name then
			result[#result + 1] = xml.ChildNodes[i]
		end
	end
	return result
end

local info = TextField.new(arial12, "loading...")
info:setTextColor(0xffffff)
info:setPosition(60, 14)
stage:addChild(info)

local function onComplete(event)
	-- parse xml
	local xml = XmlParser:ParseXmlText(event.data)

	-- get weather data from xml
	local channel = getChild(xml, "channel")
	local title = getChild(channel, "title")
	local item = getChild(channel, "item")

	info:setText(title.Value)
		
	local forecasts = getChildren(item, "yweather:forecast")

	-- display weather data
	for i=1,#forecasts do
		local day = forecasts[i].Attributes["day"]
		local low = forecasts[i].Attributes["low"]
		local high = forecasts[i].Attributes["high"]
		local code = forecasts[i].Attributes["code"]
		local text = forecasts[i].Attributes["text"]
	
		local y = i * 94 - 78;

		local bitmap = Bitmap.new(Texture.new("images/" .. code .. ".png"))
		bitmap:setPosition(10, y)
		stage:addChild(bitmap)
		
		local daytf = TextField.new(arial24, day)
		daytf:setPosition(26, y + 82)
		stage:addChild(daytf)
		
		local lowhightf = TextField.new(arial12, high.."F - "..low.."F")
		lowhightf:setPosition(100, y + 30)
		stage:addChild(lowhightf)

		local texttf = TextField.new(arial12, text)
		texttf:setTextColor(0x000080)
		texttf:setPosition(100, y + 46)
		stage:addChild(texttf)
	end
end

local function onError()
	info:setText("error")
end

-- create loader to get xml weather forecast data from yahoo
local loader = UrlLoader.new("http://xml.weather.yahoo.com/forecastrss/USCA0987.xml")

-- register to COMPLETE and ERROR events
loader:addEventListener(Event.COMPLETE, onComplete)
loader:addEventListener(Event.ERROR, onError)
