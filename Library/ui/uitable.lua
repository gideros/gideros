--!NEEDS:uipanel.lua
UI.Table=Core.class(UI.Panel,function() return nil end)

--[[ Column description
string (field name) or
{ field="field" or function(), name="header" or Sprite, width=number, weight=number }
]]

--[[
	setData() accepts a field builder and a rowBuilder (both optional)
	if builder is present:
	- it is called with data, column description, and ghost capability as arguments
	- it returns actual data, a Sprite, or a ghost if allowed as first result
	- it may return actual data as a second result, if first result was a sprite or a ghost
	if rowBuilder is present, it is called with row index and row data as arguments to produce a row container
]]
function UI.Table:init(columns,direction,cellSpacingX,cellSpacingY,fixed)--no data
	self._istyle.__Cache={}
	self.headers={}
	self.cells={}
	self.data={}
	self.dataRows={}
	self.direction=direction --Horizontal if not false or nil
	self.fixedGrid=Sprite.setGhosts and fixed
	self:setWorldAlign(not Oculus)
	self:setLayoutParameters{ equalizeCells=true, worldAlign=not Oculus, fixedGrid = self.fixedGrid }
	self.cheader=UI.Panel.new()
	self.cheader:setLayoutConstraints{ gridy=0, gridx=0, fill=Sprite.LAYOUT_FILL_BOTH }
	self.cheader:setStyle("table.styRowHeader")
	self.cheader:setLocalStyle("table.styRowHeaderLocal")
	self:setColumns(columns)
	self:setCellSpacingX(cellSpacingX)
	self:setCellSpacingY(cellSpacingY)
	self.minRowSize=0
	self.selection={}
	UI.Control.onMouseClick[self]=self
	UI.Control.onLongClick[self]=self
	UI.Control.onLongPrepare[self]=self
	UI.Control.onKeyDown[self]=self
	self:setCheckClip(true)
end

function UI.Table:updateStyle(...)
	if not self.__cacheUpdating then
		local function updateRowsStyle()
			if self.datarows then 
				for _,r in ipairs(self.datarows) do
					if r.ghosts then
						r._ghostStyleCache={}
						for j=1,#self.columns,1 do
							local cellCode=vector(r.row,j,0,0)
							local cell = self.cells[cellCode]
							if cell then
								if cell.ghostModel and cell.updateGhostStyle then
									cell:updateGhostStyle()
								end
							end
						end
						r:setGhosts(r.ghosts) 
					end
				end
			end
		end
		if next(self._istyle.__Cache) then
			self.__cacheUpdating=true
			self._istyle.__Cache={}
			self:updatedStyle() --This will trigger another style update on self, prevent looping
			updateRowsStyle()
		elseif self.hasGhosts then
			updateRowsStyle()
		end
	else		
		self.__cacheUpdating=nil
	end
	UI.Panel.updateStyle(self,...)
end
	
function UI.Table:newClone() 
	local _columns,_data=self.columns,self.data
	self.columns,self.data=nil,nil
	self:setData(nil,self.builder)
	self:setColumns(nil)
	self.headers={}
	self.cells={}
	self.dataRows={}
	self:setColumns(_columns)
	self:setData(_data,self.builder)
	UI.Control.onMouseClick[self]=self
	UI.Control.onKeyDown[self]=self
end

function UI.Table:onMouseClick(x,y,c)
	local rn,cn,_,_,hdr=self:getRowColumnFromPoint(x,y)
	local nc=cn and self.columns[cn]
--[[ NICO: I'd prefer this	
	if nc and nc.Clickable and hdr then
		UI.dispatchEvent(self.widget,"ColumnAction",cn,nc)
	end
]]
	UI.Focus:request(self)
	
	local nh=cn and hdr and self.headers[cn]
	if nh and nh.onClickColumn then
		local handler=nh:onClickColumn(nc)
		if handler then
			self.data = handler(self,self.data,nc)
			self:setData(self.data,self.builder)
			return true -- TODO should use handler result
		end
	end
	
	local d=cn and rn and not hdr and self.data[rn]
	if d and type(d)=="table" then
		if d.onClickDoubleCell and c>1 then
			local nh=self.cells[vector(rn,cn,0,0)]
			nh.indexCell=cn
			nh.rowData=d
			if nh then d.onClickDoubleCell(self,nc,cn,d) return true end
		end
		if d.onClickCell then
			local nh=self.cells[vector(rn,cn,0,0)]
			nh.indexCell=cn
			nh.rowData=d
			if nh then d.onClickCell(self,nc,cn,d) return true end
		end
	end

	return false
end

function UI.Table:onLongClick(x,y)
	local rn,cn,_,_,hdr=self:getRowColumnFromPoint(x,y)
	local nc=cn and self.columns[cn]
--[[ NICO: I'd prefer this	
	if nc and nc.LongClickable and hdr then
		UI.dispatchEvent(self.widget,"ColumnLongAction",cn,nc)
	end
]]
	local nh=cn and hdr and self.headers[cn]
	if nh and nh.onClickLongColumn then
		nc.indexCol=cn
		local handler=nh:onClickLongColumn(nc)
		if handler then 
			handler(self,nc)
			return true 
		end		
	end
	
	local d=rn and not hdr and self.data[rn]
	if d and type(d)=="table" then
		if d.onClickLongCell then
			local nh=self.cells[vector(rn,cn,0,0)]
			nh.indexCell=cn
			nh.rowData=d
			if nh then d.onClickLongCell(self,nc,cn,d) end
		end
	end

end
function UI.Table:onLongPrepare(x,y,r)
	local rn,cn,_,_,hdr=self:getRowColumnFromPoint(x,y)
	local nc=cn and hdr and self.columns[cn]
--[[ NICO: I'd prefer this	
	if nc and nc.LongClickable then
	end
]]
	local nh=cn and hdr and self.headers[cn]
	local d=rn and not hdr and self.data[rn]
	if (nh and nh.onClickLongColumn) or
		(d and type(d)=="table" and d.onClickLongCell) then
		if not self.prepare then 
			self.prepare=UI.Behavior.LongClick.makeIndicator(self,{}) 
		end
		self.prepare:indicate(self,x,y,r)
		return true
	elseif self.prepare then
		self.prepare:indicate(self,x,y,-1)
	end
end

UI.Table.onKeyDown=UI.Selection.handleKeyEvent

function UI.Table:updateRow(rowWidget)
	if self and rowWidget and rowWidget.d then
		local rowsel = if self.selection[rowWidget.d] then true else false
		
		if self.columns and rowWidget.row and rowWidget.isSelected~=rowsel then
			rowWidget._ghostStyleCache={}
			for j=1,#self.columns,1 do
				local cellCode=vector(rowWidget.row,j,0,0)
				local cell = self.cells[cellCode]
				if cell then
					if cell.ghostModel then
						if cell.setGhostState then
							cell:setGhostState(rowWidget,rowsel and "table.styCellSelected" or "table.styCell")
						end
					else
						cell:setStateStyle(rowsel and "table.styCellSelected" or "table.styCell")
						cell:setStyleInheritance("state")
					end
				end
			end
			rowWidget.isSelected=rowsel
			if rowWidget.ghosts then
				rowWidget:setGhosts(rowWidget.ghosts) 
			end
		end
	end
end

function UI.Table:setColumns(columns)
	local lp={}
	lp.columnWidths={}
	lp.columnWeights={}
	self.columns=columns or {}
	self.cachedLayout=nil
	for i,c in ipairs(self.columns) do
		lp.columnWidths[i]=c.width or 0
		lp.columnWeights[i]=c.weight or 0
		local cnh = self.headers[i]
		if cnh then
			if cnh.destroy then cnh:destroy() end
			cnh:removeFromParent()
		end
		local nh=nil
		if c.font then
			print("DEPRECATED: Column Header font is deprecated, use Column header style instead")
			c.style={ font=c.font }
		end
		if type(c.name)=="function" then
			nh=c.name(c)
		else
			nh=c.name
		end
		if nh then
			nh=UI.Utils.makeWidget(nh,nil)
			if c.style then nh:setStyle(c.style) end
		end
		self.headers[i]=nh
		if nh then
			self.cheader:addChild(nh)
			local gx,gy=0,0
			if self.direction then gy=i-1 else gx=i-1 end
			nh:setLayoutConstraints{ gridy=gy,gridx=gx,ipadx=2,fill=Sprite.LAYOUT_FILL_BOTH }
		end
	end
	while #self.headers>#self.columns do
		table.remove(self.headers):removeFromParent()
	end
	self.cheader:setVisible(#self.headers>0)
	local gw,gh=1,1
	if self.direction then gh=#self.columns else gw=#self.columns end
	self.cheader:setLayoutConstraints{ gridy=0,gridx=0,gridwidth=gw,gridheight=gh,fill=Sprite.LAYOUT_FILL_BOTH, group=true }
	self:setLayoutParameters(lp)
	if self.dataRows then
		for _,rw in pairs(self.dataRows) do
			rw.cache={}
		end
	end
	self:setData(self.data,self.builder)
end
function UI.Table:setCellSpacingX(cellSpacingX)
	self:setLayoutParameters({cellSpacingX=cellSpacingX})
end
function UI.Table:setCellSpacingY(cellSpacingY)
	self:setLayoutParameters({cellSpacingY=cellSpacingY})
end
function UI.Table:setEqualizeCells(equalizeCells)
	self:setLayoutParameters({ equalizeCells = equalizeCells})
end
function UI.Table:setFixedGrid(fixed)
	self.fixedGrid=Sprite.setGhosts and fixed
	self:setLayoutParameters({ fixedGrid = self.fixedGrid})
end
function UI.Table:setMinimumRowSize(rowSize)
	self.minRowSize=rowSize
	self:setData(self.data,self.builder)
end
function UI.Table:setOddEvenStyle(en)
	self.oddEvenStyle=en
	self:setData(self.data,self.builder)
end

function UI.Table:setView(view)
	local nsel={ }
	if view then
		self.viewModel={}
		self.modelView={}
		for n,m in ipairs(view) do 
			local d=self.data[m]
			if d then
				if self.selection[d] then table.insert(nsel,d) end
				table.insert(self.viewModel,m)
				self.modelView[m]=#self.viewModel
			end
		end
	else
		for k,_ in pairs(self.selection) do table.insert(nsel,k) end
		self.viewModel=nil
		self.modelView=nil
	end
	UI.Selection.select(self,{})
	for _,n in pairs(self.dataRows) do n:removeFromParent() end
	local weights={ 0 }
	local heights={ self.minRowSize }
	for i,n in ipairs(self.viewModel) do
		table.insert(weights,0)
		table.insert(heights,self.minRowSize)
		local rowWidget=self.dataRows[self.data[n]]
		rowWidget:setLayoutConstraints(
			if self.direction then {
				gridx=i, 
				gridheight=#self.columns,
				fill=Sprite.LAYOUT_FILL_BOTH,
				group=true,
			}
			else {
				gridy=i, 
				gridwidth=#self.columns,
				fill=Sprite.LAYOUT_FILL_BOTH,					
				group=true,
			})
		if self.oddEvenStyle then
			rowWidget:setBaseStyle(if (i&1)==1 then "table.styRowOdd" else "table.styRowEven")
		end
		self:addChild(rowWidget)
		for j,c in ipairs(self.columns) do
			local nhd=self.cells[vector(n,j,0,0)]
			local gx,gy=0,0
			if self.direction then gx=i gy=j-1 else gy=i gx=j-1 end
			nhd:setLayoutConstraints({ gridx=gx, gridy=gy})
		end
	end
	table.insert(weights,1)
	local lp={}
	if self.direction then 
		lp.columnWeights=weights
		lp.columnWidths=heights
	else 
		lp.rowWeights=weights
		lp.rowHeights=heights
	end
	self:setLayoutParameters(lp)
	self:addChild(self.cheader)
	UI.Selection.select(self,nsel)
end

function UI.Table:setData(data,builder,rowBuilder) --! will keep selection (call resetData)
	self.viewModel=nil
	self.modelView=nil
	self.cachedLayout=nil
	
	local cache
	self.hasGhosts=nil
	if data and self.data and builder==self.builder then
		cache={}
		local keep={}
		for _,d in ipairs(data) do keep[d]=true end
		for i,d in ipairs(self.data) do
			if keep[d] then
				cache[d]=self.dataRows[d]
			end
		end
	end

	for cellCode,nh in pairs(self.cells) do 
		local d=self.data[cellCode.x]
		local c=self.columns[cellCode.y]
		local cacheKey=c and (builder or c.field)
		if (nh.removeFromParent) and not (cacheKey and cache and cache[d] and cache[d].cache[c] and cache[d].cache[c].cacheKey==cacheKey) then 			
			if nh.destroy then nh:destroy() end 
			nh:removeFromParent()
			UI.Control.onMouseClick[nh]=nil
			if nh.behaviorPrepare then
				nh.behaviorPrepare:destroy()
				nh.behaviorPrepare=nil
				nh.onWidgetLongAction=nil
			end
		end
	end
	self.cells={}

	self.data=data or {}
	for d,n in pairs(self.dataRows) do if not (cache and cache[d]) then n:removeFromParent() end end
	self.dataRows={}
	self.builder=builder
	local nsel={ }
	if data then
		local nd=0
		for i,d in ipairs(data) do
			if self.selection[d] then nd+=1 nsel[nd]=d end
		end
	end
	UI.Selection.select(self,{})
	
	local weights={ 0 }
	local heights={ self.minRowSize }
	local allowGhost=self.fixedGrid
	if data then
		for i,d in ipairs(data) do
			weights[i+1]=0
			heights[i+1]=self.minRowSize								
			local rowWidget=(cache and cache[d]) or (rowBuilder and rowBuilder(i,d)) or UI.Panel.new()
			self.dataRows[d]=rowWidget
			local lastIndex=rowWidget.row
			if not rowWidget.row then
--				rowWidget._istyle.__Cache={}
				rowWidget.d=d
				rowWidget.cache={}
			end
			rowWidget.row=i
			if i~=lastIndex then
				rowWidget:setLayoutConstraints(
					if self.direction then {
						gridx=i, 
						gridheight=#self.columns,
						fill=Sprite.LAYOUT_FILL_BOTH,
						group=true,
					}
					else {
						gridy=i, 
						gridwidth=#self.columns,
						fill=Sprite.LAYOUT_FILL_BOTH,					
						group=true,
					})
				if self.oddEvenStyle and not (lastIndex and ((i&1)==(lastIndex&1))) then
					rowWidget:setBaseStyle(if (i&1)==1 then "table.styRowOdd" else "table.styRowEven")
				end
			end
			local ghosts
			for j,c in ipairs(self.columns) do
				local cellCode=vector(i,j,0,0)
				local nh,nhd = nil,nil
				local cacheKey=builder or c.field
				if cacheKey and cache and cache[d] and cache[d].cache[c] and cache[d].cache[c].cacheKey==cacheKey then
					nh=cache[d].cache[c]
				else
					if builder or type(c.field)=="function" then
						if builder then nh,nhd=builder(d,c,allowGhost)
						else nh,nhd=c.field(d,allowGhost) end
					else
						nh=d[c.field]
					end
					nhd=nhd or nh
					if not (nh and nh.ghostModel) then
						nh=UI.Utils.makeWidget(nh,nhd,nil)
					end
					rowWidget.isSelected=nil
				end
				if not nh.ghostModel then rowWidget.cache[c]=nh end
				nh.cacheKey=cacheKey
				local sameLoc=nh.indexRow==i
				nh.indexRow=i --Row index in data array --need?
				if nh.indexColumn~=j then
					if nh.ghostModel then
						ghosts=ghosts or {}
						ghosts[#ghosts+1]=nh.ghostModel
					elseif allowGhost then
						rowWidget:addChild(nh)
					else
						rowWidget:addChildAt(nh,j)
					end
					nh.indexColumn=j
					sameLoc=false
				end
				nh.d=nhd --Cell Data (resolved by builder/field function)
				self.cells[cellCode]=nh
				if not sameLoc then
					local gx,gy=0,0
					if self.direction then gx=i gy=j-1 else gy=i gx=j-1 end
					if nh.ghostModel then
						nh.ghostModel.gridx=gx
						nh.ghostModel.gridy=gy
					else
						nh:setLayoutConstraints({gridx=gx, gridy=gy})
					end
				end
			end
			rowWidget.ghosts=ghosts
			self:addChildAt(rowWidget,i) --NO if i~=lastIndex then self:addChildAt(rowWidget,i) end
			self:updateRow(rowWidget)
			self.hasGhosts=true
		end
	end
	table.insert(weights,1)
	local lp=self:getLayoutParameters()
	if self.direction then 
		lp.columnWeights=weights
		lp.columnWidths=heights
	else 
		lp.rowWeights=weights
		lp.rowHeights=heights
	end
	self:setLayoutParameters(lp)
	self:addChild(self.cheader)
	for _,h in ipairs(self.headers) do
		self.cheader:addChild(h)
	end
	UI.Selection.select(self,nsel)
end
function UI.Table:resetData() --reset selection
	self.selection={}
	self:setData()	
end

function UI.Table:moveColumn(index,to)
	local function swap(t)
		table.insert(t,to,table.remove(t,index))
	end
	local imap={}
	for j=1,#self.columns do imap[j]=j end
	swap(imap)
	swap(self.columns)
	--Double check: nil in arrays...
	swap(self.headers)
	for i,h in ipairs(self.headers) do
		local gx,gy=0,0
		if self.direction then gy=i-1 else gx=i-1 end
		h:setLayoutConstraints{ gridy=gy,gridx=gx }
		local column = self.columns[i]
		if column then column.indexCol=i end
		Sprite.addChildAt(self.cheader,h,i) --Use Sprite call to avoid restyling
	end
	
	local allowGhost=self.fixedGrid
	if self.data then
		local ncells={}
		for i,d in ipairs(self.data) do
			local ii=if self.modelView then self.modelView[i] else i
			if ii then
				local rowWidget=self.dataRows[d]
				for j,c in ipairs(self.columns) do
					local nhd=self.cells[vector(i,imap[j],0,0)]
					ncells[vector(i,j,0,0)]=nhd
					if nhd.indexCell then nhd.indexCell=j end
					if not allowGhost then
						Sprite.addChildAt(rowWidget,nhd,j) --Use Sprite call to avoid restyling
					end
					local gx,gy=0,0
					if self.direction then gx=ii gy=j-1 else gy=ii gx=j-1 end
					if nhd.ghostModel then
						nhd.ghostModel.gridx=gx
						nhd.ghostModel.gridy=gy
					else
						nhd:setLayoutConstraints({gridx=gx, gridy=gy})
					end
				end
				if allowGhost then
					rowWidget:setGhosts(rowWidget.ghosts)
				end
			end
		end
		self.cells=ncells
	end

	local lp=self:getLayoutParameters()
	if self.direction then 
		swap(lp.rowWeights)
		swap(lp.rowHeights)
	else 
		swap(lp.columnWeights)
		swap(lp.columnWidths)
	end
	self:setLayoutParameters(lp)
	self:addChild(self.cheader)
end

function UI.Table:uiSelectData(d) --spr,data --without onMouseClick --force row selection
	if self and d and self.dataRows[d] then
		return self.dataRows[d],d --spr,data
	end
end

function UI.Table:getDataRow(d)
	return self.dataRows[d] and self.dataRows[d].row
end

function UI.Table:uiAdjustScrollArea(xi,yi,xa,ya)
	local ch=self.cheader:getHeight()	
	if ch>0 then
		local _,ay=self.cheader:getAnchorPosition()
		local cm=ch-ay
		if yi<cm then -- If our proposed scroll goes under header bar, reduce scroll
			yi-=ch
		end
	end
	return xi,yi,xa,ya
end

function UI.Table:uiSelect(x,y)
	local rn,cn,_,_,hdr=self:getRowColumnFromPoint(x,y)
	if hdr or not rn then return end
	local data=self.data[rn]
	return self.dataRows[data],data
end

function UI.Table:uiSelectRange(d1,d2)
	local r1,r2=self.dataRows[d1],self.dataRows[d2]
	local ret={}
	if r1 and r2 then 
		local i1,i2=r1.row><r2.row,r1.row<>r2.row
		for i=i1,i2 do
			local ii=if self.viewModel then self.viewModel[i] else i
			local d=self.data[ii]
			ret[self.dataRows[d]]=d
		end
	end
	return ret
end

function UI.Table:uiSelectDirection(d,dx,dy)
	local r=self.dataRows[d]
	if dx and dy then 
		local dd=(if self.direction then dx else dy)//1
		local i=r.row+dd
		local ii=if self.viewModel then self.viewModel[i] else i
		local dd=self.data[ii]
		if dd then 
			local dr=self.dataRows[dd]
			if dr then 
				return { [dr]=dd }
			end
		end
	end
end

function UI.Table:uiSelectAll()
	local ret={}
	local c=(self.viewModel and #self.viewModel) or (self.data and #self.data) or 0
	for i=1,c do
		local ii=if self.viewModel then self.viewModel[i] else i
		local d=self.data[ii]
		ret[self.dataRows[d]]=d
	end
	return ret
end

function UI.Table:uiSelection(sel) --row selection only --implements Row DATA .ClickCell and .onClickCell to have cell selection
	if self.selection then
		for data,rowWidget in pairs(self.selection) do
			if not sel or not sel[rowWidget] then
				self.selection[data]=nil
				rowWidget:setStateStyle({})
				self:updateRow(rowWidget)
			end
		end
	end
	if sel then
		for rowWidget,data in pairs(sel) do
			if not self.selection[data] then
				self.selection[data]=rowWidget
				rowWidget:setStateStyle("table.styRowSelected")
				self:updateRow(rowWidget)
			end
		end
	end
end

function UI.Table:updateVisible(y,viewh)
	y=y or self.rangey or 100000
	viewh=viewh or self.rangeh
	self.rangey=y
	self.rangeh=viewh
	local li=self:getLayoutInfo()
	local lp=Sprite.getLayoutParameters(self,true)
	local ls=li.minHeight
	local lmax=if self.viewModel then #self.viewModel else #self.data
	local ll=1
	y-=li.starty
	local csy=(lp.cellSpacingY+.5)//1
	while ll<=lmax and ls[ll] and y>ls[ll] do y-=ls[ll]+csy ll+=1 end
	local lf=ll<>2
	while ll<=lmax and ls[ll] and viewh>0 do viewh-=ls[ll] ll+=1 end
	lf-=1
	ll=(ll<>(lf+1))
	if lf>0 and ll>lf then
		local i0=1
		local i1=lf-1
		local i2=ll+1
		local i3=lmax
		self:setHiddenChildren({
				{i0,i1},
				{i2,i3}
			})
	else
		self:setHiddenChildren(nil)
	end
end

function UI.Table:onViewportScrolled(viewport,x,y,rangex,rangey,vieww,viewh)
	self.cheader:setAnchorPosition(0,-y)
	self:updateVisible(y,viewh)
end

-- Column Resizing
function UI.Table:setColumnWidth(columnIndex,width)
	local lp=self:getLayoutParameters() or {}
	local col = self.columns[columnIndex]
	if col then col.width = width or 0 end
	lp.columnWidths[columnIndex]=width or 0
	self:setLayoutParameters(lp)
end

function UI.Table:setAllowColumnResize(en)
	self.allowColumnResize=en
	local cen
	if en then cen=self end
	UI.Control.onDragStart[self]=cen
	UI.Control.onDrag[self]=cen
	UI.Control.onDragEnd[self]=cen
	UI.Control.onMouseMove[self]=UI.Control.HAS_CURSOR and cen
end

function UI.Table:getRowColumnFromPoint(x,y)
	local ch=self.cheader:getHeight()
	local cy=self.cheader:getY()
	local _,cay=self.cheader:getAnchorPosition() cy-=cay
	local lp=Sprite.getLayoutParameters(self,true) or {}
	local li=self:getLayoutInfo()
	local ls,d,lcs
	if self.direction then 
		ls=li.minHeight 
		d=y-li.starty 
		lcs=(lp.cellSpacingY+.5)//1 
	else
		ls=li.minWidth
		d=x-li.startx 
		lcs=(lp.cellSpacingX+.5)//1 
	end
	local n=1
	while ls[n] and d>ls[n] do
		d-=(ls[n]+lcs)
		n+=1
	end
	if d<0 then n=nil end
	
	local gy=1
	local ls=li.minHeight
	local d2=y-li.starty
	local sp=(lp.cellSpacingY+.5)//1
	if self.direction then ls=li.minWidth d2=x-li.startx sp=(lp.cellSpacingX+.5)//1 end
	while gy<=#ls and d2>=ls[gy] do d2-=ls[gy]+sp gy+=1 end
	if gy>=2 then 
		if self.viewModel then gy=self.viewModel[gy] end
		gy-=1
	else
		gy=nil
	end
	return gy,n,d2,d,(y>=cy) and (y<(cy+ch))
end

function UI.Table:checkResizePoint(x,y,forDrag)
	local ch=self.cheader:getHeight()
	local cy=self.cheader:getY()
	local _,cay=self.cheader:getAnchorPosition() cy-=cay
	if not #self.headers or y<cy or y>(cy+ch) then return nil end
	local sw=self:resolveStyle("table.szResizeHandle")/2
	local lp=Sprite.getLayoutParameters(self,true) or {}
	local lcs=(lp.cellSpacingX or 0)/2
	local li=self:getLayoutInfo()
	--local th=li.reqHeight
	for n,h in ipairs(self.headers) do
		local hx=h:getX()+li.startx
		local hw=li.minWidth[n]<>h:getWidth()
		local edge=hx+hw+lcs
		if x>(edge-sw) and x<(edge+sw) then
			local tw=0
			for _,w in ipairs(li.weightX) do tw+=w end
			if tw==0 then tw=1 end
			if forDrag then
				self._colRsz={columnIndex=n, point=x, bw=lp.columnWidths[n],bgap=(hw-lp.columnWidths[n])*tw/li.weightX[n],wg=li.weightX[n]/tw}
			end
			return true
		end
	end	
end

function UI.Table:onMouseMove(x,y)
	UI.Control.setLocalCursor(if self._colRsz or self:checkResizePoint(x,y) then "sizeHor" else nil)
end

function UI.Table:onDragStart(x,y)
	return self:checkResizePoint(x,y,true)
end

function UI.Table:onDrag(x,y)
	if self._colRsz then
		local nw=(x-self._colRsz.point)/(1-self._colRsz.wg)+self._colRsz.bw
		nw=nw<>0
		self._colRsz.newsize=nw
		self:setColumnWidth(self._colRsz.columnIndex,nw)
	end
end

function UI.Table:onDragEnd(x,y)
	if self._colRsz and self._colRsz.newsize then
		self:setColumnWidth(self._colRsz.columnIndex,self._colRsz.newsize)
		UI.dispatchEvent(self,"ColumnResize",self._colRsz.columnIndex,self._colRsz.newsize)
	end
	self._colRsz=nil
end


-- DND
function UI.Table:setAllowColumnReorder(en)
	self.allowColumnReorder=en
	UI.Dnd.Source(self,en)
	UI.Dnd.Target(self,en)
end

function UI.Table:probeDndData(x,y)
	local ch=self.cheader:getHeight()
	local cy=self.cheader:getY()
	local _,cay=self.cheader:getAnchorPosition() cy-=cay
	local li=self:getLayoutInfo()
	if not #self.headers or y<cy or y>(cy+ch) or x<li.startx then return nil end
	return true
end

function UI.Table:getDndData(x,y)
	local ch=self.cheader:getHeight()
	local cy=self.cheader:getY()
	local _,cay=self.cheader:getAnchorPosition() cy-=cay
	local li=self:getLayoutInfo()
	if not #self.headers or y<cy or y>(cy+ch) or x<li.startx then return nil end
	local th=li.reqHeight
	local sw=self:resolveStyle("table.szResizeHandle")/2
	for n,h in ipairs(self.headers) do
		local hx=h:getX()+li.startx
		local hw=li.minWidth[n]<>h:getWidth()
		if x>hx+sw and x<(hx+hw-sw) then	
			local chc=UI.Utils.colorVector("dnd.colSrcHighlight",self._style)
			local srcMarker=Pixel.new(chc,hw,th)
			srcMarker:setPosition(hx,cy)
			self:addChild(srcMarker)
			
			local marker=self:getColumnDndMarker(n,h)
			
			self.dndSrcMarker=srcMarker
						
			return { type=self.headers, value=n, visual=marker }
		end
	end	
end

function UI.Table:getColumnDndMarker(index,columnDef)
	return UI.Dnd.MakeMarker(self.headers[index])
end
	
function UI.Table:cleanupDndData(data,target)
	if data.type==self.headers then
		self.dndSrcMarker:removeFromParent()
		self.dndSrcMarker=nil
	end
	self.cachedLayout=nil
end

function UI.Table:offerDndData(data,x,y)
	if self.dndDstMarker then self.dndDstMarker:setVisible(false) end
	if data and data.type==self.headers then
		if not #self.headers then return end
		if not self.dndDstMarker then
			local li=self:getLayoutInfo()
			local th=li.reqHeight
			local sw=self:resolveStyle("dnd.szInsertPoint")
			local chc=UI.Utils.colorVector("dnd.colDstHighlight",self._style)
			local dstMarker=Pixel.new(chc,sw,th)
			dstMarker:setAnchorPoint(0.5,0)
			self:addChild(dstMarker)
			dstMarker:setVisible(false)
			self.dndDstMarker=dstMarker
		end
		local li=self.cachedLayout or self:getLayoutInfo()
		self.cachedLayout=li
		
		for n,h in ipairs(self.headers) do
			local hx=h:getX()
			local hw=li.minWidth[n]<>h:getWidth()
			if x>hx and x<(hx+hw) then
				local ip=n
				local ipx=hx
				if x>(hx+hw/2) then ip+=1 ipx+=hw end
				if ip==data.value or ip==data.value+1 then 
					ip=nil 
				elseif ip>data.value then
					ip-=1
				end
				if ip~=data.insert then
					data.insert=ip
					local lcs=if (ip and ip>1) then ((Sprite.getLayoutParameters(self,true) or {}).cellSpacingX or 0)/2 else 0
					self.dndDstMarker:setX(ipx-lcs)	
				end
				if ip then
					self.dndDstMarker:setVisible(true)	
					return true
				end
			end
		end	
	else
		self.cachedLayout=nil
		if self.dndDstMarker then
			self.dndDstMarker:removeFromParent()
			self.dndDstMarker=nil
		end
	end	
end

function UI.Table:setDndData(data)
	if data and data.type==self.headers then
		self:moveColumn(data.value,data.insert)
		UI.dispatchEvent(self,"ColumnMove",data.value,data.insert)
	end
end

UI.Table.Definition= {
	name="Table",
	icon="ui/icons/panel.png",
	class="UI.Table",
	constructorArgs={ "Columns","Direction","CellSpacingX","CellSpacingY","FixedGrid" },
	properties={
		{ name="Columns", type="tableColumns", setter=UI.Viewport.setColumns },
		{ name="Data", type="table", setter=UI.Viewport.setData },
		{ name="Direction", type="boolean" },
		{ name="MinimumRowSize", type="number", setter=UI.Table.setMinimumRowSize },
		{ name="CellSpacingX", type="number", setter=UI.Table.setCellSpacingX },
		{ name="CellSpacingY", type="number", setter=UI.Table.setCellSpacingY },
		{ name="EqualizeCells", type="boolean", setter=UI.Table.setEqualizeCells },
		{ name="FixedGrid", type="boolean", setter=UI.Table.setFixedGrid },
		{ name="OddEvenStyle", type="boolean", setter=UI.Table.setOddEvenStyle },
		{ name="AllowColumnReorder", type="boolean", setter=UI.Table.setAllowColumnReorder },
		{ name="AllowColumnResize", type="boolean", setter=UI.Table.setAllowColumnResize },
	},
}