--!NEEDS:ui/uipanel.lua
local r3d=require "reactphysics3d"

AUI=AUI or {}
AUI.Editor=Core.class(UI.Panel,function() return UI.Colors.black end)

local EditorModel=Core.class(Object)
function EditorModel:init(editor,assetitem)
	self.transform=Matrix.new()
	self.assetLib=assetitem.lib
	self.assetName=assetitem.name
	self.editor=editor
	
	local m=assetitem.lib:getModel(assetitem.name)
	self.sprite=m
	m.model=self
	if m.animations and m.animations[1] then
		local anim
		for _,a in ipairs(m.animations) do 
			if a.name and a.name:lower()=="idle" then anim=a end
		end 
		D3Anim.setAnimation(m,anim or m.animations[1],"main",true)
	end
end

function EditorModel:makeBody(bodySpec)
	local body=self.editor.collisions:createBody()
	body:setType(r3d.Body.STATIC_BODY)

	local m=self.sprite
	local ft=bodySpec and bodySpec.transform
	if not ft then
		ft=Matrix.new()
		ft:setPosition(m.center[1],m.center[2],m.center[3])
	end
	local sx,sy,sz=ft:getScale()
	local stx,sty,stz=self.transform:getScale()
	local dimx,dimy,dimz=((m.max[1]-m.min[1])/2)<>0.01,((m.max[2]-m.min[2])/2)<>0.01,((m.max[3]-m.min[3])/2)<>0.01 --Those are half dimensions
	local shapedim=vector(dimx,dimy,dimz,0)
	local shape
	local shapetype=bodySpec and bodySpec.shape
	dimx=dimx*sx*stx
	dimy=dimy*sy*sty
	dimz=dimz*sz*stz
	if shapetype=="sphere" then
		shape=r3d.SphereShape.new(dimx<>dimy<>dimz)
	else
		shape=r3d.BoxShape.new(dimx,dimy,dimz)
	end
	if shape then
		body.shapedim=shapedim
		body.shape=shape
		body.shapetype=shapetype
		local fft=ft:duplicate()
		local tx,ty,tz=fft:getPosition()
		fft:setPosition(tx*stx,ty*sty,tz*stz)
		fft:setScale(1,1,1)
		body.fixture=body:createFixture(shape,fft,1000)
		body.fixtureTransform=ft
	end
	self.body=body
	body.model=self
	local ft=self.transform:duplicate()
	ft:setScale(1,1,1)
	self.body:setTransform(ft)
end

function EditorModel:removeBody()
	if self.body then
		self.editor.collisions:destroyBody(self.body)
		self.body=nil
	end
end

function EditorModel:getTransform(physics)
	if physics and self.body and self.body.fixture then 
		local ft=self.transform:duplicate()
		ft:multiply(self.body.fixtureTransform)
		self.bodyUpdate=ft
		return ft
	else 
		return self.transform
	end
end
function EditorModel:update(event,bodyUpdate)
	self.sprite:setMatrix(self.transform)
	if self.body then
		if self.bodyUpdate or bodyUpdate then
			local stx,sty,stz=self.transform:getScale()
			if self.bodyUpdate then
				local wt=self.transform:duplicate()
				wt:invert()
				wt:multiply(self.bodyUpdate)
				self.body.fixtureTransform=wt
			end
			self.bodyUpdate=nil
			self.body:destroyFixture(self.body.fixture)
			
			local sx,sy,sz=self.body.fixtureTransform:getScale()
			
			local dimx=self.body.shapedim.x*sx*stx
			local dimy=self.body.shapedim.y*sy*sty
			local dimz=self.body.shapedim.z*sz*stz
			if self.body.shapetype=="sphere" then
				self.body.shape=r3d.SphereShape.new(dimx<>dimy<>dimz)
			else
				self.body.shape=r3d.BoxShape.new(dimx,dimy,dimz)
			end
			
			local fft=self.body.fixtureTransform:duplicate()
			local tx,ty,tz=fft:getPosition()
			fft:setPosition(tx*stx,ty*sty,tz*stz)
			fft:setScale(1,1,1)
			self.body.fixture=self.body:createFixture(self.body.shape,fft,1000)
		end
		local ft=self.transform:duplicate()
		ft:setScale(1,1,1)
		self.body:setTransform(ft)
	end
	if self.editor then
		if  self.editor.selection==self then
			self.editor:updateSelectedModel()
		end
		self.editor:updateView()
	end
	if event then
		UI.dispatchEvent(self.editor,"SceneModelUpdated",self)
	end
end
function EditorModel:getPropertyList()
	local pl={
		{ name="Identification", category=true, },
		{ name="Type", readonly=true },
		{ name="Name" },
		{ name="Tag" },
		{ name="Transform", category=true, },
		{ name="Position", type="vector" },
		{ name="Rotation", type="vector" },
		{ name="Scale", type="vector" },
		{ name="Physics", category=true, },
		{ name="ColShape", type="set", typeset={ "Box", "Sphere" }, label="Shape"},
		{ name="ColType", type="set", typeset={ "Static", "Dynamic", "Kinematic", "Ignore"}, label="Type"},
		{ name="ColPosition", type="vector", label="Position" },
		{ name="ColRotation", type="vector", label="Rotation" },
		{ name="ColScale", type="vector", label="Scale" },
	}
	return pl
end
local ColShapeMap={
	box=1,
	sphere=2,
}
local ColShapeList={
	"box",
	"sphere",
}
local ColTypeMap={
	static=1,
	dynamic=2,
	kinematic=3,
	ignore=4,
}
local ColTypeList={
	"static",
	"dynamic",
	"kinematic",
	"ignore",
}

function EditorModel:getProperty(name)
	if name=="Name" then return self.name
	elseif name=="Tag" then return self.tag
	elseif name=="Type" then return self.assetName
	--
	elseif name=="Position" then return vector(self.transform:getPosition())
	elseif name=="Rotation" then return vector(self.transform:getRotationX(),self.transform:getRotationY(),self.transform:getRotationZ())
	elseif name=="Scale" then return vector(self.transform:getScale())
	--
	elseif name=="ColShape" then return ColShapeMap[self.body.shapetype] or 1
	elseif name=="ColType" then return ColTypeMap[self.body.bodytype] or 1
	elseif name=="ColPosition" then return vector(self.body.fixtureTransform:getPosition())
	elseif name=="ColRotation" then return vector(self.body.fixtureTransform:getRotationX(),self.body.fixtureTransform:getRotationY(),self.body.fixtureTransform:getRotationZ())
	elseif name=="ColScale" then return vector(self.body.fixtureTransform:getScale())
	end
end

function EditorModel:setProperty(name,value)
	local bodyUpdate=false
	if name=="Name" then self.name=value
	elseif name=="Tag" then self.tag=value
	elseif name=="Position" then self.transform:setPosition(value.x,value.y,value.z)
	elseif name=="Rotation" then 
		self.transform:setRotationX(value.x)
		self.transform:setRotationY(value.y)
		self.transform:setRotationZ(value.z)
	elseif name=="Scale" then self.transform:setScale(value.x,value.y,value.z) bodyUpdate=true
	--
	elseif name=="ColShape" then self.body.shapetype=ColShapeList[value] bodyUpdate=true
	elseif name=="ColType" then self.body.bodytype=ColTypeList[value]
	elseif name=="ColPosition" then self.body.fixtureTransform:setPosition(value.x,value.y,value.z) bodyUpdate=true
	elseif name=="ColRotation" then 
		self.body.fixtureTransform:setRotationX(value.x)
		self.body.fixtureTransform:setRotationY(value.y)
		self.body.fixtureTransform:setRotationZ(value.z) bodyUpdate=true
	elseif name=="ColScale" then self.body.fixtureTransform:setScale(value.x,value.y,value.z) bodyUpdate=true
	end
	self:update(true,bodyUpdate)
end


function AUI.Editor:init()
	self.collisions=r3d.World.new(0,-9.8,0)
	self:addEventListener(Event.LAYOUT_RESIZED,self.resized,self)
	self.v=D3View.new()
	self:addChild(self.v)
	self.s=self.v:getScene()
	-- SETUP GRID
	self.grid=self:makeGrid(99)
	self.s:addChild(self.grid)
	self.placementGrid=self:makeGrid(9)
	self.placementGrid:setColorTransform(1,.8,.6)
	self.s:addChild(self.placementGrid)
	self.placementGrid:setVisible(false)
	
	self.transform={
		x=0,y=0,z=0, --Target position
		yaw=0,pitch=20, --Rotation
		zoom=10, --Zoom
	}
	self:updateTransform()
	--Default settings
	self.snap=0.2
	
	-- Listeners
	UI.Control.onDragStart[self]=self
	UI.Control.onDrag[self]=self
	UI.Control.onDragEnd[self]=self
	UI.Control.onMouseWheel[self]=self
	UI.Control.onMouseClick[self]=self
	UI.Control.onKeyDown[self]=self
	UI.Control.onKeyUp[self]=self
	UI.Control.onEnterFrame[self]=self

	self.models=Sprite.new()
	self.models.name="Scene"
	self.s:addChild(self.models)

	UI.Dnd.Target(self,true)

	self.physicsLayer=r3d.DebugDraw.new(self.collisions)
	self.physicsLayer:setVisible(false)
	self.s:addChild(self.physicsLayer)
	
	self:updateView()
end

function AUI.Editor:updateSelectedModel()
	if self.selection then
		self.placementGrid:setVisible(true)
		local px,py,pz=self.selection:getTransform():getPosition()
		local zp,sp=self:isZPlane()
		if zp then
			py=py//1
			if sp then
				self.placementGrid:setRotationX(0)
				self.placementGrid:setRotation(90)
			else
				self.placementGrid:setRotationX(90)
				self.placementGrid:setRotation(0)
			end
		else
			px=px//1
			pz=pz//1
			self.placementGrid:setRotationX(0)
			self.placementGrid:setRotation(0)
		end
		self.placementGrid:setPosition(px,py,pz)
	else
		self.placementGrid:setVisible(false)
	end
end

function AUI.Editor:updateView()
	self.collisions:step(0)
end

function AUI.Editor:getSceneData()
	return self.models
end

function AUI.Editor:clearSceneData()
	local function recursiveClean(group)
		while group:getNumChildren()>0 do
			local c=group:getChildAt(1)
			if c.model then
				self:removeModel(c.model) 
			else
				recursiveClean(c)
				c:removeFromParent()
			end
		end
	end
	return recursiveClean(self.models)
end

function AUI.Editor:addModel(assetitem,parentGroup)
	local mdl=EditorModel.new(self,assetitem)
	self.models:addChild(mdl.sprite)
	mdl:makeBody({shape="box"})
	return mdl
end

function AUI.Editor:removeModel(model)
	model:removeBody()
	model.sprite:removeFromParent()	
end

function AUI.Editor:updateTransform()
	local t=self.transform
	local ex=t.x+math.sin(^<t.yaw)*math.cos(^<t.pitch)*t.zoom
	local ez=t.z+math.cos(^<t.yaw)*math.cos(^<t.pitch)*t.zoom
	local ey=t.y+math.sin(^<t.pitch)*t.zoom
	self.v:lookAt(ex,ey,ez,t.x,t.y,t.z,0,1,0)
	Lighting.setLight(ex,ey,ez,.3)
	self.tmatrix=self.v.view:getTransform()
	if self.selection then self:updateSelectedModel() end
end

function AUI.Editor:makeGrid(sz)
	local grid=Mesh.new(true)
	grid:setPrimitiveType(Mesh.PRIMITIVE_LINES)
	local v,n={},{}
	sz=sz/2
	for i=-sz,sz do
		local nn=#n+1
		v[#v+1]=i 
		v[#v+1]=0
		v[#v+1]=-sz
		v[#v+1]=i 
		v[#v+1]=0
		v[#v+1]=sz
		n[#n+1]=nn
		n[#n+1]=nn+1
		v[#v+1]=-sz 
		v[#v+1]=0
		v[#v+1]=i
		v[#v+1]=sz 
		v[#v+1]=0
		v[#v+1]=i
		n[#n+1]=nn+2
		n[#n+1]=nn+3
	end
	grid:setVertexArray(v)	
	grid:setIndexArray(n)
	return grid
end

function AUI.Editor:resized()
	local lw,lh=self:getDimensions()
	self.v:setSize(lw,lh)
	self.v:setClip(0,0,lw,lh)
end

--Control
local function isSnapped()
	local mods=UI.Control.Meta.modifiers or 0
	return (mods&KeyCode.MODIFIER_CTRL)==0
end

function AUI.Editor:isPhysics()
	return self.physics
--[[	local mods=UI.Control.Meta.modifiers or 0
	return (mods&KeyCode.MODIFIER_SHIFT)~=0]]
end

function AUI.Editor:setPhysics(en)
	self.physics=en
	self.physicsLayer:setVisible(en)
end



function AUI.Editor:isZPlane()
	local yw=((self.transform.yaw+45)%360)//90
	return self.transform.pitch<10,(yw&1)==1
end

function AUI.Editor:locateModel(x,y,keepSel)
	local nearest={}
	local selIndex
	local function raycastcallback(c)
		nearest[#nearest+1]=c.body
		if c.body and c.body.model and c.body.model==self.selection then selIndex=#nearest end
		return nil
	end
	
	x,y=self:localToGlobal(x,y)
	local vx,vy=self.v.view:globalToLocal(x,y)
	local vp=self.v.view:getProjection()
	vp:multiply(self.v.view:getTransform())
	vp:invert()
	local sx,sy,sz,sw=vp:transformPoint(vx,vy,0)
	if sw~=0 then sx/=sw sy/=sw sz/=sw else return end
	local ex,ey,ez,ew=vp:transformPoint(vx,vy,1)
	if ew~=0 then ex/=ew ey/=ew ez/=ew else return end

	self.collisions:raycast(sx,sy,sz, ex,ey,ez, raycastcallback)
	if keepSel and selIndex then return nearest[selIndex].model end
	if selIndex then selIndex=(selIndex%#nearest)+1 end
	local n=nearest[selIndex or 1]
	
	return n and n.model
end

function AUI.Editor:onEnterFrame()
	D3Anim.tick()
end

function AUI.Editor:onMouseWheel(x,y,w,wd)
	if self.selection and (UI.Control.Meta.mouseButton or 0)~=0 then
		local im=self.selection:getTransform(self:isPhysics())
		local s=MatrixMaths.getScale({im:getMatrix()})
		local sf=(2^(w/600))
		im:setScaleX(s.x*sf)
		im:setScaleY(s.y*sf)
		im:setScaleZ(s.z*sf)
	
		self.selection:update(true,true)
	else
		self.transform.zoom=(self.transform.zoom-wd*.1)<>0.1
		self:updateTransform()
		if self._dragStart then
			self._dragStart.x=x
			self._dragStart.y=y
			self._dragStart.t=table.clone(self.transform)
			self._dragStart.mat=self.tmatrix
		end
	end
	return true
end

function AUI.Editor:getZDirection(x,y)
	x,y=self:localToGlobal(x or 0,y or 0)
	local vx,vy=self.v.view:globalToLocal(x,y)
	local vp=self.v.view:getProjection()
	vp:multiply(self.v.view:getTransform())
	vp:invert()
	local sx,sy,sz,sw=vp:transformPoint(vx,vy,0)
	if sw~=0 then sx/=sw sy/=sw sz/=sw else return end
	local ex,ey,ez,ew=vp:transformPoint(vx,vy,1)
	if ew~=0 then ex/=ew ey/=ew ez/=ew else return end
	ex-=sx ey-=sy ez-=sz
	ex,ey,ez=math.normalize(ex,ey,ez)
	return ex,ey,ez,sx,sy,sz
end
	
function AUI.Editor:moveModel(model,x,y,ox,oy,oz,physics)	
	local ex,ey,ez,sx,sy,sz=self:getZDirection(x,y)
	
	if self:isZPlane() then 
		local ad=math.length(ox-sx,oz-sz)
		oy=sy+ey*ad
	else
		local ad=-sy/ey
		ox=sx+ex*ad
		oz=sz+ez*ad
	end
	local function snap(v)
		if not isSnapped() then return v end
		return (v//self.snap)*self.snap
	end
	model:getTransform(physics):setPosition(snap(ox),snap(oy),snap(oz))
	model:update(true)
end


function AUI.Editor:rotateModel(model,im,a,vx,vy,vz,event,snapped,physics)
	local im=im or model:getTransform(physics)
	local m=im:duplicate()
	m:setPosition(0,0,0)
	m:rotate(a,vx,vy,vz)
	local r=MatrixMaths.getRotation({m:getMatrix()})
	im=model:getTransform(physics)
	im:setRotationX(r.x)
	im:setRotationY(r.y)
	im:setRotationZ(r.z)
	
	model:update(event)
end
	
function AUI.Editor:onDragStart(x,y)
	local model=self:locateModel(x,y,true)
	if model~=self.selection then model=nil end
	self._dragStart={x=x,y=y,btn=UI.Control.Meta.mouseButton or 1,t=table.clone(self.transform),mat=self.tmatrix,model=model}
	if model then
		self._dragStart.tm=model:getTransform():duplicate()
	end
	UI.Focus:request(self)
	return true
end
function AUI.Editor:onDragEnd(x,y)
	self._dragStart=nil	
	return true
end
function AUI.Editor:onDrag(x,y)
	if self._dragStart then
		local sx,sy=self:getSize()
		local dx=(x-self._dragStart.x)/sx
		local dy=(y-self._dragStart.y)/sy
		local t=self._dragStart.t
		local tm=self._dragStart.tm
		local model=self._dragStart.model
		if self._dragStart.btn==1 then --pan
			if model then
				local ox,oy,oz=tm:getPosition()
				self:moveModel(model,x,y,ox,oy,oz,self:isPhysics())
				self:updateView()
			else
				local ox,oy,oz=self._dragStart.mat:transformPoint(0,0,0)
				local tx,ty,tz=self._dragStart.mat:transformPoint(-dx,dy,0)
				tx-=ox ty-=oy tz-=oz
				self.transform.x=t.x+tx*t.zoom
				self.transform.y=t.y+ty*t.zoom
				self.transform.z=t.z-tz*t.zoom
				self:updateTransform()
			end
		else --Rotate
			if model then
				local zp,sp=self:isZPlane()
				if zp then 
					dy*=360
					if isSnapped() then
						dy=15*(dy//15)
					end
					self:rotateModel(model,tm,dy,if sp then 0 else 1,0,if sp then 1 else 0,true,isSnapped(),self:isPhysics())
				else
					dx*=360
					if isSnapped() then
						dx=15*(dx//15)
					end
					self:rotateModel(model,tm,dx,0,1,0,true,isSnapped(),self:isPhysics())
				end
				self:updateView()
			else			
				self.transform.yaw=t.yaw-dx*90
				if dy>0 then
					self.transform.pitch=(t.pitch+dy*90)><89.5
				else
					self.transform.pitch=(t.pitch+dy*90)<>-89.5
				end
				self:updateTransform()
			end
		end
		return true
	end
end

function AUI.Editor:onMouseClick(x,y)
	UI.Focus:request(self)
	local model=self:locateModel(x,y)
	self:selectModel(model,true)
	
	return true
end

function AUI.Editor:onKeyDown(code)
	if self.selection then
		local model=self.selection
		if code==KeyCode.SPACE then --Rotate
			self:rotateModel(model,nil,if isSnapped() then 90 else 15,0,1,0,true)
		elseif code==KeyCode.DELETE then --Remove
			self:removeModel(model)
			self:selectModel(nil)
		end
	end
	if code==KeyCode.PAGE_UP then
		self.grid:setY(self.grid:getY()+1)
	elseif code==KeyCode.PAGE_DOWN then		
		self.grid:setY(self.grid:getY()-1)
	elseif code==KeyCode.SHIFT then
		self:setPhysics(true)
	end
	return true
end

function AUI.Editor:onKeyUp(code)
	if code==KeyCode.SHIFT then
		self:setPhysics(false)
	end
	return true
end

function AUI.Editor:selectModel(model,event)
	if model and not model.sprite then model=nil end
	if self.selection then
		self.models:setColorTransform(1,1,1,1)
		self.selection.sprite:setColorTransform(1,1,1,1)		
	end
	self.selection=model
	if self.selection then
		self.models:setColorTransform(1,.7,.7,.9)
		self.selection.sprite:setColorTransform(1,1/.7,1/.7,1/.9)
	end
	self:updateSelectedModel()
	if event then
		UI.dispatchEvent(self,"SceneModelSelected",self.selection)
	end
end

function AUI.Editor:offerDndData(data,x,y)
	if self.dndDstMarker then self.dndDstMarker:setVisible(false) end
	if data and data.type==AUI.AssetItem then
		if not self.dndModel then
			self.dndModel=self:addModel(data.value)
			self:selectModel(self.dndModel,true)
		end
		self:moveModel(self.dndModel,x,y,0,self.grid:getY(),0)
		return true
	elseif self.dndModel then
		self:removeModel(self.dndModel)
		self.dndModel=nil
	end	
end

function AUI.Editor:setDndData(data)
	if data and data.type==AUI.AssetItem then
		UI.dispatchEvent(self,"SceneChanged",self.dndModel)
		self.dndModel=nil
		UI.Focus:request(self)
	end
end