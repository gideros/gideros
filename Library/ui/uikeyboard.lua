--!NEEDS:uipanel.lua
UI.Keyboard=Core.class(UI.Panel,function() return end)

local layout_fr={
	l1={
		B="azertyuiop",
		S="AZERTYUIOP",
		N="1234567890",
		G="#°()éèçàù|",
	},
	l2={
		B="qsdfghjklm",
		S="QSDFGHJKLM",
		N="+-*/=#@°%,",
		G="~^¨£€&\"@µ\\",
	},
	l3={
		B="wxcvbn'",
		S="WXCVBN'",
		N="({[]})²",
		G=".,;:?!_"
	},
	l4={
		B=" ",
		S=" ",
		N=" ",
		G=" ",
	}
}

local platform,system = application:getDeviceInfo()
platform=platform:lower()
system=(system or ""):lower()
UI.Keyboard.NATIVE=not Oculus --Try first platform virtual keyboard
UI.Keyboard.PHYSICAL=
	if platform=="android" then false
	elseif platform=="ios" then false
	elseif platform=="web" then if system:find("android") or system:find("iphone") or system:find("ipad") then false else true
	else true
UI.Keyboard.VIRTUAL=true

UI.Keyboard.Template={
	class="UI.Panel",
	BaseStyle="keyboard.styBase",	
	layoutModel={ columnWeights={1}, rowWeights={1,1,1,1}, rowHeights={"keyboard.szLineHeight","keyboard.szLineHeight","keyboard.szLineHeight","keyboard.szLineHeight"}, cellSpacingY="keyboard.szSpacing", insets="keyboard.szMargin", equalizeCells=true },
	children={
		{	class="UI.Panel", name="keyL1", Style="keyboard.styKeys",
				layoutModel={ cellSpacingX="keyboard.szSpacing", equalizeCells=true },
				layout={ fill=1, gridy=0 },
		},
		{	class="UI.Panel", name="keyL2", Style="keyboard.styKeys",
				layoutModel={ cellSpacingX="keyboard.szSpacing", equalizeCells=true },
				layout={ fill=1, gridy=1 },
		},
		{ class="UI.Panel",
			layoutModel={ columnWeights={0,1,0}, rowWeights={1}, columnWidths={"keyboard.szSpecKey",0,"keyboard.szSpecKey"}, cellSpacingX="keyboard.szSpacing", equalizeCells=true },
			layout={ fill=1, gridy=2 },
			children={
				{ class="UI.Button", Image="keyboard.icShift", name="btShift", Style="keyboard.stySpecKeys", layout={gridx=0, fill=1} },
				{	class="UI.Panel", name="keyL3", Style="keyboard.styKeys",
					layoutModel={ cellSpacingX="keyboard.szSpacing", equalizeCells=true },
					layout={ fill=1, gridx=1 },
				},
				{ class="UI.Button", Image="keyboard.icBS", name="btBackspace", Style="keyboard.stySpecKeys", BehaviorParams={ NoDoubleClick=true, Repeat=.1 }, layout={gridx=2, fill=1} },
			}},
		{ class="UI.Panel",
			layoutModel={ columnWeights={0,0,1,0}, rowWeights={1}, columnWidths={"keyboard.szSpecKey","keyboard.szSpecKey",0,"keyboard.szEnterKey"}, cellSpacingX="keyboard.szSpacing", equalizeCells=true },
			layout={ fill=1, gridy=3 },
			children={
				{ class="UI.Button", Text="123", name="btNum", Style="keyboard.stySpecKeys", layout={gridx=0, fill=1} },
				{ class="UI.Button", Image="keyboard.icHide", name="btHide", Style="keyboard.stySpecKeys", layout={gridx=1, fill=1} },
				{	class="UI.Panel", name="keyL4", Style="keyboard.styKeys",
					layoutModel={ cellSpacingX="keyboard.szSpacing", equalizeCells=true },
					layout={ fill=1, gridx=2 },
				},
				{ class="UI.Button", Image="keyboard.icOK", name="btOK", Style="keyboard.stySpecKeys", layout={gridx=3, fill=1} },
			}},
	}
}

function UI.Keyboard:init()
	UI.BuilderSelf(UI.Keyboard.Template,self)
	UI.Control.onMouseClick[self]=self
	self.layout=layout_fr
	self:setMode("B")
end

function UI.Keyboard:setTarget(widget)
	self.target=widget
end

function UI.Keyboard:setMode(mode)
	for k=1,4 do
		local ll=self.layout["l"..k][mode]
		local tl=self["keyL"..k]
		local tlc=tl:getNumChildren()
		for cn=1,tlc do tl:removeChildAt(1) end
		local nk=utf8.len(ll)
		local lp={ columnWeights={}, rowWeights={1}, columnWidths={}, equalizeCells=true }
		for i=1,nk do
			lp.columnWeights[i]=1
			lp.columnWidths[i]="keyboard.szKey"
			local kb=UI.Button.new({ NoDoubleClick=true, Repeat=.1 })
			local kval=utf8.sub(ll,i,i)
			kb.key=kval
			kb:setText(kval)
			kb:setLayoutConstraints({ gridx=i-1, fill=1})
			tl:addChild(kb)
		end
		tl:setLayoutParameters(lp)
	end
	self.mode=mode
	if mode=="B" then
		self.btShift:setImage("keyboard.icShift")
		self.btShift:setText(nil)
		self.btNum:setText("123")
	elseif mode=="S" then
		self.btShift:setImage(if self.capslock then "keyboard.icCaps" else "keyboard.icUnShift")
		self.btShift:setText(nil)
		self.btNum:setText("123")
	elseif mode=="N" then
		self.btShift:setImage(nil)
		self.btShift:setText("#&!")
		self.btNum:setText("ABC")
	else --G
		self.btShift:setImage(nil)
		self.btShift:setText("123")
		self.btNum:setText("ABC")
	end	
end

function UI.Keyboard:onWidgetAction(w,c)
	local mode=self.mode
	if w==self.btShift then
		if c==2 then --Caps lock
			self.capslock=true
			self:setMode("S")
		else
			if mode=="B" then
				self:setMode("S")
			elseif mode=="S" then
				self.capslock=nil
				self:setMode("B")
			elseif mode=="N" then
				self:setMode("G")
			else --G
				self:setMode("N")
			end
		end
	elseif w==self.btNum then
		if mode=="B" then
			self:setMode("N")
		elseif mode=="S" then
			self:setMode("N")
		elseif mode=="N" then
			self:setMode("B")
		else --G
			self:setMode("B")
		end
	elseif w==self.btBackspace then self:emitKey("\b")
	elseif w==self.btOK then self:emitKey("\n")
	elseif w==self.btHide then UI.Focus:clear()
	else self:emitKey(w.key)
		if mode=="S" and not self.capslock then
			self:setMode("B")
		end
	end
end

function UI.Keyboard:onMouseClick()
	return true -- capture to prevent unfocus
end


function UI.Keyboard:emitKey(s)
	if not self.target or not self.target.onKeyboardKey then return end
	self.target:onKeyboardKey(s)
end

function UI.Keyboard.setKeyboardVisibility(ui)
	if ui then
		if UI.Keyboard.NATIVE and application:setKeyboardVisibility(true) then 
			return .5 --assume screen covers 50% of the screen
		elseif UI.Keyboard.PHYSICAL then
			-- Nothing, a physical keyboard is expected
			return 0
		elseif UI.Keyboard.VIRTUAL and not UI.Keyboard.INSTANCE then
			-- Out virtual keyboard, if not already displayed
			UI.Keyboard.INSTANCE=UI.Keyboard.new()
			UI.Keyboard.INSTANCE:setTarget(ui)
			UI.Keyboard.INSTANCE:setLayoutConstraints({ fillx=1, filly=0, anchory=1})
			UI.Screen.getScreenUI(ui):addChild(UI.Keyboard.INSTANCE)
			local _,sh=UI.Screen.getScreen(ui):getDimensions()
			local sz=UI.Keyboard.INSTANCE:getLayoutInfo()
			return sz.reqHeight/sh
		end
	else
		application:setKeyboardVisibility(false)
		if UI.Keyboard.INSTANCE then
			UI.Keyboard.INSTANCE:removeFromParent()
			UI.Keyboard.INSTANCE=nil
		end
	end
end

function UI.Keyboard.setKeyboardTarget(ui)
	local kinst=UI.Keyboard.INSTANCE
	if kinst then
		kinst:setTarget(ui)
	end
end
