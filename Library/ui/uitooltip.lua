--!NEEDS:uiinit.lua
UI.ToolTip={}

function UI.ToolTip.lookupAt(screen,x,y)
	local sps=screen:getChildrenAtPoint(x,y,true,false)
	local si=#sps
	local tip,sites
	while si>0 do
		tip=sps[si].ToolTip
		if type(tip)=="function" then
			x,y=screen:localToGlobal(x,y)
			x,y=sps[si]:globalToLocal(x,y)
			tip,sites=tip(sps[si],x,y)
		end
		if tip then break end
		si-=1		
	end
	if sites then
		sites=table.clone(sites)
		for _,s in ipairs(sites) do
			s.x,s.y=sps[si]:localToGlobal(s.x,s.y)
			s.x,s.y=screen:globalToLocal(s.x,s.y)			
		end
	end
	return sps[si],tip,sites
end

function UI.ToolTip.buildMarker(sprite,tip,hover)
	local content=if type(tip)=="table" and (tip.class or tip.factory) then 
			tip 
		else {
			class="UI.Label",
			Text=tostring(tip),
			layout={ fill=Sprite.LAYOUT_FILL_BOTH},
		}
	local marker = UI.Builder({
		class="UI.Panel",
		layout={ },
		layoutModel={ columnWeights={1}, rowWeights={1}},
		LocalStyle="tooltip.styMarker",
		children={ content },
	})
	return marker
end

function UI.ToolTip.placementSites(screen,target,x,y)
	local tx,ty,tw,th=target:getBounds(screen)
	if th<screen:resolveStyle("tooltip.szOffsetMax") then
		return {
			{x=x,y=ty,dx=0,dy=-1,mvtx=true},
			{x=x,y=ty+th,dx=0,dy=1,mvtx=true},
			{x=x,y=y,dx=0,dy=0,mvtx=true,mvty=true},
		}
	else
		return {
			{x=x,y=y,dx=0,dy=0,mvtx=true,mvty=true},
		}
	end
end

local currentMarker=nil
function UI.ToolTip.show(screen,marker,sites)
	if currentMarker then return nil end
	currentMarker=marker
	UI.Screen.popupAt(screen,marker,sites)
	return marker
end

function UI.ToolTip.dismiss(screen,marker)
	if currentMarker~=marker then return end
	currentMarker:removeFromParent()
	currentMarker=nil
end

function UI.ToolTip.forget(screen,marker)
	if currentMarker~=marker then return end
	currentMarker=nil
end
