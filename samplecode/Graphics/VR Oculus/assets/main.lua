local scene,view
if Oculus then
	Oculus.enableRoom(false,false) --Disable lobby
	Oculus.setTrackingSpace(3) --Room scale
	scene=Sprite.new()
	stage:addChild(scene)
else
	local sw,sh=application:getContentWidth(),application:getContentHeight()
	view=D3.View.new(sw,sh,-70)
	stage:addChild(view)
	scene=view:getScene()
	view:lookAt(0,2,0,0,1.8,-1,0,1,0)
end


local env=D3.Sphere.new(60,50)
env:mapTexture(Texture.new("envbox.jpg"))
env:setY(2)
scene:addChild(env)

Lighting.setLight(5,10,5,.3)

local moveX,moveY=0,0

local roomf=Glb.new(nil,"dungeon.glb")
local rooms=G3DFormat.buildG3D(roomf:getScene())
rooms:updateMode(D3.Mesh.MODE_LIGHTING)
rooms:setScale(1,1,1)
local posX,posY,posZ=0,2,0
env:addChild(rooms)
env:setAnchorPosition(posX,2+posY,posZ)


local rhand,rhand
if Oculus then
	--rhand=Oculus.getController("r") 
	--rhand=Oculus.getHandTrackingHand("r")
	rhand=Oculus.getVRHand("r")
	--lhand=Oculus.getController("l") 
	--lhand=Oculus.getHandTrackingHand("l")
	lhand=Oculus.getVRHand("l")
	rhand:updateMode(D3.Mesh.MODE_LIGHTING)
	scene:addChild(rhand)
	lhand:updateMode(D3.Mesh.MODE_LIGHTING)
	scene:addChild(lhand)
	rhand:setColorTransform(.92,.8,.65)
	lhand:setColorTransform(.92,.8,.65)
end

local function vrhandCorrect(im)
	local m=Matrix.new()
	return im
end

if Oculus then
	local obtn,ogrip=0,0
	local hold=false
	Oculus.inputEventHandler=function(e)
		if e.deviceType==32  then
			Oculus.updateHandTracking(e)
		else
			Oculus.updateVRHands(e)
			if (e.caps&8)>0 then
				local srt={r=e.rotation,t=e.position}
				local mat=G3DFormat.srtToMatrix(srt)
				if e.buttons&1==1 and obtn&1==0 then
				end
				if e.buttons&1==0 and obtn&1==1 then
				end
				if hold and e.gripTrigger<.3 then
					hold=false
				end
				obtn=e.buttons
				ogrip=e.grpTrigger
				rhand:setMatrix(mat)
				if e.gripTrigger>.7 then 
					hold=true
				end
				moveX,moveY=e.stickX,e.stickY
			elseif (e.caps&4)>0 then
				local srt={r=e.rotation,t=e.position}
				local mat=G3DFormat.srtToMatrix(srt)
				lhand:setMatrix(mat)
			end
		end
	end
end


local angle=0
stage:addEventListener(Event.ENTER_FRAME,function(e)
	Lighting.computeShadows(scene)
	D3Anim.updateBones()
	if not Oculus then
		angle+=.3
		view:lookAngles(posX,posY-1,posZ,0,angle,0) --For the demo reduce Y
	else
		env:setAnchorPosition(posX,2+posY,posZ)
		local hp=Oculus.getHeadPose() hp.position=nil
		local hmt=Matrix.fromSRT(hp)
		local dx,_,dy=hmt:transformPoint(moveX,0,-moveY)
		posX+=dx*.1
		posZ+=dy*.1
	end
end) 

stage:addEventListener(Event.APPLICATION_RESUME,function ()
	print("resume")
	if Oculus then
		Oculus.setTrackingSpace(3) --Room scale
	end
end)
