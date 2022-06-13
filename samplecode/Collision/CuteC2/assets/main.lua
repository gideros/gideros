require "CuteC2"
require "SceneManager"

sceneManager = SceneManager.new({
	Collisions = CollisionsScene,
	TOI = TOIScene,
	GJK = GJKScene,
	Ray = RayScene,
})

sceneManager:changeScene("Collisions")
stage:addChild(sceneManager)
sceneManager.scene1.scenesIndex = 2