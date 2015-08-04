Scene4 = gideros.class(Sprite)

function Scene4:init()
	self:addChild(Bitmap.new(Texture.new("gfx/scene4.jpg")))

	self:addEventListener("enterBegin", self.onTransitionInBegin, self)
	self:addEventListener("enterEnd", self.onTransitionInEnd, self)
	self:addEventListener("exitBegin", self.onTransitionOutBegin, self)
	self:addEventListener("exitEnd", self.onTransitionOutEnd, self)
end

function Scene4:onTransitionInBegin()
	print("scene4 - enter begin")
end

function Scene4:onTransitionInEnd()
	print("scene4 - enter end")
end

function Scene4:onTransitionOutBegin()
	print("scene4 - exit begin")
end

function Scene4:onTransitionOutEnd()
	print("scene4 - exit end")
end
