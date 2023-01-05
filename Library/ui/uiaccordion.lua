--!NEEDS:uipanel.lua
UI.Accordion=Core.class(UI.Panel,function() return nil end)

--[[ 
-- data builder function returns :
- header content as a generic sprite/string/meta if called with false
- tab content as a generic widget if called with true
- optional background for both cells as a sprite if called with true
- eventually both or all if called with false
]]

function UI.Accordion:init()
	self.data={}
	self.headers={}
	self.tabs={}
	self:setLayoutParameters{columnWeights={1},gridAnchorY=0 }
	self:setExpand(false)
	self.expanded={}
	self.datacells={}
	self.autoCollapse=true
	UI.Control.onMouseClick[self]=self
end

function UI.Accordion:newClone() 
	local _data=self.data
	self:setData(nil,self.builder)
	self:setData(_data,self.builder)
	UI.Control.onMouseClick[self]=self
end

function UI.Accordion:setExpand(e)
	self.expand = e and 1 or 0
end
function UI.Accordion:onMouseClick(x,y)
	UI.Focus:request(self)
    x,y=self:localToGlobal(x,y)
    local eb=self:getChildrenAtPoint(x,y,true)
    for _,v in ipairs(eb) do
        local cell=self.headers[v]
        if cell then
            local disabled = nil
            if cell.h and cell.h.getFlags then disabled = cell.h:getFlags().disabled end
            if disabled then --nothing
                self:setExpanded(cell.d,false,"onMouseClick")
                UI.dispatchEvent(self,"WidgetExpand",cell.d,false,"onMouseClick")
            else
                local nex=not self.expanded[cell.d]
                self:setExpanded(cell.d,nex,"onMouseClick")
                UI.dispatchEvent(self,"WidgetExpand",cell.d,nex,"onMouseClick")
                return true --stopPropagation !
            end
        end
    end
end

function UI.Accordion:setExpanded(d,e,event)--event(from onMouseClick)
	local cell=self.datacells[d]
	if not cell then return end
	local lp = self:getLayoutParameters() or { rowHeights={ }, rowWeights={ }, columnWeights={1} }
	local lc = {}
	local lprh = lp.rowHeights or { }
	local lprw = lp.rowWeights or { }
	local focus
	if e then
		self.expanded[d]=e
		if not cell.w then
			local nw,nbg=self.builder(d, true)
			cell.w=UI.Utils.makeWidget(nw,d)
			cell.bg=nbg
		end
		self:addChild(cell.w)
		lc.gridy=cell.row
		lc.fill=Sprite.LAYOUT_FILL_BOTH
		local disabled = cell.w.getFlags and cell.w:getFlags().disabled

		for i=2,#lprw,2 do
			lprw[i]=0
			local other=self.tabs[i/2]
			if other and other~=cell and self.autoCollapse and self.expanded[other.d] then
				if other.w then
					other.w:setVisible(false)
					other.w:setLayoutConstraints({height=0})
				end
				if other.bg then other.bg:removeFromParent() end
				self.expanded[other.d]=nil
				self:updateTab(other)
			end
		end
		if disabled then --keep expanded true
			lprw[cell.row+1]=0
			lc.minHeight=0
			lc.prefHeight=0
			cell.w:setVisible(false)
			if cell.bg then cell.bg:removeFromParent() end
			focus=true
		else
			lprw[cell.row+1]=self.expand or 0
			lc.minHeight=-1
			lc.prefHeight=-1
			cell.w:setVisible(true)
			if cell.bg then 
				lc.gridheight=2
				lc.gridy-=1
				self:addChildAt(cell.bg,1)
				cell.bg:setLayoutConstraints(lc)
				lc.gridy+=1
				lc.gridheight=nil
			end
			focus=true
		end
	else
		self.expanded[d]=nil
		lprw[cell.row+1]=0
		lc.minHeight=0
		lc.prefHeight=0
		if cell.w then cell.w:setVisible(false) end
		if cell.bg then cell.bg:removeFromParent() end
	end
	if next(self.expanded) then
		lprw[#lprw]=0
	else
		lprw[#lprw]=self.expand or 0
	end
	for i=1,#lprw,1 do lprh[i]=0 end
	if cell.w then
		cell.w:setLayoutConstraints(lc)
	end
	self:setLayoutParameters(lp)
	self:updateTab(cell,event)--event(from onMouseClick)
	if focus then
		self:getLayoutInfo()
		UI.Focus:clear()
		if cell.w and cell.w:isVisible() then
			UI.Focus:area(self,0,cell.h:getY(),self:getWidth(),cell.w:getY()+cell.w:getHeight()-cell.h:getY(),true,0,0)
		else
			UI.Focus:area(self,0,cell.h:getY(),self:getWidth(),cell.h:getHeight(),true,0,0)
		end
	end
end

function UI.Accordion:buildTab(d)
	local nh,nw,nbg=self.builder(d, false)
	nh=UI.Utils.makeWidget(nh,d)
	nw=nw and UI.Utils.makeWidget(nw,d)
	local cell={ w=nw,h=nh,bg=nbg }
	cell.accordion=self
	return cell
end

function UI.Accordion:updateTab(cell,event)
	if cell.h and cell.h.uiUpdate then
		cell.h:uiUpdate(cell.d,{ expanded=self.expanded[cell.d] },event) --d,mode --event(from onMouseClick)
	end
end

function UI.Accordion:moveItem(data,position)
	local scell=self.datacells[data]
	if not scell then return end
	local lp=self:getLayoutParameters()
	
	--Replace
	local si=(scell.row+1)//2
	table.remove(self.data,si)
	table.remove(self.tabs,si)
	local rw1=table.remove(lp.rowWeights,scell.row)
	local rw2=table.remove(lp.rowWeights,scell.row)
	
	scell.row=position*2-1
	table.insert(self.data,position,data)
	table.insert(self.tabs,position,scell)
	table.insert(lp.rowWeights,scell.row,rw2)
	table.insert(lp.rowWeights,scell.row,rw1)
	
	for i,cell in ipairs(self.tabs) do
		self:addChildAt(cell.h,i)
		cell.row=i*2-1
		cell.h:setLayoutConstraints{ gridy=cell.row-1 }
		if cell.w then
			cell.w:setLayoutConstraints{ gridy=cell.row }
		end
		if cell.bg then
			cell.bg:setLayoutConstraints{ gridy=cell.row-1 }
		end
	end

	self:setLayoutParameters(lp)

	for d,e in pairs(self.expanded) do
		if e then 
			local cell=self.datacells[d]
			if cell.bg then
				self:addChildAt(cell.bg,1)
			end
		end
	end	
end

function UI.Accordion:setData(data,builder)
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
	lp.columnWidths={0}
	lp.columnWeights={1}
	lp.rowWeights={ }
	self.expanded=self.expanded or {}
	for _,c in pairs(self.tabs) do
		if not cache or not cache[c.d] then
			if c.h.destroy then c.h:destroy() end 
			if c.w and c.w.destroy then c.w:destroy() end 
			if c.bg and c.bg.destroy then c.bg:destroy() end 
			self.expanded[c.d]=nil
		end
		c.h:removeFromParent()
		if c.w then
			c.w:removeFromParent()
		end
		if c.bg then
			c.bg:removeFromParent()
		end
	end
	self.tabs={}
	self.headers={}
	self.datacells={}
	if data then
		for i,d in ipairs(data) do
			table.insert(lp.rowWeights,0)
			table.insert(lp.rowWeights,0)
			local cell = cache and cache[d] or self:buildTab(d)
			cell.d=d
			cell.row=i*2-1
			self.tabs[i]=cell
			self:addChild(cell.h)
			cell.h:setLayoutConstraints{ gridy=cell.row-1, fill=Sprite.LAYOUT_FILL_HORIZONTAL, anchor=Sprite.LAYOUT_ANCHOR_NORTH }
			self.headers[cell.h]=cell
			self.datacells[d]=cell
		end
	end
	table.insert(lp.rowWeights,1) -- Last row contains nothing, but is used as a filler
	self:setLayoutParameters(lp)
end

function UI.Accordion:updateVisible(y,viewh)
	if not self.data then return end
	y=y or self.rangey or 100000
	viewh=viewh or self.rangeh
	self.rangey=y
	self.rangeh=viewh
	local li=self:getLayoutInfo()
	local lp=Sprite.getLayoutParameters(self,true)
	local ls=li.minHeight
	local lmax=#self.data
	local ll=1
	y-=lp.insetTop
	local lcs=2*(lp.cellSpacingY or 0)
	while ll<=lmax and ls[ll*2-1] and y>ls[ll*2-1] do y-=ls[ll*2-1]+ls[ll*2]+lcs ll+=1 end
	local lf=ll<>2
	while ll<=lmax and ls[ll*2-1] and viewh>0 do viewh-=ls[ll*2-1] ll+=1 end
	lf-=1
	ll=(ll<>(lf+1))
	if lf>0 and ll>lf then
		local i0=2 --Never hide first element, as it could be group background
		local i1=lf-1
		local i2=ll+1
		local i3=#self.data
		self:setHiddenChildren({
				{i0,i1},
				{i2,i3}
			})
	else
		self:setHiddenChildren(nil)
	end
end

function UI.Accordion:onViewportScrolled(viewport,x,y,rangex,rangey,vieww,viewh)
	self:updateVisible(y,viewh)
end

-- DND
function UI.Accordion:setAllowDnd(between,over)
	self.allowDnD=between or over
	self.dndBetween=between
	self.dndOver=over
	UI.Dnd.Source(self,self.allowDnD,true)
	UI.Dnd.Target(self,self.allowDnD,true)
end

function UI.Accordion:getDndData(x,y)
    x,y=self:localToGlobal(x,y)
    local eb=self:getChildrenAtPoint(x,y,true,true)
	for _,hdr in ipairs(eb) do
		local cell=self.headers[hdr]
		if cell then			
			local marker=self:getDndMarker(cell.d)
			if marker then
				local chc=UI.Utils.colorVector("dnd.colSrcHighlight",self._style)
				local srcMarker=Pixel.new(chc,cell.h:getSize())
				cell.h:addChild(srcMarker)
				
				self.dndSrcMarker=srcMarker
				
				return { type=UI.Accordion, value=cell.d, visual=marker }
			end
		end
	end
end

function UI.Accordion:getDndMarker(data)
	local cell=self.datacells[data]
	local hCell = self.getDndMarkerCell and self:getDndMarkerCell(cell) or cell.h
	return UI.Dnd.MakeMarker(hCell)
end
	
function UI.Accordion:cleanupDndData(data,target)
	if data.type==UI.Accordion then
		self.dndSrcMarker:removeFromParent()
		self.dndSrcMarker=nil
	end
	self.cachedLayout=nil
end

function UI.Accordion:offerDndData(data,x,y)
	if self.dndDstMarker then self.dndDstMarker:setVisible(false) end
	if data and data.type==UI.Accordion then
		if not self.dndDstMarker then			
			local sh=self:resolveStyle("dnd.szInsertPoint")
			local chc=UI.Utils.colorVector("dnd.colDstHighlight",self._style)
			local dstMarker=Pixel.new(chc,self:getWidth(),sh)
			dstMarker:setAnchorPoint(0,0.5)
			self:addChild(dstMarker)
			dstMarker:setVisible(false)
			self.dndDstMarker=dstMarker
		end
		
		local cell=self.datacells[data.value]
		
		local li=self.cachedLayout or self:getLayoutInfo()
		self.cachedLayout=li

		local lp=Sprite.getLayoutParameters(self,true)
		local ls=li.minHeight
		local lmax=#self.data
		local ll,ys=1,0
		y-=lp.insetTop
		local lcs=2*(lp.cellSpacingY or 0)
		while ll<=lmax and ls[ll*2-1] and y>ls[ll*2-1] do 
			local lls=ls[ll*2-1]+ls[ll*2]+lcs 
			y-=lls
			ys+=lls
			ll+=1 
		end
		
		local sh=ls[ll*2-1]+ls[ll*2]+lcs
		
		local over
		if self.dndBetween then
			local place=y/sh-0.5
			if self.dndOver then
				if place<-.3 then
				elseif place<.3 then over=true
				else ll+=1 ys+=sh end
			else
				if place>=0 then ll+=1 ys+=sh  end
			end
		elseif not self.dndOver then
			return
		else
			over=true
		end
		local lrow=cell and (cell.row+1)//2
		if over then 
			over=ll
			ll=nil
		elseif lrow then
			if ll==lrow or ll==lrow+1 then 
				ll=nil 
			elseif ll>lrow then
				ll-=1
			end
		end
		data.over=over
		if ll~=data.insert then
			data.insert=ll
			local lcsy=if (ll and ll>1) then ((Sprite.getLayoutParameters(self,true) or {}).cellSpacingY or 0)/2 else 0
			self.dndDstMarker:setY(ys-lcsy)	
		end
		if ll then
			self.dndDstMarker:setVisible(true)	
			local ret=(not self.checkDndOffer) or self:checkDndOffer(data,x,y,nil,ll) 
			if ret==nil then self.dndDstMarker:removeFromParent() self.dndDstMarker=nil end
			return ret
		elseif over  and self.tabs[over] then
			local ret=(not self.checkDndOffer) or self:checkDndOffer(data,x,y,self.tabs[over].d,ll)
			if ret==nil then self.dndDstMarker:removeFromParent() self.dndDstMarker=nil end
			return ret
		end
	elseif self.dndDstMarker then
		self.dndDstMarker:removeFromParent()
		self.dndDstMarker=nil
	end
end

function UI.Accordion:setDndData(data,source)
	if data and data.type==UI.Accordion then
		if data.insert then
			if source==self then
				if not UI.dispatchEvent(self,"ItemMove",data.value,data.insert) then
					self:moveItem(data.value,data.insert)
				end
			else
				UI.dispatchEvent(self,"DndDrop",source,data,nil,data.insert) --no over
			end
		elseif data.over then
			UI.dispatchEvent(self,"DndDrop",source,data,self.tabs[data.over].d,nil) --no insert
		end
	end
end

UI.Accordion.Definition= {
	name="Accordion",
	icon="ui/icons/panel.png",
	class="UI.Accordion",
	constructorArgs={ },
	properties={
		{ name="Color", type="color", setter=UI.Panel.setColor, getter=UI.Panel.getColor },
		{ name="Expand", type="boolean", setter=UI.Panel.setExpand },
	},
}
