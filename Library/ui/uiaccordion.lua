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
	self:setAutoCollapse(true)
	self.selection={}
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
	self:checkDragResize()
end

function UI.Accordion:setAutoCollapse(e)
	self.autoCollapse = e
	self:checkDragResize()
end

function UI.Accordion:checkDragResize()
	local ed=if self.autoCollapse or (self.expand==0) then nil else self
	UI.Control.onDragStart[self]=ed
	UI.Control.onDrag[self]=ed
	UI.Control.onDragEnd[self]=ed
    if UI.Control.HAS_CURSOR then 
		UI.Control.onMouseMove[self]=ed
	end
end

function UI.Accordion:getCellSprites(d)
	local cell = self.datacells[d]
	if cell then return cell.h,cell.w,cell.bg end
end

function UI.Accordion:getHeaderAt(x,y)
    local eb=self:getChildrenAtPoint(x,y,true,true,self)
    for _,v in ipairs(eb) do
        local cell=self.headers[v]
        if cell then return cell end
    end
end

function UI.Accordion:onMouseMove(x,y)
	local cell=self:getHeaderAt(x,y)
	if cell and cell.row>1 then
		UI.Control.setLocalCursor("splitV")
	end
end

function UI.Accordion:onDragStart(x,y,ed,ea,change,long)
	if long or self.autoCollapse then return end
	local cell=self:getHeaderAt(x,y)
	if not cell then return end
	local rh={}
	for _,c in ipairs(self.tabs) do
		rh[c.row+1] = not (c.h and c.h:isVisible())
	end
	local crow=cell.row-1
	while crow>=2 and rh[crow] do crow-=2 end
	if crow<2 then return end
	UI.Focus:request(self)
	self.dragging={ oy=y, cell=cell, crow=crow, rh=rh, layout=self:getLayoutInfo(), lp=self:getLayoutParameters(true) }
	self.dragging.nlp=table.clone(self.dragging.lp)
	self.dragging.nlp.rowWeights=table.clone(self.dragging.lp.rowWeights)
	--print(_inspect(rh),crow,cell.row)
	return true
end

function UI.Accordion:onDragEnd(x,y)
	if self.dragging and self.dragging.nlp then
		local nlp=self.dragging.nlp
		local rw=nlp.rowWeights
		local ws=0
		for _,w in ipairs(rw) do ws+=w end
		if ws>1 then
			local rwn=#rw
			for i=1,rwn do rw[i]/=ws end
			self:setLayoutParameters(nlp)
		end
	end
	self.dragging=nil
end

function UI.Accordion:onDrag(x,y)
	if not self.dragging then return end
	local dy=y-self.dragging.oy
	local cell=self.dragging.cell
	local crow=self.dragging.crow
	local li=self.dragging.layout
	--local rh=self.dragging.rh
	
	local _,mh=self:getDimensions()
	local delta=mh-li.reqHeight
	local ws,_wl=0,0
	for i=1,#li.weightY do 
		local w=li.weightY[i]
		ws+=w
		if ((i&1)==1) and w>0 then _wl=i end
	end
	
	local lp=self.dragging.lp
	local lpr=lp.rowWeights
	local nlp=self.dragging.nlp
	local nw=nlp.rowWeights
	
	local wm=dy*ws/delta
	local sp=cell.row
	local lwm=wm
	if dy>0 then
		while sp<=#nw and wm>0 do
			local w1=lpr[sp]><wm
			wm-=w1
			nw[sp]=lpr[sp]-w1			
			sp+=1
		end
		nw[crow]=lpr[crow]+lwm-wm
	else
		while sp>0 and wm<0 do
			local w1=lpr[sp]><-wm
			wm+=w1
			nw[sp]=lpr[sp]-w1			
			sp-=1
		end		
		nw[cell.row+1]=lpr[cell.row+1]-lwm+wm
	end
	--print(dy,cell.row,json.encode(lpr),delta,ws,_wl,wm,json.encode(nw))
	
	for i=2,#nw-1,2 do
		local other=self.tabs[i/2]
		if other then
			local lc = {}
			lc.gridy=other.row
			lc.fill=Sprite.LAYOUT_FILL_BOTH
			if nw[i]==0 and self.expanded[other.d] then
				if other.w then
					other.w:setVisible(false)
					other.w:setLayoutConstraints({height=0})
				end
				if other.bg then other.bg:removeFromParent() end
				self.expanded[other.d]=nil
				self:updateTab(other, true)
				other.w:setLayoutConstraints(lc)
			elseif nw[i]>0 and not self.expanded[other.d] then
				if not other.w then
					local onw,nbg=self.builder(other.d, true)
					other.w=UI.Utils.makeWidget(onw,other.d)
					other.bg=nbg
				end
				other.w:setVisible(true)
				if other.bg then 
					lc.gridheight=2
					lc.gridy-=1
					self:addChildAt(other.bg,1)
					other.bg:setLayoutConstraints(lc)
					lc.gridy+=1
					lc.gridheight=nil
				end
				lc.minHeight=-1
				lc.prefHeight=-1
				self:addChild(other.w)
				other.w:setLayoutConstraints(lc)
				self.expanded[other.d]=true
				self:updateTab(other, true)
			end
		end
	end
	
	self:setLayoutParameters(nlp)
	return true
end

function UI.Accordion:onMouseClick(x,y,c)
	UI.Focus:request(self)
	if UI.Selection.hasSelection(self) then
		local mods=UI.Control.Meta.modifiers or 0
		if (mods&(KeyCode.MODIFIER_CTRL|KeyCode.MODIFIER_SHIFT|KeyCode.MODIFIER_META))>0 then
			UI.Selection.handleClickEvent(self,x,y,c)
			return true
		else
			UI.Selection.select(self,{})
		end
	end
    local cell=self:getHeaderAt(x,y)
	if cell then
		local disabled = nil
		if cell.h and cell.h.getFlags then disabled = cell.h:getFlags().disabled end
		if disabled then
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

function UI.Accordion:isExpanded(d)
	local cell=self.datacells[d]
	if not cell then return end
	local lp = self:getLayoutParameters() or {  }
	local lprw = lp.rowWeights or { }
	return lprw[cell.row+1]
end

function UI.Accordion:setExpanded(d,e,event)
	local cell=self.datacells[d]
	if not cell then return end
	local lp = self:getLayoutParameters() or { rowWeights={ }, columnWeights={1} }
	local lc = {}
	local lprw = lp.rowWeights or { }
	local lprh = { }
	local wasExpanded = self.expanded[d]
	local expandedChanged = nil
	local focus = nil
	local disabled = nil
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
		disabled = cell.w:getFlags().disabled
		for i=2,#lprw,2 do
			local other=self.tabs[i/2]
			if other and other~=cell and self.autoCollapse and self.expanded[other.d] then
				lprw[i]=0
				if other.w then
					other.w:setVisible(false)
					other.w:setLayoutConstraints({height=0})
				end
				if other.bg then other.bg:removeFromParent() end
				self.expanded[other.d]=nil
				self:updateTab(other)
			end
		end
		if disabled then --expanded but not visible
			lprw[cell.row+1]=0
			lc.minHeight=0
			lc.prefHeight=0
			cell.w:setVisible(false)
			if cell.bg then cell.bg:removeFromParent() end
		else
			local ew=if type(e)=="number" then e else 1
			lprw[cell.row+1]=(self.expand or 0)*ew
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
		end
		expandedChanged = self.expanded[d]~=wasExpanded
		focus = expandedChanged
	else
		self.expanded[d]=nil
		lprw[cell.row+1]=0
		lc.minHeight=0
		lc.prefHeight=0
		if cell.w then cell.w:setVisible(false) end
		if cell.bg then cell.bg:removeFromParent() end
		expandedChanged = self.expanded[d]~=wasExpanded
		focus = false
	end
	if cell.w then cell.w:setLayoutConstraints(lc) end
	if next(self.expanded) then
		lprw[#lprw]=0
	else
		lprw[#lprw]=self.expand or 0
	end
	for i=1,#lprw,1 do lprh[i]=0 end
	lp.rowHeights=lprh
	self:setLayoutParameters(lp)
	self:updateTab(cell,event)
	if focus then
		self:getLayoutInfo()
		UI.Focus:clear()
		local h = cell.h:getHeight()
		if cell.w and cell.w:isVisible() then h = cell.w:getY()+cell.w:getHeight()-cell.h:getY() end
		UI.Focus:area(self,0,cell.h:getY(),self:getWidth(),h,true,0,0)
	end
	self:updateVisible()
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
	cell.h:setFlags({ expanded=(self.expanded[cell.d] or false) },event)
	if cell.h.uiUpdate then print("UI.Accordion: uiUpdate is deprecated")
		cell.h:uiUpdate(cell.d,{ expanded=self.expanded[cell.d] },event) --d,mode
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
			if keep[d] then cache[d]=self.tabs[i] end
		end
	end
	self.data=data
	self.builder=builder
	local lp=self:getLayoutParameters() or {}
	lp.columnWidths={0}
	lp.columnWeights={1}
	lp.rowWeights={ }
	self.expanded=self.expanded or {}
	
	local nsel={ }
	if data then
		local nd=0
		for i,d in ipairs(data) do
			if self.selection[d] then nd+=1 nsel[nd]=d end
		end
	end

	local hdrStart=1
	local bgStart=1
	for _,c in pairs(self.tabs) do
		if not cache or not cache[c.d] then
			self.expanded[c.d]=nil
			c.h:removeFromParent()
			if c.w then c.w:removeFromParent() end
			if c.bg then c.bg:removeFromParent() end
			if c.h.destroy then c.h:destroy() end 
			if c.w and c.w.destroy then c.w:destroy() end 
			if c.bg and c.bg.destroy then c.bg:destroy() end
		end
	end
	self.tabs={}
	self.headers={}
	self.datacells={}
	if data then
		for i,d in ipairs(data) do
			table.insert(lp.rowWeights,0)
			local cell = cache and cache[d] or self:buildTab(d)
			cell.d=d
			cell.row=i*2-1
			self.tabs[i]=cell
			self:addChildAt(cell.h,hdrStart)
			hdrStart+=1
			cell.h:setLayoutConstraints{ gridy=cell.row-1, fill=Sprite.LAYOUT_FILL_HORIZONTAL, anchor=Sprite.LAYOUT_ANCHOR_NORTH }
			local e=self.expanded[d]
			if cell.bg and e then 
				self:addChildAt(cell.bg,bgStart) 
				cell.bg:setLayoutConstraints{ gridy=cell.row-1}
				bgStart+=1
				hdrStart+=1
			end
			local rwg=0
			if cell.w and e then
				cell.w:setLayoutConstraints{ gridy=cell.row }
				local ew=if type(e)=="number" then e else 1
				rwg=(self.expand or 0)*ew
			end
			table.insert(lp.rowWeights,rwg)
			self.headers[cell.h]=cell
			self.datacells[d]=cell
		end
	end
	local lExpand=0
	if not next(self.expanded) then
		lExpand=self.expand or 0
	end
	table.insert(lp.rowWeights,lExpand) -- Last row contains nothing, but is used as a filler
	self:setLayoutParameters(lp)
	self:updateVisible()
	
	UI.Selection.select(self,nsel)
end

function UI.Accordion:updateVisible(y,viewh)
	if not self.data or #self.data==0 then return end
	y=y or self.rangey or 100000
	viewh=viewh or self.rangeh
	if not viewh then return end
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
	viewh+=y
	local lf=ll
	while ll<=lmax and ls[ll*2-1] and viewh>0 do viewh-=ls[ll*2-1]+ls[ll*2]+lcs ll+=1 end
	ll-=1
	--print(self.name,lf,ll,y,viewh)
	ll=(ll<>lf)
	if lf>0 then
		local cell = self.tabs[1] --Get first header
		--cell.w --This only deals with headers, background and contents are not hidden --TODO : hidden w
		--cell.bg --This only deals with headers, background and contents are not hidden --TODO : hidden bg
		local i0=self:getChildIndex(cell.h) --Get first header
		local i1=i0+lf-2
		local i2=i0+ll
		local i3=i0+#self.data-1
		--print(self.name,"Hidden:",i0,i1,i2,i3)
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
	UI.Dnd.Source(self,self.allowDnD,"auto")
	UI.Dnd.Target(self,self.allowDnD)
end

function UI.Accordion:probeDndData(x,y)
    local eb=self:getChildrenAtPoint(x,y,true,true,self)
	for _,hdr in ipairs(eb) do
		local cell=self.headers[hdr]
		if cell then
			return true
			--local marker=self:getDndMarker(cell.d)
		end
	end
end

function UI.Accordion:getDndData(x,y)
    local eb=self:getChildrenAtPoint(x,y,true,true,self)
	for _,hdr in ipairs(eb) do
		local cell=self.headers[hdr]
		if cell then
			if next(self.selection) and not self.selection[cell.d] then
				--Currently dragged item isn't in selection, clear selection
				UI.Selection.select(self,{})
			end
			local vdata,vlist
			
			if self.selection[cell.d] and next(self.selection,next(self.selection)) then
				vlist={}
				for k,_ in pairs(self.selection) do vlist[#vlist+1]=k end
			else
				vdata=cell.d
			end
			local marker=self:getDndMarker(cell.d)
			if marker then
				local chc=UI.Utils.colorVector("dnd.colSrcHighlight",self._style)
				local srcMarker=Pixel.new(chc,cell.h:getSize())
				cell.h:addChild(srcMarker)
				
				self.dndSrcMarker=srcMarker
				
				return { type=UI.Accordion, value=vdata, list=vlist, visual=marker }
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
	local function clearDstMarker(remove)
		if self.dndDstMarker then
			self.dndDstMarker:setVisible(false)
			if remove then
				self.dndDstMarker:removeFromParent()
				self.dndDstMarker=nil
			end
		end
		if self.dndDstMarkerOver then
			self.dndDstMarkerOver:setVisible(false)
			if remove then
				self.dndDstMarkerOver:removeFromParent()
				self.dndDstMarkerOver=nil
			end
		end
	end
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
			local lls=ls[ll*2-1]+(ls[ll*2] or 0)+lcs 
			y-=lls
			ys+=lls
			ll+=1 
		end
		
		local sh=(ls[ll*2-1] or 0)+(ls[ll*2] or 0)+lcs
		
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
			local ret=(not self.checkDndOffer) or self:checkDndOffer(data,x,y,nil,ll) 
			if ret==nil then 
				clearDstMarker(true)
			else
				clearDstMarker()
				self.dndDstMarker:setVisible(true)	
			end
			return ret
		elseif over and self.tabs[over] then
			local ret=(not self.checkDndOffer) or self:checkDndOffer(data,x,y,self.tabs[over].d,ll)
			if ret==nil then 
				clearDstMarker(true)
			else
				clearDstMarker()
				if not self.dndDstMarkerOver then
					local chc=UI.Utils.colorVector("dnd.colDstHighlightOver",self._style)
					local dstMarker=Pixel.new(chc)
					self.dndDstMarkerOver=dstMarker
				end
				local ocell=self.tabs[over]
				self.dndDstMarkerOver:setDimensions(ocell.h:getSize())
				ocell.h:addChild(self.dndDstMarkerOver)
				self.dndDstMarkerOver:setVisible(true)
				return ret
			end
		end
	else
		clearDstMarker(true)
		self.cachedLayout=nil
	end
end

function UI.Accordion:setDndData(data,source)
	if data and data.type==UI.Accordion then
		if data.insert then
			if source==self then
				local val=if data.list then data.list else { data.value }
				local shouldMove = UI.dispatchEvent(self,"ItemsMove",val,data.insert) or val
				if shouldMove then
					local ln=#shouldMove
					for i=ln,1,-1 do
						self:moveItem(shouldMove[i],data.insert)
					end
				end
			else
				UI.dispatchEvent(self,"DndDrop",source,data,nil,data.insert) --no over
			end
		elseif data.over then
			UI.dispatchEvent(self,"DndDrop",source,data,self.tabs[data.over].d,nil) --no insert
		end
	end
end

-- Selection
function UI.Accordion:uiSelect(x,y)
    local cell=self:getHeaderAt(x,y)
	if not cell or not cell.h then return end
	if cell.h:getFlags().notselectable then return end
	return cell.h,cell.d
end

function UI.Accordion:uiSelectRange(d1,d2)
	local r1,r2=self.datacells[d1],self.datacells[d2]
	local ret={}
	if r1 and r2 then 
		local s1,s2=(r1.row+1)//2,(r2.row+1)//2
		local i1,i2=s1><s2,s1<>s2
		for i=i1,i2 do
			local cell=self.tabs[i]
			if cell and not cell.h:getFlags().notselectable then
				ret[cell.h]=cell.d
			end
		end
	end
	return ret
end

function UI.Accordion:uiSelectDirection(d,dx,dy)
	local r=self.datacells[d]
	if dx and dy then 
		local dd=dy//1
		local i=(r.row+1)//2+dd
		local cell=self.tabs[i]
		if cell and not cell.h:getFlags().notselectable then 
			return { [cell.h]=cell.d }
		end
	end
end

function UI.Accordion:uiSelectAll()
	local ret={}
	for _,cell in ipairs(self.tabs) do
		if not cell.h:getFlags().notselectable then
			ret[cell.h]=cell.d
		end
	end
	return ret
end

function UI.Accordion:uiSelection(sel)
	if self.selection then
		for data,rowWidget in pairs(self.selection) do
			if not sel or not sel[rowWidget] or rowWidget:getFlags().notselectable then
				self.selection[data]=nil
				rowWidget:setStateStyle({})
				rowWidget:setFlags({selected=false})
			end
		end
	end
	if sel then
		for rowWidget,data in pairs(sel) do
			if not self.selection[data] and not rowWidget:getFlags().notselectable then
				self.selection[data]=rowWidget
				rowWidget:setStateStyle("accordion.styHeaderSelected")
				rowWidget:setFlags({selected=true})
			end
		end
	end
end

function UI.Accordion:uiSelectData(d) --spr,data
	if d and self.datacells[d] then
		local cell_h = self.datacells[d].h
		if cell_h:getFlags().notselectable then return end
		return cell_h,d --spr,data
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
		{ name="AutoCollapse", type="boolean", setter=UI.Panel.setAutoCollapse },
	},
}
