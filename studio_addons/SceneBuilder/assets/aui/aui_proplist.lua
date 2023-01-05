AUI=AUI or {}
AUI.PropertyList=Core.class(UI.Table,function() end)

local function buildNameOrCategory(d) 
	local fd=UI.Utils.makeWidget(d.name) 
	if d.category then
		fd:setLayoutConstraints({ fill=Sprite.LAYOUT_FILL_BOTH, gridwidth=2 })
		fd:setStyle("aui_proplist.styCategory")
	end
	return fd
end

function AUI.PropertyList:buildEditor(d) 
	if d.category then return end
	local val=self.object:getProperty(d.name) or ""
	local fd
	if not d.readonly then
		fd=UI.TextField.new(tostring(val))
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

function AUI.PropertyList:onFocusChange(w,txt,gained)
	if not gained then
		self:onTextValid(w,txt)
	end
end
 