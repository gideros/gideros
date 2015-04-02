--[[
Infinity balls
by Doctor Max
(c)2013-10-13
--]]

local rt1 = RenderTarget.new(320, 480, true)
local rt2 = RenderTarget.new(320, 480, true)
local rt3 = RenderTarget.new(320, 480, true)
local rt4 = RenderTarget.new(320, 480, true)
local rt5 = RenderTarget.new(320, 480, true)
local rt6 = RenderTarget.new(320, 480, true)
local rt7 = RenderTarget.new(320, 480, true)
local rt8 = RenderTarget.new(320, 480, true)

brt1 = Bitmap.new(rt1); brt2 = Bitmap.new(rt2)
brt3 = Bitmap.new(rt3); brt4 = Bitmap.new(rt4)
brt5 = Bitmap.new(rt5); brt6 = Bitmap.new(rt6)
brt7 = Bitmap.new(rt7); brt8 = Bitmap.new(rt8)

s1 = Sprite.new(); s2 = Sprite.new()
s3 = Sprite.new(); s4 = Sprite.new()
s5 = Sprite.new(); s6 = Sprite.new()
s7 = Sprite.new(); s8 = Sprite.new()

s1:addChild(brt1); s2:addChild(brt2)
s3:addChild(brt3); s4:addChild(brt4)
s5:addChild(brt5); s6:addChild(brt6)
s7:addChild(brt7); s8:addChild(brt8)

dx = 0; dy = 0; frame = 1

local function onEnterFrame(event)
	local t = os.timer()
	local x = math.sin((t/2) + dx) * 100 + 120
	local y = math.cos((t/3) + dy) * 200 + 210

	dx = dx + 0.005
	dy = dy + 0.009
	local bitmap = Bitmap.new(Texture.new("ball.png"))
	bitmap:setPosition(x, y)

	if frame == 1 then
		rt1:draw(bitmap)
		stage:removeChild(s1)
		stage:addChild(s2)
	elseif frame == 2 then
		rt2:draw(bitmap)
		stage:removeChild(s2)
		stage:addChild(s3)
	elseif frame == 3 then
		rt3:draw(bitmap)
		stage:removeChild(s3)
		stage:addChild(s4)
	elseif frame == 4 then
		rt4:draw(bitmap)
		stage:removeChild(s4)
		stage:addChild(s5)
	elseif frame == 5 then
		rt5:draw(bitmap)
		stage:removeChild(s5)
		stage:addChild(s6)
	elseif frame == 6 then
		rt6:draw(bitmap)
		stage:removeChild(s6)
		stage:addChild(s7)
	elseif frame == 7 then
		rt7:draw(bitmap)
		stage:removeChild(s7)
		stage:addChild(s8)
	elseif frame == 8 then
		rt8:draw(bitmap)
		stage:removeChild(s8)
		stage:addChild(s1)
end
	
	frame = frame + 1
	if frame == 9 then
		frame = 1
	end
end

stage:addChild(s1)
stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
