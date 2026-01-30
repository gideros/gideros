--!NEEDS:uiinit.lua

--[[ Example layout:
local ui=UI.Builder{
		class="UI.Panel", Color=#0x00c0c0c0, 
		Border=UI.Border.Basic.new{ thickness=3, radius=10, bgColor=0xc0c0c0,fgColor=0xFFFFFF },
		layoutModel={},
		children={
			{ 	class="UI.Panel", Color=#0,
				layoutModel=UI.Layout.Vertical,
				children={
					{ class="UI.Label", Text="Login:" },
					{ class="UI.TextField", name="tfLogin" },
					{ class="UI.Label", Text="Password:" },
					{ class="UI.TextField", name="tfPass", Password="•" },
					{ class="UI.Label", Text="Server:" },
					{ class="UI.TextField", name="tfServer" },
					{ class="UI.Label", 
						name="tfError", Text="", 
						--TextLayout={flags=FontBase.TLF_CENTER|FontBase.TLF_VCENTER|FontBase.TLF_REF_LINETOP|FontBase.TLF_BREAKWORDS}, 
						--Style={ font=TTFont.new("ui/icons/NimbusSanL-Bol.otf",10,"") }, 
						--Color=0xFF4040,
					},
					{ class="UI.Panel", Color=#0, layoutModel={}, 
						children={
							{ class="UI.Panel", Color=#0, layoutModel={},layout={}, 
								Border=UI.Border.Basic.new{ radius=3, bgColor=0x8080FF}, name="tfConnect", 
								children={
									{ class="UI.Label", Text="Connect" }
								}
							},
						}
					},
				}
			}
		}
	}
]]

--[[Special fields:
- selection: specify selection mode and/or handler
- data and dataMapper: specify the initial data set and associated mapper
- expanded: specify the initially expanded item from the data set (if any)
- class: the class or class name to instanciate
- factory: the factory to use for instanciating (if no class)
- generator(containerSprite,containerDesc,generatorDesc): a function that will create actual sub elements in a child list
- model(desc): a function that is called early in the building process, to apply common settings to this block
]]

local function getglobal(s)
	local g=_G
	for p in s:gmatch("([^.]+)") do assert(g,"Couldn't find "..s) g=g[p] end
	return g
end

local uibuilder_context

local function applyLayout(c,cd,layout)
	local li=layout and c:getLayoutConstraints()
	local cdl=cd.layout
	if cdl then
		li=table.clone(cdl,li)
	end
	if layout then
		li=li or {}
		layout(li,cdl or {})
	end

	if li then c:setLayoutConstraints(li) end
end

local CLASS_MAP={}
local function resolveClass(cname,params)
	local class=cname
	if params and params.classMap then
		class=params.classMap[class] or class
	end
	if type(class)=="string" then
		local rclass=CLASS_MAP[class] or getglobal(class)
		CLASS_MAP[class]=rclass
		class=rclass
	end
	return class
end

function UI.BuilderContext() return uibuilder_context end 
function UI.Builder(d,top,ref,params,sub)
	local hasFactory
	local oldContext=uibuilder_context or {}
	params=params or oldContext.params
	if d.localParams then params=d.localParams(params) end
	uibuilder_context={top=top or oldContext.top,params=params}
	while type(d.model)=="function" do
		local df=d.model
		d=df(d,top,ref,params) or d 
		if d.model==df then break end
	end
	assert(d.class or d.factory,"Each widget descriptor must have a class or a factory specification")
	local s,cdef,psets
	if type(top)=="table" and d.name and top[d.name] then
		s=top[d.name]
		s:setLayoutConstraints(nil)
		local rc={}
		for i=1,s:getNumChildren() do
			local rcc=s:getChildAt(i)
			if rcc._ui_built_==(ref or d) then rc[#rc+1]=rcc end
		end
		for _,c in ipairs(rc) do c:removeFromParent() end
		if d.class then
			local class=resolveClass(d.class,uibuilder_context.params)
			cdef=class.Definition
			local cargs={ }
			local nargs=0
			psets={}
		end
	elseif d.class then
		local class=resolveClass(d.class,uibuilder_context.params)
		assert(class,"Class not supplied, not found or nil")
		cdef=class.Definition
		local cargs={ }
		local nargs=0
		psets={}
		if cdef then
			if cdef.constructorArgs then
				for k,an in ipairs(cdef.constructorArgs) do
					if an=="*" then
						cargs[k]=d					
					else
						cargs[k]=d[an]
						psets[an]=true
					end
				end
				nargs=#cdef.constructorArgs
			end
		end
		s=class.new(unpack(cargs,1,nargs))
	elseif d.factory then
		hasFactory=true
		if type(d.factory)=="function" then
			s = d.factory(d.data or d,params)
		else
			local factory=d.factory
			if type(factory)=="string" then
				factory=getglobal(d.factory)
			end
			s=factory:build(d.data or d)
		end
	end
	assert(s and s.getLayoutParameters,"Constructed instance is not a sprite")
	top=uibuilder_context.top or s
	if top==true then top=s end
	uibuilder_context.top=top
	--print(d.class,s,json.encode(cargs),s:getWidth(),s:getHeight())
	if cdef then
		local ccdef=cdef
		while ccdef do
			for _,p in ipairs(ccdef.properties) do
				local an=p.name
				if (d[an]~=nil) and not psets[an] then
					if p.type=="sprite" and not d[an].hitTestPoint then
						local c,hf=UI.Builder(d[an],top)
						hasFactory=hasFactory or hf
						local cd=d[an]
						applyLayout(c,cd);
						(p.setter or s["set"..p.name])(s,c)
					else
						(p.setter or s["set"..p.name])(s,d[an])
					end
					psets[an]=true
				end		
			end
			if ccdef~=UI.Panel.Definition then
				ccdef=(ccdef.super or UI.Panel).Definition
			else
				ccdef=nil
			end
		end
	end
	local layoutParams
	local layout=nil
	if d.layoutModel then 
		if type(d.layoutModel)=="function" then
			layoutParams={}
			layout=d.layoutModel(s,layoutParams) 
		else
			layoutParams=d.layoutModel
			if type(layoutParams.handler)=="function" then
				layout=layoutParams.handler(s,layoutParams)
			end
		end	
	end
	if d.children then
		for _,cd in ipairs(d.children) do
			if next(cd) then --Empty tables are placeholder, don't build anything
				local bset ={ }
				local function generate(cd,regen)
					if type(cd.generator)=="function" then
						for _,ss in ipairs(cd.generator(s,d,cd)) do
							regen(ss,regen)
						end
					else
						bset[#bset+1]=cd
					end
				end
				generate(cd,generate)
				for _,cd in ipairs(bset) do
					local c,hf=UI.Builder(cd,top,nil,params,true)
					hasFactory=hasFactory or hf
					c._ui_built_=(ref or d)
					applyLayout(c,cd,layout)
					s:addChild(c)
				end
			end
		end
	end
	if d.name then top[d.name]=s s.name=d.name end
	if d.selection then
		if type(d.selection)=="function" then
			UI.Selection.enable(s,UI.Selection.CLICK,d.selection)
		elseif type(d.selection)=="table" then
			UI.Selection.enable(s,d.selection.mode,d.selection.handler)
		else
			assert(type(d.selection)=="number","selection info must be a table, a function or a selection mode")
			UI.Selection.enable(s,d.selection,nil)
		end
	end
	if d.behavior then
		d.behavior.new(s,d.behaviorParams)
	end
	local border=s.getBorder and s:getBorder()
	if border and border.insets then
		layoutParams=layoutParams or s:getLayoutParameters() or {}
		local bi=border.insets
		if type(bi)=="table" then
			layoutParams.insetTop=(s:resolveStyle(layoutParams.insetTop) or 0)<>s:resolveStyle(bi.top)
			layoutParams.insetBottom=(s:resolveStyle(layoutParams.insetBottom) or 0)<>s:resolveStyle(bi.bottom)
			layoutParams.insetLeft=(s:resolveStyle(layoutParams.insetLeft) or 0)<>s:resolveStyle(bi.left)
			layoutParams.insetRight=(s:resolveStyle(layoutParams.insetRight) or 0)<>s:resolveStyle(bi.right)
		else
			bi=s:resolveStyle(bi)
			layoutParams.insetTop=(s:resolveStyle(layoutParams.insetTop) or 0)<>bi
			layoutParams.insetBottom=(s:resolveStyle(layoutParams.insetBottom) or 0)<>bi
			layoutParams.insetLeft=(s:resolveStyle(layoutParams.insetLeft) or 0)<>bi
			layoutParams.insetRight=(s:resolveStyle(layoutParams.insetRight) or 0)<>bi
		end
	end
	if layoutParams then 
		s:setLayoutParameters(layoutParams)
	end
	if not sub then
		applyLayout(s,d)
	end
	if d.visible~=nil then s:setVisible(d.visible) end
	--Set data as last step
	if d.data and s.setData then
		s:setData(d.data,d.dataMapper)
		if d.expanded and s.setExpanded then print("UI.Accordion: d.expanded is deprecated") s:setExpanded(d.data[d.expanded],true) end
	end
	uibuilder_context=oldContext	
	return s,hasFactory
end

function UI.BuilderSelf(template,top,ref,params)
	template.name="__builder_tmp_name"
	top.__builder_tmp_name=top
	UI.Builder(template,top,ref,params)
	template.name=nil
	top.__builder_tmp_name=nil
end

UI.Factory=Core.class(Object)

function UI.Factory:init(template,customizer,initializer)
	self.template=template
	self.customizer=customizer
	self.initializer=initializer
	function self.new(data) return self:build(data) end
end

function UI.Factory:build(data)
	local b = nil
	if Sprite.clone then
		if not self._Template then
			local tpf
			b,tpf=UI.Builder(self.template,true)
			if self.initializer then self.initializer(b) end
			if not tpf then 
				self._Template=b
				b=nil
			end
		end
		b=b or self._Template:clone()
	else
		b=UI.Builder(self.template,true)
		if self.initializer then self.initializer(b) end
	end
	b.setData=self.customizer
	if data then b:setData(data) end
	return b
end

function UI.Factory:builder(data)
	return function(d) return self:build(data or d) end
end

UI.Factory.Definition= {
	constructorArgs={ "*" },
	properties={},
}

UI.OrientableLayout=Core.class(Object)
function UI.OrientableLayout:init(parent,layouts,ui)
	self.layouts=layouts
	self.parent=parent
	self.current=nil
	self.ui=ui
	if ui then ui.root=ui end 
	--Setup
	for k,v in pairs(self.layouts) do
		self.ui=UI.Builder(v.l,self.ui,self)
		local li=self.ui:getLayoutInfo(10000,10000,1)
		v.rw=li.reqWidth
		v.rh=li.reqHeight
	end
	self:build()
	self.parent:addEventListener(Event.LAYOUT_RESIZED,self.build,self)
end
function UI.OrientableLayout:build()
	self.parent:getLayoutInfo()
	local w,h=self.parent:getSize()
	local t=self.layouts[next(self.layouts)].l
	local tp=0
	local r=w/h
	for k,v in pairs(self.layouts) do
		local dw,dh=w-v.rw,h-v.rh
		local pri=v.priority or 1
		local rmin=v.rmin or 0
		local rmax=v.rmax or 1000
		if r<rmin or r>rmax then pri=0 end
		if dw<0 or dh<0 then pri=0 end
		if pri>tp then tp=pri t=v.l end
	end
	--if w>h then t=self.landscape end
	if self.current~=t then
		self.current=t
		self.ui=UI.Builder(t,self.ui,self)
	end
end
function UI.OrientableLayout:get()
	return self.ui
end
