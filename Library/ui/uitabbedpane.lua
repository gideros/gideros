--!NEEDS:uipanel.lua

--[[ 
-- data builder function returns :
- header content as a generic sprite/string/meta if called with false
- tab content as a generic widget if called with true
- eventually both if called with false
]]

UI.TabbedPane=Core.class(UI.Panel,function () return nil end)
function UI.TabbedPane:init()
	self.data={}
	self.headers={}
	self.tabs={}
	self:setLayoutParameters{columnWeights={1},equalizeCells=true}
	self.current=0
	self.datacells={}
	self.contentPane=UI.Panel.new()
	self.contentPane:setLocalStyle("tabbedpane.styPane")
	self.contentPane:setLayoutParameters({columnWeights={1},rowWeights={1}})
	self:addChild(self.contentPane)
	self.hline=UI.Panel.new()
	self.hline:setLocalStyle("tabbedpane.styHLine")
	self:addChild(self.hline)
	UI.Control.onMouseClick[self]=self	
end

function UI.TabbedPane:newClone() 
	local _data=self.data
	self:setData(nil,self.builder)
	self:setData(_data,self.builder)
	UI.Control.onMouseClick[self]=self
end

function UI.TabbedPane:setTabFill(en)
	if self.columnFill~=en then
		self.columnFill=en
		self:setData(self.data,self.builder)
	end
end

function UI.TabbedPane:buildTab(d)
	local nh,nw=self.builder(d, false)
	nh=UI.Utils.makeWidget(nh,d)
	nw=nw and UI.Utils.makeWidget(nw,d)
	local ch=UI.Panel.new()
	ch:setLayoutParameters({columnWeights={1},rowWeights={1}})
	nh:setLayoutConstraints({})
	ch:addChild(nh)
	local cell={ w=nw,h=nh,ch=ch }
	return cell
end

function UI.TabbedPane:setCurrent(index,event)
	self.current=index
	local ctab
	local cidx=2
	for i,cell in ipairs(self.tabs) do
		local cur=(index==i)
		cell.h:setFlags({ expanded=cur },event)
		if not cur then 
			if cell.w then
				cell.w:setVisible(false)
			end
			self:addChildAt(cell.ch,cidx)
			cidx+=1
		else
			if not cell.w then
				local nw=self.builder(cell.d, true)
				cell.w=UI.Utils.makeWidget(nw,cell.d)
			end
		end
		if cur then
			self.contentPane:addChild(cell.w)
			cell.w:setVisible(true)
			ctab=cell
			cell.ch:setLocalStyle(
				if i==1 then "tabbedpane.styTabCurrentFirst" 
				elseif i==#self.tabs and self.columnFill then "tabbedpane.styTabCurrentLast"
				else "tabbedpane.styTabCurrent")
		else
			cell.ch:setLocalStyle("tabbedpane.styTabOther")
		end
	end
	if ctab then
		self:addChildAt(self.hline,cidx)
		self:addChildAt(ctab.ch,cidx+1)
	end
end


function UI.TabbedPane:setData(data,builder)
	--Same builder: check for data to keep
	local cache
	if data and self.data and builder==self.builder then
		cache={}
		local keep={}
		for _,d in ipairs(data) do keep[d]=true end
		for i,d in ipairs(self.data) do
			if keep[d] then
				cache[d]=self.tabs[i]
			end
		end
	end
	self.data=data
	self.builder=builder
	local lp=self:getLayoutParameters() or {}
	lp.columnWeights={ 0 }
	lp.columnWidths={ "tabbedpane.szInset" }
	lp.rowWeights={0,1}
	for _,c in pairs(self.tabs) do
		if not cache or not cache[c.d] then
			if c.h.destroy then c.h:destroy() end 
			if c.w and c.w.destroy then c.w:destroy() end 
			if c.bg and c.bg.destroy then c.bg:destroy() end 
		end
		c.ch:removeFromParent()
		if c.w then
			c.w:removeFromParent()
		end
	end
	self.tabs={}
	self.headers={}
	self.datacells={}
	local cfill=if self.columnFill then 1 else 0
	if data then
		for i,d in ipairs(data) do
			table.insert(lp.columnWeights,cfill) 
			table.insert(lp.columnWidths,0)
			local cell = cache and cache[d] or self:buildTab(d)
			cell.d=d
			cell.tabIndex=i
			self.tabs[i]=cell
			self:addChild(cell.ch)
			cell.ch:setLayoutConstraints{ gridx=cell.tabIndex, fill=1 }
			self.headers[cell.ch]=cell
			self.datacells[d]=cell
		end
	end
	table.insert(lp.columnWidths,"tabbedpane.szInset")
	table.insert(lp.columnWeights,if self.columnFill then 0 else 1) 
	self:setLayoutParameters(lp)
	self.contentPane:setLayoutConstraints({gridx=0, gridy=1, gridwidth=#self.tabs+2,gridheight=1, fill=1})
	self.hline:setLayoutConstraints({gridx=0, gridy=0, gridwidth=#self.tabs+2,gridheight=1, fill=1})
	self:setCurrent((self.current<>1)><#self.data)
end

function UI.TabbedPane:onMouseClick(x,y)
	UI.Focus:request(self)
    local eb=self:getChildrenAtPoint(x,y,true,true,self)
    for _,v in ipairs(eb) do
        local cell=self.headers[v]
        if cell then
            local disabled = nil
            if cell.h and cell.h.getFlags then disabled = cell.h:getFlags().disabled end
            if not disabled then
				self:setCurrent(cell.tabIndex,"onMouseClick")
                UI.dispatchEvent(self,"WidgetChanged",cell.d,cell.tabIndex)
                return true --stopPropagation !
            end
        end
    end
end

UI.TabbedPane.Definition= {
  name="TabbedPane",
  icon="ui/icons/panel.png",
  class="UI.TabbedPane",
  constructorArgs={  },
  properties={   
	{ name="TabFill", type="boolean", setter=UI.TabbedPane.setTabFill },
  },
}
