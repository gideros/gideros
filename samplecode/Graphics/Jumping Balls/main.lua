--[[

A nice example of balls moving on the screen, using classes

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

-- load background image and add it to the stage
local background = Bitmap.new(Texture.new("field.png"))
stage:addChild(background)

-- create ball sprites
local ball1 = Ball.new("ball1.png")
local ball2 = Ball.new("ball2.png")
local ball3 = Ball.new("ball3.png")
local ball4 = Ball.new("ball4.png")
local ball5 = Ball.new("ball5.png")

-- and add ball sprites to the stage
stage:addChild(ball1)
stage:addChild(ball2)
stage:addChild(ball3)
stage:addChild(ball4)
stage:addChild(ball5)
