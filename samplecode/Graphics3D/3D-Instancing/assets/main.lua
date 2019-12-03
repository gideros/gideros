--Create on scene
application:setBackgroundColor(0)
local view=D3.View.new(application:getContentWidth(),application:getContentHeight(),45,0.1,100)
stage:addChild(view)
local scene=view:getScene()


local cube=D3.Cube.new(0.1,0.1,0.1)
scene:addChild(cube)
cube:updateMode(D3.Mesh.MODE_LIGHTING,0)

local gs,ge=3,0.3
local gc=gs*gs*gs
cube:setInstanceCount(gc)
local matrix=Matrix.new()
for i=1,gc do
	local x=(i-1)%gs
	local y=((i-1)//gs)%gs
	local z=((i-1)//gs)//gs
	matrix:setPosition(x*ge-(gs-1)/2*ge,y*ge-(gs-1)/2*ge,z*ge-(gs-1)/2*ge)
	cube:setInstanceMatrix(i,matrix)
end
cube:updateInstances()

view:lookAt(0,0,-5,0,0,0)

local xform=Matrix.new()
local a=0
stage:addEventListener(Event.ENTER_FRAME,function()
	xform:setRotationY(xform:getRotationY()+1)
	xform:setRotationX(xform:getRotationX()+2)
	cube:setLocalMatrix(xform)
	a=a+1
	local ac,as=math.cos(^<a),math.sin(^<a)
	view:lookAt(ac*10,as*2,as*10,0,0,0)
end)