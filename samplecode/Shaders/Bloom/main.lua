--!NEDDS:luashader/effects.lua

application:setScaleMode("pixelPerfect")
local dw=application:getDeviceWidth()
local dh=application:getDeviceHeight()
application:setLogicalDimensions(dw,dh)
local scrw=application:getContentWidth()
local scrh=application:getContentHeight()
print(scrw,scrh)
scene=Shape.new()
scene:setFillStyle(Shape.SOLID,0)
scene:beginPath()
scene:moveTo(0,0)
scene:lineTo(scrw,0)
scene:lineTo(scrw,scrh)
scene:lineTo(0,scrh)
scene:lineTo(0,0)
scene:endPath()

lines={}

local function makeShot()
	line=Shape.new()
	line:setLineStyle(10,math.random()*256*256*256)
	line:beginPath()
	local sz=math.random(30)
	line:moveTo(0,sz)
	line:lineTo(0,-sz)
	line:endPath()
	local ang=math.random()*math.pi*2
	line.dy=-math.cos(ang)
	line.dx=math.sin(ang)
	line.life=500
	line:setRotation(ang*180/math.pi)
	line:setPosition(math.random()*scrw,math.random()*scrh)
	table.insert(lines,line)
	return line
end
--
local sw=scene:getWidth()
local sh=scene:getHeight()

stage:addChild(scene)
local bloom=ShaderEffect.Bloom.new(sw,sh)
bloom:setRadiusAndGlow(7,4)
bloom:apply(stage)
local bloomApplied=true
stage:setClip(0,0,sw,sh)

stage:addEventListener(Event.MOUSE_DOWN,function ()
	if bloomApplied then
		bloom:remove(stage)
		bloomApplied=false
	else
		bloom:apply(stage)
		bloomApplied=true
	end
end)

local shot=0
stage:addEventListener(Event.ENTER_FRAME,function ()
	shot=shot+1
	if (shot==5) then
		shot=0
		scene:addChild(makeShot())
	end
	for _,l in ipairs(lines) do
		local lpx,lpy=l:getPosition()
		lpx=lpx+l.dx*2
		lpy=lpy+l.dy*2
		l:setPosition(lpx,lpy)
		l.life=l.life-1
		if (l.life<0) then
			l:removeFromParent()
		end
	end
end)
