--!NEEDS:uiinit.lua
--[[
Interpret controls and trigger the following events:
* onMouseDown, onMouseUp, onMouseMove, onMouseWheel (RAW events)
* onMouseClick(click count) (following onMouseUp)
* onLongPrepare(ratio) (following onMouseDown)
* onLongDown(longSteps) (following onMouseDown)
* onLongClick(longSteps) (following onMouseUp)
* onDragStart(x,y,dist,angle,changed,long,touchCount) (following onMouseDown)
* onDrag(x,y,dist,angle,touchCount)  (following onMouseMove)
* onDragEnd()  (following onMouseUp)
* onMousePresence(x,y,enter)
* onLingerStart(sprite,x,y), onLingerEnd(x,y)
]]

UI.Control={
 onMouseDown={},
 onMouseUp={},
 onMouseMove={},
 onMouseClick={},
 onMouseWheel={},
 onMousePresence={},
 onLongDown={},
 onLongPrepare={},
 onLongClick={},
 onDragStart={},
 onDrag={},
 onDragEnd={},
 onEnterFrame={},
 onLingerStart={},
 onLingerEnd={},
 onKeyDown={},
 onKeyUp={},
 onKeyChar={},
 stopPropagation={},
 isRawTouch={},
 Meta={}, --mouseButton, modifiers, deviceType
 HasPresence=false, --set to true if presence capable device is found (mouse)
 DOUBLECLICK_THRESHOLD=0.1, --s
 LONGCLICK_START=0.3, --s before indicating a long click is preparing
 LONGCLICK_THRESHOLD=1, --s before announcing a long down
 LINGER_THRESHOLD=1, --s after mouse stop to trigger linger event
 DRAG_THRESHOLD=20, --px to assume a drag and cancel potential long click
 WHEEL_DISTANCE=20, --px to scroll for a wheel tick
 REPEAT_TIMER=0.5, --s key repetition trigger
 REPEAT_PERIOD=0.1, --s key repetition period
 RIGHTCLICK_IS_LONG=true,
 HAS_CURSOR=true,
 ENABLE_DESKTOP_GESTURES=false, --Set to true to enable mouse specific behavior (not touch)
}
local _weak={ __mode = "kv"}
setmetatable(UI.Control.onMouseDown,_weak)
setmetatable(UI.Control.onMouseUp,_weak)
setmetatable(UI.Control.onMouseMove,_weak)
setmetatable(UI.Control.onMouseClick,_weak)
setmetatable(UI.Control.onMouseWheel,_weak)
setmetatable(UI.Control.onMousePresence,_weak)
setmetatable(UI.Control.onLongDown,_weak)
setmetatable(UI.Control.onLongPrepare,_weak)
setmetatable(UI.Control.onLongClick,_weak)
setmetatable(UI.Control.onDragStart,_weak)
setmetatable(UI.Control.onDrag,_weak)
setmetatable(UI.Control.onDragEnd,_weak)
setmetatable(UI.Control.onEnterFrame,_weak)
setmetatable(UI.Control.onLingerStart,_weak)
setmetatable(UI.Control.onLingerEnd,_weak)
setmetatable(UI.Control.onKeyDown,_weak)
setmetatable(UI.Control.onKeyUp,_weak)
setmetatable(UI.Control.onKeyChar,_weak)
setmetatable(UI.Control.stopPropagation,_weak)
setmetatable(UI.Control.isRawTouch,_weak)

local Contexts={}
local function declareContext(plane)
	Contexts[plane or stage]={ cp_inertia={}, ccount=0, surface=plane, ctouch={} }
end
declareContext()

if not Sprite.spriteToLocal then
	Sprite._hitTestPoint=Sprite.hitTestPoint
	function Sprite:hitTestPoint(x,y,shape,ref)
		if ref then 
			x,y=ref:localToGlobal(x,y)
		end
		return self:_hitTestPoint(x,y,shape)		
	end
end

local debugClick = _DEBUG_CLICK
if debugClick then print("UI.Control debugClick !!!!!!!!!!!!!!!!!!!!!!!!!!") end

local _curCursor
local function updateCursor()
	if not UI.Control.HAS_CURSOR then return end
	local c=UI.Control.LocalCursor or UI.Control.GlobalCursor or "arrow"
	if c~=_curCursor then
		application:set("cursor",c)
		_curCursor=c
	end
end

-- Not position related
local function ndispatch(event,...)
	for k,v in pairs(UI.Control[event]) do
		local s=nil
		if v and type(v)=="table" then
			if v[event] then
				s=v
				v=s[event]
			else
				v=v.handler
				s=v.target
			end
		end
		if v and type(v)=="function" then
			if s then v(s,...) else v(...) end
		end
	end
end

-- Focus related
local function fdispatch(event,...)
	local f=UI.Focus:get()
	while f do
		local s=UI.Control[event][f]
		if f==s then
			local v=s[event]
			if v and type(v)=="function" then
				if v(s,...) then return end
			end
			if UI.Control.stopPropagation[f] then return end			
		end
		f=f:getParent()
	end
end

local function processEnterLeave(ctx,l,x,y)
	if not ctx.presenceList then ctx.presenceList={} end
	local enter={}
	local kept={}
	for _,v in ipairs(l) do
		if v.target then
			kept[v.target]=true
			if not ctx.presenceList[v.target] then
				enter[v.target]=true
			else
				ctx.presenceList[v.target]=nil
			end
		end
	end
	for v,_ in pairs(ctx.presenceList) do
		local lx,ly	if ctx.surface then lx,ly=v:spriteToLocal(ctx.surface,x,y) else lx,ly=v:globalToLocal(x,y) end
		if v.onMousePresence then v:onMousePresence(lx,ly,false) end
	end
	for v,_ in pairs(enter) do
		local lx,ly	if ctx.surface then lx,ly=v:spriteToLocal(ctx.surface,x,y) else lx,ly=v:globalToLocal(x,y) end
		if v.onMousePresence then v:onMousePresence(lx,ly,true) end
	end
	ctx.presenceList=kept
end

-- Position related
local function idispatch(ctx,event,cps,x,y,...)
	if debugClick then print("uicontrol","idispatch","event",event,"CPS:",cps,"X/Y:"..x.."/"..y) end
	local t={}
	local spriteStack
	local sps
	UI.Control.CurrentContext=ctx
	for k,v in pairs(UI.Control[event]) do
		if v and type(v)=="table" then
			local h=nil
			if not v.hitTestPoint and v.handler then
				h=v.handler
				v=v.target
			end
			if not spriteStack then --populate spriteStack
				sps=(ctx.surface or stage):getChildrenAtPoint(x,y,true,false,ctx.surface)
				spriteStack={}
				for kk,vv in ipairs(sps) do spriteStack[vv]=kk end
			end
			if getmetatable(v) and v.hitTestPoint and (cps==v or ((cps==nil) and x and y and spriteStack[v])) then -- Sprite or Instance
				if cps then
					table.insert(t,{handler=h, target=v, _parents=1000})
				else
					if spriteStack[v] then
						if debugClick then print("uicontrol","idispatch","name",v.name,"parents",spriteStack[v],"hitTestPoint",v.hitTestPoint and v:hitTestPoint(x,y,true,ctx.surface)) end
						table.insert(t,{handler=h, target=v, _parents=(spriteStack[v] or 0)-(if h then 0.1 else 0)})
					end
				end
			elseif v and v.global then
				v._parents=1000
				table.insert(t,v)
			end
		elseif v and type(v)=="function" then
			table.insert(t,{handler=v,_parents=1000})
		end
	end
	if event=="onMousePresence" then processEnterLeave(ctx,t,x,y) return end
	if #t==0 then return end
	for k,v in pairs(UI.Control.stopPropagation) do
		if v and type(v)=="table" then
			local stop=spriteStack and spriteStack[v]
			if stop and v.stopPropagation then 
				local lx,ly	if ctx.surface then lx,ly=v:spriteToLocal(ctx.surface,x,y) else lx,ly=v:globalToLocal(x,y) end
				stop=v:stopPropagation(lx,ly)
			end
			if stop then
				if debugClick then print("uicontrol","idispatch","STOP","name",v.name,"parents",spriteStack[v]) end
				table.insert(t,{stop=v, _parents=spriteStack[v]-0.5})
			end
		end
	end
	table.sort(t,function(a,b) return a._parents>b._parents end)
	local sg,pg,ag,a = nil,nil,nil,nil
	local called={}
	ctx._currentDispatchStack=sps
	for nv,v in ipairs(t) do --v.handler or s[event] must return true to stop propagation
		if pg and v._parents<pg then 
			ctx._currentDispatchStack=nil
			return sg,ag
		end --stopPropagation !
		local r,s=nil,v.target
		if debugClick then print("uicontrol","idispatch","event",event,v,"v._parents",v._parents,"pg",pg,"sg",sg,"v.handler?",v.handler,"r?",r,"s?",s,s and s:getClass(),"s[event]?",s and s[event],"STOP:",v.stop) end
		if v.handler then
			if type(v.handler)=="function" then
				if s then
					local lx,ly	if ctx.surface then lx,ly=s:spriteToLocal(ctx.surface,x,y) else lx,ly=s:globalToLocal(x,y) end
					r,a=v.handler(s,lx,ly,...) --stopPropagation ?
				else
					r,a=v.handler(x,y,...) --stopPropagation ?
					s=true
				end
			elseif v.handler[event] then
				if s then
					local lx,ly	if ctx.surface then lx,ly=s:spriteToLocal(ctx.surface,x,y) else lx,ly=s:globalToLocal(x,y) end
					r,a=v.handler[event](v.handler,s,lx,ly,...) --stopPropagation ?
					called[v.handler]=true
				else
					r,a=v.handler[event](v.handler,x,y,...) --stopPropagation ?
					called[v.handler]=true
					s=true
				end			
			end
		elseif s then
			if s[event] then
				local lx,ly	if ctx.surface then lx,ly=s:spriteToLocal(ctx.surface,x,y) else lx,ly=s:globalToLocal(x,y) end
				r,a=s[event](s,lx,ly,...) --stopPropagation ?
				called[s]=true
			end
		end
		if v.stop then --stopPropagation !
			local sp=v.stop
			if not sg and type(sp)=="table" and sp.getParent then
				sp=UI.Screen.getScreen(sp)
				if sp and sp[event] and not called[sp] then 
					local lx,ly	if ctx.surface then lx,ly=sp:spriteToLocal(ctx.surface,x,y) else lx,ly=sp:globalToLocal(x,y) end
					r,a=sp[event](sp,lx,ly,...)
					if r then sg=sp ag=a end
				end
			end
			ctx._currentDispatchStack=nil
			return sg,ag 
		end 
		if r then
			sg=s
			pg=v._parents
			ag=a
		end
	end
	ctx._currentDispatchStack=nil
	return sg,ag or a
end

function UI.Control.getDispatchStack(surface)
	local ctx=if surface then Contexts[surface] else UI.Control.CurrentContext or Contexts[stage]
	return ctx._currentDispatchStack
end

function UI.Control.isPresent(s,surface)
	local ctx=if surface then Contexts[surface] else UI.Control.CurrentContext or Contexts[stage]
	return ctx.presenceList[s]
end

function UI.Control.isDesktopGesture()
	return UI.Control.ENABLE_DESKTOP_GESTURES and UI.Control.Meta.deviceType=="mouse"
end

function UI.Control.queryStack(x,y,plane)
	local sps=(plane or stage):getChildrenAtPoint(x,y,true,false)
	local t={}
	local spriteStack={}
	for kk,vv in ipairs(sps) do 
		table.insert(t,{target=vv, _parents=kk})
		spriteStack[vv]=kk 
	end
	for k,v in pairs(UI.Control.stopPropagation) do
		if v and type(v)=="table" then
			local stop=true
			if v.stopPropagation then
				local lx,ly
				if plane then 
					lx,ly=v:spriteToLocal(plane,x,y) 
				else
					lx,ly=v:globalToLocal(x,y)
				end
				stop=v:stopPropagation(lx,ly)
			end
			if stop and spriteStack and spriteStack[v] then
				table.insert(t,{stop=v, _parents=spriteStack[v]-0.5})
			end
		end
	end
	table.sort(t,function(a,b) return a._parents>b._parents end)
	local tt={}
	for _,v in ipairs(t) do
		if v.stop then break end
		table.insert(tt,v.target)
	end
	return tt
end

--Touch control
--[[
local cpx,cpy,cpd,cpa --Touch center/origin pos, Distance and angle
local cpt,cpr,cph,cps --Touch start time, released time, linger context, target sprite
local cp_inertia={} --Inertia info
local ccount=0,clong,cdrag --Touch click count, long detected, drag in progress
local ctouch --Raw touch mode: map touch ids to sprites
]]
local function dispatch(ctx,event,x,y,...)
	return idispatch(ctx,event,nil,x,y,...)
end
local function dispatchS(ctx,event,x,y,...)
	return idispatch(ctx,event,ctx.cps,x,y,...)
end
local function dispatchSS(ctx,event,cps,x,y,...)
	return idispatch(ctx,event,cps,x,y,...)
end

local function fetchAllTouches(e,ctouch)
	local ex,ey,dist,angle,ec,eb,em,et,eid,etmap = -1,-1,0,0,0,nil,nil,nil,nil,nil
	local ts=e.allTouches
	if ts then
		eid=e.touch.id
		etmap={}
		local tmini,tmaxi = nil,nil
		local idmi,idma = 1000,0
		for _,t in ipairs(ts) do 
			etmap[t.id]=t
			if t.id and not ctouch[t.id] then
				if t.id>idma then 
					tmaxi=t
					idma=t.id
				end
				if t.id<idmi then 
					tmini=t
					idmi=t.id
				end
				ec+=1
				eb=eb or t.mouseButton
				em=em or t.modifiers
				if not et then et=t.type
				elseif et~=t.type then
					et="various"
				end
			end
		end
		if ctouch[eid] and not tmini then
			tmini=etmap[eid]
			tmaxi=mini
		end
		if tmini and tmaxi then
			ex=(tmini.x+tmaxi.x)/2
			ey=(tmini.y+tmaxi.y)/2
			dist=(tmini.x-tmaxi.x)^2+(tmini.y-tmaxi.y)^2
			angle=^>math.atan2(tmaxi.y-tmini.y,tmaxi.x-tmini.x)
		end
	end
	UI.Control.Meta.mouseButton=eb or 0
	UI.Control.Meta.modifiers=em or 0
	UI.Control.Meta.deviceType=et
	return ex,ey,dist,angle,ec,eid,etmap
end

local function touchChanged(ctx,e)
	local ex,ey,ed,ea,ec=fetchAllTouches(e,ctx.ctouch)
	ctx.cpx=ex
	ctx.cpy=ey
	ctx.cpd=ed
	ctx.cpa=ea
	if ctx.cdrag then
		local cs,i=dispatch(ctx,"onDragStart",ex,ey,ed,ea,true,nil,ec)
		if cs then
			ctx.cp_inertia.amount=i
			ctx.cp_inertia.tm=nil 
		end
	end
end

local function unlinger(ctx)
	if ctx.cph and ctx.cph.s then		
		idispatch(ctx,"onLingerEnd",ctx.cph.s,ctx.cph.x,ctx.cph.y)
	end
	ctx.cph=nil
end

local function onTouchMove(e)
	local ctx=Contexts[e.surface or stage]
	unlinger(ctx)
	UI.Control.LocalCursor=nil
	local ex,ey,ed,ea,ec,_,etmap=fetchAllTouches(e,ctx.ctouch)
	if not UI.Control.HasPresence then
		dispatch(ctx,"onMousePresence",ex,ey)
	end
	for tid,tspr in pairs(ctx.ctouch) do
		dispatchSS(ctx,"onMouseMove",tspr,etmap[tid].x,etmap[tid].y,true)
	end
	if ec==0 then return end
	local ncps=dispatch(ctx,"onMouseMove",ex,ey,true)
	if not ctx.cdrag and ctx.cpx and ctx.cpy and ((math.distance(ctx.cpx,ctx.cpy,ex,ey)>UI.Control.DRAG_THRESHOLD) or (ed>0)) then
		if ctx.cpt then
			local pressTime=os.timer()-ctx.cpt
			if debugClick then print("uicontrol","onTouchMove",pressTime,UI.Control.LONGCLICK_START,pressTime>UI.Control.LONGCLICK_START) end
			if pressTime>UI.Control.LONGCLICK_START then
				dispatchS(ctx,"onLongPrepare",ex,ey,-1)
			end
		end
		local long=ctx.clong
		if ctx.clong then
			dispatchS(ctx,"onLongDown",ex,ey,0)
			ctx.clong=nil
		else
			--We weren't in a long operation, reset targetted sprite to query regular drag target
			ctx.cps=nil
			ncps=nil
		end
		ctx.cdrag=true
		local i
		ctx.cps=ctx.cps or ncps
		ncps,i=dispatch(ctx,"onDragStart",ctx.cpx,ctx.cpy,ctx.cpd,ctx.cpa,nil,long,ec)
		ctx.cp_inertia.tm=nil 
		ctx.cp_inertia.amount=i
	end
	ctx.cps=ctx.cps or ncps
	if ctx.cdrag then
		if ctx.cp_inertia.amount then
			if ctx.cp_inertia.tm and (os:timer()-ctx.cp_inertia.tm)>0.01 then
				local dtm=os:timer()-ctx.cp_inertia.tm
				local vx,vy=(ex-ctx.cp_inertia.x)/dtm,(ey-ctx.cp_inertia.y)/dtm
				ctx.cp_inertia.vx=vx*ctx.cp_inertia.amount
				ctx.cp_inertia.vy=vy*ctx.cp_inertia.amount
			end
			ctx.cp_inertia.x=ex
			ctx.cp_inertia.y=ey
			ctx.cp_inertia.tm=os:timer()
		end
		local cs,i=dispatchS(ctx,"onDrag",ex,ey,ed,ea,ec)
		if cs then
			ctx.cp_inertia.amount=i
			if not i then
				ctx.cp_inertia.vx=nil
				ctx.cp_inertia.vy=nil
				ctx.cp_inertia.tm=nil
			end
		end
	end
	updateCursor(ctx)
end

local function touchClear(ctx,ex,ey,ed,ea,cancel)
	local _,cps=next(ctx.ctouch)
	if cps then
		dispatchSS(ctx,"onMouseUp",cps,ex,ey)
		UI.Control.Meta.mouseButton=0
		return 
	end
	dispatch(ctx,"onMouseUp",ex,ey)
	if ctx.cdrag then
		ctx.cp_inertia.amount=nil
		if not ctx.cp_inertia.vx then
			ctx.cp_inertia.tm=nil
			dispatchS(ctx,"onDragEnd",ex,ey,ed,ea)
			ctx.cdrag=nil
		end
		ctx.cpt=nil
		ctx.cpr=nil
	elseif ctx.clong then
		if not cancel and not dispatchS(ctx,"onLongClick",ex,ey,1) then
			--Fallback
			dispatchS(ctx,"onMouseClick",ex,ey,1,if ctx.cpt then os.timer()-ctx.cpt else nil)
		end
		ctx.cpt=nil
		ctx.cpr=nil
	else
		ctx.cpr=os.timer()
		ctx.ccount=ctx.ccount+1
	end
	if debugClick then print("uicontrol","touchClear","clong or cpt",ctx.clong or ctx.cpt,"clong",ctx.clong,"cpt",ctx.cpt) end
	if ctx.clong or ctx.cpt then
		dispatchS(ctx,"onLongPrepare",ex,ey,-1)
	end
	ctx.clong=nil
	UI.Control.Meta.mouseButton=0
end

local function onTouchDown(e)
	local ctx=Contexts[e.surface or stage]
	unlinger(ctx)
	local ex,ey,ed,ea,ec,eid,etmap=fetchAllTouches(e,ctx.ctouch)
	if eid and etmap and next(UI.Control.isRawTouch) then
		local cps=dispatch(ctx,"isRawTouch",etmap[eid].x,etmap[eid].y)
		if cps then
			cps=dispatch(ctx,"onMouseDown",etmap[eid].x,etmap[eid].y)
			ctx.ctouch[eid]=cps
			return
		end
	end
	if ctx.cpt then
		ctx.cp_inertia.vx=nil
		ctx.cp_inertia.vy=nil
		ctx.cp_inertia.tm=nil
		ctx.cpr=nil
		touchChanged(ctx,e)
		onTouchMove(e)
	else
		if ctx.cdrag and ctx.cp_inertia.tm and not ctx.cp_inertia.amount and ctx.cp_inertia.vx then
			dispatchS(ctx,"onDragEnd",ctx.cp_inertia.x,ctx.cp_inertia.y)
		end
		ctx.cp_inertia.vx=nil
		ctx.cp_inertia.vy=nil
		ctx.cp_inertia.tm=nil
		ctx.clong=((UI.Control.Meta.mouseButton or 0)&2)>0
		if ctx.clong then 
			--[[
			local t=Core.enableAllocationTracking(false)
			if t then
				local counts={}
				for k,v in pairs(t) do
					counts[v]=(counts[v] or 0)+1
				end
				local sort={}
				for k,_ in pairs(counts) do
					table.insert(sort,k)
				end
				table.sort(sort,function(a,b) return counts[a]>counts[b] end)
				for _,k in ipairs(sort) do
					print(counts[k],k)
				end
			else
				print("--NEW TABLES")
				Core.enableAllocationTracking(true)
			end]]
			Core.profilerReset()
			collectgarbage()
			--print("LUA MEMORY:",gcinfo()/1024,"TEXTURE MEMORY:",application:getTextureMemoryUsage()/1024)
		end
		ctx.cps=dispatch(ctx,if ctx.clong then "onLongDown" else "onMouseDown",ex,ey,if ctx.clong then 1 else nil)
		ctx.cpx=ex
		ctx.cpy=ey
		ctx.cpd=ed
		ctx.cpa=ea
		ctx.ctouch[eid]=ctx.cps
		ctx.cpt=if ctx.cps then nil else os.timer()
		if not ctx.cpr then ctx.ccount=0 end
		ctx.cpr=nil
		ctx.cdrag=nil
	end
end

local function onTouchUp(e)
	local ctx=Contexts[e.surface or stage]
	local ex,ey,ed,ea,_,eid,etmap=fetchAllTouches(e,ctx.ctouch)
	if #e.allTouches<=1 then
		touchClear(ctx,ex,ey,ed,ea)
		ctx.ctouch[eid]=nil
	else
		-- Some touches left
		-- Directly 'touch up' for raw touches
		local cps=ctx.ctouch[eid]
		if cps then
			dispatchSS(ctx,"onMouseUp",cps,etmap[eid].x,etmap[eid].y)
		end
		ctx.ctouch[eid]=nil
		-- Generate a touch move for the first remaining touch
		local ne={ surface=e.surface, touch={ id=eid }, allTouches={} }
		for k,t in ipairs(e.allTouches) do if t.id~=eid then table.insert(ne.allTouches,t) end end
		if #ne.allTouches then
			touchChanged(ctx,ne)
			onTouchMove(ne)
		end
	end
end

local function onTouchCancel(e)
	local ctx=Contexts[e.surface or stage]
	unlinger(ctx)
	local ex,ey,ed,ea,_,eid=fetchAllTouches(e,ctx.ctouch)
	touchClear(ctx,ex or 0,ey or 0,ed,ea,true)
	if eid then
		ctx.ctouch[eid]=nil
	else
		table.clear(ctx.ctouch)
	end
end

local function onMouseWheel(e)
	local ctx=Contexts[e.surface or stage]
	unlinger(ctx)
	if e and e.wheel then
		UI.Control.Meta.modifiers=e.modifiers
		dispatch(ctx,"onMouseWheel",e.x,e.y,e.wheel,(e.wheel/120)*UI.Control.WHEEL_DISTANCE)
	end
end
local function onMouseHover(e)
	local ctx=Contexts[e.surface or stage]
	unlinger(ctx)
	UI.Control.LocalCursor=nil
	UI.Control.HasPresence=true
	UI.Control.Meta.modifiers=e.modifiers
	if ctx.clong or ctx.cdrag then
		onTouchCancel(e)
	end

	dispatch(ctx,"onMousePresence",e.x,e.y)
	dispatch(ctx,"onMouseMove",e.x,e.y,false)
	ctx.cph={ time=os.timer(), x=e.x, y=e.y }
	updateCursor(ctx)
end
local function onMouseEnter(e)
	local ctx=Contexts[e.surface or stage]
	unlinger(ctx)
	UI.Control.LocalCursor=nil
	UI.Control.HasPresence=true
	UI.Control.Meta.modifiers=e.modifiers	
	dispatch(ctx,"onMousePresence",e.x,e.y)
	ctx.cph={ time=os.timer(), x=e.x, y=e.y }
	updateCursor(ctx)
end
local function onMouseLeave(e)
	local ctx=Contexts[e.surface or stage]
	unlinger(ctx)
	UI.Control.Meta.modifiers=e.modifiers
	dispatch(ctx,"onMousePresence",e.x,e.y)
	ctx.enteredList=nil
	updateCursor(ctx)
end

local function onKeyDown(evt)
	local ctx=Contexts[evt.surface or stage]
	unlinger(ctx)
	UI.Control.Meta.modifiers=evt.modifiers
	ctx.keyDown={ keyCode=evt.keyCode, realCode=evt.realCode, keyTimer=os:timer()+UI.Control.REPEAT_TIMER }
	fdispatch("onKeyDown",evt.keyCode,evt.realCode)
end

local function onKeyUp(evt)
	local ctx=Contexts[evt.surface or stage]
	ctx.keyDown=nil
	UI.Control.Meta.modifiers=evt.modifiers
	fdispatch("onKeyUp",evt.keyCode,evt.realCode)
end

local function onKeyChar(evt)
	UI.Control.Meta.modifiers=evt.modifiers
	fdispatch("onKeyChar",evt.text)
end

local function onEnterFrame(e)
	for _,ctx in pairs(Contexts) do
		local ktm=os.timer()
		if ctx.cpt and not ctx.cpr then
			if not ctx.cdrag then
				local pressTime=ktm-ctx.cpt
				if debugClick then print("uicontrol","onEnterFrame",pressTime,UI.Control.LONGCLICK_THRESHOLD,UI.Control.LONGCLICK_START,pressTime>UI.Control.LONGCLICK_THRESHOLD,pressTime>UI.Control.LONGCLICK_START,"clong?",ctx.clong) end
				if pressTime>UI.Control.LONGCLICK_THRESHOLD then
					if not ctx.clong then
						ctx.clong=true
						local ncps=dispatch(ctx,"onLongPrepare",ctx.cpx,ctx.cpy,1)
						local ncps2=dispatch(ctx,"onLongDown",ctx.cpx,ctx.cpy,1)
						ctx.cps=ctx.cps or ncps or ncps2
					end
				elseif pressTime>UI.Control.LONGCLICK_START then
					local longRatio=(pressTime-UI.Control.LONGCLICK_START)/(UI.Control.LONGCLICK_THRESHOLD-UI.Control.LONGCLICK_START)
					local ncps=dispatchS(ctx,"onLongPrepare",ctx.cpx,ctx.cpy,longRatio><1)
					ctx.cps=ctx.cps or ncps
				end
			end
		elseif ctx.cpr then
			local clickTime=ktm-ctx.cpr
			if clickTime>UI.Control.DOUBLECLICK_THRESHOLD then
				dispatchS(ctx,"onMouseClick",ctx.cpx,ctx.cpy,ctx.ccount,if ctx.cpt then os.timer()-ctx.cpt else nil)
				ctx.cpr=nil
				ctx.cpt=nil
				ctx.ccount=0
			end
		elseif ctx.cph and not ctx.cph.trig then
			local ltime=ktm-ctx.cph.time
			if ltime>UI.Control.LINGER_THRESHOLD then
				ctx.cph.s=dispatch(ctx,"onLingerStart",ctx.cph.x,ctx.cph.y)
				ctx.cph.trig=true
			end
		end
		if ctx.cdrag and ctx.cp_inertia.tm and not ctx.cp_inertia.amount and ctx.cp_inertia.vx then
			local dtm=ktm-ctx.cp_inertia.tm
			local vdecay=0.1^dtm
			ctx.cp_inertia.tm=ktm
			ctx.cp_inertia.x+=ctx.cp_inertia.vx*dtm
			ctx.cp_inertia.y+=ctx.cp_inertia.vy*dtm
			ctx.cp_inertia.vx*=vdecay
			ctx.cp_inertia.vy*=vdecay
			if math.abs(ctx.cp_inertia.vx+ctx.cp_inertia.vy)<10 then
				ctx.cp_inertia.tm=nil
				ctx.cp_inertia.vx=nil
				ctx.cp_inertia.vy=nil
				dispatchS(ctx,"onDragEnd",ctx.cp_inertia.x,ctx.cp_inertia.y)
			else
				local cs,i=dispatchS(ctx,"onDrag",ctx.cp_inertia.x,ctx.cp_inertia.y)			
				if cs and not i then
					ctx.cp_inertia.vx=nil
					ctx.cp_inertia.vy=nil
					ctx.cp_inertia.tm=nil
					dispatchS(ctx,"onDragEnd",ctx.cp_inertia.x,ctx.cp_inertia.y)
				end
			end
		end
		local kd=ctx.keyDown
		if kd then
			if ktm>kd.keyTimer then
				kd.keyTimer=ktm+UI.Control.REPEAT_PERIOD
				fdispatch("onKeyDown",kd.keyCode,kd.realCode,true)
			end
		end
	end
	ndispatch("onEnterFrame",e.deltaTime)
	collectgarbage("step",100)
end

function UI.Control.setLocalCursor(cursor,forced)
	if forced or not UI.Control.LocalCursor then
		UI.Control.LocalCursor=cursor
	end
end

function UI.Control.setGlobalCursor(cursor)
	UI.Control.GlobalCursor=cursor
	updateCursor()
end

stage:addEventListener(Event.ENTER_FRAME,onEnterFrame)
stage:addEventListener(Event.TOUCHES_BEGIN,onTouchDown)
stage:addEventListener(Event.TOUCHES_END,onTouchUp)
stage:addEventListener(Event.TOUCHES_MOVE,onTouchMove)
stage:addEventListener(Event.TOUCHES_CANCEL,onTouchCancel)
if (Event.MOUSE_WHEEL~=nil) then
	stage:addEventListener(Event.MOUSE_WHEEL,onMouseWheel)
end
if (Event.MOUSE_HOVER~=nil) then
	stage:addEventListener(Event.MOUSE_HOVER,onMouseHover)
end
if Event.MOUSE_ENTER then
	stage:addEventListener(Event.MOUSE_ENTER,onMouseEnter)
	stage:addEventListener(Event.MOUSE_LEAVE,onMouseLeave)
end
stage:addEventListener(Event.KEY_DOWN,onKeyDown)
stage:addEventListener(Event.KEY_UP,onKeyUp)
stage:addEventListener(Event.KEY_CHAR,onKeyChar)

UI.Control.VirtualInput={
	onTouchDown=onTouchDown,
	onTouchUp=onTouchUp,
	onTouchMove=onTouchMove,
	onTouchCancel=onTouchCancel,
	onMouseHover=onMouseHover,
	onMouseWheel=onMouseWheel,
	onMouseEnter=onMouseEnter,
	onMouseLeave=onMouseLeave,
	declareSurface=declareContext,
}
