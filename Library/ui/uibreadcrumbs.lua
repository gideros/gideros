--!NEEDS:uipanel.lua
UI.BreadCrumbs=Core.class(UI.Panel,function() return end)
UI.BreadCrumbs.ELIPSIS={}
UI.BreadCrumbs.SEPARATOR={}

UI.BreadCrumbs.Template={
	class="UI.Panel",
	layoutModel={ columnWeights={0,0,0,1}, rowWeights={1}, },
	children={
		{ 	class="UI.Viewport", name="vpTrail", layout={fill=Sprite.LAYOUT_FILL_BOTH, gridx=3 },
			Content={
				class="UI.Panel", name="pnContent",
				layoutModel={ columnWeights={0}, rowWeights={1}, },
				layout={ anchorx=0 },
		}},
	}
}

function UI.BreadCrumbs:init()
	UI.BuilderSelf(UI.BreadCrumbs.Template,self)
	self.vpTrail:setFlags({disabled=true})
	self.dataMap={}
end

function UI.BreadCrumbs:updateStyle(...)
	UI.Panel.updateStyle(self,...)
end

function UI.BreadCrumbs:buildItem(n,d,isRoot,isLast)
	local sp=d
	local style,bstyle
	if self.builder then
		sp=self.builder(d)
	end
	if sp==UI.BreadCrumbs.ELIPSIS then
		sp="..."
		bstyle="breadcrumbs.styElipsis"
		style={}
	elseif sp==UI.BreadCrumbs.SEPARATOR then
		sp=">"
		bstyle="breadcrumbs.stySeparator"
		style={}		
	elseif isRoot then
		style="breadcrumbs.styRoot"
		bstyle="breadcrumbs.styItem"
	elseif isLast then
		style="breadcrumbs.styLast"
		bstyle="breadcrumbs.styItem"
	else
		bstyle="breadcrumbs.styItem"
		style={}
	end
	sp=UI.Utils.makeWidget(sp,d,nil)
	sp:setLocalStyle(bstyle)
	sp:setStateStyle(style)
	UI.Behavior.Button.new(sp,nil)
	local lc=sp:getLayoutConstraints() or { fill=Sprite.LAYOUT_FILL_BOTH }
	lc.insetRight="breadcrumbs.szSpacing"
	self.dataMap[sp]=n
	return sp,lc
end
	
function UI.BreadCrumbs:setData(data,builder)
	self.data=data or {}
	self.builder=builder
	for sp,_ in pairs(self.dataMap) do
		sp:removeFromParent()
	end
	self.dataMap={} 
	if #self.data==0 then return end
	local sp,lc=self:buildItem(1,self.data[1],true,false)
	self:addChild(sp)
	sp:setLayoutConstraints(lc)
	
	if #self.data==1 then return end
	
	sp,lc=self:buildItem(0,UI.BreadCrumbs.SEPARATOR,false,false)
	lc.gridx=1
	sp:setLayoutConstraints(lc)
	self:addChild(sp)

	sp,lc=self:buildItem(0,UI.BreadCrumbs.ELIPSIS,false,false)
	lc.gridx=2
	sp:setLayoutConstraints(lc)
	self:addChild(sp)
	self.lbElipsis=sp
	self.lbElipsis:setVisible(false)
	
	local gx=0
	for i=2,#self.data do
		if i>2 then
			sp,lc=self:buildItem(0,UI.BreadCrumbs.SEPARATOR,false,false)
			lc.gridx=gx
			sp:setLayoutConstraints(lc)
			gx+=1
			self.pnContent:addChild(sp)
		end
		
		sp,lc=self:buildItem(i,self.data[i],false,i==#self.data)
		lc.gridx=gx
		sp:setLayoutConstraints(lc)
		self.pnContent:addChild(sp)		
		gx+=1
	end	
end

function UI.BreadCrumbs:push(data)
	table.insert(self.data,data)
	self:setData(self.data,self.builder)
end

function UI.BreadCrumbs:onRangeChange(w,rw,rh)
	if w==self.vpTrail then
		if rw>0 then
			self.vpTrail:setScrollAmount(1,0)
			self.lbElipsis:setVisible(true)
		else
			self.lbElipsis:setVisible(false)
		end
	end
end

function UI.BreadCrumbs:onWidgetAction(w)
	if self:getFlags().disabled then return end
	local n=self.dataMap[w]
	if n and n>0 then
		while #self.data>n do table.remove(self.data) end
		self:setData(self.data,self.builder)
		UI.dispatchEvent(self,"WidgetChange",self.data[n])
	end
	return true
end

UI.BreadCrumbs.Definition= {
	name="BreadCrumbs",
	icon="ui/icons/panel.png",
	class="UI.BreadCrumbs",
	constructorArgs={ },
	properties={
	},
}
