application:setBackgroundColor(0x56433c)

require "Easing"
require "SceneManager"
require "Utils"

local manager = SceneManager.new{
	["Game"] = GameScene
}
manager:changeScene("Game")
stage:addChild(manager)