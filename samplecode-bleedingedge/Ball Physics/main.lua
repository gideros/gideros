--[[

This sample shows how Bo2D API can be used to show real world physics capabilities

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

require "box2d"

stage:setOrientation(Stage.LANDSCAPE_LEFT)

local world = b2.World.new(0.0, 0.0, true)

local ball = Ball.new(world, 100, 100, 2)

local balls = {}

for i = 1,5 do 
	balls[i] = Ball.new(world, math.random(320), math.random(320), 1)
	stage:addChild(balls[i])
end	

stage:addChild(ball)
