--!NEEDS:uitable.lua
UI.Toolbox=Core.class(UI.Table,function (header,direction) return nil,direction end)

function UI.Toolbox:init(header,direction)
	self:setLocalStyle("toolbox.styToolbox")
	self:setStyleInheritance("local")
	self:setBaseStyle("toolbox.styContainer")
	self:setColumns({
		{ 
		field=function(d)
			if self.builder then return self.builder(d) end			
		end, 
		name=function()
			return header			
		end
		}
	})
	if self.cheader then 
		self.cheader:setBaseStyle(if direction then "toolbox.styHeaderHorizontal" else "toolbox.styHeaderVertical")
	end
end

UI.Toolbox.Definition= {
  name="Toolbox",
  icon="ui/icons/panel.png",
  class="UI.Toolbox",
  constructorArgs={ "Header", "Direction" },
  properties={
    { name="Header", type="sprite" },
  },
}

--[[
ToolPie Builder:
	Called with data as argument and returns
	- content as a generic sprite/string/meta
	- placement rules as either a table, an index, or nil to use next slot
		If a table, it can contain:
		- a slot index (slot)
		- a slot span (if using multiple slots)
		- a custom placement function (placer) with will be called with following args: 
			- the sprite to place
			- the diameter of a slot
			- the first slot position (vector)
			- the last slot position (vector)
]]
UI.ToolPie=Core.class(UI.Panel,function(radius,thickness,angle,direction,span) end)

UI.ToolPie.Definition= {
  name="Toolpie",
  icon="ui/icons/panel.png",
  class="UI.ToolPie",
  constructorArgs={ "Radius", "Thickness", "MidAngle", "Direction", "AngularSpan", "SlotCount" },
  properties={
    { name="Radius", type="number" },
    { name="SlotSize", type="number" },
    { name="Thickness", type="number" },
    { name="MidAngle", type="number" },
    { name="AngularSpan", type="number" },
    { name="EdgeSize", type="number" },
    { name="Direction", type="boolean" },
    { name="SlotCount", type="number" },
  },
}

local Arc=Core.class(Mesh,function() return false end)

Arc.Shader=Core.class(UI.Shader)
function Arc.Shader:init(params)
	self.params=params
	self.shader=self.overrideStandardShader(Arc.Shader,Shader.SHADER_PROGRAM_TEXTURE,0,function(spec)
@shader
		function spec.vertexShader(vVertex,vColor,vTexCoord)
			local Thalf=(^<vAngleRad.x)/2 
			local Phy=Thalf*vVertex.x
			local Ru=vAngleRad.y+vAngleRad.z
			local Rd=cos(Thalf)*(vAngleRad.y-vAngleRad.z)
			local Rmed=(Rd+Ru)/2
			local Rp=Rmed+(Ru-Rmed)*vVertex.y
			local Vint=hF2(tan(Phy)*Rp,Rp)
			
		    local angle=^<(vAngleRad.x*hF1(InstanceID)+vAngleRad.w)+Thalf
			local ca=-cos(angle)
			local sa=hlslYSwap*sin(angle)
			local rot=hF22(ca,sa,-sa,ca)
			Vint=Vint*rot
			
			fTexCoord=hF2(tan(Phy)*Rp,Rp)
			fTinf=hF4(angle,vAngleRad.yz,^<vAngleRad.w)
			return vMatrix*hF4(Vint,0,1)
		end
		table.insert(spec.varying,{name="fTinf",type=Shader.CFLOAT4})
		table.insert(spec.uniforms,{name="fTextureInfo",type=Shader.CFLOAT4,sys=Shader.SYS_TEXTUREINFO})
		table.insert(spec.uniforms,{name="fNineV",type=Shader.CFLOAT4,})
		table.insert(spec.uniforms,{name="fNineT",type=Shader.CFLOAT4,})
		table.insert(spec.uniforms,{name="fAngleSpan",type=Shader.CFLOAT,})
		table.insert(spec.uniforms,{name="vAngleRad",type=Shader.CFLOAT4,vertex=true})
		table.insert(spec.uniforms,{name="colLayer1",type=Shader.CFLOAT4,vertex=false})
		table.insert(spec.uniforms,{name="colLayer2",type=Shader.CFLOAT4,vertex=false})
		table.insert(spec.uniforms,{name="colLayer3",type=Shader.CFLOAT4,vertex=false})
		table.insert(spec.uniforms,{name="colLayer4",type=Shader.CFLOAT4,vertex=false})
		function spec.fragmentShader() : Shader

			local Phy=atan2(fTexCoord.x,fTexCoord.y)
			local R=fTexCoord.y/cos(Phy)
			local D=(fTinf.x-fTinf.w-Phy)/^<fAngleSpan
			local frag=lF4(0,0,0,0)
			if D<0 or D>1 or R>(fTinf.y+fTinf.z) or R<(fTinf.y-fTinf.z) then 
				discard()
			else
				local tn=hF2(D,(R-fTinf.y)/(2*fTinf.z)+0.5)
				local af=fTinf.y*^<fAngleSpan
				local rf=2*fTinf.z
				local bn=fNineV/hF4(af,af,rf,rf)
				if tn.x<bn.x then
					tn.x=mix(0,fNineT.x,tn.x/bn.x)
				elseif tn.x>(1-bn.y) then
					tn.x=mix(1-fNineT.y,1,(tn.x-(1-bn.y))/bn.y)
				else
					tn.x=mix(fNineT.x,1-fNineT.y,(tn.x-bn.x)/(1-bn.x-bn.y))
				end
				if tn.y<bn.z then
					tn.y=mix(0,fNineT.z,tn.y/bn.z)
				elseif tn.y>(1-bn.w) then
					tn.y=mix(1-fNineT.w,1,(tn.y-(1-bn.w))/bn.w)
				else
					tn.y=mix(fNineT.z,1-fNineT.w,(tn.y-bn.z)/(1-bn.z-bn.w))
				end
				tn=tn*fTextureInfo.xy
				local t=texture2D(fTexture,tn)
				
				frag=lF4(lF3(colLayer1.rgb),1)*t.r*colLayer1.a
				local f2=lF4(lF3(colLayer2.rgb),1)*t.g*colLayer2.a
				frag=frag*(1-f2.a)+f2
				f2=lF4(lF3(colLayer3.rgb),1)*t.b*colLayer3.a
				frag=frag*(1-f2.a)+f2
				f2=lF4(lF3(colLayer4.rgb),1)*t.a*colLayer4.a
				frag=frag*(1-f2.a)+f2

			end

			frag=lF4(fColor)*frag
			if (frag.a==0.0) then discard() end
			return lF4(frag)
		end
	end)
end

function Arc:init(innerRadius,outerRadius,startAngle,endAngle)
	self.innerRadius=innerRadius or 50
	self.outerRadius=outerRadius or 100
	self.startAngle=startAngle or 5
	self.endAngle=endAngle or 135
	self.corners={20,20,20,20}
	self.tcorners={0.4,0.4,0.4,0.4}
	self.shader=Arc.Shader.new({})
	self.shader:apply(self)	
	self:setVertexArray(-1,-1,-1,1,1,1,1,-1)
	self:setTextureCoordinateArray(-1,-1,-1,1,1,1,1,-1)
	self:setIndexArray(1,2,3,1,3,4)
	self:update()
end

function Arc:setParameters(innerRadius,outerRadius,startAngle,endAngle)
	self.innerRadius=innerRadius or self.innerRadius
	self.outerRadius=outerRadius or self.outerRadius
	self.startAngle=startAngle or self.startAngle
	self.endAngle=endAngle or self.endAngle
	self:update()
end

function Arc:update()
	local theta=30
	local span=self.endAngle-self.startAngle
	local instn=(span//theta)<>1
	theta=span/instn
	self:setInstanceCount(instn)
	self:setShaderConstant("vAngleRad",Shader.CFLOAT4,1,
		theta,
		(self.innerRadius+self.outerRadius)/2,
		(self.outerRadius-self.innerRadius)/2,
		self.startAngle)
	self:setShaderConstant("fAngleSpan",Shader.CFLOAT,1,span)
	self:setShaderConstant("fNineV",Shader.CFLOAT4,1,unpack(self.corners))
	self:setShaderConstant("fNineT",Shader.CFLOAT4,1,unpack(self.tcorners))
end

function Arc:setTextureCorners(tcv,tct)
	self.corners={tcv,tcv,tcv,tcv}
	self.tcorners={tct,tct,tct,tct}
	self:setShaderConstant("fNineV",Shader.CFLOAT4,1,unpack(self.corners))
	self:setShaderConstant("fNineT",Shader.CFLOAT4,1,unpack(self.tcorners))
end

function UI.ToolPie:init(radius,thickness,angle,direction,span,nslot)
	self.arc=Arc.new()
	self.arc._style={ __Parent=self._style }
	self.arc.shader:setParameters({
		colLayer1="toolpie.colRing1",
		colLayer2="toolpie.colRing2",
		colLayer3="toolpie.colRing3",
		colLayer4="toolpie.colRing4",
	})
	self:addChild(self.arc)
	self.thickness=thickness or "2s"
	self.radius=radius or "5s"
	self.midAngle=angle or 0
	self.direction=direction or false
	self.angleSpan=span
	self.edgeSize=0
	self.slotCount=nslot
	self.data={}
	self.slots={}
	self.datacells={}
	self:update()
	self:setLayoutParameters{}
end

function UI.ToolPie:updateStyle(fromParent)
	UI.Panel.updateStyle(self)
	local bTex=self:resolveStyle("toolpie.txRing")
	self.arc:setTexture(bTex)
	local rcV=self:resolveStyle("toolpie.szRingCorner")
	local rcT=self:resolveStyle("toolpie.szRingCornerTextureRatio")
	self.arc:setTextureCorners(rcV,rcT)
	self.arc.shader:apply(self.arc)
	self:update()
end

function UI.ToolPie:setMidAngle(angle)
	self.midAngle=angle
	self:update()
end

function UI.ToolPie:setAngularSpan(angle)
	self.angleSpan=angle
	self:update()
end

function UI.ToolPie:setEdgeSize(sz)
	self.edgeSize=sz
	self:update()
end

function UI.ToolPie:setDirection(cww)
	self.direction=ccw
	self:update()
end

function UI.ToolPie:setRadius(radius)
	self.radius=radius
	self:update()
end

function UI.ToolPie:setThickness(thickness)
	self.thickness=thickness
	self:update()
end

function UI.ToolPie:setSlotSize(slot)
	self.slot=slot
	self:update()
end

function UI.ToolPie:setSlotCount(slotn)
	self.slotCount=slotn
	self:update()
end


function UI.ToolPie:update()
	local rm=self:resolveStyle(self.radius)
	local tm=self:resolveStyle(self.thickness)
	local slot=tm
	if self.slot then 
		slot=self:resolveStyle(self.slot)
	elseif self.angleSpan and self.slotCount then
		slot=((^<self.angleSpan)/self.slotCount)*rm
	end
	self.placerRadius=rm
	self.placerSlotSize=slot
	self:place()
	local span=if self.angleSpan then self.angleSpan else ^>(self.maxSlots*slot/rm)
	local edge=self:resolveStyle(self.edgeSize)
	edge=^>(edge/rm)
	if self.direction then span=-span end	
	self.arc:setParameters(rm-tm/2,rm+tm/2,self.midAngle-span/2-edge,self.midAngle+span/2+edge)
end

function UI.ToolPie:setData(data,builder) 
	--Same builder: check for data to keep
	local cache
	if data and self.data and builder==self.builder then
		cache={}
		local keep={}
		for _,d in ipairs(data) do keep[d]=true end
		for i,d in ipairs(self.data) do
			if keep[d] then
				cache[d]=self.slots[i]
			end
		end
	end
	self.data=data
	self.builder=builder
		
	for _,c in pairs(self.slots) do
		c.spr:removeFromParent()
		if not cache or not cache[c.d] then
			if c.spr.destroy then c.spr:destroy() end 
		end
	end
	self.slots={}
	self.datacells={}
	
	if data then
		for i,d in ipairs(data) do
			local cell = cache and cache[d] or self:buildSlot(d)
			self.slots[i]=cell
			self:addChild(cell.spr)
			self.datacells[d]=cell
		end
		self:update()
	end	
end

function UI.ToolPie:buildSlot(d)
	local spr
	local placer
	if self.builder then
		spr,placer=self.builder(d)
	else
		spr=d
	end
	spr=UI.Utils.makeWidget(spr,d)
	if type(placer)=="number" then
		placer={slot=placer}
	else
		placer=placer or { }
	end
	return { d=d,spr=spr,placer=placer }	
end

function UI.ToolPie:place()
	local place=0
	local sdim=self.placerSlotSize
	local rd=self.placerRadius
	for _,cell in ipairs(self.slots) do
		place=cell.placer.slot or place+1
		cell.slot=place
		local span=cell.placer.span or 1
		place=place+span-1
	end
	self.maxSlots=self.slotCount or place
	
	local span=if self.angleSpan then self.angleSpan else ^>(self.maxSlots*sdim/rd)
	if self.direction then span=-span end	

	local function getLoc(sn)
		sn-=0.5
		if self.direction then sn=-sn end	
		local ag=sn*sdim/rd+^<(self.midAngle-span/2)
		local v=vector(math.sin(ag)*rd,-math.cos(ag)*rd)
		return v
	end
	for _,cell in ipairs(self.slots) do
		local firstLoc=getLoc(cell.slot)
		if cell.placer.placer then
			local lastLoc=getLoc(cell.slot+(cell.placer.span or 1)-1)			
			cell.loc1=firstLoc
			cell.loc2=lastLoc
			cell.placer.placer(cell.spr,sdim,firstLoc,lastLoc)
		else
			cell.loc1=firstLoc
			cell.loc2=firstLoc
			cell.spr:setLayoutConstraints{ offsetx=firstLoc.x,offsety=firstLoc.y,originx=-0.5,originy=-0.5, fill=0, gridx=0,gridy=0 }
		end
	end
end

function UI.ToolPie:getLocation(d)
	local cell=self.datacells[d]
	if not cell then return end
	return cell.loc1,cell.loc2
end