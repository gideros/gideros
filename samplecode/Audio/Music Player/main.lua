--[[

An example showing how to play MP3 files

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

-- create info text
local info = TextField.new(nil, "A minimalist example of MP3 player")
info:setPosition(150, 180)
stage:addChild(info)

-- load the mp3 file
local sound = Sound.new("Pitx_-_Black_Rainbow.mp3")

-- and start playing
local channel = sound:play()

-- create play button
local playtex = Texture.new("play.png")
local playup = Bitmap.new(playtex)
local playdown = Bitmap.new(playtex)
playdown:setPosition(1, 1)
local play = Button.new(playup, playdown)
play:setPosition(50, 100)
stage:addChild(play)

-- create pause button
local pausetex = Texture.new("pause.png")
local pauseup = Bitmap.new(pausetex)
local pausedown = Bitmap.new(pausetex)
pausedown:setPosition(1, 1)
local pause = Button.new(pauseup, pausedown)
pause:setPosition(104, 100)
stage:addChild(pause)

function onPlayClick()
	channel:setPaused(false)
end

function onPauseClick()
	channel:setPaused(true)
end

play:addEventListener("click", onPlayClick)
pause:addEventListener("click", onPauseClick)

local bar = Bitmap.new(Texture.new("bar.png"))
stage:addChild(bar)
bar:setPosition(160, 118)

local marker = Bitmap.new(Texture.new("marker.png"))
stage:addChild(marker)

local time = TextField.new()
time:setPosition(370, 127)
stage:addChild(time)

-- this function converts seconds to a readable minute:second string
local function seconds2str(sec)
	local minstr = tostring(math.floor(sec / 60))
	local secstr = tostring(sec % 60)
	
	if #minstr == 1 then
		minstr = "0" .. minstr
	end

	if #secstr == 1 then
		secstr = "0" .. secstr
	end

	return minstr..":"..secstr
end

-- updates the time label
local function onTimer()
	local sec = math.floor(channel:getPosition() / 1000);
	time:setText(seconds2str(sec))

	local t = channel:getPosition() / sound:getLength()
	
	marker:setY(bar:getY() - 9)
	marker:setX(bar:getX() + t * bar:getWidth() - 4)
end

-- create a timer that is fired at every 1000 msec
t = Timer.new(1000, 0)
t:addEventListener(Event.TIMER, onTimer)
t:start()
onTimer()


