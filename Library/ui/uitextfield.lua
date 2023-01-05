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
  self.caretColor=vector(0,0,0,1)
  self.password=layout and layout.password
  self.multiline=layout and layout.multiline
  self:setText(text or "")
  if self.setWorldAlign then self:setWorldAlign(true) end
end

function TextEdit:setFont(font)
  self.font=font or Font.getDefault()
  self.caretHeight=self.font:getLineHeight()
  local la=self.font:getAscender()
  local bb=0
  if self.font.getDescender then 
	bb=(self.caretHeight-la-self.font:getDescender())/2
  end
  self.caretOffset=la+bb
  self:_setFont(font)
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
	self.hasTextInput = application:setTextInput(textType,text,index1,index2,nil,nil,nil,tostring(self))--,"label","action","hint","context")
    if debugTextInput then print("UI.TextField setTextInput","self.hasTextInput?",self.hasTextInput,"type=",textType,"text=",text,"index1=",index1,"index2=",index2) end
  end
end
function TextEdit:startEditing() --TEMP_TOSEE attention certains claviers sur samsung peuvent doubler la string si correction à la main du texte (mettre clavier par defaut et réinitilisation du clavier pour corriger le bug)
  if not self.caret then return end
  self.editing=true
  self.keyDown=nil
  self:setCaretPos(#self:_getText(),true)--focusing
  self.caret:removeFromParent()
  self:addChild(self.caret)
  self.caret:setAnchorPosition(0,self.caretOffset)
  self:addEventListener(Event.KEY_CHAR,self.onKeyChar,self)
  self:addEventListener(Event.KEY_DOWN,self.onKeyDown,self)
  self:addEventListener(Event.KEY_UP,self.onKeyUp,self)
  if Event.TEXT_INPUT then
	if debugTextInput then print("UI.TextField addEventListener Event.TEXT_INPUT") end
    self:addEventListener(Event.TEXT_INPUT,self.onTextInput,self)
  end
  self:addEventListener(Event.ENTER_FRAME,self.onFrame,self)
  self:addEventListener(Event.REMOVED_FROM_STAGE,self.onRemove,self)
end
function TextEdit:stopEditing()
  if not self.caret then return end
  self.editing=nil
  self:hideLast()
  self:removeEventListener(Event.KEY_CHAR,self.onKeyChar,self)
  self:removeEventListener(Event.KEY_DOWN,self.onKeyDown,self)
  self:removeEventListener(Event.KEY_UP,self.onKeyUp,self)
  if Event.TEXT_INPUT then
	if debugTextInput then print("UI.TextField removeEventListener Event.TEXT_INPUT") end
    self:removeEventListener(Event.TEXT_INPUT,self.onTextInput,self)
  end
  self:removeEventListener(Event.ENTER_FRAME,self.onFrame,self)
  self:removeEventListener(Event.REMOVED_FROM_STAGE,self.onRemove,self)
  self.caret:removeFromParent()
end
function TextEdit:setSelectionStartEnd(ss,se)
	if not ss then
		self:setSelected(0,0)
		return
	end
	self.selStartEndStart=self.selStartEndStart or ss
	local ps=self.selStartEndStart><se
	local pe=self.selStartEndStart<>se
	self:setSelected(ps,pe-ps,true)
end
function TextEdit:setSelected(ss,sp,keepBounds)
	if not self.selMark then return end
	local tx=self:_getText()
	ss=((ss or 0)<>0)><#tx
	sp=((sp or 0)<>0)><(#tx-ss)
	self.selStart=ss
	self.selSpan=sp
	if not keepBounds then self.selStartEndStart=nil end
	if self.editing and sp>0 then
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
end
function TextEdit:getSelectedText()
	if (self.selSpan or 0)>0 then
		--We don't care about password case, since passwords should never be copyable anyway
		local t=self:_getText()
		return t:sub(self.selStart+1,self.selStart+self.selSpan)
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
    self.caretPos=index
    local cx,cy=self:getPointFromTextPosition(index)
    self.caretX=cx
    self.caretY=cy
    self.caret:setPosition(self.caretX,self.caretY)
    self.caret:setVisible(true)
    if self.caretListener then self.caretListener(self.caretPos,self.caretX,self.caretY) end
    if focusing then 
		index=self:passwordPos(index,true)
		setTextInput(self,self.text,index,index) 
	end
  end
end
function TextEdit:getCaretPosition()
  return (self and self.caret) and self.caret:getPosition()
end
function TextEdit:setCaretPosition(cx,cy)
  if self and self.caret then
    self:hideLast()
	if not self.multiline then cy=self.font:getLineHeight()/2 end
    local ti,mx,my=self:getTextPositionFromPoint(cx,cy)
    self.caretPos=ti
    self.caretX=mx
    self.caretY=my
    self.caret:setPosition(self.caretX,self.caretY)
    self.caret:setVisible(true)
    if self.caretListener then self.caretListener(self.caretPos,self.caretX,self.caretY) end
	local extPos=self:passwordPos(self.caretPos,true)
    setTextInput(self,self.text,extPos,extPos)
  end
end
function TextEdit:goLeft(update,select)
	self:hideLast()
	local t=self:_getText()
	local ocp=self.caretPos
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
		local cutcount=utf8.len(t,ss,ss+sp)
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
	self:setCaretPos(self.caretPos)
	self:textChanged()
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
	local ctrl=((modifiers or 0)&7)==KeyCode.MODIFIER_CTRL
	local shift=((modifiers or 0)&7)==KeyCode.MODIFIER_SHIFT
	if keyCode==KeyCode.LEFT then
		self:goLeft(true,shift)
	elseif keyCode==KeyCode.RIGHT then
		self:goRight(true,shift)
	elseif keyCode==KeyCode.UP then
		self:goUp(true,shift)
	elseif keyCode==KeyCode.DOWN then
		self:goDown(true,shift)
	elseif keyCode==KeyCode.DELETE then
		self:deleteChar()
	elseif keyCode==KeyCode.BACKSPACE and self.caretPos>0 then --BACK (BS 8)
		self:backspace()
	elseif keyCode==KeyCode.BACK then --BACK (BS 301)
		self._textfield:unfocus({ TAG="TextEdit",reason="BACK" })
	elseif ctrl and keyCode==KeyCode.V and not self.hasTextInput then
		application:getClipboard("text/plain",function (res,data,mime)
			if res then
				self:addChars(data) 
			end
		end)
	elseif ctrl and keyCode==KeyCode.C and not self.hasTextInput then
		local seltext=self:getSelectedText()
		if seltext then
			application:setClipboard(seltext,"text/plain",function (res) end)
		end
	elseif ctrl and keyCode==KeyCode.X and not self.hasTextInput then
		local seltext=self:getSelectedText()
		if seltext then
			application:setClipboard(seltext,"text/plain",function (res) end)
			self:cutSelection()
			self:textChanged()
		end
	else
	--print("processKey",keyCode)
	end
end
function TextEdit:onKeyDown(e)
  self.keyDown=e.keyCode
  self.keyTimer=os:timer()+REPEAT_TIMER
  --print("onKeyDown",self.keyDown)
  self:processKey(self.keyDown,e.modifiers)
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
  if self.keyDown then
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

local MARGIN=0
UI.TextField.EditorBox={ class=UI.Panel, name="editorBox",layout={gridx=0, fill=Sprite.LAYOUT_FILL_BOTH }, 
		  layoutModel={ columnWeights={1}, rowWeights={1} },
		  ParentStyleInheritance="global",
		  children={ 
			{ class=Sprite, name="smark",layout={fill=Sprite.LAYOUT_FILL_BOTH }}, 
			{ class=TextEdit, name="editor",layout={fill=Sprite.LAYOUT_FILL_BOTH }}, 
		  },
	}
UI.TextField.Template={
	class="UI.TextField", 
	layoutModel={ columnWeights={1}, rowWeights={1} },
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
  
  self.editor._textfield=self
  
  self.editor:setPassword(layout.password)
  self.editor:setMultiline(layout.multiline)
  self.editor:setLayout(layout)
  self:setTextType(textType)
  
  local scol="textfield.colSelection"
  local smark=self.smark
  smark.p1=Pixel.new(vector(0,0,0,0),0,0) smark:addChild(smark.p1) smark.p1:setColor(scol) 
  smark.p2=Pixel.new(vector(0,0,0,0),0,0) smark:addChild(smark.p2) smark.p1:setColor(scol) 
  smark.p3=Pixel.new(vector(0,0,0,0),0,0) smark:addChild(smark.p3) smark.p1:setColor(scol) 
  function smark:setSpan(p1x,p1y,p1w,p1h,p2x,p2y,p2w,p2h,p3x,p3y,p3w,p3h)
	  self.p1:setPosition(p1x,p1y) self.p1:setDimensions(p1w,p1h)
	  self.p2:setPosition(p2x,p2y) self.p2:setDimensions(p2w,p2h)
	  self.p3:setPosition(p3x,p3y) self.p3:setDimensions(p3w,p3h)
	  self.p2:setVisible(p2h>0)
	  self.p3:setVisible(p3h>0)
  end
  self.editor.selMark=smark
  self.editor.caret=Pixel.new(vector(0,0,0,0),0,0)
  self.editor.caretListener=function (p,x,y) self:ensureVisible(x,y) end
  UI.Control.onMouseClick[self]=self
  UI.Control.onDragStart[self]=self
  UI.Control.onDrag[self]=self
  UI.Control.onDragEnd[self]=self

  if UI.Control.HAS_CURSOR then UI.Control.onMouseMove[self]=self end
  self.editorBox:addEventListener(Event.LAYOUT_RESIZED,function (self,e)
	  self:setSize(e.width,e.height)
  end,self)
  self:setText(text)
  self:updateStyle()
  self._flags.focusable=UI.Focus.KEYBOARD
  return self
end

function UI.TextField:newClone() assert(false,"Cloning not supported") end

function UI.TextField:setSize(w,h)
  self.width=w
  self.height=h
  self.editor:setCaretPos(self.editor.caretPos)
end

function UI.TextField:onMouseMove(x,y)
	UI.Control.setLocalCursor(self.cursor)
end

function UI.TextField:onMouseClick(x,y,c)
  if not self._flags.disabled and not self._flags.readonly then
	local shift=((UI.Control.Meta.modifiers or 0)&7)==KeyCode.MODIFIER_SHIFT
    if not self:focus({ TAG="UI.TextField",reason="ON_CLICK" } ) then	
		local ocp=self.editor.caretPos
		self.editor:setCaretPosition(self.editor:globalToLocal(self:localToGlobal(x,y)))
		if c>1 then
			self.editor:setSelected(0,#self.editor:getText())
		elseif shift then
			self.editor:setSelectionStartEnd(ocp,self.editor.caretPos)
		else
			self.editor:setSelected(0,0)
		end
    end
	return true
  end
end

function UI.TextField:onDragStart(x,y)
	if not self._flags.disabled and not self._flags.readonly then
		self:focus({ TAG="UI.TextField",reason="ON_CLICK" } )
		self.editor:setCaretPosition(self.editor:globalToLocal(self:localToGlobal(x,y)))
		self.dragSelStart=self.editor.caretPos
		local shift=((UI.Control.Meta.modifiers or 0)&7)==KeyCode.MODIFIER_SHIFT
		if not shift then
			self.editor:setSelected(0,0)
		end
		return true
	end
end

function UI.TextField:onDrag(x,y)
	if self.dragSelStart then
		self.editor:setCaretPosition(self.editor:globalToLocal(self:localToGlobal(x,y)))
		local dcur=self.editor.caretPos
		self.editor:setSelectionStartEnd(self.dragSelStart,dcur)
		return true
	end
end

function UI.TextField:onDragEnd(x,y)
	self.dragSelStart=nil
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

function UI.TextField:ensureVisible(x,y)
  local miX=(x+self.caretWidth-self.width+MARGIN*2)<>0
  local maX=(x-MARGIN)<>0
  local X=self.editorBox:getAnchorPosition()
  local W=self.editor:getWidth()+self.caretWidth-(self.width-MARGIN*2)
  local NX=(((X<>miX)><maX))><(W)<>0
  self.editorBox:setAnchorPosition(NX,0)
  if self.editor.caret and self.editor.caretX then
	self.editor.caret:setPosition(self.editor.caretX,self.editor.caretY) --fix caret position
  end
  self.editorBox:setClip(NX-MARGIN/2,-MARGIN/2,self.width-MARGIN,self.height-MARGIN)
  if self.editor.editing then
	UI.Focus:area(self,x,y-self.editor.caretOffset,1,self:resolveStyle("1em"))
  end
end

function UI.TextField:focus(event)
  if not self._flags.disabled and not self._flags.readonly then
    local gained=UI.Focus:request(self)
    if gained then
      UI.dispatchEvent(self,"FocusChange",self:getText(),true,event) --focused
    end
    return gained
  end
end

function UI.TextField:unfocus(event)
  local same=UI.Focus:relinquish(self)
  if same then
	self.editor:setSelected(0,0)
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
		if self.isTip then
			self.editor:setPassword(self.pass)
			self.isTip=nil
			self:updateTip()
		end
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

function UI.TextField:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	if changes.disabled or changes.readonly then
		self:unfocus({ TAG="UI.TextField",reason="DISABLE" } )
	end 
	self.cursor=nil
	local s="textfield.styNormal"
	if self._flags.disabled then
		s="textfield.styDisabled"
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
		if changes.focused and not self._flags.disabled and not self._flags.readonly then 
			self.editor:startEditing()
		else
			self.editor:stopEditing()
		end
	end
end

function UI.TextField:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	if not self.editor then return end
	local lh=self:resolveStyle("1em")
	local lhm=lh+MARGIN*2
	self.editor:setFont(self:resolveStyle("font"))
	local fgcolor = if self.isTip then "textfield.colTipText" else "textfield.colForeground"
	local fg=UI.Utils.colorVector(fgcolor,self._style)
	self.editor:setTextColor(fg)
	self.editor:setCaretColor(fg)
	self.editor.caretHeight=lh
	self.editor.caret:setDimensions(self.caretWidth,lh)
	local minwidth=self:resolveStyle(self.ominwidth or 0)
	local minheight=self:resolveStyle(self.ominheight or lhm)
	self.editorBox:setLayoutConstraints{width=(MARGIN*2)<>minwidth, height=minheight,insets=self:resolveStyle("textfield.szMargin") }
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
		UI.TextField.EditorBox,
		{ class="UI.ToggleButton", name="button",layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH },
			LocalStyle="buttontextfieldcombo.styButton" },
	}}

function UI.ButtonTextFieldCombo:updateStyle(...)
	UI.TextField.updateStyle(self,...)
	self.button:setImage(self:resolveStyle("buttontextfieldcombo.icButton"))
end

function UI.ButtonTextFieldCombo:setFlags(changes)
	UI.TextField.setFlags(self,changes)
	UI.ToggleButton.setFlags(self.button,changes)
	local fl=self:getFlags()
	local s="buttontextfieldcombo.styButton"
	if fl.disabled then
		s="buttontextfieldcombo.styButtonDisabled"
	end
	self.button:setStateStyle(s)
end

UI.ButtonTextField=Core.class(UI.TextField,function (text,layout,minwidth,minheight,pass,textType)
  return text,layout,minwidth,minheight,pass,textType,UI.ButtonTextField.Template
end)

UI.ButtonTextField.Template={
	class="UI.TextField", 
	layoutModel={ columnWeights={1,0}, rowWeights={1} },
	BaseStyle="textfield.styBase",
	children={ 
		UI.TextField.EditorBox,
		{ class="UI.ToggleButton", name="button",layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH },
			LocalStyle="buttontextfield.styButton" },
	}}

function UI.ButtonTextField:updateStyle(...)
	UI.TextField.updateStyle(self,...)
	self.button:setImage(self:resolveStyle("buttontextfield.icButton"))
	local fl=self:getFlags()
	local s="buttontextfield.styButton"
	if fl.disabled then
		s="buttontextfield.styButtonDisabled"
	end
	self.button:setStateStyle(s)
end

UI.PasswordField=Core.class(UI.TextField,function (text,layout,minwidth,minheight,pass,textType)
  return text,layout,minwidth,minheight,pass or "•",textType,UI.PasswordField.Template
end)

UI.PasswordField.Template={
	class="UI.TextField", 
	layoutModel={ columnWeights={1,0}, rowWeights={1} },
	BaseStyle="textfield.styBase",
	children={ 
		UI.TextField.EditorBox,
		{ class="UI.ToggleButton", name="button",layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH, insetRight="textfield.szMargin" }, 
			LocalStyle="passwordfield.styButton", },
	}}

function UI.PasswordField:updateStyle(...)
	UI.TextField.updateStyle(self,...)
	self.button:setImage(self:resolveStyle("passwordfield.icButton"))
	local fl=self:getFlags()
	local s="passwordfield.styButton"
	if fl.disabled then
		s="passwordfield.styButtonDisabled"
	end
	self.button:setStateStyle(s)
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
