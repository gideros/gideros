--!NEEDS:uiimagetext.lua
--!NEEDS:uibehavior.lua
--!NEEDS:uistyle.lua
--!NEEDS:uibutton.lua
--!NEEDS:uilayout.lua
--!NEEDS:uiselection.lua

UI.Combobox=Core.class(UI.PopupButton,function () return nil end)

--Default combobox model, for string editing
UI.Combobox.Model=Core.class(Object)
function UI.Combobox.Model:getEditorTemplate()
	return { class="UI.TextField", TextLayout={flags=FontBase.TLF_SINGLELINE|FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER} }
end
function UI.Combobox.Model:setEditing(editor,editing)
	editor:setFlags({readonly=not editing})
	if editing then
		editor:focus()
	end
end
function UI.Combobox.Model:setEditorValue(editor,value,mapper)
	editor:setText(value)
end
function UI.Combobox.Model:getEditorValue(editor)
	return editor:getText()
end
function UI.Combobox.Model:setListData(list,data,mapper)
	list:setData(data,mapper)
end
function UI.Combobox.Model:getListColumns()
	return	{{ 
		weight=1,
		field=function(d) 
			local fd=UI.Utils.makeWidget(d) 
			if type(d)~="table" then
				fd:setLayoutConstraints({ fill=Sprite.LAYOUT_FILL_BOTH })
			end
			return fd
		end}}
end

--List and Button combobox model
UI.Combobox.ModelButton=Core.class(UI.Combobox.Model)
function UI.Combobox.ModelButton:getEditorTemplate()
	return { class="UI.ButtonTextFieldCombo", TextLayout={flags=FontBase.TLF_SINGLELINE|FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER} }
end

--- Combobox object
function UI.Combobox._modeler(d,s)
	d=table.clone(d)
	local dm=s.dataModel or UI.Combobox.Model.new()	
	s.dataModel=dm
	local editorTemplate=dm:getEditorTemplate()
	editorTemplate.name="editor"
	editorTemplate.layout=editorTemplate.layout or {fill=Sprite.LAYOUT_FILL_BOTH}
	d.children={editorTemplate}
	return d
end

--- Combobox
UI.Combobox.Template={
	class="UI.PopupButton",
	model=UI.Combobox._modeler,
	BaseStyle="combobox.styBase",
	StyleInheritance="base",
	--ContentOffset={-10,20}, --Placement margin account for border and spacing
	Content={
		class="UI.Panel", 
		BaseStyle="combobox.styListContainer",
		layoutModel={ columnWeights={1}, rowWeights={1}},
		children={
			{ class="UI.Viewport", 
				Scrollbar={UI.Viewport.SCROLLBAR.NONE,UI.Viewport.SCROLLBAR.AUTO}, --Horizontal,Vertical
				layout={ fill=Sprite.LAYOUT_FILL_BOTH },
				name="popupView", 
				Content={ class="UI.Table", 
					layout={ fill=Sprite.LAYOUT_FILL_BOTH },
					name="popupList", selection=UI.Selection.SINGLE },
			},
		}},
}

function UI.Combobox:init(datamodel)
	self.dataModel=datamodel
	UI.BuilderSelf(UI.Combobox.Template,self)
	self.index=0
	self.autoClose=true
	self.popupList:setColumns(self.dataModel:getListColumns())
	self.popupList.onSelectionChange=function(s,w,data)
		self:setCurrent(data[next(data)])
		if self.autoClose then
			self:setFlags({ticked=false})
		end
		UI.dispatchEvent(self,"WidgetChange",self:getCurrent())
		return true
	end
end

function UI.Combobox:setPopupStyle(style)
	self.popupList:setStyle(style)
end

function UI.Combobox:setAutoClose(auto)
	self.autoclose=auto
end

function UI.Combobox:getMinimumContentSize()
	return self:getWidth(),0,function (s,vw,vh)
		local _,_,pew,peh=self.popupView:getContentSize()
		local pfw,pfh=vw+pew+0.1,vh+peh+0.1 --TODO: arbirary margin
		s.w=s.w<>pfw
		s.h=s.h<>pfh
	end
end

function UI.Combobox:setData(data,mapper)
	self.data=data or {}
	self.mapper=self.mapper or mapper
	self.dataModel:setListData(self.popupList,self.data,self.mapper)
	self:setIndex(0)
	self.content:setVisible(#self.data>0)
end

function UI.Combobox:setCurrent(value,editing)
	self.dataModel:setEditing(self.editor,false)
	self.dataModel:setEditorValue(self.editor,value,self.mapper)
	self.index=0
	local sv = nil
	if self.data then
		for k,v in ipairs(self.data) do
			if v==value then
				self.index=k
				sv=v
			end
		end
	end
	UI.Selection.select(self.popupList,{ sv })
	if editing then
		self.dataModel:setEditing(self.editor,true)
	else
		UI.Focus:relinquish(self)
	end
end

function UI.Combobox:getCurrent()
	return self.dataModel:getEditorValue(self.editor)
end

function UI.Combobox:setIndex(v)
	self.index=v
	if v>0 and v<=#self.data then
		self:setCurrent(self.data[v])
		return
	end
	self.index=0
	UI.Selection.select(self.popupList,{})
	self.dataModel:setEditing(self.editor,false)
	UI.Focus:relinquish(self)
end

function UI.Combobox:getIndex()
	return self.index
end

function UI.Combobox:setFlags(c)
	UI.PopupButton.setFlags(self,c)
	local readonly = self:getFlags().readonly
	if readonly then
		if c.disabled~=nil or c.readonly~=nil or c.focused~=nil then self.editor:setFlags({disabled=c.disabled, readonly=c.readonly, focused=c.focused}) end
	else 
		if c.disabled~=nil or c.readonly~=nil then self.editor:setFlags({disabled=c.disabled, readonly=c.readonly}) end
	end

	if c.ticked==nil then return end
	local ticked=self:getFlags().ticked
	if readonly and not ticked then
		self.editor:setFlags({ticked=ticked})
	end

	if ticked then
		self.preEdit=self:getCurrent()
		self.dataModel:setEditing(self.editor,(not readonly))
	else
		local value=self:getCurrent()
		if value~=self.preEdit then
			self:setCurrent(value) --Force index/selection update based on edited content
			UI.dispatchEvent(self,"WidgetChange",value)
		else
			self.dataModel:setEditing(self.editor,false)
		end
		self.preEdit=nil
	end	
end

UI.Combobox.Definition= {
  name="Combobox",
  icon="ui/icons/panel.png",
  class="UI.Combobox",
  constructorArgs={ "DataModel" },
  properties={
    { name="AutoClose", type="boolean", setter=UI.Combobox.setAutoClose },
    { name="PopupStyle", type="style", setter=UI.Combobox.setPopupStyle },
  },
}

--- ComboboxButton
UI.ComboboxButton=Core.class(UI.Combobox,function(dataModel) return dataModel or UI.Combobox.ModelButton.new() end)

function UI.ComboboxButton:setFlags(c)
	UI.Combobox.setFlags(self,c)
	self.editor.button:setFlags({ ticked=c.ticked })
end

function UI.ComboboxButton:onWidgetAction(w)
	if w and self.editor and self.editor.button and w==self.editor.button then
		local ticked = w:getFlags().ticked
		self:setFlags({ticked=ticked})
	end
end

UI.ComboboxButton.Definition= {
  name="ComboboxButton",
  icon="ui/icons/panel.png",
  class="UI.ComboboxButton",
  super=UI.Combobox,
  constructorArgs={ "DataModel" },
  properties={
	
  },
}

--- Drop down list
UI.Dropdown=Core.class(UI.ComboboxButton,function(dataModel) return dataModel or UI.Dropdown.Model.new() end)
function UI.Dropdown:setIndex(v)
	if (#self.data>0) and (v<=0 or v>#self.data) then
		v=1
	end
	UI.Combobox.setIndex(self,v)
end

-- Dropdown list generic holder
UI.Dropdown.GenericEditor=Core.class(UI.Panel,function () end)

UI.Dropdown.GenericEditor.Template={
	class="UI.Panel", 
	layoutModel={ columnWeights={1,0}, rowWeights={1} },
	BaseStyle="textfield.styBase",
	children={ 
		{ class="UI.Panel", layoutModel={ columnWeights={1}, rowWeights={1} },
			layout={ fill=Sprite.LAYOUT_FILL_BOTH, insets="textfield.szMargin" },
			name="pnValue"
		},
		{ class="UI.ToggleButton", name="button",layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH },
			Image="dropdown.icButton",
			LocalStyle="dropdown.styButton"
			},
	}}
function UI.Dropdown.GenericEditor:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	local s="textfield.styNormal"
	if self._flags.disabled then
		s="textfield.styDisabled"
	else	
		if self._flags.readonly then
			s=if self._flags.focused then "textfield.styFocusedReadonly" else "textfield.styReadonly"
		elseif self._flags.focused then
			s="textfield.styFocused"
		end
	end
	self:setStateStyle(s)
	UI.ToggleButton.setFlags(self.button,changes)
end

function UI.Dropdown.GenericEditor:init()
	UI.BuilderSelf(UI.Dropdown.GenericEditor.Template,self)
end

-- Dropdown List model
UI.Dropdown.Model=Core.class(UI.Combobox.ModelButton)
function UI.Dropdown.Model:getEditorTemplate()
	return { class="UI.Dropdown.GenericEditor" }
end
function UI.Dropdown.Model:setEditing(editor,editing)
end
function UI.Dropdown.Model:setEditorValue(editor,value,mapper)
	if self.currentValue then
		editor.pnValue:removeChildAt(1)
	end
	self.currentValue=value
	if value then
		local vh
		if mapper then
			vh=UI.Utils.makeWidget(mapper,value) 
		else
			vh=UI.Utils.makeWidget(value)
		end
		editor.pnValue:addChild(vh)
	end
end
function UI.Dropdown.Model:getEditorValue(editor)
	return self.currentValue
end
