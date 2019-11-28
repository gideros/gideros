D3View=Core.class(Sprite)
function D3View:init(sw,sh,fov,near,far)
	self.view=Viewport.new()
	self.projection=Matrix.new()
	self.fov=fov or 45
	self.near=near or 0.1
	self.far=far or 1000
	self:setSize(sw or 1,sh or 1)
	self.scene=Sprite.new()
	self.view:setContent(self.scene)
	self:addChild(self.view)	
end

function D3View:setSize(w,h)
	self.sw=w
	self.sh=h
	self.projection:perspectiveProjection(self.fov,self.sw/self.sh,self.near,self.far)
	self.view:setProjection(self.projection)
	self.view:setPosition(self.sw/2,self.sh/2)
	self.view:setScale(-self.sw/2,-self.sh/2,1)
end

function D3View:lookAt(eyex,eyey,eyez,targetx,targety,targetz,upx,upy,upz)
	self.view:lookAt(eyex,eyey,eyez,targetx,targety,targetz,upx or 0,upy or 1,upz or 0)
	Lighting.setCamera(eyex,eyey,eyez)
end

function D3View:lookAngles(eyex,eyey,eyez,pitch,yaw,roll)
	self.view:lookAngles(eyex,eyey,eyez,pitch,yaw,roll)
	Lighting.setCamera(eyex,eyey,eyez)
end

function D3View:getScene() return self.scene end

D3=D3 or {}
D3.View=D3View
