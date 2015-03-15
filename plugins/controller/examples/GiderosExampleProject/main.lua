require "box2d"
require "controller"

function math.finite(x)
	return (x > -math.huge and x < math.huge)
end

--setting up some configurations
application:setOrientation(conf.orientation)
application:setLogicalDimensions(conf.width, conf.height)
application:setScaleMode(conf.scaleMode)
application:setFps(conf.fps)
application:setKeepAwake(conf.keepAwake)

--get new dimensions
conf.width = application:getContentWidth()
conf.height = application:getContentHeight()

maxPlayers = 4

--define scenes
sceneManager = SceneManager.new({
	--start scene
	["start"] = StartScene,
	["select"] = SelectScene,
	["game"] = GameScene
})
--add manager to stage
stage:addChild(sceneManager)

--start start scene
sceneManager:changeScene("start", 0.1, conf.transition, conf.easing)