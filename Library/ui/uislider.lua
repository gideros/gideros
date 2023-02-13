--!NEEDS:uipanel.lua
UI.Slider=Core.class(UI.Panel,function(params) return end)
UI.Slider.HORIZONTAL=false
UI.Slider.VERTICAL=true
function UI.Slider:init(direction,svgpath,resolution,dual,offset)
	self.knobResolution=resolution or 0.001
	self.svgPath=svgpath
	self.vertical=direction
	
	self.path=Path2D.new()
		
	self.rail=Mesh.new()
	self.prail=Mesh.new()

	self.dual=dual
	self.handleOffset=offset
	local function makeKnob()
		local knob=UI.Panel.new()
		knob:setLocalStyle("slider.styKnob")
		knob:setStateStyle("slider.styKnobNormal")
		if self.handleOffset then
			knob:setLayoutParameters({ rowWeights={1}, columnWeights={1}})
			local p=UI.Panel.new()
			p:setStyle("slider.styHand")
			p:setLayoutConstraints({fill=Sprite.LAYOUT_FILL_VERTICAL, width="slider.szHand"})
			knob:addChild(p) knob.hand=p
		end
		return knob
	end
	self.knob=makeKnob()
	if dual then
		self.knob2=makeKnob()
	end
	self:setLayoutParameters{ rowWeights={1}, columnWeights={1}}

	self:addChild(self.rail) 
	self:addChild(self.prail) 
	self:addChild(self.knob)
	if dual then
		self:addChild(self.knob2)
	end
	
	UI.Control.onDragStart[self]=self
	UI.Control.onDrag[self]=self
	UI.Control.onDragEnd[self]=self
		
	self:updatePath()
	self.knobPos={0,1}
	self.knobCenter=-1
	self.indicator={}
	self.indicatorLayer={}
	
	self:onLayout()
	
	self:addEventListener(Event.LAYOUT_RESIZED,self.onLayout,self)
end
function UI.Slider:newClone() assert(false,"Cloning not supported") end

function UI.Slider:setPath(path)
	self.svgPath=path
	self:updatePath()
end
function UI.Slider:setVertical(dir)
	self.vertical=dir
	self:updatePath()
end
function UI.Slider:setResolution(res)
	self.knobResolution=res
	self:setKnobPosition(self.knobPos[1],self.knobPos[2])
end
function UI.Slider:setCenter(ctr)
	self.knobCenter=ctr
	self:setKnobPosition(self.knobPos[1],self.knobPos[2])
end
function UI.Slider:setIndicator(indicator)
	self:makeIndicator(self.knob,self.indicator[self.knob],1)
	if self.dual then
		self:makeIndicator(self.knob2,self.indicator[self.knob2],-1)
	end
	self:placeKnob(self.knobPos)
end
function UI.Slider:setFormatter(formatter)
	self.formatter=formatter
	self:placeKnob(self.knobPos)
end

function UI.Slider:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	local ms=self:resolveStyle("slider.szKnob")
	self.handleDisp=ms*(self.handleOffset or 0)
	local function updateKnob(knob,dir)
		knob:setLayoutConstraints{ width=ms, height=ms+self.handleDisp*2, anchorx=0,anchory=0 }
		if knob.hand then
			if dir>0 then
				knob:setLayoutParameters({ insetTop=self.handleDisp+ms, insetBottom=ms*.25})
			else
				knob:setLayoutParameters({ insetBottom=self.handleDisp+ms, insetTop=ms*.25})
			end
		end
	end
	updateKnob(self.knob,1)
	self.rail:setPosition(ms/2,ms/2+self.handleDisp)
	self.prail:setPosition(ms/2,ms/2+self.handleDisp)
	self.knobSize=ms
	if self.dual then
		updateKnob(self.knob2,-1)
	end
	self:setRailStyle(self.rail,self:resolveStyle("slider.styRail"))
	self:updateRail()
	self:placeKnob(self.knobPos)
end

function UI.Slider:setRailStyle(rail,style)
	local tstyle=table.clone(style)
	tstyle.__Parent=self._style
	rail._style=tstyle
	self:setShaderSpec(tstyle.shader,rail)
	local tex=self:resolveStyle("slider.railTexture",tstyle)
	rail:setTexture(tex)
	rail.texW,rail.texH=tex:getWidth(),tex:getHeight()
	rail.corners=self:resolveStyle("slider.railCorners",tstyle)
	rail.th=self:resolveStyle("slider.szRail",tstyle)
	for i=1,#rail.corners do rail.corners[i]=self:resolveStyle(rail.corners[i],tstyle) end
	self.rail:setShaderConstant("fCornersT",Shader.CFLOAT4,1,rail.corners[5]/rail.texW,rail.corners[6]/rail.texW,rail.corners[7]/rail.texH,rail.corners[8]/rail.texH)
end
	
function UI.Slider:onLayout(e)
	self:updateRail()
	self:placeKnob(self.knobPos)
end

function UI.Slider:updateRail()
	local corners=self.rail.corners
	if not corners then return end
	local tw,th=self.rail.texW,self.rail.texH
	local TH=self.rail.th
	
	
	local pp=self.path:getPathPoints()
	self.pathLine=table.clone(pp)
	local pathsh={}
	for p=1,#pp-1,1 do
		pathsh[p]={ 
			x=pp[p].x, 
			y=pp[p].y, 
			x2=pp[p+1].x, 
			y2=pp[p+1].y,
			o=pp[p].offset,
			}
	end
	self.pathShape=pathsh
	self.pathLength=pp[#pp].offset

	local mix,miy,max,may=pp[1].x,pp[1].y,pp[1].x,pp[1].y
	for p=1,#pp do 
		mix=mix><pp[p].x
		miy=miy><pp[p].y
		max=max<>pp[p].x
		may=may<>pp[p].y
	end
	local dw,dh=self:getDimensions()
	dw=(dw-self.knobSize)<>0
	dh=(dh-self.knobSize)<>0
	local ew,eh=max-mix,may-miy
	local sc
	if ew>0 and eh>0 then
		sc=(dw/ew)><(dh/eh)
	elseif ew>0 then
		sc=(dw/ew)
	elseif eh>0 then
		sc=(dh/eh)
	else
		return
	end
	self.pathScale=sc
	self.pathRefX=mix
	self.pathRefY=miy
	
	local length=pp[#pp].offset
	table.insert(pp,1,{
			x=pp[1].x-math.sin(^<pp[1].angle)*corners[1]/sc,
			y=pp[1].y+math.cos(^<pp[1].angle)*corners[1]/sc,
			offset=-length,
			angle=pp[1].angle
		})
	table.insert(pp,{
			x=pp[#pp].x+math.sin(^<pp[#pp].angle)*corners[2]/sc,
			y=pp[#pp].y-math.cos(^<pp[#pp].angle)*corners[2]/sc,
			offset=2*length,
			angle=pp[#pp].angle
		})
	
	local meshv,meshi,mesht={},{},{}
	for p=1,#pp do 
		local angle,exp=pp[p].angle,1
		if p>1 and p<#pp then
			local ag=(angle-pp[p-1].angle)/2
			exp=math.abs(math.cos(^<ag))<>.5
			angle=(angle+pp[p-1].angle)/2
		end
		local mdx=math.sin(^<angle)/exp
		local mdy=-math.cos(^<angle)/exp
		local px,py=(pp[p].x-mix)*sc,(pp[p].y-miy)*sc
		meshv[p*4-3]=px+mdy*TH
		meshv[p*4-2]=py-mdx*TH
		meshv[p*4-1]=px-mdy*TH
		meshv[p*4-0]=py+mdx*TH
		local o=tw*(pp[p].offset/length)
		mesht[p*4-3]=o
		mesht[p*4-2]=0
		mesht[p*4-1]=o
		mesht[p*4-0]=th
	end
	for p=1,#pp-1,1 do 
		local tp=#pp-p
		meshi[tp*6-5]=p*2-1
		meshi[tp*6-4]=p*2+1
		meshi[tp*6-3]=p*2
		meshi[tp*6-2]=p*2+1
		meshi[tp*6-1]=p*2
		meshi[tp*6-0]=p*2+2
	end
	self.rail:setVertexArray(meshv)
	self.rail:setIndexArray(meshi)
	self.rail:setTextureCoordinateArray(mesht)
	self.rail:setShaderConstant("fCornersV",Shader.CFLOAT4,1,corners[1]/(TH*2),corners[2]/(TH*2),corners[3]/(TH*2),corners[4]/(TH*2))
end
	
function UI.Slider:updatePath()
	local path=self.svgPath or if self.vertical then "M0,0v1" else "M0,0h1"
	self.path:setSvgPath(path)
	self:updateRail()
end

function UI.Slider:setKnobPosition(p,p2)
	p=(((p+self.knobResolution/2)/self.knobResolution)//1)*self.knobResolution
	p=(p<>0)><1
	p2=p2 or 1
	p2=(((p2+self.knobResolution/2)/self.knobResolution)//1)*self.knobResolution
	p2=(p2<>p)><1
	if p~=self.knobPos[1] or p2~=self.knobPos[2] then
		self.knobPos={p,p2}
		self:placeKnob(self.knobPos)
	end
end

function UI.Slider:getKnobPosition()
	return self.knobPos[1],self.knobPos[2]
end

function UI.Slider:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	if changes.disabled~=nil then
		self:setStateStyle(if changes.disabled then "slider.styDisabled" else "slider.styNormal")
		self.knob:setStateStyle(if changes.disabled then "slider.styKnobDisabled" else "slider.styKnobNormal")
		if self.dual then
			self.knob2:setStateStyle(if changes.disabled then "slider.styKnobDisabled" else "slider.styKnobNormal")
		end
	end
end

function UI.Slider:onDragStart(x,y,ed,ea,change,long)
	if long then return end
	if self:getFlags().disabled then return end
	local kx,ky=self.knob:getAnchorPosition()
	if self.vertical then kx-=self.handleDisp else ky-=self.handleDisp end
	local d=(x+kx)^2+(y+ky)^2
	if d<=(self.knobSize^2) then
		self.sliding=1
		UI.dispatchEvent(self,"WidgetChanging",true)
		return true
	end
	if self.dual then
		local kx,ky=self.knob2:getAnchorPosition()
		if self.vertical then kx-=self.handleDisp else ky-=self.handleDisp end
		local d=(x+kx)^2+(y+ky)^2
		if d<=(self.knobSize^2) then
			self.sliding=2
			UI.dispatchEvent(self,"WidgetChanging",true)
			return true
		end
	end
end

function UI.Slider:onDragEnd()
	UI.dispatchEvent(self,"WidgetChanging",false)
	self.sliding=nil
end

function UI.Slider:onDrag(x,y)
	if self.sliding then
		local kp=self.knobPos[self.sliding]
		local np=self:knobPosToValue(x,y,kp)
		local nn=table.clone(self.knobPos)
		nn[self.sliding]=np
		self:setKnobPosition(nn[1],nn[2])
		if kp~=self.knobPos[self.sliding] then
			UI.dispatchEvent(self,"WidgetChange",self.knobPos[1],self.knobPos[2])
		end
		return true
	end
end

function UI.Slider:makeIndicator(ref,indicator,dir)
	self.indicator[ref]=indicator
	if indicator and not self.indicatorLayer[ref] then
		self.indicatorLayer[ref]=UI.Panel.new()
		self.indicatorLayer[ref]:setLayoutParameters{}
		self.indicatorLayer[ref]:addChild(indicator)
		local oy=0
		if (self.handleOffset or 0)==0 then -- TODO This should be configurable somehow
			dir=-1
			oy=-self.knobSize/2
		else 
			dir=-0.5
		end
		indicator:setLayoutConstraints{ originx=-.5, originy=dir, offsety=oy }
		ref:addChild(self.indicatorLayer[ref])
	elseif self.indicatorLayer[ref] and not self.indicator[ref] then
		self.indicatorLayer[ref]:removeFromParent()
	end
end
	
function UI.Slider:placeKnob(p)
	if not self.pathShape then return end
	self.rail:setShaderConstant("fRatio",Shader.CFLOAT2,1,if self.dual then p[1] else p[1]><self.knobCenter,if self.dual then p[2] else p[1]<>self.knobCenter)
	local function placeKnob(k,p,dir)
		local op=p
		p*=self.pathLength
		local pp,lp
		for _,ps in ipairs(self.pathShape) do
			if ps.o>p then
				if not pp then
					pp=ps
				else
					lp=p-pp.o
				end
				break
			end
			pp=ps
			lp=p-pp.o
		end
		local sl=math.distance(pp.x,pp.y,pp.x2,pp.y2)
		p=lp
		if p>sl then p=sl end
		local dx,dy=pp.x2-pp.x,pp.y2-pp.y
		local oy,ox=math.normalize(dx,dy)
		local nx=pp.x+dx*p/sl
		local ny=pp.y+dy*p/sl
		local px=(nx-self.pathRefX)*self.pathScale-self.handleDisp*ox*dir
		local py=(ny-self.pathRefY)*self.pathScale-self.handleDisp*oy*dir
		k:setAnchorPosition(-px,-py)
		if self.formatter then
			if not self.indicator[k] then
				self:makeIndicator(k,UI.Utils.makeWidget(""),dir)
			end
			self.indicatorLayer[k]:setAnchorPosition(-self.knobSize/2,-self.knobSize/2-self.handleDisp)
			self.formatter(self.indicator[k],op)
		end
	end
	placeKnob(self.knob,p[1],1)
	if self.dual then
		placeKnob(self.knob2,p[2],-1)
	end
end

function UI.Slider:knobPosToValue(x,y,oref)
	if not self.pathShape then return 0 end
	local px=(x-self.knobSize/2)/self.pathScale+self.pathRefX
	local py=(y-self.knobSize/2)/self.pathScale+self.pathRefY
	local hits={}
	local src={x=px,y=py}
	for n,p in ipairs(self.pathShape) do
		local d=math.edge(src,p)
		d.o=p.o+math.distance(d,p)
		d.d=math.distance(d,src)
		hits[n]=d
	end
	if oref then
		oref*=self.pathLength
		table.sort(hits,function(a,b) 			
			return (a.d+.1*math.abs(a.o-oref))<(b.d+.1*math.abs(b.o-oref))
		end)
	else
		table.sort(hits,function(a,b) return a.d<b.d end)
	end

	return hits[1].o/self.pathLength
end

UI.Slider.Definition= {
	name="Slider",
	icon="ui/icons/panel.png",
	class="UI.Slider",
	constructorArgs={ "Vertical", "Path", "Resolution","Dual","HandleOffset"  },
	properties={
		{ name="Path", type="string" },
		{ name="Vertical", type="boolean" },
		{ name="Resolution", type="number"},
		{ name="Center", type="number"},
		{ name="Indicator", type="sprite"},
		{ name="Formatter", type="function"},
		{ name="Dual", type="boolean" },
		{ name="HandleOffset", type="number" },
	},
}

UI.ArcSlider=Core.class(UI.Slider,function(astart,aend,res,dual,offset) 
	astart=astart or 0
	aend=aend or 90
	local sc=1000
	local e1x,e1y=math.sin(^<astart)*sc,-math.cos(^<astart)*sc
	local e2x,e2y=math.sin(^<aend)*sc,-math.cos(^<aend)*sc
	local large=0
	if (aend-astart)>180 then large=1 end
	local p="M"..e1x..","..e1y..
			"A"..sc..","..sc..","..^>(aend-astart)..","..large..",1,"..e2x..","..e2y

	return false,p,res,dual,offset
end)

UI.ArcSlider.Definition= {
	name="Slider",
	icon="ui/icons/panel.png",
	class="UI.ArcSlider",
	constructorArgs={ "StartAngle", "EndAngle", "Resolution","Dual","HandleOffset"  },
	properties={
		{ name="StartAngle", type="number"},
		{ name="EndAngle", type="number"},
		{ name="Resolution", type="number"},
		{ name="Center", type="number"},
		{ name="Formatter", type="function"},
		{ name="Indicator", type="sprite"},
	},
}


