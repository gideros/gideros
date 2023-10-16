--!NEEDS:uiinit.lua
--[[ Sprite selection methods
spr,data=:uiSelectData(d) 	-- Returns dataSprite and data to select without onMouseClick
spr,data=:uiSelect(x,y) 	-- Returns dataSprite and data associated with click coordinates, if any
{ spr=data }=:uiSelectRange(d1,d2) --Returns sprite/data pairs in the given range
:uiSelection(selection) 	-- Process selection change
]]

UI.Selection={
	NONE=0,
	CLICK=1,
	SINGLE=2,
	MULTIPLE=3
}

local function accept(s,sel) 
	UI.dispatchEvent(s,"SelectionChange",sel)
end

-- onChange handler is called with (widget,selection). it is allowed to change selection!
UI.Selection.Set=function(s,mode,onChange)
	local uis=UI.Selection
	local ename="UISEL__"..tostring(s)
	if s._uisel_holder and s._uisel_holder.mode==uis.MULTIPLE then
		s._uisel_handlerDragStart=nil
		s._uisel_handlerDrag=nil
		s._uisel_handlerDragEnd=nil
		s._uisel_handlerPrepare=nil
		UI.Control.onDragStart[ename]=nil
		UI.Control.onDrag[ename]=nil
		UI.Control.onDragEnd[ename]=nil
		UI.Control.onLongPrepare[ename]=nil
	end
	if mode==uis.NONE then
		s._uisel_holder=nil
		s._uisel_handler=nil
		UI.Control.onMouseClick[ename]=nil
	else
		assert(s.uiSelect,"Sprite is not selectable")
		s._uisel_holder={ mode=mode, sel={}, handler=onChange or accept }
		s._uisel_handler={ target=s, handler=UI.Selection._selHandler }
		UI.Control.onMouseClick[ename]=s._uisel_handler
		if mode==uis.MULTIPLE then
			s._uisel_handlerDragStart={ target=s, handler=UI.Selection._selHandlerDragStart }
			s._uisel_handlerDrag={ target=s, handler=UI.Selection._selHandlerDrag }
			s._uisel_handlerDragEnd={ target=s, handler=UI.Selection._selHandlerDragEnd }
			s._uisel_handlerPrepare={ target=s, handler=UI.Selection._selHandlerPrepare }
			UI.Control.onDragStart[ename]=s._uisel_handlerDragStart
			UI.Control.onDrag[ename]=s._uisel_handlerDrag
			UI.Control.onDragEnd[ename]=s._uisel_handlerDragEnd
			UI.Control.onLongPrepare[ename]=s._uisel_handlerPrepare
			s._uisel_holder.prepare=UI.Behavior.LongClick.makeIndicator(s,{}) 
		end
	end
end

local function uiUnselectAll(s)
	local sh=s._uisel_holder
	local uis=UI.Selection
	if sh.mode==uis.CLICK then
	else
		sh.sel={}
		s:uiSelection(sh.sel)
		sh.handler(s,sh.sel)
	end
end

local function uiUpdateSelection(s,spr,data,action)
	local selpoint=spr
	if s then
		local sh=s._uisel_holder
		local uis=UI.Selection
		if sh.mode==uis.CLICK and spr then
			sh.handler(s,data,spr)
		elseif sh.mode==uis.SINGLE and spr then
			sh.sel={}
			sh.sel[spr]=data
			s:uiSelection(sh.sel)
			sh.handler(s,sh.sel)
		else
			if action=="click" and spr then
				local mods=UI.Control.Meta.modifiers or 0
				local ctrl=(mods&KeyCode.MODIFIER_CTRL)>0
				local shift=(mods&KeyCode.MODIFIER_SHIFT)>0
				if shift and sh.selPoint and sh.sel[sh.selPoint] then
					selpoint=sh.selPoint --Keep
					local sdata=sh.sel[sh.selPoint]
					if not ctrl then sh.sel={} end
					local range=s:uiSelectRange(sdata,data)
					for ks,kd in pairs(range) do
						sh.sel[ks]=kd
					end
				elseif ctrl then
					if sh.sel[spr] then
						sh.sel[spr]=nil
					else
						sh.sel[spr]=data
					end
				else
					if sh.sel[spr] then
						sh.sel={}
					else
						sh.sel={ [spr]=data }
					end
				end
				sh.range=nil
			elseif action=="rstart" and s.uiSelectRange and spr then
				sh.range={ s=data,e=data, unmark=sh.sel[spr], lsel=table.clone(sh.sel) }
			elseif action=="rmove" and sh.range and spr then
				sh.range.e=data
			elseif action=="rend" then
				sh.range=nil
			end
			if sh.range and sh.range.s and sh.range.e then
				local range=s:uiSelectRange(sh.range.s,sh.range.e)
				sh.sel=table.clone(sh.range.lsel)
				for ks,kd in pairs(range) do
					if sh.range.unmark then
						sh.sel[ks]=nil
					else
						sh.sel[ks]=kd
					end
				end
			end
			s:uiSelection(sh.sel)
			sh.handler(s,sh.sel)
		end
	end
	if s then s._uisel_holder.selPoint=selpoint end
end

UI.Selection._selSelectData=function(s,data) --without onMouseClick --force row selection
	local spr,data=s:uiSelectData(data) --spr,data
	if spr then 
		uiUpdateSelection(s,spr,data)
	else
		uiUnselectAll(s)
	end
end

function UI.Selection.select(s,dataList) --Software selection
	if dataList==nil then return end
	local sh=s._uisel_holder
	local uis=UI.Selection
	if sh==nil then return end
	sh.sel={}
	sh.selPoint=nil
	if sh.mode==uis.SINGLE then
		local data=dataList[1]
		if data~=nil then
			local spr,data=s:uiSelectData(data) --spr,data
			if spr then	sh.sel[spr]=data end
		end
	else
		for _,data in ipairs(dataList) do
			if data~=nil then
				local spr,data=s:uiSelectData(data) --spr,data
				if spr then	sh.sel[spr]=data end
			end
		end
	end
	s:uiSelection(sh.sel)
end

function UI.Selection.handleKeyEvent(s,kc,rc)
	local sh=s._uisel_holder
	if not sh then return end
	local uis=UI.Selection
	if sh.mode==uis.MULTIPLE or sh.mode==uis.SINGLE then
		local modifiers=UI.Control.Meta.modifiers or 0
		local ctrl=((modifiers or 0)&15)==KeyCode.MODIFIER_CTRL
		local meta=((modifiers or 0)&15)==KeyCode.MODIFIER_META
		ctrl=ctrl or meta -- for MAC
--		local shift=(modifiers&KeyCode.MODIFIER_SHIFT)>0
		if sh.mode==uis.MULTIPLE then
			if kc==KeyCode.A and ctrl and s.uiSelectAll then
				sh.sel=table.clone(s:uiSelectAll())
				s:uiSelection(sh.sel)
				sh.handler(s,sh.sel)
				return true
			end
		end
		--All selection modes
		if s.uiSelectDirection then
			local msel_s,msel_d=next(sh.sel)
			if msel_s and next(sh.sel,msel_s) and sh.selPoint and sh.sel[sh.selPoint]then
				msel_s=sh.selPoint
				msel_d=sh.sel[sh.selPoint]
			end
			if msel_d then
				local dx,dy
				if kc==KeyCode.UP then dx=0 dy=-1
				elseif kc==KeyCode.DOWN then dx=0 dy=1
				elseif kc==KeyCode.LEFT then dx=-1 dy=0
				elseif kc==KeyCode.RIGHT then dx=1 dy=0
				end
				local nsel=s:uiSelectDirection(msel_d,dx,dy)
				if nsel and next(nsel) then
					local spr=next(nsel)
					if ctrl then
					else
						sh.sel=nsel
						s:uiSelection(sh.sel)
						sh.handler(s,sh.sel)
						UI.Focus:area(spr,0,0,spr:getWidth(),spr:getHeight(),true,-1,-1)
					end
				end
			end
		end
	end
end

function UI.Selection._selHandler(s,x,y,c)
	local spr,data=s:uiSelect(x,y)
	if spr then 
		UI.Focus:request(s)
		uiUpdateSelection(s,spr,data,if c and c>=2 then "doubleclick" else "click")
	else
	end
end

function UI.Selection._selHandlerDragStart(s,x,y,d,a,change,long)
	if not long then return end
	local spr,data=s:uiSelect(x,y)
	if spr then 
		s._uisel_holder.dragging=true
		uiUpdateSelection(s,spr,data,"rstart") 
		return true
	end
end
function UI.Selection._selHandlerDrag(s,x,y)
	if not s._uisel_holder.dragging then return end
	local spr,data=s:uiSelect(x,y)
	if spr then 
		UI.Focus:area(spr,0,0,spr:getWidth(),spr:getHeight(),true,-1,-1)
		UI.Focus:request(s)
		uiUpdateSelection(s,spr,data,"rmove")
	end
end
function UI.Selection._selHandlerDragEnd(s,x,y)
	if not s._uisel_holder.dragging then return end
	local spr,data=s:uiSelect(x,y)
	UI.Focus:request(s)
	uiUpdateSelection(s,spr,data,"rend")
	s._uisel_holder.dragging=false
end
function UI.Selection._selHandlerPrepare(s,x,y,ratio)
	s._uisel_holder.prepare:indicate(s,x,y,ratio)
	return true
end
