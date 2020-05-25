-- settings
local tapTime=0.3
local tapLongTime=0.3
local tapDisperse=7
local swipeDistance=15
local swipeDisperse=50
local tapPriority=true -- true if taps take priority over swipes

local insert,remove=table.insert,table.remove
local gestureStack={}
local gestureNames={"swipeRight","swipeLeft","swipeDown","swipeUp","tap","longTap"}

local touches={}
stage:addEventListener(Event.TOUCHES_BEGIN, function(e)
	local t=e.touch
	if not touches[t.id] then		
		local touch={}
		touch.x=t.rx
		touch.y=t.ry
		touch.time=os.timer()
		touches[t.id]=touch
	end
end)

stage:addEventListener(Event.TOUCHES_MOVE,function(e)
 
end)

stage:addEventListener(Event.TOUCHES_END,function(e)
	local t=e.touch
	local touch=touches[t.id]
	if touch then --if touch is defined
		local x,y=t.rx,t.ry
		local dx=t.rx-touch.x -- get swipe distance
		local dy=t.ry-touch.y
		local adx=-dx<>dx -- get abs swipe distance
		local ady=-dy<>dy
		if adx<=tapDisperse and
			ady<=tapDisperse then
			--can be a tap
			if os.timer()-touch.time>=tapLongTime then
				if tapPriority then
					insert(gestureStack,1,{6,x,y}) -- long tap
				else
					gestureStack[#gestureStack+1]={6,x,y} -- long tap
				end
			else
				if tapPriority then
					insert(gestureStack,1,{5,x,y}) -- tap
				else
					gestureStack[#gestureStack+1]={5,x,y} -- tap
				end
			end
		else
			--can be a swipe
 
			if adx<=swipeDisperse and ady>swipeDistance then -- vertical
				if dy<0 then -- up
					gestureStack[#gestureStack+1]={4,dy}
				else -- down
					gestureStack[#gestureStack+1]={3,dy}
				end
			elseif adx>swipeDistance and
				ady<=swipeDisperse then -- horizontal
				if dx<0 then -- left
					gestureStack[#gestureStack+1]={2,dx}
				else -- right
					gestureStack[#gestureStack+1]={1,dx}
				end
			end
		end
		touches[t.id]=nil
	end
end)

function getGesture()
	if #gestureStack>0 then
		local id=gestureStack[1]
		return id,gestureNames[id[1]]
	end
end

function removeGesture()
	if #gestureStack>0 then
		remove(gestureStack,1)
		return getGesture()
	end
end

function emptyGestures()
	gestureStack={}
end