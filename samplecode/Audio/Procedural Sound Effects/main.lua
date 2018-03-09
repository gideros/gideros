--[[ 

This example demonstrates a generic Button class

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

-- create a label to show number of clicks
local label = TextField.new(nil, "Press to cycle SFX")
-- load all the .sfx files
local sfx1 = Sound.new("testfx1.sfx")
local sfx2 = Sound.new("testfx2.sfx")
local sfx3 = Sound.new("testfx3.sfx")
local sfx4 = Sound.new("testfx4.sfx")
local sfx5 = Sound.new("testfx5.sfx")
local sfx6 = Sound.new("testfx6.sfx")
local sfx7 = Sound.new("testfx7.sfx")
local sfx8 = Sound.new("testfx8.sfx")
local sfx9 = Sound.new("testfx9.sfx")
local sfx10 = Sound.new("testfx10.sfx")
label:setPosition(120,240)
stage:addChild(label)

-- create the up and down sprites for the button
local up = Bitmap.new(Texture.new("button_up.png"))
local down = Bitmap.new(Texture.new("button_down.png"))

-- create the button
local button = Button.new(up, down)

-- register to "click" event
local click = 0
button:addEventListener("click", 
	function() 
		click = click + 1
		-- play the next .sfx file
		label:setText("SFX " .. click)
		if click == 1 then
			sfx1:play()
		elseif  click == 2 then
			sfx2:play()
		elseif  click == 3 then
			sfx3:play()
		elseif  click == 4 then
			sfx4:play()
		elseif  click == 5 then
			sfx5:play()
		elseif  click == 6 then
			sfx6:play()
		elseif  click == 7 then
			sfx7:play()
		elseif  click == 8 then
			sfx8:play()
		elseif  click == 9 then
			sfx9:play()
		elseif  click == 10 then
			sfx10:play()
			click = 0
		end
	end)

button:setPosition(40, 150)
stage:addChild(button)
