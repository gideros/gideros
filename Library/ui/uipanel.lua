--!NEEDS:uiinit.lua

--[[ FLAGS:
- ticked 
- selected
- focused
- focusable (Generic, whether a component can take focus or not if not implicit)
- expanded (Tree, Accordion)
- disabled
- readonly (Textfield, 'not editable')
- third (Third state, checkbox)
- focusgroup (Focus:next() will stay within this group)
]]

--[[ ABOUT STYLE LAYERS:
	- Style: External style override, use this when you want to change the default style of a whole object tree from the outside
	- LocalStyle: External/Internal style override, use this in custom components to override the base style of a subcomponent
	- BaseStyle: Base style for a component, that is style common to all states or default state
	- StateStyle: State specific style of a component
	- InternalStyle: Style definitions for this panel component, should never be propagated to children
]]

UI.Panel=Core.class(Pixel,function(bc)
  return 0,0
end)

function UI.Panel:init(bc) 
	self._flags={} 
	self._istyle={ __Parent=UI.Style._style }
	self._style={ __Parent=self._istyle }
	Sprite.setStyle(self,self._style)
	self:setColor(bc or "colWidgetBack")
end

function UI.Panel:destroy()
	self:removeFromParent()
end

function UI.Panel:setFlags(changes)
	table.clone(changes,self._flags)
end
function UI.Panel:getFlags() return self._flags end
function UI.Panel:setFlagsAll(changes)
	self:setFlags(changes)
	for i=1,self:getNumChildren() do
		local c=self:getChildAt(i)
		if c.setFlagsAll then c:setFlagsAll(changes) end
	end
end

local function sstyle(self,sc,style,p)
	local p=p or rawget(sc,"__Parent")
	local cc=rawget(sc,"__Cache")
	table.clear(sc)
	if type(style)=="string" then
		rawset(sc,"__Reference",style)
	else
		assert(type(style)=="table","Style is undefined:"..tostring(style))
		table.clone(style,sc)
	end
	sc.__Parent=p
	if cc then table.clear(cc) sc.__Cache=cc end
	Sprite.setStyle(self,self._style,true)
end
function UI.Panel:setStyle(style)
	sstyle(self,self._istyle,style or {})
end
function UI.Panel:setLocalStyle(style)
	local relink=false
	if not self._lstyle then
		self._lstyle={}
		(self._bstyle or self._sstyle or self._style).__Parent=self._lstyle
		relink=true
	end
	sstyle(self,self._lstyle,style or {},self._istyle)
	if relink then self:setStyleInheritance(nil) end
end
function UI.Panel:setBaseStyle(style)
	local relink=false
	if not self._bstyle then
		self._bstyle={}
		(self._sstyle or self._style).__Parent=self._bstyle
		relink=true
	end
	sstyle(self,self._bstyle,style or {},self._lstyle or self._istyle)
	if relink then self:setStyleInheritance(nil) end
end
function UI.Panel:setStateStyle(style)
	local relink=false
	if not self._sstyle then
		self._sstyle={}
		self._style.__Parent=self._sstyle
		relink=true
	end
	sstyle(self,self._sstyle,style or {},self._bstyle or self._lstyle or self._istyle)
	if relink then self:setStyleInheritance(nil) end
end
function UI.Panel:setInternalStyle(style)
	sstyle(self,self._style,style or {},self._sstyle or self._bstyle or self._lstyle or self._istyle)
end

--TODO move this to utils ?
local function getglobal(s)
	local g=_G
	for p in s:gmatch("([^.]+)") do assert(g,"Couldn't find "..s) g=g[p] end
	return g
end

local function resolveClass(cname)
	local class=cname
	if type(class)=="string" then
		class=getglobal(class)
	end
	return class
end

function UI.Panel:updatedStyle()
	Sprite.setStyle(self,self._style,true)
end

function UI.Panel:updateStyle(fromParent)
	Pixel.updateStyle(self)
	
	if not self.border and not self._borderSpec and self:resolveStyle("brdWidget") then self._borderSpec="brdWidget" end
	if self._borderSpec then self:setBorder(self._borderSpec) end
	self:setShaderSpec(self:resolveStyle("shader"),self)
end

function UI.Panel:setShaderSpec(shaderSpec,target)
	local sshader=shaderSpec
	target=target or self
	if sshader~=target._shaderSpec then
		if target._shaderSpec then
			if target._shaderEffect and target._shaderEffect.remove then 
				target._shaderEffect:remove(target) 
			else
				target:setShader(nil)
			end
			target._shaderEffect=nil
		end
		target._shaderSpec=sshader
		if target._shaderSpec then
			if target._shaderSpec.class then
				target._shaderEffect=resolveClass(target._shaderSpec.class).new(target._shaderSpec.params)
			else
				target._shaderEffect=self._shaderSpec
			end
			if target._shaderEffect.apply then 
				target._shaderEffect:apply(target) 
			elseif target._shaderEffect.getClass then
				assert(target._shaderEffect:getClass()=="Shader","Shader specification should be a ShaderEffect or a Shader")
				target:setShader(target._shaderEffect)
			end
		end
	elseif target._shaderEffect and target._shaderEffect.apply then 
		target._shaderEffect:apply(target) 
	end
end

function UI.Panel:inheritableStyle(parentStyleInheritance)
	local s=self
	local bstyle=s._istyle
	local inheritance=parentStyleInheritance or s.styleInheritance
	if inheritance=="local" then
		bstyle=s._lstyle or s._istyle
	elseif inheritance=="base" then
		bstyle=s._bstyle or s._lstyle or s._istyle
	elseif inheritance=="state" then
		bstyle=s._sstyle or s._bstyle or s._lstyle or s._istyle
	elseif inheritance=="internal" then
		bstyle=s._style or s._sstyle or s._bstyle or s._lstyle or s._istyle
	end
	return bstyle
end

local function linkStyle(s,c,noupd)
	if c.updatedStyle then
		local bstyle=s._istyle
		local inheritance=c.parentStyleInheritance or s.styleInheritance
		if inheritance=="local" then
			bstyle=s._lstyle or s._istyle
		elseif inheritance=="base" then
			bstyle=s._bstyle or s._lstyle or s._istyle
		elseif inheritance=="state" then
			bstyle=s._sstyle or s._bstyle or s._lstyle or s._istyle
		elseif inheritance=="internal" then
			bstyle=s._style or s._sstyle or s._bstyle or s._lstyle or s._istyle
		end
		c._istyle.__Parent=bstyle
		if not noupd and s:isOnStage() then
			Sprite.setStyle(c,c._style,true)
		end
	else
		--assert(c:getClass()=="TextField","Not a UI.Panel: "..c:getClass())
	end
end

function UI.Panel:newClone()
	self.__parent=nil --Fixed in future Gideros
	local tc=table.clone
	local t,t2
	t=tc(self._istyle) t.__Parent=UI.Style._style self._istyle=t t2=t
	if self._lstyle then t=tc(self._lstyle) t.__Parent=t2 self._lstyle=t t2=t end
	if self._bstyle then t=tc(self._bstyle) t.__Parent=t2 self._bstyle=t t2=t end
	if self._sstyle then t=tc(self._sstyle) t.__Parent=t2 self._sstyle=t t2=t end
	t=tc(self._style) t.__Parent=t2 self._style=t
	
	Sprite.setStyle(self,t,false)

	self._flags=tc(self._flags)
	local lb=self.behavior
	if lb then lb=table.clone(lb) self.behavior=lb lb:clone(self) end
	--Relink children
	local ct=self.__children
	if ct then
		local lc,lv=nil,nil
		while true do
			lc,lv=next(ct,lc)
			if lv then
				linkStyle(self,lv,true)
			else
				break
			end
		end
	end
end

function UI.Panel:setStyleInheritance(mode)
	if mode then
		if mode==self.styleInheritance then return end
		self.styleInheritance=mode
	end
	local sc=self.__children
	if sc then
		local lc,lv=nil,nil
		while true do
			lc,lv=next(sc,lc)
			if lv then
				linkStyle(self,lv,true)
			else
				break
			end
		end
	end
end

function UI.Panel:getStyleInheritance()
	return self.styleInheritance
end

function UI.Panel:setParentStyleInheritance(mode)
	if mode then
		if mode==self.parentStyleInheritance then return end
		self.parentStyleInheritance=mode
	end
	local p=self:getParent()
	if p then
		linkStyle(p,self)
	end
end

function UI.Panel:getParentStyleInheritance()
	return self.parentStyleInheritance
end

function UI.Panel:addChild(c)
	local op=c:getParent()
	local id=Sprite.addChild(self,c)
	if op~=self then
		linkStyle(self,c)
	end
	return id
end

function UI.Panel:addChildAt(c,n)
	local op=c:getParent()
	local id=Sprite.addChildAt(self,c,n)
	if op~=self then
		linkStyle(self,c)
	end
	return id
end

function UI.Panel:setBorder(border)
  if type(border)=="string" then 
	self._borderSpec=border
	border=self:resolveStyle(border)
  else
	self._borderSpec=nil
  end
  if border and not(next(border)) then border=nil end --Null border
  if border and not border.apply then --Check for a border spec
    assert(border.class,"Border spec must have a class:"..(self._borderSpec or "nil"))
    border=border.class.new(border.params or border)
  end
  local layoutParams=Sprite.getLayoutParameters(self,true) or { insetTop=0, insetRight=0, insetBottom=0, insetLeft=0}
  if self.border then 
	local bi=self.border.insets
	if type(bi)=="table" then
		layoutParams.insetTop-=self:resolveStyle(bi.top)
		layoutParams.insetBottom-=self:resolveStyle(bi.bottom)
		layoutParams.insetLeft-=self:resolveStyle(bi.left)
		layoutParams.insetRight-=self:resolveStyle(bi.right)
	else
		bi=self:resolveStyle(bi)
		layoutParams.insetTop-=bi
		layoutParams.insetBottom-=bi
		layoutParams.insetLeft-=bi
		layoutParams.insetRight-=bi
	end
	self.border:remove(self) 
  end
  self.border=border
  if self.border then self.border:apply(self) end

	if border then
		local bi=border.insets
		if type(bi)=="table" then
			layoutParams.insetTop+=self:resolveStyle(bi.top)
			layoutParams.insetBottom+=self:resolveStyle(bi.bottom)
			layoutParams.insetLeft+=self:resolveStyle(bi.left)
			layoutParams.insetRight+=self:resolveStyle(bi.right)
		else
			bi=self:resolveStyle(bi)
			layoutParams.insetTop+=bi
			layoutParams.insetBottom+=bi
			layoutParams.insetLeft+=bi
			layoutParams.insetRight+=bi
		end
	end
	self:setLayoutParameters({ insetRight=layoutParams.insetRight, insetTop=layoutParams.insetTop, insetBottom=layoutParams.insetBottom, insetLeft=layoutParams.insetLeft })

end

function UI.Panel:getBorder()
  return self.border
end

function UI.Panel:setImage(img)
  if type(img)=="string" then
    img=Texture.new(img,true,{ mipmap=true})
  end
  self:setTexture(img)
end

function UI.Panel:setToolTip(tt)
	self.ToolTip=tt
end

function UI.Panel:getToolTip()
	return self.ToolTip
end

function UI.Panel:getColor()
  return Pixel.getColor(self)
end

UI.Panel.Definition= {
  name="Panel",
  icon="ui/icons/panel.png",
  class="UI.Panel",
  constructorArgs={ "Color" },
  properties={
    { name="Style", type="style" },
    { name="StyleInheritance", type="styleInheritance" },
    { name="ParentStyleInheritance", type="styleInheritance" },
    { name="LocalStyle", type="style" },
    { name="BaseStyle", type="style" },
    { name="InternalStyle", type="style" },
    { name="Flags", type="flags" },
    { name="Color", type="color" },
    { name="Image", type="image" },
    { name="Border", type="border" },
	{ name="ToolTip", type="string" },
  },
}


UI.Viewport=Core.class(UI.Panel)

function UI.Viewport:init(bc)
  self.scrollbarMode={0,0}
  self.panMode=0
  self:setLayoutParameters{ columnWeights={1,0},rowWeights={1,0} }
  self.ipanel=UI.Panel.new()
  self.ipanel:setLayoutConstraints({ fill=Sprite.LAYOUT_FILL_BOTH })
  self.ipanel:addEventListener(Event.LAYOUT_RESIZED,self._resized,self)
  self:addEventListener(Event.LAYOUT_RESIZED,self._resized,self)
  self:addChild(self.ipanel)
  self.cp=UI.Panel.new()
  self.cp:setDimensions(1,1)
  self.cp:setLayoutParameters{ resizeContainer=true }
  self.cp:addEventListener(Event.LAYOUT_RESIZED,self._resized,self)
  self.ipanel:addChild(self.cp)
  UI.Control.onMouseWheel[self]=self
  UI.Control.onDragStart[self]=self
  UI.Control.onDrag[self]=self
  UI.Control.onDragEnd[self]=self
  self:_resized()
end

function UI.Viewport:newClone() assert(false,"Cloning not supported") end

UI.Viewport.SCROLLBAR={
	NONE=0,
	OVER=1,
	AUTO_OVER=2,
	POP_OVER=3,
	ALWAYS=9,
	AUTO=10,
	POP=11,
	DECORATIVE=16, --Scrollbar is just decorative and cannot be dragged
}

UI.Viewport.PANMODE={
	AUTO=0, -- Pan on touch, not on click
	NONE=1, -- Don't pan on user action
	SCROLLBAR=2, --Only pan with scrollbars (when they are present)
	ALWAYS=3, --Pan by all means, touch, click or scroll
}

function UI.Viewport:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	if changes.disabled then
		local s=if changes.disabled then nil else self
		UI.Control.onMouseWheel[self]=s
		UI.Control.onDragStart[self]=s
		UI.Control.onDrag[self]=s
		UI.Control.onDragEnd[self]=s		
	end
end

function UI.Viewport:setPanmode(mode)
	self.panMode=mode
end

function UI.Viewport:setScrollbar(mode)
	local function setupScrollBar(sname,mode,vert)
		mode=tonumber(mode or 0)
		if (mode&3)==0 then
			if self[sname] then 
				self[sname]:destroy()
				self[sname]=nil
			end
		else
			if not self[sname] then
				self[sname]=UI.Scrollbar.new(vert)
				if vert then
					self[sname]:setLayoutConstraints({gridx=1,fill=Sprite.LAYOUT_FILL_BOTH})
				else
					self[sname]:setLayoutConstraints({gridy=1,fill=Sprite.LAYOUT_FILL_BOTH})
				end
				self[sname]:setDecorative((mode&UI.Viewport.SCROLLBAR.DECORATIVE)>0)
				self:addChild(self[sname])
			end			
		end
	end
	if type(mode)~="table" then
		mode={mode,mode}
	end
	self.scrollbarMode=mode
	setupScrollBar("sbHoriz",mode[1],false)
	setupScrollBar("sbVert",mode[2],true)
	local x,y=self.cp:getPosition()
	self:checkRange(x,y)
end

function UI.Viewport:setContent(c)
  if self.content then
    self.content:removeFromParent() 
  end
  self.content=c
  if c then 
    self.cp:addChild(c)
    c:setLayoutConstraints(c:getLayoutConstraints() or {})
  end
  self:_resized()
end

local function getPortSize(self)
 local function select(mode,v,i,n)
  	mode=tonumber(mode or 0)
	if ((mode&8)>0) and not (((mode&15)==10) and n) then return i end
	return v
 end
  local vpw,vph=self:getDimensions()
  local ipw,iph=self.ipanel:getDimensions()
  vpw=select(self.scrollbarMode[2],vpw,ipw,self.rangeh==0)
  vph=select(self.scrollbarMode[1],vph,iph,self.rangew==0)
  return vpw,vph
end

function UI.Viewport:_resized()
	if self._resizing then return end
	self._resizing=true
	local vpw,vph=getPortSize(self)
	local lp = self.cp:getLayoutParameters()
	if lp.columnWidths[1]~=vpw or lp.rowHeights[1]~=vph then 
		self.cp:setLayoutParameters{ resizeContainer=true, columnWidths={vpw}, rowHeights={vph}}
	end
	local cpw,cph = self.cp:getDimensions()
	if cpw and cph and (cpw<vpw or cph<vph) then self.cp:setDimensions(vpw,vph) end
	local t = self.cp:getLayoutInfo(vpw,vph)
	local vw,vh=t.reqWidth,t.reqHeight --NO self.cp:getDimensions()
	self.ipanel:setClip(0,0,vpw,vph)
	local orw,orh=self.rangew,self.rangeh
	self.rangew,self.rangeh=math.max(0,vw-vpw),math.max(0,vh-vph) 
	if orw~=self.rangew or orh~=self.rangeh then
		UI.dispatchEvent(self,"RangeChange",self.rangew,self.rangeh)
	end
	local x,y=self.cp:getPosition()
	self:checkRange(x,y)
	self._resizing=false
end

function UI.Viewport:checkRange(x,y,fromScroll,event)
	local function updateScrollBar(sname,mode,pos,range,page)
		mode=tonumber(mode or 0)&3
		local bar=self[sname]
		if bar then
			if mode==1 or ((range>0) and ((mode==2) or self._dragStart)) then
				bar:setVisible(true)
				bar:setScrollPosition(pos/(page+range),page/(page+range))
			else
				bar:setVisible(false)
			end
		end
	end
	local rx,ry=self.cp:getPosition()
	x=((0><x)<>-self.rangew)
	y=((0><y)<>-self.rangeh)
	self.cp:setPosition(x,y)
	if not fromScroll then
		local vpw,vph=getPortSize(self)
		updateScrollBar("sbHoriz",self.scrollbarMode[1],-x,self.rangew,vpw)
		updateScrollBar("sbVert",self.scrollbarMode[2],-y,self.rangeh,vph)
	end
	if self.content and self.content.onViewportScrolled then
		local vpw,vph=getPortSize(self)
		self.content:onViewportScrolled(self,-x,-y,self.rangew,self.rangeh,vpw,vph)
	end
	local changed=not ((rx==x) and (ry==y))
	if event and changed then
		UI.dispatchEvent(self,"ViewChange",x,y,changed)
	end
	return changed
end

function UI.Viewport:setScrollPosition(x,y)
	return self:checkRange(-x,-y)
end

function UI.Viewport:setScrollAmount(x,y)
  return self:checkRange(-self.rangew*x,-self.rangeh*y)
end

function UI.Viewport:onWidgetChange(w,ratio,page)
	if self.panMode==UI.Viewport.PANMODE.NONE then return end
	if w and w==self.sbHoriz then
		local _,y=self.cp:getPosition()
		self:checkRange(-self.rangew*ratio/(1-page),y,true)
		return true
	elseif w and w==self.sbVert then
		local x,_=self.cp:getPosition()
		self:checkRange(x,-self.rangeh*ratio/(1-page),true)
		return true
	end
end

function UI.Viewport:onMouseWheel(x,y,wheel,distance)
  if self.panMode==UI.Viewport.PANMODE.NONE then return end
  if (self.rangeh or 0)==0 then return end --Not scrollable
  
  self._dragStart={mx=x-self.cp:getX(),my=y-self.cp:getY()}
  local tx,ty=self.cp:getPosition()
  self:checkRange(tx,ty,false,true)
  self:onDrag(x,y+distance)
  self:onDragEnd()
  return true
end

function UI.Viewport:onDragStart(x,y,ed,ea,change,long)
  if long then return false end
  if self.panMode==UI.Viewport.PANMODE.NONE or self.panMode==UI.Viewport.PANMODE.SCROLLBAR then return end
  if self.panMode==UI.Viewport.PANMODE.AUTO then
	  if UI.Control.isDesktopGesture() then return end
  end
  if (self.rangeh or 0)==0 and (self.rangew or 0)==0 then return end --Not scrollable
  self._dragStart={mx=x-self.cp:getX(),my=y-self.cp:getY()}
  local tx,ty=self.cp:getPosition()
  self:checkRange(tx,ty,false,true)
  return true,1 --1 is inertia amount
end

function UI.Viewport:onDrag(x,y)
  if self._dragStart and self._dragStart.mx and self._dragStart.my then
	local inertia
	if self:checkRange(x-self._dragStart.mx,y-self._dragStart.my,false,true) then
		inertia=1
	end
    return nil,inertia
  end
end

function UI.Viewport:onDragEnd(x,y)
  self._dragStart=nil
  local tx,ty=self.cp:getPosition()
  self:checkRange(tx,ty,false,true)
end

function UI.Viewport:onFocusArea(src,x,y,w,h,ax,ay)
  if src~=self then --Ensure we are dealing with a contained widget
    local vpw,vph=self:getDimensions()  
    local cpx,cpy=self.cp:getPosition()
    local xb,yb=x+w,y+h
	--CP space
    x,y=self.cp:spriteToLocal(src,x,y)
    xb,yb=self.cp:spriteToLocal(src,xb,yb)
	--Range space
    local xi,yi=x><xb,y><yb
	x=(x-vpw)<>0
	xb=(xb-vpw)<>0
	y=(y-vph)<>0
	yb=(yb-vph)<>0
    local xa,ya=x<>xb,y<>yb
	if self.content.uiAdjustScrollArea then
		xi,yi,xa,ya=self.content:uiAdjustScrollArea(xi,yi,xa,ya)
	end
	--Adjust
	local function adjust(cp,mi,ma,a)
		if ma>mi then 
			a=a or 0.5			
			if a>=0 then cp=mi+(ma-mi)*a end
		elseif cp>mi then cp=mi
		elseif cp<ma then cp=ma
		end
		return cp
	end
	cpx=adjust(-cpx,xi,xa,ax)
	cpy=adjust(-cpy,yi,ya,ay)
    --Recompute CP size now in case it hasn't been updated yet
    local t = self.cp:getLayoutInfo(vpw,vph)
    local vw,vh=t.reqWidth,t.reqHeight
    self.rangew,self.rangeh=math.max(0,vw-vpw),math.max(0,vh-vph)
    self:checkRange(-cpx,-cpy)
    return true
  end
end

UI.Viewport.Definition= {
  name="Viewport",
  icon="ui/icons/panel.png",
  class="UI.Viewport",
  constructorArgs={ "Color" },
  properties={
    { name="Color", type="color", setter=UI.Viewport.setColor, getter=UI.Viewport.getColor },
    { name="Content", type="sprite", setter=UI.Viewport.setContent },
    { name="Scrollbar", type="scrollbarMode", setter=UI.Viewport.setScrollbar },
	{ name="Panmode", type="panMode", setter=UI.Viewport.setPanmode },
  },
}


UI.Image=Core.class(UI.Panel,function (bc) return bc or "image.colTint" end )
function UI.Image:setImage(img)
  if type(img)=="string" then
	self._imageRef =img
	img=self:resolveStyle(img)
  else
	self._imageRef =nil
  end
  UI.Panel.setImage(self,img)
end

function UI.Image:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	if self._imageRef then self:setImage(self._imageRef) end
end

UI.Image.Definition= {
  name="Image",
  icon="ui/icons/panel.png",
  class="UI.Image",
  constructorArgs={ "Tint" },
  properties={
    { name="Tint", type="color", setter=UI.Panel.setColor, getter=UI.Panel.getColor },
    { name="Image", type="string", setter=UI.Image.setImage },
  },
}

UI.HilitPanel=Core.class(UI.Panel,function() return end)
function UI.HilitPanel:init()
	UI.Control.onMousePresence[self]=self
	self:setStyle("hilitpanel.styNormal")
end
function UI.HilitPanel:onMousePresence(x,y,p)
	self:setStyle(if p then "hilitpanel.styHighlight" else "hilitpanel.styNormal")
end