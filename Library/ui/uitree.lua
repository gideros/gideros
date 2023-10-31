--!NEEDS:uipanel.lua
UI.Tree=Core.class(UI.Panel,function() return nil end)

--[[ 
-- data builder function returns:
- cell content as a generic sprite/string/meta
- children data or nil
]]

function UI.Tree:init()
	self.cells={}
	self.data={}
	self:setLayoutParameters{ }
	self.expanded={}
	self.expandboxes={}
	self.datacells={}
	self.selection={}
	UI.Control.onMouseClick[self]=self
end

function UI.Tree:newClone() assert(false,"Cloning not supported") end

function UI.Tree:onMouseClick(x,y)
	UI.Focus:request(self)
	x,y=self:localToGlobal(x,y)
	local eb=self:getChildrenAtPoint(x,y,true)
	for _,v in ipairs(eb) do
		local k=self.expandboxes[v]
		if k then
			local nex=not self.expanded[k.node]
			self:setExpanded(k.node,nex)
			UI.dispatchEvent(self,"WidgetExpand",k.node,nex)
		end
	end
	return true --stopPropagation !
end

function UI.Tree:setExpanded(d,e)
	local cell=self.datacells[d]
	if not cell then return end
	if not cell.sub then self:updateCell(self.datacells[d]) return end
	if not e then e=nil end
	self.expanded[d]=e
	if e and type(cell.sub)=="function" then
		cell.sub=cell.sub(cell.node)
		if type(cell.sub)=="table" then
			self:populateChild(cell.subcell,cell.sub,self.builder)
		end
	end
	self:updateCell(self.datacells[d])
end

function UI.Tree:buildCell(d,builder)
	local cells="tree.szBox"
	local nh,sub=builder(d)
	local nd=nh
	nh=UI.Utils.makeWidget(nh,d,nil,true)
	local cell={ w=nh,sub=sub,node=d,d=nd,lay=UI.Panel.new() }
	cell.lay:setLayoutParameters{ columnWeights={0,1}, rowWeights={1,0,1,0}, columnWidths={cells,0}, rowHeights={0,cells,0,0} }
	cell.lay.cell=cell
	cell.tic=UI.Panel.new()
	cell.tic:setStyle("tree.stySub")
	local lic=UI.Panel.new()
	lic:setStyle("tree.styVert")
	cell.prebar=lic
	lic=UI.Panel.new()
	lic:setStyle("tree.styVert")
	cell.subbar=lic
	if sub then
		local subcell=UI.Panel.new()
		subcell:setStyle("tree.stySubCell")
		subcell.cells={}
		subcell:setLayoutParameters{ }
		cell.subcell=subcell
	end
	if cell.w then
		cell.lay:addChild(cell.w)
		cell.w:setLayoutConstraints{ gridy=0,gridx=1,gridheight=3,ipadx=0,fill=Sprite.LAYOUT_FILL_NONE,anchor=Sprite.LAYOUT_ANCHOR_WEST }
	end
	cell.lay:addChild(cell.tic)
	cell.tic:setLayoutConstraints{ gridy=1,gridx=0,ipadx=0,fill=Sprite.LAYOUT_FILL_BOTH }
	cell.lay:addChild(cell.prebar)
	cell.prebar:setLayoutConstraints{ gridy=0,gridx=0,ipadx=0,fill=Sprite.LAYOUT_FILL_BOTH }
	cell.lay:addChild(cell.subbar)
	cell.subbar:setLayoutConstraints{ gridy=2,gridx=0,ipadx=0,fill=Sprite.LAYOUT_FILL_BOTH }
	if cell.sub then
		cell.lay:addChild(cell.subcell)
		cell.subcell:setLayoutConstraints{ gridy=3,gridx=1,ipadx=0,fill=Sprite.LAYOUT_FILL_BOTH }
		if type(cell.sub)=="table" then
			self:populateChild(cell.subcell,cell.sub,builder)
		end
	end
	cell.tree=self

	self.datacells[d]=cell
	return cell
end

function UI.Tree:updateCell(cell)
	local it="tree.stySub"
	if cell.sub then
		it="tree.styExpand"
		if self.expanded[cell.node] then it="tree.styCollapse" end
		self.expandboxes[cell.tic]=cell
	else
		self.expandboxes[cell.tic]=nil
		if cell.last then it="tree.styEnd" end
	end
	cell.tic:setStyle(it)
	local gh=1
	cell.subbar:setVisible(not cell.last)
	if cell.sub then
		cell.subcell:setVisible(self.expanded[cell.node])
		gh=2
	end
	cell.subbar:setLayoutConstraints{ gridy=2,gridx=0,gridheight=gh,ipadx=0,fill=Sprite.LAYOUT_FILL_BOTH }
	if cell.w then
		cell.w:setFlags({ expanded=self.expanded[cell.d], selected=self.selection[cell.node] or false })
	end
end

function UI.Tree:attachCell(cell,p,idx,last)
	cell.last=last
	cell.parentCell=p
	cell.index=idx
	cell.lay:setLayoutConstraints{ gridy=idx-1,gridx=0,gridwidth=2,fill=Sprite.LAYOUT_FILL_BOTH }
	p:addChild(cell.lay)
	self:setExpanded(cell.node,self.expanded[cell.node])
end

function UI.Tree:populateChild(ch,data,builder)
	local lp=ch:getLayoutParameters() or {}
	lp.columnWidths={ "tree.szBox",0 }
	lp.columnWeights={0,1}
	lp.rowWeights={ }
	for _,c in pairs(ch.cells) do if c.w.destroy then c.w:destroy() end c.lay:removeFromParent() end
	ch.cells={} --TODO reuse cells
	if data then
		for i,d in ipairs(data) do
			table.insert(lp.rowWeights,0)
			local cellCode=i
			local cell=self:buildCell(d,builder)
			ch.cells[cellCode]=cell
			self:attachCell(cell,ch,i,i==#data)
		end
	end
	if ch==self then
		table.insert(lp.rowWeights,1)
	end
	ch:setLayoutParameters(lp)
end

function UI.Tree:setData(data,builder) --! will keep selection (call resetData)
	self.data=data	
	self.builder=builder
	self:populateChild(self,data,builder)
end

function UI.Tree:updateStyle(...)
	self:setData(self.data,self.builder)
	UI.Panel.updateStyle(self,...)
end

function UI.Tree:resetData() --reset selection
	self.selection={}
	self:setData()
end

function UI.Tree:moveItem(data,parent,position)
	local scell=self.datacells[data]
	if not scell then return end
	--TODO
end

function UI.Tree:uiSelectData(d) return d and self.datacells[d],d end --spr,data --without onMouseClick --force row selection

function UI.Tree:uiSelect(x,y)
	x,y=self:localToGlobal(x,y)
	local eb=self:getChildrenAtPoint(x,y,true)
	local sset={}
	for _,v in ipairs(eb) do sset[v]=true end
	for _,v in ipairs(eb) do
		if v.cell and v.cell.tree==self and sset[v.cell.w] then
			v=v.cell
			return v,v.node
		end
	end
end

function UI.Tree:uiSelection(sel)
	if self.selection then
		local csel=table.clone(self.selection)
		for data,cell in pairs(csel) do
			if not sel or not sel[cell] then
				self.selection[data]=nil				
				self:updateCell(self.datacells[data])
			end
		end
	end
	if sel then
		for cell,data in pairs(sel) do
			self.selection[data]=cell
			self:updateCell(self.datacells[data])
		end
	end
end

-- DND
function UI.Tree:setAllowDnd(between,over)
	self.allowDnD=between or over
	self.dndBetween=between
	self.dndOver=over
	UI.Dnd.Source(self,self.allowDnD,"auto")
	UI.Dnd.Target(self,self.allowDnD)
end

function UI.Tree:probeDndData(x,y)
	local _,data=self:uiSelect(x,y)
	if data then return true end
end

function UI.Tree:getDndData(x,y)
	local _,data=self:uiSelect(x,y)
	if data then
		local cell=self.datacells[data]
		local marker=self:getDndMarker(data)

		local chc=UI.Utils.colorVector("dnd.colSrcHighlight",self._style)
		local srcMarker=Pixel.new(chc,cell.w:getSize())
		cell.w:addChild(srcMarker)
		
		self.dndSrcMarker=srcMarker
		
		return { type=UI.Tree, value=data, visual=marker }
	end
end

function UI.Tree:getDndMarker(data)
	local cell=self.datacells[data]
	return UI.Dnd.MakeMarker(cell.w)
end
	
function UI.Tree:cleanupDndData(data,target)
	if data.type==UI.Tree then
		self.dndSrcMarker:removeFromParent()
		self.dndSrcMarker=nil
	end
	self.cachedLayout=nil
end

function UI.Tree:offerDndData(data,x,y)
	if self.dndDstMarker then self.dndDstMarker:setVisible(false) end
	if data and data.type==UI.Tree then
		if not self.dndDstMarker then			
			local sh=self:resolveStyle("dnd.szInsertPoint")
			local chc=UI.Utils.colorVector("dnd.colDstHighlight",self._style)
			local dstMarker=Pixel.new(chc,0,sh)
			dstMarker.sh=sh
			dstMarker:setAnchorPoint(0,0.5)
			self:addChild(dstMarker)
			dstMarker:setVisible(false)
			self.dndDstMarker=dstMarker
		end
		
		local cell=self.datacells[data.value]
		
		local gx,gy=self:localToGlobal(x,y)
		local eb=self:getChildrenAtPoint(gx,gy,true)
		local ocell
		for _,v in ipairs(eb) do
			if v.cell and v.cell.tree==self then
				ocell=v.cell
			end
		end
		if ocell then
			local _,place=ocell.lay:globalToLocal(gx,gy)
			local lp,ll,ys=ocell.parentCell,ocell.index,0
			local over
			if self.dndBetween then
				place=place/ocell.lay:getHeight()-.5
				if self.dndOver then
					if place<-.3 then
					elseif place<.3 then over=true
					else ll+=1 ys=ocell.lay:getHeight() end
				else
					if place>=0 then ll+=1 ys=ocell.lay:getHeight()  end
				end
			elseif not self.dndOver then
				return
			else
				over=true
			end
			if over then 
				over=ocell.d
				ll=nil
			elseif cell and cell.parentCell==lp then
				if ll==cell.index or ll==cell.index+1 then 
					ll=nil 
				elseif ll>cell.index then
					ll-=1
				end
			end
			data.over=over
			if ll~=data.insert or lp~=data.insertParent then
				data.insert=ll
				data.insertParent=lp
				local lcs,xs=0,0
				xs,ys=self:spriteToLocal(ocell.lay,xs,ys)
				self.dndDstMarker:setPosition(xs,ys-lcs)	
				self.dndDstMarker:setDimensions(self:getWidth()-xs,self.dndDstMarker.sh)
			end
			if ll then
				self.dndDstMarker:setVisible(true)	
				return (not self.checkDndOffer) or self:checkDndOffer(data,x,y,nil,lp and lp.d,ll)
			elseif over  then
				return (not self.checkDndOffer) or self:checkDndOffer(data,x,y,ocell.d,lp and lp.d,ll)
			end
		end		
	elseif self.dndDstMarker then
		self.dndDstMarker:removeFromParent()
		self.dndDstMarker=nil
	end	
end

function UI.Tree:setDndData(data,source)
	if data and data.type==UI.Tree then
		if data.insert then
			if source==self then
				self:moveItem(data.value,data.insertParent and data.insertParent.d,data.insert)
				UI.dispatchEvent(self,"ItemMove",data.value,data.insert,data.insertParent and data.insertParent.d)
			else
				UI.dispatchEvent(self,"DndDrop",source,data,nil,data.insert) --no over
			end
		elseif data.over then
			UI.dispatchEvent(self,"DndDrop",source,data,data.over,nil) --no insert
		end
	end
end

UI.Tree.Definition= {
	name="Tree",
	icon="ui/icons/panel.png",
	class="UI.Tree",
	constructorArgs={ },
	properties={
	},
}
