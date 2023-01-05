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
	return { class="UI.TextField", TextLayout={flags=FontBase.TLF_SINGLELINE|FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER}, Editable=false }
end
function UI.Combobox.Model:setEditing(editor,editing)
	editor:setFlags({readonly=not editing})
	if editing then
		editor:focus()
	end
end
function UI.Combobox.Model:setEditorValue(editor,value)
	editor:setText(value)
end
function UI.Combobox.Model:getEditorValue(editor)
	return editor:getText()
end
function UI.Combobox.Model:setListData(list,data)
	list:setData(data)
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
	return { class="UI.ButtonTextFieldCombo", TextLayout={flags=FontBase.TLF_SINGLELINE|FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER}, Editable=false }
end
function UI.Combobox.ModelButton:setEditing(editor,editing)
	--NO editor:setFlags({readonly=not editing})
	if editing then editor:focus() end
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
		layoutModel=UI.Layout.Vertical,
		children={
			{ class="UI.Table", 
				layout={ fill=Sprite.LAYOUT_FILL_BOTH },
				name="popupList", selection=UI.Selection.SINGLE },
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

function UI.Combobox:setAutoClose(auto)
	self.autoclose=auto
end

function UI.Combobox:getMinimumContentSize()
	return self:getWidth(),0
end

function UI.Combobox:setData(data)
	self.data=data or {}
	self.dataModel:setListData(self.popupList,self.data)
	self:setIndex(0)
	self.content:setVisible(#self.data>0)
end

function UI.Combobox:setCurrent(value,editing)
	self.dataModel:setEditing(self.editor,false)
	self.dataModel:setEditorValue(self.editor,value)
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
	if c.disabled~=nil or c.readonly~=nil then self.editor:setFlags({disabled=c.disabled, readonly=c.readonly}) end 
	if c.ticked==nil then return end
	local ticked=self:getFlags().ticked
	if ticked then
		self.preEdit=self:getCurrent()
		self.dataModel:setEditing(self.editor,true)
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
  },
}

--- ComboboxButton
UI.ComboboxButton=Core.class(UI.Combobox,function() return UI.Combobox.ModelButton end)

function UI.ComboboxButton:setFlags(c)
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
		self.preEdit=self:getCurrent()
		self.dataModel:setEditing(self.editor)
	else
		local value=self:getCurrent()
		if value~=self.preEdit then
			self:setCurrent(value) --Force index/selection update based on edited content
			UI.dispatchEvent(self,"WidgetChange",value)
		else
			self.dataModel:setEditing(self.editor)
		end
		self.preEdit=nil
	end
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
  constructorArgs={ "DataModel" },
  properties={
	
  },
}
