--[[
Demonstration of GTween class

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
]]

local sprite = Bitmap.new(Texture.new("box.png"))
stage:addChild(sprite)

GTween.new(sprite, 2, {x = 240}, {delay = 0.2, ease = easing.outBounce, repeatCount = 2, reflect = true})
