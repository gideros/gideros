--!NEEDS:uiinit.lua

--[[ dnd package contains:
* type: the type of data being moved
* data: the data payload
* visual: the visual marker

DND callbacks:
* getDndData(source,x,y): Called on a source when a drag start action occurs. Returns the dragged dnd package or nil to abort.
* offerDndData(target,data,x,y): Called on a candidate target during dnd move.
	Returns a value indicating if data can be accepted by the target.
	If nil, target isn't eligible and won't receive further callbacks. 
	If falsy, data isn't accepted and setDndData won't be called. 
	If data is nil, the target can clear up its indicators.
* setDndData(target,data,source): Called on a target to process the drop action.
* cleanupDndData(source,data,target): Called on the source to terminate the dnd action. if target is nil, the action was canceled

DND States (to affect visual during DnD, through UI.Dnd.SetState(data)):
- nil: no state or ok state
- DENIED: Drop impossible at this location
]]
UI.Dnd={
	State={ 
		DENIED="denied",
	}
}
UI.Dnd._sources={}
UI.Dnd._targets={}
UI.Dnd._context=nil

local dndPrepare=nil
local function dndLongPrepare(ex,ey,ratio)
	for k,sinfo in pairs(UI.Dnd._sources) do
		if sinfo.long and k:hitTestPoint(ex,ey,true) then
			if not dndPrepare then
				dndPrepare=UI.Behavior.LongClick.makeIndicator(k,{})
			end
			local lx,ly=k:globalToLocal(ex,ey)
			dndPrepare:indicate(k,lx,ly,ratio)
		end
	end
end

local function dndDragStart(x,y,dist,angle,changed,long)
	local function checkSource(k)
		local sinfo=UI.Dnd._sources[k]
		if sinfo and ((not sinfo.long) or long) and k:hitTestPoint(x,y,true) then
			local lx,ly=k:globalToLocal(x,y)
			local data=k:getDndData(lx,ly)
			if data then
				UI.Dnd._context={ source=k, data=data }
				UI.Dnd._context.visual=UI.Dnd._context.data.visual
				local s=UI.Screen.getScreen(k) or stage
				if s.dnd then s=s.dnd end
				s:addChild(UI.Dnd._context.visual)
				UI.Dnd._context.visual:setPosition(x,y)
				return not UI.Dnd._context.data.propagateEvent
			end
		end
	end
	local stack=UI.Control.queryStack(x,y)
	for _,k in ipairs(stack) do
		local ck=checkSource(k)
		if ck~=nil then return ck end
	end
	for k,v in pairs(UI.Dnd._sources) do
		if v.always then 
			local ck=checkSource(k)
			if ck~=nil then return ck end
		end
	end
end

local function dndDrag(x,y)
	if UI.Dnd._context then
		local oldTarget=UI.Dnd._context.target
		UI.Dnd._context.target=nil
		UI.Dnd._context.visual:setPosition(x,y)
		for k,_ in pairs(UI.Dnd._targets) do
			if k:hitTestPoint(x,y,true) then
				local lx,ly=k:globalToLocal(x,y)
				UI.Dnd._context.offerStatus=(not k.offerDndData) or k:offerDndData(UI.Dnd._context.data,lx,ly)
				if UI.Dnd._context.offerStatus~=nil then
					UI.Dnd._context.target=k
					break
				end
			end
		end
		if oldTarget and oldTarget~=UI.Dnd._context.target and oldTarget.offerDndData then
			oldTarget:offerDndData(nil) 
		end
	end	
end

local function dndDragEnd(x,y)
	if UI.Dnd._context then
		if UI.Dnd._context.target then
			if UI.Dnd._context.offerStatus then
				UI.Dnd._context.target:setDndData(UI.Dnd._context.data,UI.Dnd._context.source)
			end
			UI.Dnd._context.target:offerDndData(nil) 
		end
		if UI.Dnd._context.source.cleanupDndData then
			UI.Dnd._context.source:cleanupDndData(UI.Dnd._context.data,UI.Dnd._context.target)
		end
		UI.Dnd._context.visual:removeFromParent()
		UI.Dnd.SetState(UI.Dnd._context,nil)
	end
	UI.Dnd._context=nil
end

function UI.Dnd.Source(s,en,long, always)
	if en and not next(UI.Dnd._sources) then
		UI.Control.onDragStart._dnd=dndDragStart
		UI.Control.onDrag._dnd=dndDrag
		UI.Control.onDragEnd._dnd=dndDragEnd
		UI.Control.onLongPrepare._dnd=dndLongPrepare
	end
	UI.Dnd._sources[s]=if en then { long=long, always=always } else nil
	if not next(UI.Dnd._sources) then
		UI.Control.onDragStart._dnd=nil
		UI.Control.onDrag._dnd=nil
		UI.Control.onDragEnd._dnd=nil	
		UI.Control.onLongPrepare._dnd=nil
	end
end

function UI.Dnd.Target(s,en)
	UI.Dnd._targets[s]=en
end

function UI.Dnd.SetState(data,state)
	if not data.visual then return end
	if data.visual.setDndState then
		data.visual:setDndState(state)
	end
end
	
local grayScaleShader
function UI.Dnd.MakeMarker(widget)
	local mw,mh=widget:getSize()
	local mx,my=widget:getPosition()
	local rt=RenderTarget.new(mw,mh,true,{mipmap=true})
	rt:draw(widget,-mx,-my)
	
	local marker = UI.Builder({
		class="UI.Panel",
		layoutModel={ columnWeights={1}, rowWeights={1}},
		children={{
			class="UI.Panel",
			name="outer",
			layout={ originx=-.5, originy=-.5, },
			layoutModel={ columnWeights={1}, rowWeights={1}},
			LocalStyle="dnd.styMarker",
			children={{
				name="image",
				class="UI.Image",
				Tint="dnd.colMarkerTint",
				Image=rt,
				layout={ width=mw, height=mh, fill=Sprite.LAYOUT_FILL_BOTH},
			}},
		}},
	})
	marker.setDndState=function (self,state)
		if self.visualState~=state then
			self.visualState=state
			if state==UI.Dnd.State.DENIED then
				if not grayscaleShader then grayscaleShader=UI.Shader.Grayscale.new() end
				grayscaleShader:apply(self.image)
				self.outer:setStateStyle("dnd.styMarkerDenied")
			else
				grayscaleShader:remove(self.image)
				self.outer:setStateStyle({})
			end
		end
	end

	marker:setAnchorPoint(.5,.5)
	return marker
end