--[[ 

This is an example of sound capabilities of Gideros Studio

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

local s1 = SoundButton.new("1-up.png", "1-down.png", "1.wav")
local s2 = SoundButton.new("2-up.png", "2-down.png", "2.wav")
local s3 = SoundButton.new("3-up.png", "3-down.png", "3.wav")

s1:setPosition(10, 50)
s2:setPosition(110, 50)
s3:setPosition(210, 50)

stage:addChild(s1)
stage:addChild(s2)
stage:addChild(s3)


local label = TextField.new(nil, "click buttons to play sounds") -- we pass first parameter as nil to use the system font
label:setPosition(86, 170)
stage:addChild(label)
