--[[
This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
]]

SoundButton = Core.class(Sprite)

function SoundButton:init(upimage, downimage, soundfile)
	local up = Bitmap.new(Texture.new(upimage))
	local down = Bitmap.new(Texture.new(downimage))
	local button = Button.new(up, down)

	self:addChild(button)

	local sound = Sound.new(soundfile)
	
	button:addEventListener("click", function() sound:play() end)
end

