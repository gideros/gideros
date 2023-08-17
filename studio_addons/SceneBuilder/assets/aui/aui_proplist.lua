AUI=AUI or {}
AUI.PropertyList=Core.class(UI.Table,function() end)

local function buildNameOrCategory(d) 
	local fd=UI.Utils.makeWidget(d.label or d.name) 
	if d.category then
		fd:setLayoutConstraints({ fill=Sprite.LAYOUT_FILL_BOTH, gridwidth=2 })
		fd:setStyle("aui_proplist.styCategory")
	end
	return fd
end

local VectorEditor=Core.class(UI.Panel,function (item) return end)
VectorEditor.Template={
	class="UI.Panel", 
	layoutModel={ insets="aui_asitem.szSpacing", cellSpacingX="aui_asitem.szSpacing", columnWeights={1,1,1}, rowWeights={1}, equalizeCells=true},
	children={
		{ class="UI.TextField", name="tfX", layout={gridx=0, fill=Sprite.LAYOUT_FILL_BOTH } },
		{ class="UI.TextField", name="tfY", layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH } },
		{ class="UI.TextField", name="tfZ", layout={gridx=2, fill=Sprite.LAYOUT_FILL_BOTH } },
	}
}	

local function simplifyNumber(n)
	return string.format("%.3f",n)
end
function VectorEditor:init(value)
	UI.BuilderSelf(self.Template,self)
	self.tfX:setText(simplifyNumber(value.x))
	self.tfY:setText(simplifyNumber(value.y))
	self.tfZ:setText(simplifyNumber(value.z))
end
function VectorEditor:onTextValid(w,txt)
	local v=vector(tonumber(self.tfX:getText()),tonumber(self.tfY:getText()),tonumber(self.tfZ:getText()))
	self.tfX:setText(simplifyNumber(v.x))
	self.tfY:setText(simplifyNumber(v.y))
	self.tfZ:setText(simplifyNumber(v.z))
	UI.dispatchEvent(self,"WidgetChange",v)
	return true
end

function VectorEditor:onFocusChange(w,txt,gained)
	if not gained then
		self:onTextValid(w,txt)
	end
	return true
end

function AUI.PropertyList:buildEditor(d) 
	if d.category then return end
	local val=self.object:getProperty(d.name) or ""
	local fd
	if not d.readonly then
		if d.type=="vector" then
			fd=VectorEditor.new(val)
		elseif d.type=="action" then
			fd=UI.Button.new()
			fd:setText(d.actionLabel or d.name)
		elseif d.type=="set" then
			fd=UI.Spinner.new(d.typeset)
			fd:setValue(val)
		else
			fd=UI.TextField.new(tostring(val))
		end
		fd.property=d
	end	
	fd=fd or UI.Utils.makeWidget(tostring(val)) 
	fd:setLayoutConstraints({fill=1})
	return fd
end

function AUI.PropertyList:init()
	self:setColumns({
		{ field=buildNameOrCategory },
		{ weight=1,	field=function(d) return self:buildEditor(d) end },
	})
end

function AUI.PropertyList:setObject(s)
	self.object=s
	self:setData(s and s:getPropertyList())
end

function AUI.PropertyList:onTextValid(w,txt)
	if w and w.property then
		local p=w.property
		local pval=if p.type=="number" then tonumber(txt) else txt
		self.object:setProperty(p.name,pval)
	end
end

function AUI.PropertyList:onWidgetChange(w,pval)
	if w and w.property then
		local p=w.property
		self.object:setProperty(p.name,pval)
	end
end

function AUI.PropertyList:onFocusChange(w,txt,gained)
	if not gained then
		self:onTextValid(w,txt)
	end
end
 