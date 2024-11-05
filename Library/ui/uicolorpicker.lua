--!NEEDS:uipanel.lua
--!NEEDS:uishaders.lua

UI.ColorRangeView=Core.class(UI.Image)

UI.ColorRangeView.ShaderCache={}

UI.ColorRangeView.Spiral=Core.class(UI.Shader)
function UI.ColorRangeView.Spiral:init(params)
	self.shader=self.overrideStandardShader(UI.Shader.Grayscale,Shader.SHADER_PROGRAM_TEXTURE,0,function(spec)
			function spec.fragmentShader() : Shader
				--[[
				local p=4
				local tx=(fTexCoord.x*p)%1
				local ty=(fTexCoord.y*p)%1
				local td=floor(fTexCoord*p)
				local tm=td.x/(p*p)+td.y/p
				]]
				--local td=floor(fTexCoord*2)
				--local tm=td.x/4+td.y/2
				local fr=0.6
				local sr=1
				local lr=1.4
				local td=fTexCoord*2-1
				local h=0.5+atan2(td.x,td.y)/6.28
				local tr=(td.x*td.x+td.y*td.y)^0.5
				
				local s=hF1(if tr<fr then 1 else (sr-tr)/(sr-fr))<>0
				local v=((tr/fr)><1)-(((tr-sr)/(lr-sr))<>0)
				local hsv=hF3(h,s,v)
				
				local K = hF4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0)
				local p = abs(fract(hsv.xxx + K.xyz) * 6.0 - K.www)
				local rgb = hsv.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), hsv.y)
	
				--local fc=t.r*0.4+t.g*0.5+t.b*0.1
				local frag=hF4(rgb.xyz,1)
				frag*=fColor
				if (frag.a==0.0) then discard() end
				return lF4(frag)
			end
		end)
	function self:toColor(pt)
		local fr=0.6
		local sr=1
		local lr=1.4
		local td=pt*2-vector(1,1)
		local h=0.5+math.atan2(td.x,td.y)/6.28
		local tr=(td.x*td.x+td.y*td.y)^0.5
		
		local s=(if tr<fr then 1 else (sr-tr)/(sr-fr))<>0
		local v=((tr/fr)><1)-(((tr-sr)/(lr-sr))<>0)
		
		local px=((math.abs((h%1)*6-3)-1)<>0)><1
		local py=((math.abs(((h+2/3)%1)*6-3)-1)<>0)><1
		local pz=((math.abs(((h+1/3)%1)*6-3)-1)<>0)><1
		local rgb=v*vector((1-s)+px*s,(1-s)+py*s,(1-s)+pz*s)
		return UI.Color(rgb.x,rgb.y,rgb.z,1)
	end
end

function UI.ColorRangeView:setRangeMode(sclass)
	local s=UI.ColorRangeView.ShaderCache[sclass]
	if not s then
		s=sclass.new()
		UI.ColorRangeView.ShaderCache[sclass]=s
	end
	--self:setShaderSpec(s,self)
	s:apply(self)
	self.colorShader=s
end

function UI.ColorRangeView:selectColorAt(x,y)
	if not self.colorShader then return UI.Color(0,0,0,1) end
	local w,h=self:getDimensions()
	x=(x<>0)><w
	y=(y<>0)><h
	self.imSelect:setAnchorPosition(w/2-x,h/2-y)
	self.imSelect:setVisible(true)
	return self.colorShader:toColor(vector(x/w,y/h))
end

function UI.ColorRangeView:clearSelection()
	self.imSelect:setVisible(false)
end

function UI.ColorRangeView:init()
	self:setRangeMode(UI.ColorRangeView.Spiral)
	UI.ColorRangeView.DummyTexture=UI.ColorRangeView.DummyTexture or Texture.new(null,16,16)
	self:setImage(UI.ColorRangeView.DummyTexture)
	self:setLayoutParameters({})
	self.imSelect=UI.Image.new()
	self.imSelect:setImage("colorpicker.icCrosshair")
	self.imSelect:setLayoutConstraints({width="colorpicker.szCrosshair",height="colorpicker.szCrosshair"})
	self.imSelect:setVisible(false)
	self:addChild(self.imSelect)
end

UI.ColorPicker=Core.class(UI.Panel,function () return nil end)

UI.ColorPicker.Template={
	class="UI.ColorPicker", 
	layoutModel={ cellSpacingX="colorpicker.szCellSpacing", cellSpacingY="colorpicker.szCellSpacing",insets="colorpicker.szCellSpacing",columnWeights={0,0,0}, rowWeights={0,0}, },
	LocalStyle="colorpicker.styBase",
	children={
		{ class="UI.ColorRangeView", name="pnPicker", layout={gridx=0, gridy=0, width="colorpicker.szColorRange", height="colorpicker.szColorRange", fill=Sprite.LAYOUT_FILL_BOTH },

		},
		{ class="UI.Panel", name="pnColor", Color=0, layout={gridx=0, gridy=1, height="colorpicker.szColorSample", fill=Sprite.LAYOUT_FILL_BOTH },

		},
		{ class="UI.Panel", name="pnHisto", layout={gridx=2, gridy=0, fill=Sprite.LAYOUT_FILL_BOTH },
			layoutModel={ cellSpacingX="colorpicker.szCellSpacing", cellSpacingY="colorpicker.szCellSpacing", },
		},
		{ class="UI.Panel", name="pnButtons", layout={gridx=2, gridy=1, fill=Sprite.LAYOUT_FILL_BOTH },
			layoutModel={ cellSpacingX="colorpicker.szCellSpacing", rowWeights={1}, columnWeights={1,1} },
			children={
				{ class="UI.Button", name="btValidate", layout={gridx=0,width="colorpicker.szButtonWidth", fill=Sprite.LAYOUT_FILL_BOTH },
					Image="colorpicker.icValidate",
				},
				{ class="UI.Button", name="btCancel", layout={gridx=1,width="colorpicker.szButtonWidth", fill=Sprite.LAYOUT_FILL_BOTH },
					Image="colorpicker.icCancel",
				},
			}
		},
	}}
	

function UI.ColorPicker:init(buttons)
	self.defaultColor=UI.Colors.white
	UI.BuilderSelf(UI.ColorPicker.Template,self)
	function self.pnPicker.onMouseDown(s,x,y)
		self:pickColorAt(x,y)
		return true
	end
	function self.pnPicker.onMouseMove(s,x,y,b)
		if b then 
			self:pickColorAt(x,y)
		end
		return true
	end
	UI.Control.onMouseDown[self.pnPicker]=self.pnPicker
	UI.Control.onMouseMove[self.pnPicker]=self.pnPicker
	
	local hdim=vector(3,if buttons then 4 else 5)
	local hcw={}
	local hch={}
	for i=1,hdim.x do hcw[i]="colorpicker.szHistoWidth" end
	for i=1,hdim.y do hch[i]="colorpicker.szHistoHeight" end
	self.pnHisto:setLayoutParameters({rowHeights=hch, columnWidths=hcw})
	self.history={}
	self.historyHolder={}
	for hy=1,hdim.y do
		for hx=1,hdim.x do
			local cb=UI.Panel.new(0xFFFFFF)
			cb:setLayoutConstraints({ gridx=hx-1, gridy=hy-1, fill=Sprite.LAYOUT_FILL_BOTH})
			self.pnHisto:addChild(cb)
			self.historyHolder[#self.historyHolder+1]=cb
			cb.histoNum=#self.historyHolder
			UI.Behavior.Button.new(cb)
		end
	end
	self.color=self.defaultColor
	self.originalColor=self.color
	self.pnColor:setColor(self.color)
	
	if not buttons then
		self.pnHisto:setLayoutConstraints({gridheight=2})
		self.pnButtons:setVisible(false)
	end
end

function UI.ColorPicker:pickColorAt(x,y)
	local c=self.pnPicker:selectColorAt(x,y)
	self.colorHisto=nil
	self.color=c
	self.pnColor:setColor(c)
	UI.dispatchEvent(self,"WidgetChange",c)
end

function UI.ColorPicker:onWidgetAction(w)
	if w==self.btValidate then
		if self.colorHisto then
			table.remove(self.history,self.colorHisto)
		end
		table.insert(self.history,1,self.color)
		self:setHistory(self.history)
		UI.dispatchEvent(self,"WidgetAction",self.color,true)
		return true
	elseif w==self.btCancel then
		self:setColor(self.originalColor)
		UI.dispatchEvent(self,"WidgetAction",self.color,false)
		return true
	elseif w.histoNum then		
		self.colorHisto=w.histoNum
		local c=self.history[w.histoNum] or self.colorDefault
		self.color=c
		self.pnColor:setColor(c)
		self.pnPicker:clearSelection()
		UI.dispatchEvent(self,"WidgetChange",c)
		return true
	end
end

function UI.ColorPicker:setColor(c)
	self.color=c
	self.originalColor=c
	self.pnColor:setColor(c)
	self.colorHisto=nil
	self.pnPicker:clearSelection()
end


function UI.ColorPicker:getColor()
	return self.color
end

function UI.ColorPicker:setHistory(clist)
	self.history={}
	for i,h in ipairs(self.historyHolder) do
		local c=clist[i] or self.defaultColor
		h:setColor(c)
		self.history[i]=c
	end
end


function UI.ColorPicker:getHistory()
	return self.history
end

UI.ColorPicker.Definition= {
	name="ColorPicker",
	class="UI.ColorPicker",
	icon="ui/icons/label.png",
	constructorArgs={ "Buttons" },
	properties={
		{ name="Buttons", type="boolean" },
		{ name="Color", type="color" },
	},
}

UI.ButtonColorCombo=Core.class(UI.Panel,function () return "colTesting" end)

UI.ButtonColorCombo.Template={
	class="UI.Panel", 
	layoutModel={ columnWeights={1,0}, rowWeights={1} },
	BaseStyle="textfield.styBase",
	children={ 
		{ class="UI.Panel", layoutModel={ columnWeights={1}, rowWeights={1} },
			LocalStyle="colorpicker.styColorBox",
			layout={ fill=Sprite.LAYOUT_FILL_BOTH, insets="colorpicker.szColorBoxInset" },
			children={ 
				{ class="UI.Panel", name="pnColor", layout={  fill=Sprite.LAYOUT_FILL_BOTH }},
			},
		},
		{ class="UI.ToggleButton", name="button",layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH },
			Image="colorpicker.icPicker",
			LocalStyle="buttontextfieldcombo.styButton"
			},
	}}

function UI.ButtonColorCombo:setFlags(changes)
	--UI.TextField.setFlags(self,changes)
	UI.ToggleButton.setFlags(self.button,changes)
end

function UI.ButtonColorCombo:init()
	UI.BuilderSelf(UI.ButtonColorCombo.Template,self)
end

--- Color picker
UI.ColorPickerBox=Core.class(UI.PopupButton,function () return nil end)
UI.ColorPickerBox.Template={
	class="UI.PopupButton",	
	BaseStyle="combobox.styBase",
	children={
		{ class="UI.ButtonColorCombo", 
		layout={fill=Sprite.LAYOUT_FILL_BOTH, width="datepicker.szWidth",}, 
		name="editor",
		}
	},
	--ContentOffset={-10,20}, --Placement margin account for border and spacing
	Content={
		class="UI.Panel",
		name="popup",
		layoutModel={ },
		children={{
			class="UI.ColorPicker",
			name="clPicker",
			Buttons=true,
			layout={ fill=Sprite.LAYOUT_FILL_BOTH}
		}}
	},
}

--- Combobox object
function UI.ColorPickerBox:init()
	UI.BuilderSelf(UI.ColorPickerBox.Template,self)	
	function self.popup.onWidgetChange(s,w,c)
		if w==self.clPicker then
			--self.editor.pnColor:setColor(c)
			return true
		end
	end
	function self.popup.onWidgetAction(s,w,c,v)
		if w==self.clPicker then
			if v then
				self:setColor(c)
			else
				self:setFlags({ticked=false})
				self.editor.button:setFlags({ticked=false})
				UI.Focus:relinquish(self)
			end
			return true
		end
	end
end

function UI.ColorPickerBox:setColor(color) 
	self.clPicker:setColor(color)
	self.editor.pnColor:setColor(color)
	self:setFlags({ticked=false})
	self.editor.button:setFlags({ticked=false})
	UI.Focus:relinquish(self)
end

function UI.ColorPickerBox:getColor()
	return self.clPicker:getColor()
end

function UI.ColorPickerBox:setFlags(c)
	UI.PopupButton.setFlags(self,c)
	local readonly = self:getFlags().readonly
	if readonly then
		if c.disabled~=nil or c.readonly~=nil or c.focused~=nil then self.editor:setFlags({disabled=c.disabled, readonly=c.readonly, focused=c.focused}) end
	else --TOSEE focused? ticked?
		if c.disabled~=nil or c.readonly~=nil then self.editor:setFlags({disabled=c.disabled, readonly=c.readonly}) end
	end
	if c.ticked==nil then return end
	local ticked=self:getFlags().ticked
	if readonly then
		if not ticked then self.editor:setFlags({ticked=ticked}) end
	else --TOSEE ticked?
		
	end
	if ticked then
		self.preEdit=self:getColor()
	else
		self.editor.button:setFlags({ticked=false})
		local value=self:getColor()
		if value~=self.preEdit then
			UI.dispatchEvent(self,"WidgetChange",value)
		end
		self.preEdit=nil
	end
end

function UI.ColorPickerBox:onWidgetAction(w,c,v)
	if w==self.editor.button then
		local ticked = w:getFlags().ticked
		self:setFlags({ticked=ticked})
		return true
	end
end


UI.ColorPickerBox.Definition= {
  name="ColorPickerBox",
  icon="ui/icons/panel.png",
  class="UI.ColorPickerBox",
  constructorArgs={},
  properties={
	  
  },
}