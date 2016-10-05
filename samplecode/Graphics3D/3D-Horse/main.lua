local scrw,scrh=application:getContentWidth(),application:getContentHeight()

local view=Viewport.new()
local vp=Matrix.new()
vp:perspectiveProjection(45,-scrw/scrh,0.1,10000)
view:setProjection(vp)
view:setPosition(scrw/2,scrh/2)
view:setScale(scrw/2,scrh/2,1)
stage:addChild(view)

local horse=loadObj("horse","LD_HorseRtime02.obj")
Lighting.apply(horse)
--print(horse.min[1],horse.min[2],horse.min[3],horse.max[1],horse.max[2],horse.max[3])
view:setContent(horse)

view:addEventListener(Event.ENTER_FRAME, function (self)
	local frm=Core.frameStatistics().frameCounter
	local angle=frm/70
	local ey=math.sin(frm/200)*5+5
	local ex,ez=math.sin(angle)*20,math.cos(angle)*20
	view:lookAt(ex,ey,ez,0,0,0,0,1,0)
	local lx,ly,lz=view:getTransform():transformPoint(100,100,100)
	Lighting.setLight(lx,ly,lz,0.3)
end)

