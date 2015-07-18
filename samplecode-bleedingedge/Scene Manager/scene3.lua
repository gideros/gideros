Scene3 = gideros.class(Sprite)

function Scene3:init()
	self:addChild(Bitmap.new(Texture.new("gfx/scene3.jpg")))

	self:addEventListener("enterBegin", self.onTransitionInBegin, self)
	self:addEventListener("enterEnd", self.onTransitionInEnd, self)
	self:addEventListener("exitBegin", self.onTransitionOutBegin, self)
	self:addEventListener("exitEnd", self.onTransitionOutEnd, self)
end

function Scene3:onTransitionInBegin()
	print("scene3 - enter begin")
end

function Scene3:onTransitionInEnd()
	print("scene3 - enter end")
end

function Scene3:onTransitionOutBegin()
	print("scene3 - exit begin")
end

function Scene3:onTransitionOutEnd()
	print("scene3 - exit end")
end
