--[[

Hierarchy example. It uses sprites to group another sprites.

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]


dot1tex = Texture.new("dot-1.png")
dot2tex = Texture.new("dot-2.png")
group1tex = Texture.new("group-1.png")
group2tex = Texture.new("group-2.png")

group1 = Sprite.new()

group1:addChild(Bitmap.new(group1tex))

for i=0,3 do
	for j=0,3 do
		local dot = Bitmap.new(dot1tex)
		dot:setPosition(i * 45 + 10, j * 45 + 60)
		group1:addChild(dot)
	end
end

group1:setPosition(10, 10)
stage:addChild(group1)


group2 = Sprite.new()

group2:addChild(Bitmap.new(group2tex))

for i=0,3 do
	for j=0,3 do
		local dot = Bitmap.new(dot2tex)
		dot:setPosition(i * 45 + 10, j * 45 + 60)
		group2:addChild(dot)
	end
end

group2:setPosition(100, 200)
stage:addChild(group2)
