local scenes = SceneManager.new({
	Scene1,
	Scene2,
	Scene3,
	Scene4,
	Scene1,
	Scene2,
	Scene3,
	Scene4,
	Scene1,
	Scene2,
	Scene3,
	Scene4,
	Scene1,
	Scene2,
	Scene3,
	Scene4,
})

stage:addChild(scenes)

local left = Button.new(Bitmap.new(Texture.new("left-up.png")), Bitmap.new(Texture.new("left-down.png")))
left:setPosition(106, 400)
stage:addChild(left)

local right = Button.new(Bitmap.new(Texture.new("right-up.png")), Bitmap.new(Texture.new("right-down.png")))
right:setPosition(166, 400)
stage:addChild(right)

left:addEventListener("click", function() scenes:previousScene() end)
right:addEventListener("click", function() scenes:nextScene() end)
