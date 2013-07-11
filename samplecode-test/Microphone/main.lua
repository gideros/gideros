require "microphone"

local levelMeter = LevelMeter.new()
stage:addChild(levelMeter)
levelMeter:setY(30)

local microphone = Microphone.new(nil, 22050, 1, 16)

microphone:addEventListener(Event.DATA_AVAILABLE, function(event)
	print("*")
	levelMeter:setLevel(event.peakAmplitude)
end)

microphone:setOutputFile("|D|record.wav")

local record = Button.new(	Bitmap.new(Texture.new("record-up.png")),
							Bitmap.new(Texture.new("record-down.png")),
							Bitmap.new(Texture.new("record-disabled.png")))
record:setPosition(70, 130)
stage:addChild(record)


local recordStop = Button.new(	Bitmap.new(Texture.new("stop-up.png")),
								Bitmap.new(Texture.new("stop-down.png")))
recordStop:setPosition(70, 130)


local play = Button.new(	Bitmap.new(Texture.new("play-up.png")),
							Bitmap.new(Texture.new("play-down.png")),
							Bitmap.new(Texture.new("play-disabled.png")))
play:setPosition(70, 200)
play:setDisabled(true)
stage:addChild(play)


local playStop = Button.new(Bitmap.new(Texture.new("stop-up.png")),
							Bitmap.new(Texture.new("stop-down.png")))
playStop:setPosition(70, 200)


local function onRecord()
	play:setDisabled(true)
	record:removeFromParent()
	stage:addChild(recordStop)
	microphone:start()
end
record:addEventListener(Event.CLICK, onRecord)

local function onRecordStop()
	play:setDisabled(false)
	recordStop:removeFromParent()
	stage:addChild(record)
	microphone:stop()
	levelMeter:setLevel(0)
	play:setDisabled(false)
end
recordStop:addEventListener(Event.CLICK, onRecordStop)


local sound = nil
local channel = nil

local function onPlayStop()
	record:setDisabled(false)
	playStop:removeFromParent()
	stage:addChild(play)
	channel:stop()
end
playStop:addEventListener(Event.CLICK, onPlayStop)

local function onPlay()
	record:setDisabled(true)
	play:removeFromParent()
	stage:addChild(playStop)
	sound = Sound.new("|D|record.wav")
	channel = sound:play()
	channel:addEventListener(Event.COMPLETE, onPlayStop)
end
play:addEventListener(Event.CLICK, onPlay)
