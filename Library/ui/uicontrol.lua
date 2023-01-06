--!NEEDS:uiinit.lua
--[[
Interpret controls and trigger the following events:
* onMouseDown, onMouseUp, onMouseMove, onMouseWheel (RAW events)
* onMouseClick(click count) (following onMouseUp)
* onLongPrepare(ratio) (following onMouseDown)
* onLongDown(longSteps) (following onMouseDown)
* onLongClick(longSteps) (following onMouseUp)
* onDragStart() (following onMouseDown)
* onDrag()  (following onMouseMove)
* onDragEnd()  (following onMouseUp)
]]

UI.Control={
 onMouseDown={},
 onMouseUp={},
 onMouseMove={},
 onMouseClick={},
 onMouseWheel={},
 onMouseEnter={},
 onMouseLeave={},
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
 Meta={},
 DOUBLECLICK_THRESHOLD=0.1, --s
 LONGCLICK_START=0.3, --s before indicating a long click is preparing
 LONGCLICK_THRESHOLD=1, --s before announcing a long down
 LINGER_THRESHOLD=1, --s after mouse stop to trigger linger event
 DRAG_THRESHOLD=20, --px to assume a drag and cancel potential long click
 WHEEL_DISTANCE=20, --px to scroll for a wheel tick
 RIGHTCLICK_IS_LONG=true,
 HAS_CURSOR=true,
}
local _weak={ __mode = "kv"}
setmetatable(UI.Control.onMouseDown,_weak)
setmetatable(UI.Control.onMouseUp,_weak)
setmetatable(UI.Control.onMouseMove,_weak)
setmetatable(UI.Control.onMouseClick,_weak)
setmetatable(UI.Control.onMouseWheel,_weak)
setmetatable(UI.Control.onMouseEnter,_weak)
setmetatable(UI.Control.onMouseLeave,_weak)
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

-- Position related
local function idispatch(event,cps,x,y,...)
	if debugClick then print("uicontrol","idispatch","event",event,"CPS:",cps,"X/Y:"..x.."/"..y) end
	local t={}
	local spriteStack
	for k,v in pairs(UI.Control[event]) do
		if v and type(v)=="table" then
			local h=nil
			if not v.hitTestPoint and v.handler then
				h=v.handler
				v=v.target
			end
			if debugClick then print("uicontrol","idispatch","hitTestPoint",v.hitTestPoint and v:hitTestPoint(x,y,true)) end
			if not spriteStack then --populate spriteStack
				local sps=stage:getChildrenAtPoint(x,y,true,false)
				spriteStack={}
				for kk,vv in ipairs(sps) do spriteStack[vv]=kk end
			end
			if getmetatable(v) and v.hitTestPoint and (cps==v or ((cps==nil) and x and y and spriteStack[v])) then -- Sprite or Instance
				if cps then
					table.insert(t,{handler=h, target=v, _parents=1000})
				else
					if spriteStack[v] then
						table.insert(t,{handler=h, target=v, _parents=spriteStack[v] or 0})
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
	if #t==0 then return end
	for k,v in pairs(UI.Control.stopPropagation) do
		if v and type(v)=="table" then
			local stop=true
			if v.stopPropagation then 
				local lx,ly=v:globalToLocal(x,y)
				stop=v:stopPropagation(lx,ly)
			end
			if stop and spriteStack and spriteStack[v] then
				table.insert(t,{stop=v, _parents=spriteStack[v]-0.5})
			end
		end
	end
	table.sort(t,function(a,b) return a._parents>b._parents end)
	local sg,pg,ag,a = nil,nil,nil,nil
	for _,v in ipairs(t) do --v.handler or s[event] must return true to stop propagation
		if pg and v._parents<pg then return sg,ag end --stopPropagation !
		local r,s=nil,v.target
		if debugClick then print("uicontrol","idispatch","event",event,v,"v._parents",v._parents,"pg",pg,"sg",sg,"v.handler?",v.handler,"r?",r,"s?",s,s and s:getClass(),"s[event]?",s and s[event],"STOP:",v.stop) end
		if v.handler then
			if type(v.handler)=="function" then
				if s then
					local lx,ly=s:globalToLocal(x,y)
					r,a=v.handler(s,lx,ly,...) --stopPropagation ?
				else
					r,a=v.handler(x,y,...) --stopPropagation ?
					s=true
				end
			elseif v.handler[event] then
				if s then
					local lx,ly=s:globalToLocal(x,y)
					r,a=v.handler[event](v.handler,s,lx,ly,...) --stopPropagation ?
				else
					r,a=v.handle[event](v.handler,x,y,...) --stopPropagation ?
					s=true
				end			
			end
		elseif s then
			if s[event] then
				local lx,ly=s:globalToLocal(x,y)
				r,a=s[event](s,lx,ly,...) --stopPropagation ?
			end
		end
		if v.stop then --stopPropagation !
			local sp=v.stop
			if not sg and type(sp)=="table" and sp.getParent then
				sp=UI.Screen.getScreen(sp)
				if sp and sp[event] then 
					local lx,ly=sp:globalToLocal(x,y)
					r,a=sp[event](sp,lx,ly,...)
					if r then sg=sp ag=a end
				end
			end
			return sg,ag 
		end 
		if r then
			sg=s
			pg=v._parents
			ag=a
		end
	end
	return sg,ag or a
end

function UI.Control.queryStack(x,y)
	local sps=stage:getChildrenAtPoint(x,y,true,false)
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
				local lx,ly=v:globalToLocal(x,y)
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
local cpx,cpy,cpd,cpa --Touch center/origin pos, Distance and angle
local cpt,cpr,cph,cps --Touch start time, released time, linger context, target sprite
local cp_inertia={} --Inertia info
local ccount=0,clong,cdrag,ctouch --Touch click count, long detected, drag in progress, raw touch mode
local function dispatch(event,x,y,...)
	return idispatch(event,nil,x,y,...)
end
local function dispatchS(event,x,y,...)
	return idispatch(event,cps,x,y,...)
end

local function fetchAllTouches(ts)
	local ex,ey,dist,angle,ec,eb,em = nil,nil,nil,nil,0,nil,nil
	if ts then
		local tmini,tmaxi = nil,nil
		local idmi,idma = 1000,0
		for _,t in ipairs(ts) do 
			if t.id then
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
			end
		end
		if tmini and tmaxi then
			ex=(tmini.x+tmaxi.x)/2
			ey=(tmini.y+tmaxi.y)/2
			dist=(tmini.x-tmaxi.x)^2+(tmini.y-tmaxi.y)^2
			angle=^>math.atan2(tmaxi.y-tmini.y,tmaxi.x-tmini.x)
		end
	end
	UI.Control.Meta.mouseButton=eb
	UI.Control.Meta.modifiers=em
	return ex,ey,dist,angle,ec
end

local function touchChanged(e)
	local ex,ey,ed,ea,ec=fetchAllTouches(e.allTouches)
	cpx=ex
	cpy=ey
	cpd=ed
	cpa=ea
	local cs,i=dispatch("onDragStart",cpx,cpy,cpd,cpa,true)
	if cs then
		cp_inertia.amount=i
		cp_inertia.tm=nil 
	end
end

local function unlinger()
	if cph and cph.s then		
		idispatch("onLingerEnd",cph.s,cph.x,cph.y)
	end
	cph=nil
end

local function onTouchMove(e)
	unlinger()
	UI.Control.LocalCursor=nil
	local ex,ey,ed,ea,ec=fetchAllTouches(e.allTouches)
	local ncps=dispatch("onMouseMove",ex,ey,true)
	if ctouch then return end
	if not cdrag and cpx and cpy and ((math.distance(cpx,cpy,ex,ey)>UI.Control.DRAG_THRESHOLD) or (ed>0)) then
		if cpt then
			local pressTime=os.timer()-cpt
			if debugClick then print("uicontrol","onTouchMove",pressTime,UI.Control.LONGCLICK_START,pressTime>UI.Control.LONGCLICK_START) end
			if pressTime>UI.Control.LONGCLICK_START then
				dispatchS("onLongPrepare",ex,ey,-1)
			end
		end
		local long=clong
		if clong then
			dispatchS("onLongDown",ex,ey,0)
			clong=nil
		else
			--We weren't in a long operation, reset targetted sprite to query regular drag target
			cps=nil
			ncps=nil
		end
		cdrag=true
		local i
		cps=cps or ncps
		ncps,i=dispatch("onDragStart",cpx,cpy,cpd,cpa,nil,long)
		cp_inertia.tm=nil 
		cp_inertia.amount=i
	end
	cps=cps or ncps
	if cdrag then
		if cp_inertia.amount then
			if cp_inertia.tm and (os:timer()-cp_inertia.tm)>0.01 then
				local dtm=os:timer()-cp_inertia.tm
				local vx,vy=(ex-cp_inertia.x)/dtm,(ey-cp_inertia.y)/dtm
				cp_inertia.vx=vx*cp_inertia.amount
				cp_inertia.vy=vy*cp_inertia.amount
			end
			cp_inertia.x=ex
			cp_inertia.y=ey
			cp_inertia.tm=os:timer()
		end
		local cs,i=dispatchS("onDrag",ex,ey,ed,ea)
		if cs then
			cp_inertia.amount=i
			if not i then
				cp_inertia.vx=nil
				cp_inertia.vy=nil
				cp_inertia.tm=nil
			end
		end
	end
	updateCursor()
end

local function touchClear(ex,ey,ed,ea,cancel)
	if ctouch then 
		dispatchS("onMouseUp",ex,ey)
		UI.Control.Meta.mouseButton=0
		return 
	end
	dispatch("onMouseUp",ex,ey)
	if cdrag then
		cp_inertia.amount=nil
		if not cp_inertia.vx then
			cp_inertia.tm=nil
			dispatchS("onDragEnd",ex,ey,ed,ea)
		end
	elseif clong then
		if not cancel and not dispatchS("onLongClick",ex,ey,1) then
			--Fallback
			dispatchS("onMouseClick",ex,ey,1)
		end
	else
		cpr=os.timer()
		ccount=ccount+1
	end
	if debugClick then print("uicontrol","touchClear","clong or cpt",clong or cpt,"clong",clong,"cpt",cpt) end
	if clong or cpt then
		dispatchS("onLongPrepare",ex,ey,-1)
	end
	cpt=nil
	UI.Control.Meta.mouseButton=0
end

local function onTouchDown(e)
	unlinger()
	if cpt then
		cp_inertia.vx=nil
		cp_inertia.vy=nil
		cp_inertia.tm=nil
		touchChanged(e)
		onTouchMove(e)
	else
		if cdrag and cp_inertia.tm and not cp_inertia.amount and cp_inertia.vx then
			dispatchS("onDragEnd",cp_inertia.x,cp_inertia.y)
		end
		cp_inertia.vx=nil
		cp_inertia.vy=nil
		cp_inertia.tm=nil
		local ex,ey,ed,ea,ec=fetchAllTouches(e.allTouches)
		clong=((UI.Control.Meta.mouseButton or 0)&2)>0
		if clong then 
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
		end
		cps=dispatch(if clong then "onLongDown" else "onMouseDown",ex,ey,if clong then 1 else nil)
		cpx=ex
		cpy=ey
		cpd=ed
		cpa=ea
		ctouch=cps
		cpt=if ctouch then nil else os.timer()
		if not cpr then ccount=0 end
		cpr=nil
		cdrag=nil
	end
end

local function onTouchUp(e)
	local ex,ey,ed,ea,ec=fetchAllTouches(e.allTouches)
	if #e.allTouches<=1 then
		touchClear(ex,ey,ed,ea)
	else
		-- Some touches left, generate a touch move for the first remaining touch
		local id=e.touch.id
		local ne={ allTouches={} }
		for k,t in ipairs(e.allTouches) do if t.id~=id then table.insert(ne.allTouches,t) end end
		if #ne.allTouches then
			touchChanged(ne)
			onTouchMove(ne)
		end
	end
end

local function onTouchCancel(e)
	unlinger()
	local ex,ey,ed,ea=fetchAllTouches(e)
	touchClear(ex or 0,ey or 0,ed,ea,true)
end

local function onMouseWheel(evt)
	unlinger()
	if evt and evt.wheel then
		UI.Control.Meta.modifiers=evt.modifiers
		dispatch("onMouseWheel",evt.x,evt.y,evt.wheel,(evt.wheel/120)*UI.Control.WHEEL_DISTANCE)
	end
end
local function onMouseHover(evt)
	unlinger()
	UI.Control.LocalCursor=nil
	UI.Control.Meta.modifiers=evt.modifiers
	dispatch("onMouseMove",evt.x,evt.y,false)
	cph={ time=os.timer(), x=evt.x, y=evt.y }
	updateCursor()
end
local function onMouseEnter(evt)
	unlinger()
	UI.Control.LocalCursor=nil
	UI.Control.Meta.modifiers=evt.modifiers
	dispatch("onMouseEnter",evt.x,evt.y)
	cph={ time=os.timer(), x=evt.x, y=evt.y }
	updateCursor()
end
local function onMouseLeave(evt)
	unlinger()
	UI.Control.Meta.modifiers=evt.modifiers
	dispatch("onMouseLeave",evt.x,evt.y)
end

local function onKeyDown(evt)
	UI.Control.Meta.modifiers=evt.modifiers
	fdispatch("onKeyDown",evt.keyCode,evt.realCode)
end

local function onKeyUp(evt)
	UI.Control.Meta.modifiers=evt.modifiers
	fdispatch("onKeyUp",evt.keyCode,evt.realCode)
end

local function onKeyChar(evt)
	UI.Control.Meta.modifiers=evt.modifiers
	fdispatch("onKeyChar",evt.ktext)
end

local function onEnterFrame(e)
	if cpt then
		if not cdrag then
			local pressTime=os.timer()-cpt
			if debugClick then print("uicontrol","onEnterFrame",pressTime,UI.Control.LONGCLICK_THRESHOLD,UI.Control.LONGCLICK_START,pressTime>UI.Control.LONGCLICK_THRESHOLD,pressTime>UI.Control.LONGCLICK_START,"clong?",clong) end
			if pressTime>UI.Control.LONGCLICK_THRESHOLD then
				if not clong then
					clong=true
					local ncps=dispatch("onLongPrepare",cpx,cpy,1)
					local ncps2=dispatch("onLongDown",cpx,cpy,1)
					cps=cps or ncps or ncps2
				end
			elseif pressTime>UI.Control.LONGCLICK_START then
				local longRatio=(pressTime-UI.Control.LONGCLICK_START)/(UI.Control.LONGCLICK_THRESHOLD-UI.Control.LONGCLICK_START)
				local ncps=dispatchS("onLongPrepare",cpx,cpy,longRatio><1)
				cps=cps or ncps
			end
		end
	elseif cpr then
		if (os.timer()-cpr)>UI.Control.DOUBLECLICK_THRESHOLD then
			cpr=nil
			dispatchS("onMouseClick",cpx,cpy,ccount)
			ccount=0
		end
	elseif cph and not cph.trig then
		local ltime=os.timer()-cph.time
		if ltime>UI.Control.LINGER_THRESHOLD then
			cph.s=dispatch("onLingerStart",cph.x,cph.y)
			cph.trig=true
		end
	end
	if cdrag and cp_inertia.tm and not cp_inertia.amount and cp_inertia.vx then
		local dtm=os.timer()-cp_inertia.tm
		local vdecay=0.1^dtm
		cp_inertia.tm=os:timer()
		cp_inertia.x+=cp_inertia.vx*dtm
		cp_inertia.y+=cp_inertia.vy*dtm
		cp_inertia.vx*=vdecay
		cp_inertia.vy*=vdecay
		if math.abs(cp_inertia.vx+cp_inertia.vy)<10 then
			cp_inertia.tm=nil
			cp_inertia.vx=nil
			cp_inertia.vy=nil
			dispatchS("onDragEnd",cp_inertia.x,cp_inertia.y)
		else
			local cs,i=dispatchS("onDrag",cp_inertia.x,cp_inertia.y)			
			if cs and not i then
				cp_inertia.vx=nil
				cp_inertia.vy=nil
				cp_inertia.tm=nil
				dispatchS("onDragEnd",cp_inertia.x,cp_inertia.y)
			end
		end
	end
	ndispatch("onEnterFrame")
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
