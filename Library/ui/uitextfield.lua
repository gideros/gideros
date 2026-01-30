--!NEEDS:uipanel.lua
local REPEAT_TIMER=0.5
local REPEAT_PERIOD=0.1
local HIDE_TIMER=1

local debugTextInput = nil
if debugTextInput then print("UI.TextField debugTextInput !!!!!!!!!!!!!!!!!!!!!!!") end

local function countCharsUtf8(text)
  local pe=1
  local n=0
  while pe<=#text do
    local b=text:byte(pe)
    if b<128 or b>=192 then n+=1 end
    pe+=1
  end 
  return n
end

local function skipCharsUtf8(text,n)
  local pe=1
  while pe<=#text and n>0 do
    pe+=1
    local b=text:byte(pe)
    if b==nil then break end
    if b<128 or b>=192 then n-=1 end
  end 
  return pe
end

local function removeCharUtf8(text,pos)
  local pe=pos
  while pe<=#text do
    pe+=1
    local b=text:byte(pe)
    if b==nil or b<128 or b>=192 then break end
  end 
  return text:sub(1,pos-1)..text:sub(pe)
end

local function addCharsUtf8(text,pos,c)
  if pos>1 then
    text=text:sub(1,pos-1)..c..text:sub(pos)
  else
    text=c..text:sub(pos)
  end
  return text,pos+#c
end

TextEdit=Core.class(TextField)
TextEdit._getText=TextEdit.getText
TextEdit._setText=TextEdit.setText
TextEdit._setFont=TextEdit.setFont

function TextEdit:init(font,text,layout)
	self.font=font or Font.getDefault()
	self.caretHeight=self.font:getLineHeight()
	local la=self.font:getAscender()
	local bb=0
	if self.font.getDescender then 
		bb=(self.caretHeight-la-self.font:getDescender())/2
	end
	self.caretOffset=la+bb
	self.lineAscender=la
	self.caretColor=UI.Color(0,0,0,1)
	self.password=layout and layout.password
	self.multiline=layout and layout.multiline
	self:setText(text or "")
	if self.setWorldAlign then self:setWorldAlign(not Oculus) end
end

function TextEdit:setFont(font)
	if self.font==font then return end
	self.font=font or Font.getDefault()
	self.caretHeight=self.font:getLineHeight()
	local la=self.font:getAscender()
	local bb=0
	if self.font.getDescender then 
		bb=(self.caretHeight-la-self.font:getDescender())/2
	end
	self.caretOffset=la+bb
	self.lineAscender=la
	self:setCaretPos(self.caretPos)
	self:_setFont(self.font)
end

function TextEdit:setPassword(p)
  if self then
    self.password=p
	self:setText(self:getText())
  end
end

function TextEdit:setMultiline(p)
  if self then
    self.multiline=p
  end
end

function TextEdit:setText(t)
  self:hideLast()
  self.text=t
  if self.password and t then
    self:_setText(self.password:rep(countCharsUtf8(t)))
  else
    self:_setText(t)
  end
end
function TextEdit:getText()
  return self.text
end
function TextEdit:passwordPos(idx,fromPass)
	if not self.password then return idx end
	if fromPass then
		idx=idx//#self.password
		idx=utf8.charpos(self.text,idx)
	else
		idx=utf8.widthindex(self.text,idx)
		idx*=#self.password
	end
	return idx
end
local function setTextInput(self,text,index1,index2) --,"label","action","hint")
  if self and application.setTextInput then
	local textType = self.textType or Application.TEXTINPUT_CLASS_TEXT
	if self.password then --setTextInput --TVARIANT_PASSWORD (ne marche pas sur téléphone Christian - marche sur Samsung Android 10 Sandrine)
		textType = textType|Application.TEXTINPUT_TVARIANT_PASSWORD
	end
	self.hasTextInput = not Oculus and application:setTextInput(textType,text,index1,index2,nil,nil,nil,tostring(self))--,"label","action","hint","context")
    if debugTextInput then print("UI.TextField setTextInput","self.hasTextInput?",self.hasTextInput,"type=",textType,"text=",text,"index1=",index1,"index2=",index2) end
  end
end
function TextEdit:startEditing(noKeyEvents) --TEMP_TOSEE attention certains claviers sur samsung peuvent doubler la string si correction à la main du texte (mettre clavier par defaut et réinitilisation du clavier pour corriger le bug)
	if not self.caret then return end
	self.editing=true
	self.keyDown=nil
	self.caret:removeFromParent()
	self:addChild(self.caret)
	self.caret:setAnchorPosition(0,self.caretOffset)
	self.noKeyEvents=noKeyEvents
	if not noKeyEvents then
		self:addEventListener(Event.KEY_CHAR,self.onKeyChar,self)
		self:addEventListener(Event.KEY_DOWN,self.onKeyDown,self)
		self:addEventListener(Event.KEY_UP,self.onKeyUp,self)
	end
	if Event.TEXT_INPUT then
		if debugTextInput then print("UI.TextField addEventListener Event.TEXT_INPUT") end
		self:addEventListener(Event.TEXT_INPUT,self.onTextInput,self)
	end
	self:addEventListener(Event.ENTER_FRAME,self.onFrame,self)
	self:addEventListener(Event.REMOVED_FROM_STAGE,self.onRemove,self)
	self:setSelected(self.selStart,self.selSpan)
	setTextInput(self,self.text,self.caretPos,self.caretPos)
end
function TextEdit:stopEditing()
	if not self.caret then return end
	self.editing=nil
	self:hideLast()
	if not self.noKeyEvents then
		self:removeEventListener(Event.KEY_CHAR,self.onKeyChar,self)
		self:removeEventListener(Event.KEY_DOWN,self.onKeyDown,self)
		self:removeEventListener(Event.KEY_UP,self.onKeyUp,self)
	end
	if Event.TEXT_INPUT then
		if debugTextInput then print("UI.TextField removeEventListener Event.TEXT_INPUT") end
		self:removeEventListener(Event.TEXT_INPUT,self.onTextInput,self)
	end
	self:removeEventListener(Event.ENTER_FRAME,self.onFrame,self)
	self:removeEventListener(Event.REMOVED_FROM_STAGE,self.onRemove,self)
	self.caret:removeFromParent()
	if self.selMark then
		self.selMark:setVisible(false)
	end
end
function TextEdit:setSelectionStartEnd(ss,se,force)
	if not ss then
		self:setSelected(0,0,nil,force)
		return
	end
	self.selStartEndStart=self.selStartEndStart or ss
	local ps=self.selStartEndStart><se
	local pe=self.selStartEndStart<>se
	self:setSelected(ps,pe-ps,true,force)
end
function TextEdit:getSelectionStartEnd()
	if self.selStart and self.selSpan and self.selSpan>0 then
		return self.selStart,self.selStart+self.selSpan
	end
end
function TextEdit:setSelected(ss,sp,keepBounds,force)
	if not self.selMark then return end
	local tx=self:_getText()
	ss=((ss or 0)<>0)><#tx
	sp=((sp or 0)<>0)><(#tx-ss)
	local changed=(ss~=self.selStart) or (sp~=self.selSpan)
	self.selStart=ss
	self.selSpan=sp
	if not keepBounds then self.selStartEndStart=nil end
	if (self.editing or force) and sp>0 then
		self.selMark:setVisible(true)
		local sx,sy,sl=self:getPointFromTextPosition(ss)
		local ex,ey,el=self:getPointFromTextPosition(ss+sp)
		local w=self:getWidth()
		local lh=self.caretHeight
		if sl==el then
			self.selMark:setSpan(sx,sy-self.caretOffset,ex-sx,lh, 0,0,0,0, 0,0,0,0)
		else
			self.selMark:setSpan(sx,sy-self.caretOffset,w-sx,lh, 0,sy+lh-self.caretOffset,w,(el-sl-1)*lh, 0,ey-self.caretOffset,ex,lh)
		end
	else
		self.selMark:setVisible(false)
	end
	if changed then
		UI.dispatchEvent(self._textfield,"TextSelection",if sp>0 then ss else nil,if sp>0 then ss+sp else nil)
	end
end
function TextEdit:getSelectedText(mode)
	if (self.selSpan or 0)>0 then
		--We don't care about password case, since passwords should never be copyable anyway
		local t=self:_getText()
		local full=t:sub(self.selStart+1,self.selStart+self.selSpan)
		if not mode then
			full=full:gsub("\e%[([^%]]*)]","")
		end
		return full
	end
end
function TextEdit:setCaretColor(c)
  self.caretColor=c
  if self.caret then
    self.caret:setColor(c)
  end
end
function TextEdit:setCaretPos(index,focusing)
  if self and self.caret then
    local tx=self:_getText()
	if not index then index=#tx --fix userdata
	elseif index and type(index)~="number" then index=#tx end --fix userdata
    index=(index<>0)><#tx
	local changed=self.caretPos~=index
    self.caretPos=index
    local cx,cy=self:getPointFromTextPosition(index)
	if cy==0 then --No text, assume font ascender, will be fixed in gideros 2024.4
		cy=self.lineAscender
	end
    self.caretX=cx
    self.caretY=cy
    self.caret:setPosition(self.caretX,self.caretY)
    self.caret:setVisible(true)
    if self.caretListener then self.caretListener(self.caretPos,self.caretX,self.caretY) end
	if changed then
		UI.dispatchEvent(self._textfield,"CaretPosition",self.caretPos,self.caretX,self.caretY)
	end
    if focusing then 
		index=self:passwordPos(index,true)
		setTextInput(self,self.text,index,index) 
	end
  end
end
function TextEdit:getCaretPosition()
	if not self.caret then return end
	return self.caret:getPosition()
end
function TextEdit:setCaretPosition(cx,cy,forSelection)
  if self and (forSelection or self.caret) then
    self:hideLast()
	if not self.multiline then 
		local _,my,_,mh=self:getBounds(self,false)
		cy=my+mh/2
	end
    local ti,mx,my=self:getTextPositionFromPoint(cx,cy)
	if my==0 then --No text, assume font ascender, will be fixed in gideros 2024.4
		my=self.lineAscender
		ti=0 --Bug fix
	end
	local changed=(ti~=self.caretPos) or (mx~=self.caretX) or (my~=self.caretY)
    self.caretPos=ti
    self.caretX=mx
    self.caretY=my
	if self.caret then
		self.caret:setPosition(self.caretX,self.caretY)
		self.caret:setVisible(true)
	end
    if self.caretListener then self.caretListener(self.caretPos,self.caretX,self.caretY) end
	if changed then
		UI.dispatchEvent(self._textfield,"CaretPosition",self.caretPos,self.caretX,self.caretY)
	end
	local extPos=self:passwordPos(self.caretPos,true)
    setTextInput(self,self.text,extPos,extPos)
  end
end
function TextEdit:goLeft(update,select)
	self:hideLast()
	local t=self:_getText()
	local ocp=self.caretPos
	if self.caretPos>0 and t:sub(self.caretPos,self.caretPos)=="]" then --Skip all escape sequences
		local rt=t:reverse() --reverse the string to find the next ESC
		local sl=#t
		while t:sub(self.caretPos,self.caretPos)=="]" do --Skip all escape sequences
			local _,s=rt:find("[\e",sl-self.caretPos+1,true)
			if not s then break end
			s=sl-s+1
			local _,e=t:find("]",s+3,true)
			if e==self.caretPos then
				self.caretPos=s-1
			else
				break
			end
		end
	end
	while self.caretPos>0 do
		self.caretPos=self.caretPos-1
		local b=t:byte(self.caretPos+1)
		if b==nil or b<128 or b>=192 then break end
	end
	
	if update then 
		self:setCaretPos(self.caretPos)
		self:setSelectionStartEnd(select and ocp,self.caretPos)
	end
end
function TextEdit:goRight(update,select)
	self:hideLast()
	local t=self:_getText()
	local ocp=self.caretPos
	while t:sub(self.caretPos+1,self.caretPos+2)=="\e[" do --Skip all escape sequences
		local _,e=t:find("]",self.caretPos+3,true)
		if e then self.caretPos=e else break end
	end
	while self.caretPos<#t do
		self.caretPos=self.caretPos+1
		local b=t:byte(self.caretPos+1)
		if b==nil or b<128 or b>=192 then break end
	end
	if update then 
		self:setCaretPos(self.caretPos)
		self:setSelectionStartEnd(select and ocp,self.caretPos)
	end
end
function TextEdit:goHome(update,select)
	self:hideLast()
	local ocp=self.caretPos
	self:setCaretPosition(0,self.caretY)
	if update then 
		self:setCaretPos(self.caretPos)
		self:setSelectionStartEnd(select and ocp,self.caretPos)
	end
end
function TextEdit:goEnd(update,select)
	self:hideLast()
	local ocp=self.caretPos
	self:setCaretPosition(self:getWidth(),self.caretY)
	if update then 
		self:setCaretPos(self.caretPos)
		self:setSelectionStartEnd(select and ocp,self.caretPos)
	end
end
function TextEdit:goUp(update,select)
	if self.multiline then
		local ocp=self.caretPos
		self:setCaretPosition(self.caretX,self.caretY-self.font:getLineHeight())
		self:setSelectionStartEnd(select and ocp,self.caretPos)
	end
end
function TextEdit:goDown(update,select)
	if self.multiline then
		local ocp=self.caretPos
		self:setCaretPosition(self.caretX,self.caretY+self.font:getLineHeight())
		self:setSelectionStartEnd(select and ocp,self.caretPos)
	end
end
function TextEdit:validate()
	self:setSelected(0,0)
	self._textfield:unfocus({ TAG="TextEdit",reason="VALIDATE" })
	UI.dispatchEvent(self._textfield,"TextValid",self:getText())
end
function TextEdit:textChanged()
	UI.dispatchEvent(self._textfield,"TextChange",self:getText())
end
function TextEdit:cutSelection()
	if (self.selSpan or 0)>0 then
		local ss=self.selStart
		local sp=self.selSpan
		self:setSelected(0,0)
		self:setCaretPos(ss)
		local t=self:_getText()
		local cutcount=utf8.len(t,ss+1,ss+1+sp)
		t=t:sub(1,ss)..t:sub(ss+sp+1)
		if self.password then
			self:_setText(t)
			local rc=self.caretPos//#self.password
			self.text=utf8.sub(self.text,1,rc)..utf8.sub(self.text,rc+cutcount)
		else
			self:setText(t) 
		end		
		return true
	end
end
function TextEdit:deleteChar()
	if self:cutSelection() then self:textChanged() return end
	local t=self:_getText()
	t=removeCharUtf8(t,self.caretPos+1)
	if self.password then
		self:_setText(t)  
		local p=skipCharsUtf8(self.text,self.caretPos//#self.password)
		self.text=removeCharUtf8(self.text,p)
	else
		self:setText(t) 
	end
	self:setCaretPos(self.caretPos)
	self:textChanged()
end
function TextEdit:hideLast()
  if self.lastVisible and self.password then
	local t=self:_getText()
    if self.lastVisible.s>1 then
      t=t:sub(1,self.lastVisible.s-1)..self.password:rep(self.lastVisible.nc)..t:sub(self.lastVisible.e)   
    else
      t=self.password:rep(self.lastVisible.nc)..t:sub(self.lastVisible.e)    
    end
    self:_setText(t)
    self:setCaretPos(self.lastVisible.s+(#self.password)*self.lastVisible.nc)
    self.lastVisible=nil
  end
end
function TextEdit:addChars(c)
	local np = nil
	self:cutSelection()
	local cp1=self.caretPos
	if self.password then
		self:hideLast()
		local t=self:_getText()
		t,np=addCharsUtf8(t,self.caretPos+1,c)
		self.lastVisible={s=self.caretPos+1,e=np,nc=utf8.len(c),t=os:timer()+HIDE_TIMER} --onTextInput : --on ne gère pas le .lastVisible si password
		self:_setText(t)
		local p=skipCharsUtf8(self.text,self.caretPos//#self.password)
		self.caretPos=np-1
		self.text=addCharsUtf8(self.text,p,c)
	else
		local t=self:_getText()
		t,np=addCharsUtf8(t,self.caretPos+1,c)
		self.caretPos=np-1
		self:setText(t)
	end
	local cp2=self.caretPos
	self:setCaretPos(self.caretPos)	
	self:textChanged()
	return cp1,cp2
end
function TextEdit:backspace()
	if self:cutSelection() then self:textChanged() return end
	local cp=self.caretPos
	self:goLeft(true)
	if cp>self.caretPos then
		self:deleteChar()
	end
end

function TextEdit:onKeyChar(e) --!"SUPPR" (sur MAC pas d'event onKeyChar donc géré dans processKey) (sur PC DELETE (DEL 127) donc on bloque et on gère dans processKey)
  --print("onKeyChar",".keyCode",e.keyCode or "-",".realCode",e.realCode or "-","e.text",e.text or "-","byte(1)",e.text:byte(1),"self.caretPos",self.caretPos)
  if e.text:byte(1)==127 then --nothing --DELETE (DEL 127) --!!on bloque
  elseif e.text=='\b' then --BACK (BS 8)
    --NO --self:backspace() --déjà fait dans processKey keyCode==KeyCode.BACKSPACE
  elseif e.text=='\n' or e.text=='\r' then --(CR 13) --TEMP_TOSEE attention certains claviers sur samsung peuvent doubler la string si correction Ã  la main du texte (mettre clavier par defaut et réinitilisation du clavier pour corriger le bug)
    if not self.multiline then 
      self:validate()
    else
		local dos=application:getDeviceInfo() --TOSEE: On Huawei P20 '\n' isn't inserted by the builtin editor
		if (not self.hasTextInput) or dos=="Android" then 
			self:addChars("\n") 
			if self.hasTextInput then
				self:setCaretPos(self.caretPos,true)
			end
		end
    end
  elseif e.text:byte(1)==9 then --TABULATION (TAB 9)
	if not UI.Focus:focusNext(UI.Focus:focusRoot(self._textfield)) then
		self._textfield:unfocus({ TAG="TextEdit",reason="TABULATION" })
	end
  elseif e.text:byte(1)>=32 then
    --print(e.text:byte(1,#e.text),self.caretPos)
     if not self.hasTextInput then self:addChars(e.text) end
  else
    --print("onKeyChar",e.text.."["..e.text:byte(1).."]")
  end
end
function TextEdit:processKey(keyCode,modifiers) --!"SUPPR" (sur MAC fn+DELETE=KeyCode.DELETE=403 - on ne prend pas en compte la combinaison ctrl+D)
	--print("processKey","keyCode",keyCode,modifiers)
	local ctrl=((modifiers or 0)&15)==KeyCode.MODIFIER_CTRL
	local meta=((modifiers or 0)&15)==KeyCode.MODIFIER_META
	ctrl=ctrl or meta -- for MAC
	local shift=((modifiers or 0)&15)==KeyCode.MODIFIER_SHIFT
	
	if keyCode==KeyCode.LEFT then
		self:goLeft(true,shift)
	elseif keyCode==KeyCode.RIGHT then
		self:goRight(true,shift)
	elseif keyCode==KeyCode.UP then
		self:goUp(true,shift)
	elseif keyCode==KeyCode.DOWN then
		self:goDown(true,shift)
	elseif keyCode==KeyCode.HOME then
		self:goHome(true,shift)
	elseif keyCode==KeyCode.END then
		self:goEnd(true,shift)
	elseif keyCode==KeyCode.DELETE then
		self:deleteChar()
	elseif keyCode==KeyCode.BACKSPACE and self.caretPos>0 then --BACK (BS 8)
		self:backspace()
	elseif keyCode==KeyCode.BACK then --BACK (BS 301)
		self._textfield:unfocus({ TAG="TextEdit",reason="BACK" })
	elseif ctrl and keyCode==KeyCode.A then
		self:setSelected(0,#self:getText())	
	elseif ((ctrl and keyCode==KeyCode.V) or (shift and keyCode==KeyCode.INSERT)) and not self.hasTextInput then
		UI.Clipboard:get("text/plain",function (res,data,mime)
			if res then
				self:addChars(data) 
			end
		end)
	elseif ctrl and (keyCode==KeyCode.INSERT or keyCode==KeyCode.C) and not self.hasTextInput then
		local seltext=self:getSelectedText()
		if seltext then
			UI.Clipboard:set(seltext,"text/plain",function (res) end)
		end
	elseif ctrl and (keyCode==KeyCode.DELETE or keyCode==KeyCode.X) and not self.hasTextInput then
		local seltext=self:getSelectedText()
		if seltext then
			UI.Clipboard:set(seltext,"text/plain",function (res) end)
			self:cutSelection()
			self:textChanged()
		end
	else
		--print("processKey",keyCode)
		return false
	end
	return true
end
function TextEdit:onKeyDown(e)
  self.keyDown=e.keyCode
  self.keyTimer=os:timer()+REPEAT_TIMER
  --print("onKeyDown",self.keyDown)
  return self:processKey(self.keyDown,e.modifiers)
end
function TextEdit:onKeyUp(e)
  --print("onKeyUp",self.keyDown)
  self.keyDown=nil
end
function TextEdit:onTextInput(event)
  if self and event then
	if event.context and event.context~=tostring(self) then return end
	if debugTextInput then print("UI.TextField onTextInput",".text=",event.text,".selectionStart,.selectionEnd=",event.selectionStart,event.selectionEnd) end --event.type="textInput"
    local s 			= event.text or ""
    local indexStart 	= event.selectionStart or 0
	--onTextInput : --on ne gère pas le .lastVisible si password
	self:setText(s)
	self:setCaretPos(self:passwordPos(indexStart,false))
    self:textChanged()
  end
end
function TextEdit:onFrame()
  local t=os.timer()
  local cf=(t*2)&1
  self.caret:setVisible(cf==1)
  if not self.noKeyEvents and self.keyDown then
    if t>self.keyTimer then
      self.keyTimer=t+REPEAT_PERIOD
      self:processKey(self.keyDown)
    end
  end
  if self.lastVisible and os.timer()>self.lastVisible.t then
	self:hideLast()
  end
end
function TextEdit:onRemove()
  self:stopEditing()
end

TextEdit.Definition= {
  name="TextField",
  constructorArgs={  },
  properties={
  },
}

local function initLayout(layout,pass)
  local flags = FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER
  layout = layout or {}
  layout.flags=layout.flags or flags
  if layout and layout.flags then
    if layout.flags&FontBase.TLF_BREAKWORDS==FontBase.TLF_BREAKWORDS then layout.breakChar = layout.breakChar or "…"
    else layout.flags = layout.flags | FontBase.TLF_NOWRAP end
  end
  if layout then layout.password=pass end
  return layout
end

UI.TextField=Core.class(UI.Panel,function (text,layout,minwidth,minheight,pass,textType)
  return nil
end)

UI.TextField.EditorBox={ class=UI.Panel, name="editorBox",layout={gridx=0, fill=Sprite.LAYOUT_FILL_BOTH, }, 
		  layoutModel={ columnWeights={1}, rowWeights={1} },
		  ParentStyleInheritance="global",
		  children={ 
			{ class=Sprite, name="smark" }, 
			{ class=TextEdit, name="editor",layout={fill=Sprite.LAYOUT_FILL_BOTH }}, 
		  },
	}

UI.TextField.EmbeddedEditorBox={
	class="UI.Panel", 
	layout={fill=Sprite.LAYOUT_FILL_BOTH, },
	layoutModel={ columnWeights={1}, rowWeights={1}, insets="textfield.szMargin" },
	children={
		UI.TextField.EditorBox,
	}}
	
UI.TextField.Template={
	class="UI.TextField", 
	layoutModel={ columnWeights={1}, rowWeights={1}, insets="textfield.szMargin" },
	BaseStyle="textfield.styBase",
	children={
		UI.TextField.EditorBox,
	}}

function UI.TextField:init(text,layout,minwidth,minheight,pass,textType,template)
	UI.BuilderSelf(template or self.Template,self)
	self.caretWidth=2
	layout=initLayout(layout,pass)
	self.ominwidth,self.ominheight=minwidth,minheight
	self.width=0
	self.height=0
	self.pass=pass
	self.scrollbarMode={0,0}

	self.editor._textfield=self

	self.editor:setPassword(layout.password)
	self.editor:setMultiline(layout.multiline)
	self.editor:setLayout(layout)
	self:setTextType(textType)

	local scol="textfield.colSelection"
	local smark=self.smark
	local colNone=UI.Colors.transparent
	smark.p1=Pixel.new(colNone,0,0) smark:addChild(smark.p1) smark.p1:setColor(scol) 
	smark.p2=Pixel.new(colNone,0,0) smark:addChild(smark.p2) smark.p2:setColor(scol) 
	smark.p3=Pixel.new(colNone,0,0) smark:addChild(smark.p3) smark.p3:setColor(scol) 
	function smark:setSpan(p1x,p1y,p1w,p1h,p2x,p2y,p2w,p2h,p3x,p3y,p3w,p3h)
		self.p1:setPosition(p1x,p1y) self.p1:setDimensions(p1w,p1h)
		self.p2:setPosition(p2x,p2y) self.p2:setDimensions(p2w,p2h)
		self.p3:setPosition(p3x,p3y) self.p3:setDimensions(p3w,p3h)
		self.p2:setVisible(p2h>0)
		self.p3:setVisible(p3h>0)
	end
	self.editor.selMark=smark
	self.editor.caret=Pixel.new(colNone,0,0)
	self.editor.caretListener=function (p,x,y) 		
		self:ensureVisible(x,y)
	end
	UI.Control.onMouseClick[self]=self
	UI.Control.onDragStart[self]=self
	UI.Control.onDrag[self]=self
	UI.Control.onDragEnd[self]=self
	UI.Control.onMouseWheel[self]=self

	if UI.Control.HAS_CURSOR then 
	UI.Control.onMouseMove[self]=self
	end
	self.editorBox:addEventListener(Event.LAYOUT_RESIZED,function (self,e)
		self:setSize(e.width,e.height)
	end,self)
	self:setText(text)
	self:updateStyle()
	self._flags.focusable=UI.Focus.KEYBOARD
  
	if layout.multiline then
		self:setScrollbar(2)
	end
	return self
end

function UI.TextField:newClone() assert(false,"Cloning not supported") end

function UI.TextField:setSize(w,h)
	self.width=w
	self.height=h
	self.editor:setSelected(self.editor.selStart,self.editor.selSpan)
	self.editor:setCaretPos(self.editor.caretPos)
end

function UI.TextField:onMouseMove(x,y)
	if self.editorBox:hitTestPoint(x,y,true,self) then
		UI.Control.setLocalCursor(self.cursor)
	end
end

function UI.TextField:onMouseClick(x,y,c)
  if not self._flags.disabled and (self._flags.selectable or not self._flags.readonly) then
	local shift=((UI.Control.Meta.modifiers or 0)&15)==KeyCode.MODIFIER_SHIFT
    if not self:focus({ TAG="UI.TextField",reason="ON_CLICK" } ) then	
		local ocp=self.editor.caretPos
		x,y=self.editor:spriteToLocal(self,x,y)
		self.editor:setCaretPosition(x,y,true)
		local function extendSelection(t,cp,ncp,test)
			local limit=260 --Max 260 characters
			-- Look for word start
			while limit>0 do
				limit-=1
				local pcp=utf8.next(t,cp,-1)
				if pcp and test(pcp,cp-1) then
					cp=pcp
				else
					break
				end
			end
			-- Look for word end
			while limit>0 do
				limit-=1
				local pcp=utf8.next(t,ncp,1) or (#t+1)
				if pcp and test(ncp,pcp-1) then
					ncp=pcp
					if ncp>#t then break end
				else
					break
				end
			end
			return cp,ncp
		end
		local cc=c or 0
		if cc>3 or (cc>1 and self.password) then --Full selection
			self.editor:setSelected(0,#self.editor:getText())
		elseif cc==3 then -- Line selection
			local t=self.editor:getText()
			local bcp=self.editor.caretPos --Number of bytes prior to clicked character
			local cp=bcp+1 --next character position (1-based)
			local ncp=utf8.next(t,cp,1) or (#t+1) --position of the character after
			cp,ncp=extendSelection(t,cp,ncp,function (i1,i2)
				local code=utf8.byte(t:sub(i1,i2))
				return not (code==13 or code==10 or code==0)
			end)
			bcp=cp-1
			self.editor:setSelected(bcp,ncp-cp)
		elseif cc==2 then --Word selection
			local t=self.editor:getText()
			local bcp=self.editor.caretPos --Number of bytes prior to clicked character
			local cp=bcp+1 --next character position (1-based)
			local ncp=utf8.next(t,cp,1) or (#t+1) --position of the character after
			local curChar=t:sub(cp,ncp-1)
			local alnum=utf8.find(curChar,"%w")
			if alnum then
				cp,ncp=extendSelection(t,cp,ncp,function (i1,i2)
					return utf8.find(t:sub(i1,i2),"%w")
				end)
				bcp=cp-1
			end
			self.editor:setSelected(bcp,ncp-cp)
		elseif shift then
			self.editor:setSelectionStartEnd(ocp,self.editor.caretPos)
		else
			self.editor:setSelected(0,0)
		end
    end
	return true
  end
end

function UI.TextField:onLongPrepare(ex,ey,ratio)
	if not self.prepare then
		self.prepare=UI.Behavior.LongClick.makeIndicator(self,{})
	end
	self.prepare:indicate(self,ex,ey,ratio)
	if ratio<0 then self.prepare=nil end
	return true
end

function UI.TextField:onLongClick(x,y,c)
  if not self._flags.disabled and (self._flags.selectable or self.editor.editing) then
	local hasSel=self.editor:getSelectedText()
	if hasSel or not self._flags.readonly then
		local popup=UI.Builder({
			class="UI.Panel", 
			Style="textfield.styCutPaste",
			LocalStyle="textfield.styCutPasteBox",
			layoutModel={ 
				rowHeights={ "textfield.szCutPasteButton" }, 
				columnWidths={ },
				cellSpacingX="textfield.szSpacing", },
			children={
				{ class="UI.Image", Image="textfield.icCut", name="btCut", behavior=UI.Behavior.Button, layout={ gridx=0, fill=Sprite.LAYOUT_FILL_BOTH, width="textfield.szCutPasteButton", },},
				{ class="UI.Image", Image="textfield.icCopy", name="btCopy", behavior=UI.Behavior.Button, layout={ gridx=1, fill=Sprite.LAYOUT_FILL_BOTH, width="textfield.szCutPasteButton", },},
				{ class="UI.Image", Image="textfield.icPaste", name="btPaste", behavior=UI.Behavior.Button, layout={ gridx=2, fill=Sprite.LAYOUT_FILL_BOTH, width="textfield.szCutPasteButton", },},
			}
		})
		local popupX,popupY=self.editor:getCaretPosition()
		popupY = popupY - self.editor.caretOffset
		
		if not hasSel then
			popup.btCut:setVisible(false)
			popup.btCopy:setVisible(false)
			popup:setStateStyle("textfield.styCutPasteSingle")
		end
		if self._flags.readonly then
			popup.btPaste:setVisible(false)
			popup.btCut:setVisible(false)
		end
		popup.uitextfield=self
		function popup:onWidgetAction(w)
			if w==self.btCut then
				local seltext=self.uitextfield.editor:getSelectedText()
				if seltext then
					UI.Clipboard:set(seltext,"text/plain",function (res) end)
					self.uitextfield.editor:cutSelection()
					self.uitextfield.editor:textChanged()
				end
			elseif w==self.btCopy then
				local seltext=self.uitextfield.editor:getSelectedText()
				if seltext then
					UI.Clipboard:set(seltext,"text/plain",function (res) end)
				end
			elseif w==self.btPaste then
				UI.Clipboard:get("text/plain",function (res,data,mime)
					if res then
						self.uitextfield.editor:addChars(data) 
					end
				end)
			end
			self.uitextfield:dismissCutPaste()
			return true
		end
		self.cutpastePopup=popup
		UI.Screen.popupAt(self.editorBox,popup,{{x=popupX, y=popupY,dy=-1,dx=0,mvtx=true,mvty=true}})
	end
	return true
  end
end

function UI.TextField:dismissCutPaste()
	if self.cutpastePopup then 
		self.cutpastePopup:removeFromParent()
		self.cutpastePopup=nil
	end
end

function UI.TextField:setScrollbar(mode)
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
					self[sname]:setLayoutConstraints({gridx=1,fill=Sprite.LAYOUT_FILL_BOTH, insetLeft="textfield.szMargin"})
				else
					self[sname]:setLayoutConstraints({gridy=1,fill=Sprite.LAYOUT_FILL_BOTH, insetTop="textfield.szMargin"})
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
	local cx=self:checkRange()
	self:checkRange(cx)
end

function UI.TextField:updateScrollbars(x,y,pw,ph,w,h)
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
	updateScrollBar("sbHoriz",self.scrollbarMode[1],x,w,pw)
	updateScrollBar("sbVert",self.scrollbarMode[2],y,h,ph)
end

function UI.TextField:checkRange(x,y,fromScroll)  
	local X,Y=self.editorBox:getAnchorPosition()
	local PW,PH=self.width,self.height
	local W=self.editor:getWidth()-PW
	local H=self.editor:getHeight()-PH

	if x or y then
		x=x or X
		y=y or Y
		local maX=x<>0
		local maY=y<>0

		local NX=maX><(W)<>0
		local NY=maY><(H)<>0

		self.editorBox:setAnchorPosition(NX,NY)
		self.editorBox:setClip(NX,NY,self.width,self.height)
		if not fromScroll then
			self:updateScrollbars(NX,NY,PW,PH,W,H)
		end
		local changed=NX~=X or NY~=Y
		return changed
	end
	return X,Y,W,H
end

function UI.TextField:setScrollPosition(x,y)
	return self:checkRange(x,y)
end

function UI.TextField:setScrollAmount(x,y)
	local _,_,rw,rh=self:checkRange()
	return self:checkRange(rw*x,rh*y)
end

function UI.TextField:onWidgetChange(w,ratio,page)
	if w and w==self.sbHoriz then
		local _,_,rw,_=self:checkRange()
		self:checkRange(rw*ratio/(1-page),nil,true)
		return true
	elseif w and w==self.sbVert then
		local _,_,_,rh=self:checkRange()
		self:checkRange(nil,rh*ratio/(1-page),true)
		return true
	end
end

function UI.TextField:onMouseWheel(x,y,wheel,distance)
	local cx,cy,_,h=self:checkRange()
	if h>0 then
		self:checkRange(cx,cy-distance)
		return true
	end
	return false
end

function UI.TextField:onDragStart(x,y,ed,ea,change,long)
	local sel=long or UI.Control.isDesktopGesture()
	if not sel then
		local cx,cy=self:checkRange()
		self._dragStart={mx=-x-cx,my=-y-cy}
		return true,1
	elseif not self._flags.disabled and (self._flags.selectable or not self._flags.readonly) then
		self:focus({ TAG="UI.TextField",reason="ON_CLICK" } )
		x,y=self.editor:spriteToLocal(self,x,y)
		self.editor:setCaretPosition(x,y,true)
		self.dragSelStart=self.editor.caretPos
		local shift=((UI.Control.Meta.modifiers or 0)&15)==KeyCode.MODIFIER_SHIFT
		if not shift then
			self.editor:setSelected(0,0,nil,true)
		end
		return true
	end
end

function UI.TextField:onDrag(x,y)
	if self._dragStart then
		local inertia
		if self:checkRange(-x-self._dragStart.mx,-y-self._dragStart.my) then
			inertia=1
		end
		return nil,inertia
	elseif self.dragSelStart then
		x,y=self.editor:spriteToLocal(self,x,y)
		self.editor:setCaretPosition(x,y,true)
		local dcur=self.editor.caretPos
		self.editor:setSelectionStartEnd(self.dragSelStart,dcur,true)
		return true
	end
end

function UI.TextField:onDragEnd(x,y)
	self.dragSelStart=nil
	self._dragStart=nil
end

function UI.TextField:onKeyboardKey(key)
  if not self._flags.disabled and not self._flags.readonly then
	if key=="\b" then
		self.editor:backspace()
	else
		local e={ text=key }
		self.editor:onKeyChar(e)
	end
  end
end

function UI.TextField:onKeyChar(key)
  if not self._flags.disabled and not self._flags.readonly then
	self.editor:onKeyChar({ text=key })
  end
end

function UI.TextField:onKeyUp(kc, rc)
  if not self._flags.disabled and not self._flags.readonly then
	self.editor:onKeyUp({ keyCode=kc, realCode=rc, modifiers=UI.Control.Meta.modifiers })
  end
end

function UI.TextField:onKeyDown(kc, rc)
  if not self._flags.disabled and not self._flags.readonly then
	local modifiers=UI.Control.Meta.modifiers
	if not self.editor:onKeyDown({ keyCode=kc, realCode=rc, modifiers=modifiers }) then
		--[[ If we need to catch more shortcut sequences
		local ctrl=((modifiers or 0)&15)==KeyCode.MODIFIER_CTRL
		local meta=((modifiers or 0)&15)==KeyCode.MODIFIER_META
		ctrl=ctrl or meta -- for MAC
		local shift=((modifiers or 0)&15)==KeyCode.MODIFIER_SHIFT
		if ctrl and kc==KeyCode.H then
		else
			return false
		end]]
	else
		return true
	end
  end
end

function UI.TextField:ensureVisible(x,y)
  local miX=(x+self.caretWidth-self.width)<>0
  local maX=(x)<>0
  local miY=(y-self.editor.caretOffset+self.editor.caretHeight-self.height)<>0
  local maY=(y-self.editor.caretOffset)<>0
  local X,Y=self.editorBox:getAnchorPosition()
  local PW,PH=self.width,self.height
  local W=self.editor:getWidth()-PW
  local H=self.editor:getHeight()-PH
  local NX=(((X<>miX)><maX))><(W)<>0
  local NY=(((Y<>miY)><maY))><(H)<>0
  
  self.editorBox:setAnchorPosition(NX,NY)
  if self.editor.caret and self.editor.caretX and self.editor.caretY then
	self.editor.caret:setPosition(self.editor.caretX,self.editor.caretY) --fix caret position
  end
  self.editorBox:setClip(NX,NY,self.width,self.height)
  self:updateScrollbars(NX,NY,PW,PH,W,H)
  if self.editor.editing then
	UI.Focus:area(self,x,y-self.editor.caretOffset,self.caretWidth,self.editor.caretHeight)
  end
end

function UI.TextField:focus(event)
	if not self._flags.disabled and (self._flags.selectable or not self._flags.readonly) then
		local gained=UI.Focus:request(self)
		if gained then
			UI.dispatchEvent(self,"FocusChange",self:getText(),true,event) --focused
		end
		return gained
	end
end

function UI.TextField:unfocus(event)
	local same=UI.Focus:relinquish(self) or (event and event.TAG=="UI.Focus")
	if same then
		UI.dispatchEvent(self,"FocusChange",self:getText(),false,event) --unfocused
	end
	return same
end

function UI.TextField:setTextType(tt)
  self.editor.textType = tt
end
function UI.TextField:setText(t,fromTip)
	self.editor:setText(t or "")
	self.editor:hideLast()
	self:ensureVisible(0,0)
	if not fromTip then
		self:setCaretPos(#self:getText())
		if self.isTip then
			self.editor:setPassword(self.pass)
			self.isTip=nil
		end
		self:updateTip()
	end
end

function UI.TextField:setTipText(t)
	self.tipText=t
	self:updateTip(true)
end

function UI.TextField:updateTip(changed)
	local tl=#self:getText()
	if (tl==0 or self.isTip) and not self._flags.focused then
		if not self.isTip or changed then
			self.editor:setPassword(nil)
			self:setText(self.tipText,true)
			self.isTip=true
		end
	else
		if self.isTip then
			self.editor:setPassword(self.pass)
			self:setText("",true)
			self.isTip=nil
		end	
	end
	local fgcolor = if self.isTip then "textfield.colTipText" else "textfield.colForeground"
	local fg=UI.Utils.colorVector(fgcolor,self._style)
	self.editor:setTextColor(fg)
	self.editor:setCaretColor(fg)
end

function UI.TextField:getText()
  return (not self.isTip and self.editor:getText()) or ""
end

function UI.TextField:setSelectionStartEnd(ss,se,force)
	return self.editor:setSelectionStartEnd(ss,se,force)
end
function UI.TextField:getSelectionStartEnd()
	return self.editor:getSelectionStartEnd()
end
function UI.TextField:getSelectedText(mode)
	return self.editor:getSelectedText(mode)
end
function UI.TextField:setCaretPos(index,focusing)
	return self.editor:setCaretPos(index,focusing)
end
function UI.TextField:getCaretPos()
	return self.editor.caretPos
end
function UI.TextField:cutSelection()
	return self.editor:cutSelection()
end
function UI.TextField:insertText(text)
	return self.editor:addChars(text)
end

function UI.TextField:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	if changes.disabled or changes.readonly then
		self:unfocus({ TAG="UI.TextField",reason="DISABLE" } )
	end 
	self._flags.focusable=if not self._flags.readonly then UI.Focus.KEYBOARD else 0
	self.cursor=nil
	local s="textfield.styNormal"
	if self._flags.disabled then
		s="textfield.styDisabled"
	elseif self._flags.error then
		s="textfield.styError"
		if self._flags.readonly then
			s=if self._flags.focused then "textfield.styErrorFocusedReadonly" else "textfield.styErrorReadonly"
		elseif self._flags.focused then
			s="textfield.styErrorFocused"
			self.cursor="IBeam"
		else
			self.cursor="IBeam"
		end
	else	
		if self._flags.readonly then
			s=if self._flags.focused then "textfield.styFocusedReadonly" else "textfield.styReadonly"
		elseif self._flags.focused then
			s="textfield.styFocused"
			self.cursor="IBeam"
		else
			self.cursor="IBeam"
		end
	end
	self:setStateStyle(s)
	if changes.focused~=nil then 
		self:updateTip()
		self:dismissCutPaste()
		if changes.focused and not self._flags.disabled and not self._flags.readonly then 
			UI.Control.onLongClick[self]=self
			UI.Control.onLongPrepare[self]=self
			UI.Control.onKeyChar[self]=self
			UI.Control.onKeyDown[self]=self
			UI.Control.onKeyUp[self]=self
			self.editor:startEditing(true)
			self:ensureVisible(self.editor:getCaretPosition())
		else
			UI.Control.onLongClick[self]=nil
			UI.Control.onLongPrepare[self]=nil
			UI.Control.onKeyChar[self]=nil
			UI.Control.onKeyDown[self]=nil
			UI.Control.onKeyUp[self]=nil
			self.editor:stopEditing(true)
		end
	end
end

function UI.TextField:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	if not self.editor then return end
	local lh=self:resolveStyle("1em")
	local lhm=lh
	self.editor:setFont(self:resolveStyle("font"))
	local fgcolor = if self.isTip then "textfield.colTipText" else "textfield.colForeground"
	local fg=UI.Utils.colorVector(fgcolor,self._style)
	self.editor:setTextColor(fg)
	self.editor:setCaretColor(fg)
	self.editor.caretHeight=lh
	if self.editor.caret then
		self.editor.caret:setDimensions(self.caretWidth,lh)
	end
	local minwidth=self:resolveStyle(self.ominwidth or 0)
	local minheight=self:resolveStyle(self.ominheight or lhm)
	self.editorBox:setLayoutConstraints{width=(0)<>minwidth, height=minheight }
end

function UI.TextField:getTextStyles(p,ss,se)
	if ss then p=ss end
	local styles={}
	for n,k,v in self:getText():sub(1,p+1):gmatch("\e%[(!?)([^=%]]+)=?([^%]]*)%]") do 
		if k then
			styles[k]=if n=="!" then nil else v
		end
	end
	local astyles,bstyles
	if se then
		astyles=table.clone(styles)
		bstyles=table.clone(styles)
		for n,k,v in self:getText():sub(p+1,se+1):gmatch("\e%[(!?)([^=%]]+)=?([^%]]*)%]") do 
			if k and n~="!" and ((v and #v>0) or (#k==1)) then -- Reset styles
				styles[k]=v
			end
			bstyles[k]=if n=="!" then nil else v
		end
	end
	return styles,astyles,bstyles
end

function UI.TextField:applyTextStyle(startTag,endTag,value)
	local stag,etag,d=startTag,endTag,value
	local tag=stag
	if tag then
		local function same(t,v)
			if not t or t=="" then t=nil end
			if not v or v=="" then v=nil end
			return v==t
		end
		local sc,sa,sb=self:getTextStyles(self:getCaretPos() or 0,self:getSelectionStartEnd())
		if d then
			if type(d)=="string" and #d>0 then
				stag=stag.."="..d
			end
			local seltext=self:getSelectedText(true)
			if seltext then
				seltext=seltext:gsub("\e%[!?"..tag.."[^%]]*%]","")
				if not same(sa[stag],d) then
					seltext="\e["..stag.."]"..seltext
				end
				if not same(sb[stag],d) then
					if type(d)=="string" and sb[tag] and #sb[tag]>0 then
						etag=etag.."="..sb[tag]
					end
					seltext=seltext.."\e["..etag.."]"
				end
				local ss,se=self:insertText(seltext) 
				self:setSelectionStartEnd(ss,se)
				self:setCaretPos(se-#etag-3)
			elseif not sc[stag] then
				local cp=self:insertText("\e["..stag.."]\e["..etag.."]") 
				self:setCaretPos(cp+#stag+3)
			end
		else
			local seltext=self:getSelectedText(true)
			if seltext then
				seltext=seltext:gsub("\e%[!?"..tag.."[^%]]*%]","")
				if sa[stag] then
					seltext="\e["..etag.."]"..seltext
				end
				if sb[stag] then
					seltext=seltext.."\e["..stag.."]"
				end
				local ss,se=self:insertText(seltext) 
				self:setSelectionStartEnd(ss,se)
				self:setCaretPos(se-#etag-3)
			elseif sc[stag] then
				local cp=self:insertText("\e["..etag.."]\e["..stag.."]") 
				self:setCaretPos(cp+#etag+3)
			end
		end
	end
end

UI.TextField.Definition= {
  name="TextField",
  class="UI.TextField",
  icon="ui/icons/label.png",
  constructorArgs={ "Text","TextLayout","MinWidth","MinHeight","Password","TextType" },
  properties={
    { name="Text", type="string", setter=UI.TextField.setText, getter=UI.TextField.getText },
    { name="Password", type="string" },
    { name="TextType", type="number", setter=UI.TextField.setTextType, },
    { name="TipText", type="string", setter=UI.TextField.setTipText, },
  },
}

UI.ButtonTextFieldCombo=Core.class(UI.TextField,function (text,layout,minwidth,minheight,pass,textType)
  return text,layout,minwidth,minheight,pass,textType,UI.ButtonTextFieldCombo.Template
end)

UI.ButtonTextFieldCombo.Template={
	class="UI.TextField", 
	layoutModel={ columnWeights={1,0}, rowWeights={1} },
	BaseStyle="textfield.styBase",
	children={ 
		UI.TextField.EmbeddedEditorBox,
		{ class="UI.ToggleButton", name="button",layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH },
			Image="buttontextfieldcombo.icButton",
			LocalStyle="buttontextfieldcombo.styButton"
			},
	}}

function UI.ButtonTextFieldCombo:setFlags(changes)
	UI.TextField.setFlags(self,changes)
	UI.ToggleButton.setFlags(self.button,changes)
end

UI.ButtonTextField=Core.class(UI.TextField,function (text,layout,minwidth,minheight,pass,textType)
  return text,layout,minwidth,minheight,pass,textType,UI.ButtonTextField.Template
end)

UI.ButtonTextField.Template={
	class="UI.TextField", 
	layoutModel={ columnWeights={1,0}, rowWeights={1} },
	BaseStyle="textfield.styBase",
	children={ 
		UI.TextField.EmbeddedEditorBox,
		{ class="UI.ToggleButton", name="button",layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH },
			Image="buttontextfield.icButton",
			LocalStyle="buttontextfield.styButton" },
	}}

function UI.ButtonTextField:setFlags(changes)
	UI.TextField.setFlags(self,changes)
	if changes.disabled~=nil then
		self.button:setFlags({disabled=changes.disabled})
	end
end


UI.PasswordField=Core.class(UI.TextField,function (text,layout,minwidth,minheight,pass,textType)
  return text,layout,minwidth,minheight,pass or "•",textType,UI.PasswordField.Template
end)

UI.PasswordField.Template={
	class="UI.TextField", 
	layoutModel={ columnWeights={1,0}, rowWeights={1} },
	BaseStyle="textfield.styBase",
	children={ 
		UI.TextField.EmbeddedEditorBox,
		{ class="UI.ToggleButton", name="button",layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH, insetRight="textfield.szMargin" }, 
			Image="passwordfield.icButton",
			LocalStyle="passwordfield.styButton", },
	}}

function UI.PasswordField:setFlags(changes)
	UI.TextField.setFlags(self,changes)
	if changes.disabled~=nil then
		self.button:setFlags({disabled=changes.disabled})
	end
end

function UI.PasswordField:onWidgetAction(w)
	if w==self.button and not self.isTip then
		if w:getFlags().ticked then
			self.editor:setPassword(nil)
		else
			self.editor:setPassword(self.pass)
		end
	end
end

if not _PRODUCTION then
	print("UI.TextField","--setTextInput --TVARIANT_PASSWORD")
	print("UI.TextField","--onTextInput --on ne gÃ¨re pas le .lastVisible si password")
end
