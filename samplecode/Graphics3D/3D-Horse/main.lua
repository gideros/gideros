application:configureFrustum(45)
application:setOrientation(Application.LANDSCAPE_LEFT)
local obj=buildHorse()
--local obj=buildCube()

stage:addChild(obj)

obj:addEventListener(Event.ENTER_FRAME, function ()
	obj:setRotationY(obj:getRotationY()+1.5)
	obj:setRotationX(obj:getRotationX()+1)
	obj:setRotation(obj:getRotation()+1.3)	
end)
