--!NEEDS:uiinit.lua

UI.Focus={ KEYBOARD=1, mode=0 }
UI.Focus.onFocus={}
local _weak={ __mode = "kv"}
setmetatable(UI.Focus.onFocus,_weak)

local function ndispatch(...)
	local dlist={}
	for k,v in pairs(UI.Focus.onFocus) do
		dlist[k]=v
	end
	for _,v in pairs(dlist) do
		local s=nil
		if v and type(v)=="table" then
			if v.onFocus then
				s=v
				v=s.onFocus
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

function UI.Focus:request(w,mode)
  local gained=false
  mode=mode or tonumber(w._flags.focusable) or 0
  if self.focused~=w then
	local lfocused=self.focused
	self.focused=nil -- Prevent sending out temporary nil focus event
    if lfocused then
		lfocused:setFlags({focused=false})
		if lfocused.unfocus then lfocused:unfocus({ TAG="UI.Focus",reason="FOCUS_CHANGE" }) end
    end
    gained=true
    self.focused=w
	
	local lmode=self.mode
    self.mode=mode
    if mode&self.KEYBOARD>0 then
		if not self.hasKbd then
			self.hasKbd=UI.Keyboard.setKeyboardVisibility(w)
		end
	elseif lmode&self.KEYBOARD>0 then
		UI.Keyboard.setKeyboardVisibility(nil)
		if self.hasKbd then
			self:offsetStage(0,0)
			self.hasKbd=nil
		end
    end
		
    assert(w and w.setFlags,"Widget must be a descendant of UI.Panel")
    w:setFlags({focused=true})
	ndispatch(w)
  end
  --print("UI.Focus",w and w.name,"FOCUS?",gained)
  return gained
end

function UI.Focus:relinquish(w)
  local same=false
  if w and self.focused==w then
    same=true
    self.focused=nil
    if self.mode&self.KEYBOARD>0 then
		UI.Keyboard.setKeyboardVisibility(nil)
    end
    w:setFlags({focused=false})
    self.mode=0
    if self.hasKbd then
		self:offsetStage(0,0)
		self.hasKbd=nil
    end
	ndispatch(nil)
  end
  --print("UI.Focus",w and w.name,"UNFOCUS?",same)
  return same
end

function UI.Focus:offsetStage(cx,cy)
	--Do this a bit later, in case we are in an event loop where coordiniates shouldn't change
	Core.asyncCall(function()
		Core.yield(true)
		stage:setPosition(cx,cy)
	end)
end

function UI.Focus:area(widget,x,y,w,h,force,anchorx,anchory)
  if self.focused==widget or force then
    UI.dispatchEvent(widget,"FocusArea",x,y,w,h,anchorx,anchory)
    if self.hasKbd then
      local gix,giy=widget:localToGlobal(x,y)
      local gax,gay=widget:localToGlobal(x+w,y+h)
      local gsx,gsy,gsw,gsh=application:getLogicalBounds()
	  gsw-=gsx gsh-=gsy gsh*=(1-self.hasKbd)
      --print(gix,giy,gax,gay,gsw,gsh,self.hasKbd)
      while widget do -- Try to show as complete container chain
		widget:getLayoutInfo()
        local ix,iy,iw,ih=widget:getBounds(stage,true)
        --print(ix,iy,iw,ih)
        --if iw>gsw or ih>gsh then break end
        if ih>gsh then break end
        gix,giy,gax,gay=ix,iy,ix+iw,iy+ih
        widget=widget:getParent()
      end
	  if not widget then
		self:offsetStage(0,0)
		return
	  end
      --print(gix,giy,gax,gay) widget:setColor(0xFF0000,1)
      local six,siy=-gix<>-gax,-giy<>-gay
      local sax,say=(gsw-gix)><(gsw-gax),(gsh-giy)><(gsh-gay)
      local cx,cy=stage:getPosition()
      --print(six,siy,sax,say,cx,cy)
      cx=((cx<>six)><sax)><0
      cy=((cy<>siy)><say)><0
      local spx = 0 --NO cx
      local spy = cy
      self:offsetStage(spx,spy)
    end
  end
end

function UI.Focus:clear()
  if self.focused then
    if self.focused.unfocus then 
      self.focused:unfocus({ TAG="UI.Focus",reason="FOCUS_CLEAR" }) 
    end
	UI.Focus:relinquish(self.focused)
  end
end

function UI.Focus:focusRoot(w,mode)
	while UI.instanceOf(w,UI.Panel) do
		if w._flags.focusgroup then return w end
		w=w:getParent()
	end
	return UI.Screen.getScreen(w)
end

function UI.Focus:focusNext(root,mode)
  local t,p,cc={},{root},nil
  while #p>0 do
    local n=table.remove(p,1)
    for i=1,n:getNumChildren() do
      if n._flags and n._flags.focusable and not n._flags.disabled then 
        table.insert(t,n) 
        if self.focused and n==self.focused then cc=#t end
      end
      table.insert(p,n:getChildAt(i))
    end
  end
  if #t==0 then return end
  cc=(cc or 0)%(#t)+1
  return self:request(t[cc],mode)  
end

function UI.Focus:focusDirection(root,dx,dy,mode,atolerance)
  if not self.focused then return end
  local fx,fy,fw,fh=self.focused:getBounds(root)
  fx=fx+fw/2
  fy=fy+fh/2
  local aref=^>math.atan2(dy,dx)
  local p={root}
  local t,td=nil,1E20
  while #p>0 do
    local n=table.remove(p,1)
    for i=1,n:getNumChildren() do
      if n._flags and n._flags.focusable and not n._flags.disabled and n~=self.focused then 
        local bx,by,bw,bh=n:getBounds(root)
        bx=bx+bw/2
        by=by+bh/2
        local a,d=^>math.atan2(by-fy,bx-fx),((bx-fx)*(bx-fx)+(by-fy)*(by-fy))^0.5
        local ad=math.abs(aref-a)
        if ad<(atolerance or 45) and d<td then
          t=n td=d
        end
      end
      table.insert(p,n:getChildAt(i))
    end
  end
  if t then
    self:request(t,mode)  
  end
end

function UI.Focus:get()
  return self.focused
end


UI.Clipboard={ data={} }
-- callback is called with result boolean in case of set, and additionnaly data and mime for get
function UI.Clipboard:get(mime,callback)
	application:getClipboard(mime,function (res,data,mime)
		if not res then 
			data=self.data[mime]
			if data then res=true end
		end
		callback(res,data,mime)
	end)
end

function UI.Clipboard:set(data,mime,callback)
	self.data[mime or "text/plain"]=data
	application:setClipboard(data,mime,callback)
end

function UI.Clipboard:clear(mime)
	if mime then 
		self.data[mime]=nil
	else
		self.data={}
	end
end
