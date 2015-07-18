--[[
Automatic selection of different resolution images according to scale

To see the automatic image resolution parameters, right click on the 
project name and select "Properties..."

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
]]

local image = Bitmap.new(Texture.new("image.png", true))
stage:addChild(image)
