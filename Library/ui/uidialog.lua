--!NEEDS:uipanel.lua
--!NEEDS:uistyle.lua

UI.Dialog=Core.class(UI.Panel,function () return "dialog.colBackground" end)
UI.Dialog.Template={
	class="UI.Panel",
	Border="dialog.border",
}

function UI.Dialog:init()
	UI.BuilderSelf(UI.Dialog.Template,self)
end

UI.Dialog.Definition= {
  name="Dialog",
  icon="ui/icons/panel.png",
  class="UI.Dialog",
  constructorArgs={ },
  properties={
  },
}

local ConfirmTemplate={
	class="UI.Dialog",
	layout={fill=Sprite.LAYOUT_FILL_HORIZONTAL},
	layoutModel={ rowWeights={1,0},columnWeights={1,1},cellSpacingX=10,cellSpacingY=5,equalizeCells=true },
	children={
		{ class="UI.Label", name="lbText", layout={gridwidth=2,fill=Sprite.LAYOUT_FILL_BOTH},TextLayout={flags=FontBase.TLF_VCENTER|FontBase.TLF_REF_LINETOP|FontBase.TLF_BREAKWORDS}},
		{ class="UI.Button", name="btNo", layout={gridy=1,fill=Sprite.LAYOUT_FILL_BOTH},},
		{ class="UI.Button", name="btYes", layout={gridy=1,gridx=1,fill=Sprite.LAYOUT_FILL_BOTH},},
	}
}

function UI.Dialog.confirm(text,yes,no,params)
	local dialog=UI.Builder(ConfirmTemplate,nil,nil,params)
	dialog.lbText:setText(text)
	dialog.btYes:setText(yes)
	dialog.btNo:setText(no)
	dialog.onWidgetAction=function(self,w)
		UI.dispatchEvent(self,"DialogClosed",w==self.btYes)
	end
	return dialog
end

local MessageTemplate={
	class="UI.Dialog",
	layout={fill=Sprite.LAYOUT_FILL_HORIZONTAL},
	layoutModel={ rowWeights={1,0},columnWeights={1},cellSpacingX=10,cellSpacingY=5,equalizeCells=true},
	children={
		{ class="UI.Label", name="lbText", layout={fill=1},TextLayout={flags=FontBase.TLF_VCENTER|FontBase.TLF_REF_LINETOP|FontBase.TLF_BREAKWORDS}},
		{ class="UI.Button", name="btOk", layout={gridy=1,fill=1},},
	}
}

function UI.Dialog.message(text,ok,params)
	local dialog=UI.Builder(MessageTemplate,nil,nil,params)
	dialog.lbText:setText(text)
	dialog.btOk:setText(ok)
	dialog.onWidgetAction=function(self,w)
		UI.dispatchEvent(self,"DialogClosed")
	end
	return dialog
end

local InputTemplate={
	class="UI.Dialog",
	layout={fill=Sprite.LAYOUT_FILL_HORIZONTAL},
	layoutModel={ rowWeights={1,0,0},columnWeights={1,1},cellSpacingX=10,cellSpacingY=5,equalizeCells=true},
	children={
		{ class="UI.Label", 	name="lbText", 	layout={gridwidth=2,fill=1},TextLayout={flags=FontBase.TLF_VCENTER|FontBase.TLF_REF_LINETOP|FontBase.TLF_BREAKWORDS}},
		{ class="UI.TextField", name="lbInput", layout={gridwidth=2,gridy=1,fill=1},TextLayout={flags=FontBase.TLF_VCENTER|FontBase.TLF_REF_LINETOP|FontBase.TLF_NOWRAP}},
		{ class="UI.Button",	name="btNo", 	layout={gridy=2,fill=1},},
		{ class="UI.Button", 	name="btYes", 	layout={gridy=2,gridx=1,fill=1},},
	}
}

function UI.Dialog.input(text,yes,no,default,params)
	local dialog=UI.Builder(InputTemplate,nil,nil,params)
	dialog.lbText:setText(text)
	dialog.btYes:setText(yes)
	dialog.btNo:setText(no)
	dialog.lbInput:setText(default)
	dialog.onWidgetAction=function(self,w)
		UI.dispatchEvent(self,"DialogClosed",w==self.btYes and self.lbInput:getText())
	end
	return dialog
end

local BoxTemplate={
	class="UI.Dialog",
	layout={fill=Sprite.LAYOUT_FILL_HORIZONTAL},
	layoutModel={ rowWeights={1,0},columnWeights={1,1},cellSpacingX=10,cellSpacingY=5,equalizeCells=true},
	children={
		{ class="UI.Panel",  name="Box", --!!index=1
		 layoutModel = { columnWeights={1}, rowWeights={1}, columnWidths={0}, rowHeights={0} },
		 layout={gridx=0,gridy=0,gridwidth=2,fill=1},
		 children={},
		}, 
		{ class="UI.Button", name="btNo", layout={gridy=1,fill=1},},
		{ class="UI.Button", name="btYes", layout={gridy=1,gridx=1,fill=1},},
	}
}

function UI.Dialog.box(children,yes,no,params) --children { { class=UI.? },{ class=UI.? },} optional
	BoxTemplate.children[1].children=children or {} --!!index=1
	local dialog=UI.Builder(BoxTemplate,nil,nil,params)
	dialog.btYes:setText(yes)
	dialog.btNo:setText(no)
	dialog.onWidgetAction=function(self,w)
		UI.dispatchEvent(self,"DialogClosed",w==self.btYes)
	end
	return dialog
end

UI.Screen=Core.class(UI.Panel)
function UI.Screen:init()
	self.__UIisScreen=true
	self:setLayoutParameters{ columnWeights={1}, rowWeights={1}}
	
	self.safe=UI.Panel.new()
	self:addChild(self.safe)
	self.safe:setLayoutParameters{ columnWeights={1}, rowWeights={1}}

	self.glass=UI.Panel.new()
	self:addChild(self.glass)
	self.glass:setLayoutConstraints{ fill=Sprite.LAYOUT_FILL_BOTH}
	--self.glass:setLayoutParameters{ columnWeights={1}, rowWeights={1} }

	self.dialogs=UI.Panel.new()
	self:addChild(self.dialogs)
	self.dialogs:setLayoutParameters{ columnWeights={1}, rowWeights={1}}
	self.dialogs:setLayoutConstraints{ fill=Sprite.LAYOUT_FILL_BOTH}

	self.dnd=UI.Panel.new()
	self:addChild(self.dnd)
	self.dnd:setLayoutConstraints{ fill=Sprite.LAYOUT_FILL_BOTH}
	self.dnd:setLayoutParameters{ columnWeights={1}, rowWeights={1} }

	stage:addEventListener(Event.APPLICATION_RESIZE,self.resized,self)
	stage:addChild(self)
	self:resized()
	
	self._flags.focusgroup=true
	UI.Control.onMouseClick[self]=self
	UI.Control.onLingerStart[self]=self
	UI.Control.onLingerEnd[self]=self
end
function UI.Screen:newClone() assert(false,"Cloning not supported") end

function UI.Screen:onMouseClick()
	UI.Focus:clear()
end

function UI.Screen:onLingerStart(x,y)
	local top,tip,sites=UI.ToolTip.lookupAt(self,x,y)
	--Process tooltip
	if top then
		local marker = UI.ToolTip.buildMarker(top,tip,true)
		self._ToolTip_Marker=UI.ToolTip.show(self,marker,sites or UI.ToolTip.placementSites(self,top,x,y))
		return true
	end
end

function UI.Screen:onLingerEnd()
	if self._ToolTip_Marker then
		UI.ToolTip.dismiss(self,self._ToolTip_Marker)
		self._ToolTip_Marker:destroy()
		self._ToolTip_Marker=nil
	end
end

function UI.Screen:resized()
	local sx,sy,sw,sh=application:getLogicalBounds()
	self:setDimensions(sw-sx,sh-sy) self:setPosition(sx,sy)
	local tsx,tsy,tsw,tsh=application:getDeviceSafeArea(true)
	self.safe:setLayoutConstraints{ fill=Sprite.LAYOUT_FILL_BOTH, insetBottom=sh-tsh,insetLeft=tsx-sx,insetRight=sw-tsw,insetTop=tsy-sy}
end

function UI.Screen:fullscreen(s) if s then self:addChildAt(s,1) end return self end
function UI.Screen:ui(s) if s then self.safe:addChild(s) end return self.safe end
function UI.Screen:dialog(s) if s then self.dialogs:addChild(s) end return self.dialogs end

function UI.Screen.getScreen(w)
	while w do
		if w.__UIisScreen then return w end
		w=w:getParent()
	end
end

function UI.Screen.getScreenUI(w)
	local s=UI.Screen.getScreen(w)
	if s then return s.safe end
end

function UI.Screen:fitToGlass(s,w,x,y,sx,sy,dx,dy,mvtx,mvty)
	local function fit(a,sa,ea,ma,ua) --Initial pos, Span, Dir/extent, Move, Upper Limit
		local xa=ua-a
		if a<0 then
			if ma then --movable
				if sa<ua then
					--Move
					a=0
				else
					--Restrict
					a=0
					sa=ua
				end
			else --not movable
				sa+=a
				a=0
			end
		elseif sa>xa then 
			if ma then --movable
				if sa<ua then
					--Move
					a-=(sa-xa)
				else
					--Restrict
					a=0
					sa=ua
				end
			else --not movable
				sa=xa
			end
		end
		return a,sa
	end
	--Transform origin
	x,y=s:localToGlobal(x,y)
	x,y=self.glass:globalToLocal(x,y)
	-- Initial placement
	x+=sx*(dx-1)/2
	y+=sy*(dy-1)/2
	-- Fit to available space
	local gw,gh=self:getDimensions()
	x,sx=fit(x,sx,dx,mvtx,gw)
	y,sy=fit(y,sy,dy,mvty,gh)
	return self.glass,x,y,sx,sy
end

function UI.Screen.addToScreenGlass(origin,w,x,y,sx,sy,dx,dy,mvtx,mvty)
	local s=UI.Screen.getScreen(origin)
	assert(s,"No UI.Screen found for widget")
	local g,x,y,sx,sy=s:fitToGlass(origin,w,x,y,sx,sy,dx,dy,mvtx,mvty)
	-- Set bounds and show
	w:setPosition(x,y)
	w:setDimensions(sx,sy)
	g:addChild(w)
end

--[[Each site is a table containg:
- x,y: the origin of the pop up window
- w,h: optional minimal width or height
- dx,dy: the direction in which it opens/spans
- mvtx,mvty: booleans  that indicates wether the popup can move along each axis to fit the available size
]]
function UI.Screen.popupAt(origin,w,sites)
  local s=UI.Screen.getScreen(origin)
  assert(s,"No UI.Screen found for widget")
  
  --APPLY STYLE: TODO GIDEROS should have a call to do this
  s:addChild(w)
  if application.__styleUpdates then
	for sp,_ in pairs(application.__styleUpdates) do
		sp:updateStyle()
	end
  end
  
  --First query min size and apply it
  local t = w:getLayoutInfo(0,0)
  w:setDimensions(t.reqWidth,t.reqHeight)
  --Re-query and adjust in case reqheight have changed
  t = w:getLayoutInfo(0,0)
  local vw,vh=t.reqWidth,t.reqHeight 
  
  local best
  local function showat(g,x,y,sx,sy)
	w:setPosition(x,y)
	w:setDimensions(sx,sy)
	g:addChild(w)
  end
  for k,v in ipairs(sites) do
	local iw=vw<>(v.w or 0)
	local ih=vh<>(v.h or 0)
	local cg,cx,cy,cw,ch=s:fitToGlass(origin,w,v.x,v.y,iw,ih,v.dx,v.dy,v.mvtx,v.mvty)
	if cw<vw or ch<vh then
		local ca=cw*ch
		if not best or best.a<ca then
			best={g=cg,x=cx,y=cy,w=cw,h=ch,a=ca,k=k}
		end
	else
		showat(cg,cx,cy,cw,ch)
		return k
	end	
  end
  if best then
	showat(best.g,best.x,best.y,best.w,best.h)
	return best.k
  end
end

UI.Screen.Definition= {
  name="Screen",
  icon="ui/icons/panel.png",
  class="UI.Screen",
  constructorArgs={ "Color" },
  properties={
    { name="Color", type="color", setter=UI.Panel.setColor, getter=UI.Panel.getColor },
    { name="Image", type="image", setter=UI.Panel.setImage },
    { name="Border", type="border", setter=UI.Panel.setBorder, getter=UI.Panel.getBorder },
    { name="Fullscreen", type="sprite", setter=UI.Screen.fullscreen },
    { name="Content", type="sprite", setter=UI.Screen.ui },
  },
}

UI.ModalScreen=Core.class(UI.Screen,function(color,insets) return color end)
UI.ModalScreen.Template={
	class="UI.Screen",
	layout={ fill=Sprite.LAYOUT_FILL_BOTH },
}

function UI.ModalScreen:init(color,insets)
	UI.BuilderSelf(UI.ModalScreen.Template,self)
	if insets then
		local lm=self.safe:getLayoutParameters()
		lm.insetLeft=insets
		lm.insetRight=insets
		lm.insetTop=insets
		lm.insetBottom=insets
		self.safe:setLayoutParameters(lm)
	end
	UI.Control.stopPropagation[self]=self
end

function UI.ModalScreen:destroy()
	UI.Control.stopPropagation[self]=nil
	self:removeFromParent()
end

function UI.ModalScreen.showDialog(screen,dialog,cb,color,insets)
	local ms=UI.ModalScreen.new(color,insets)
	screen:addChild(ms)
	ms.safe:addChild(dialog)
	ms.onDialogClosed=function (self,src,res)
		local function cleanup()
			ms:destroy()
		end
		local c=false
		if cb then
			c=cb(res,dialog,cleanup)
		end
		if not c then cleanup() end
	end
	return ms
end
