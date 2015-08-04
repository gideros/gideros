--[[
This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
]]

local sceneManager = SceneManager.new({
	["scene1"] = Scene1,
	["scene2"] = Scene2,
	["scene3"] = Scene3,
	["scene4"] = Scene4,
})

sceneManager:addEventListener("transitionBegin", function() print("manager - transition begin") end)
sceneManager:addEventListener("transitionEnd", function() print("manager - transition end") end)

stage:addChild(sceneManager)

local transitions = {
	SceneManager.moveFromLeft,
	SceneManager.moveFromRight,
	SceneManager.moveFromBottom,
	SceneManager.moveFromTop,
	SceneManager.moveFromLeftWithFade,
	SceneManager.moveFromRightWithFade,
	SceneManager.moveFromBottomWithFade,
	SceneManager.moveFromTopWithFade,
	SceneManager.overFromLeft,
	SceneManager.overFromRight,
	SceneManager.overFromBottom,
	SceneManager.overFromTop,
	SceneManager.overFromLeftWithFade,
	SceneManager.overFromRightWithFade,
	SceneManager.overFromBottomWithFade,
	SceneManager.overFromTopWithFade,
	SceneManager.fade,
	SceneManager.crossFade,
	SceneManager.flip,
	SceneManager.flipWithFade,
	SceneManager.flipWithShade,
}

local scenes = {"scene1", "scene2", "scene3", "scene4"}

local currentScene = 1
local function nextScene()
	local next = scenes[currentScene]

	currentScene = currentScene + 1
	if currentScene > #scenes then
		currentScene = 1
	end
	
	return next
end

local moveButton = Button.new(Bitmap.new(Texture.new("gfx/move-up.png")), Bitmap.new(Texture.new("gfx/move-down.png")))
moveButton:setPosition(20, 10)
stage:addChild(moveButton)

local moveWithFadeButton = Button.new(Bitmap.new(Texture.new("gfx/move with fade-up.png")), Bitmap.new(Texture.new("gfx/move with fade-down.png")))
moveWithFadeButton:setPosition(160, 10)
stage:addChild(moveWithFadeButton)

local overButton = Button.new(Bitmap.new(Texture.new("gfx/over-up.png")), Bitmap.new(Texture.new("gfx/over-down.png")))
overButton:setPosition(20, 50)
stage:addChild(overButton)

local overWithFadeButton = Button.new(Bitmap.new(Texture.new("gfx/over with fade-up.png")), Bitmap.new(Texture.new("gfx/over with fade-down.png")))
overWithFadeButton:setPosition(160, 50)
stage:addChild(overWithFadeButton)

local fadeButton = Button.new(Bitmap.new(Texture.new("gfx/fade-up.png")), Bitmap.new(Texture.new("gfx/fade-down.png")))
fadeButton:setPosition(20, 90)
stage:addChild(fadeButton)

local crossfadeButton = Button.new(Bitmap.new(Texture.new("gfx/crossfade-up.png")), Bitmap.new(Texture.new("gfx/crossfade-down.png")))
crossfadeButton:setPosition(160, 90)
stage:addChild(crossfadeButton)

local flipButton = Button.new(Bitmap.new(Texture.new("gfx/flip-up.png")), Bitmap.new(Texture.new("gfx/flip-down.png")))
flipButton:setPosition(20, 130)
stage:addChild(flipButton)

local flipWithFadeButton = Button.new(Bitmap.new(Texture.new("gfx/flip with fade-up.png")), Bitmap.new(Texture.new("gfx/flip with fade-down.png")))
flipWithFadeButton:setPosition(160, 130)
stage:addChild(flipWithFadeButton)

local flipWithShadeButton = Button.new(Bitmap.new(Texture.new("gfx/flip with shade-up.png")), Bitmap.new(Texture.new("gfx/flip with shade-down.png")))
flipWithShadeButton:setPosition(20, 170)
stage:addChild(flipWithShadeButton)

moveButton:addEventListener("click", 
	function()	
		local transition = transitions[math.random(1, 4)]
		sceneManager:changeScene(nextScene(), 1, transition, easing.outBack) 
	end)

moveWithFadeButton:addEventListener("click", 
	function()	
		local transition = transitions[math.random(5, 8)]
		sceneManager:changeScene(nextScene(), 1, transition, easing.outQuadratic) 
	end)

overButton:addEventListener("click", 
	function()	
		local transition = transitions[math.random(9, 12)]
		sceneManager:changeScene(nextScene(), 1, transition, easing.outBounce) 
	end)

overWithFadeButton:addEventListener("click", 
	function()	
		local transition = transitions[math.random(13, 16)]
		sceneManager:changeScene(nextScene(), 1, transition, easing.outQuadratic) 
	end)

fadeButton:addEventListener("click", 
	function()	
		sceneManager:changeScene(nextScene(), 1, SceneManager.fade, easing.linear) 
	end)

crossfadeButton:addEventListener("click", 
	function()	
		sceneManager:changeScene(nextScene(), 1, SceneManager.crossfade, easing.linear) 
	end)

flipButton:addEventListener("click", 
	function()	
		sceneManager:changeScene(nextScene(), 1, SceneManager.flip, easing.inOutQuadratic) 
	end)

flipWithFadeButton:addEventListener("click", 
	function()	
		sceneManager:changeScene(nextScene(), 1, SceneManager.flipWithFade, easing.inOutQuadratic) 
	end)

flipWithShadeButton:addEventListener("click", 
	function()	
		sceneManager:changeScene(nextScene(), 1, SceneManager.flipWithShade, easing.inOutQuadratic) 
	end)

sceneManager:changeScene(nextScene())
