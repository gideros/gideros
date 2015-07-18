Scene2 = gideros.class(Sprite)

function Scene2:init()
	self:addChild(Bitmap.new(Texture.new("gfx/scene2.jpg")))

	self:addEventListener("enterBegin", self.onTransitionInBegin, self)
	self:addEventListener("enterEnd", self.onTransitionInEnd, self)
	self:addEventListener("exitBegin", self.onTransitionOutBegin, self)
	self:addEventListener("exitEnd", self.onTransitionOutEnd, self)
end

function Scene2:onTransitionInBegin()
	print("scene2 - enter begin")
end

function Scene2:onTransitionInEnd()
	print("scene2 - enter end")
end

function Scene2:onTransitionOutBegin()
	print("scene2 - exit begin")
end

function Scene2:onTransitionOutEnd()
	print("scene2 - exit end")
end
