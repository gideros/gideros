require "json"

if not Oculus then return end

function Oculus.getVRHand(which)
	local name
	local rz,ry,rx=0,0,0
	if which:sub(1,1):lower()=="r" then 
		name="rhand.json"
		cname="_vrRHand"
		rz=-90
		rx=-90
	else
		name="lhand.json"
		cname="_vrLHand"
		rz=-90
		rx=90
	end
	local f=io.open("oculus/"..name)
	local js=json.decode(f:read("*a"))
	local hand=G3DFormat.buildG3D(js,nil)
	local ohand=D3.Group.new()
	ohand.objs={ hand=hand }
	ohand:addChild(hand)
	hand:setRotationY(ry)
	hand:setRotationX(rx)
	hand:setRotation(rz)
	hand:setZ(.1)
	ohand.bones=hand.bones
	Oculus[cname]=ohand
	return ohand
end

function Oculus.getHandTrackingHand(which)
	local num=1
	if which:sub(1,1):lower()=="r" then 
		cname="_htRHand"
		num=2
	else
		cname="_htLHand"
	end
	local lh=Oculus.getHandMesh(num)
	local lhs=Oculus.getHandSkeleton(num)
	local bones={}
	local boneTree
	local boneRefs={}
	for n,bone in ipairs(lhs.bones) do
		bone.basePose={r=bone.rotation,t=bone.position}
		bone.node="bone_"..n
		bones["bone_"..n]=true
		local bpart={ type="group", id="bone_"..n, parts={} }
		bpart.srt={r=bone.rotation,t=bone.position}
		bone.rotation=nil
		bone.position=nil
		if lhs.boneParents[n]>=0 then
			local p=boneRefs["bone_"..(lhs.boneParents[n]+1)]
			p.parts["bone_"..n]=bpart
		else
			boneTree=bpart
		end
		boneRefs["bone_"..n]=bpart
	end
	lh.type="mesh"
	lh.bones=lhs.bones
	
	local root={}
	root.type="group"
	root.bones=bones
	root.parts={
		mesh=lh,
		["bone_"..1]=boneTree
	}
	local hand=G3DFormat.buildG3D(root,nil)
	hand.rootPose=lhs.bones
	Oculus[cname]=hand
	return hand
end

local function loadGltf(path,file)
	local gltf=Gltf.new(path,file)
	local node=gltf:getNode(1)
	return G3DFormat.buildG3D(node)	
end

function Oculus.getController(which)
	if which:sub(1,1):lower()=="r" then 
		Oculus._vrRCtl=loadGltf("oculus/jedi","jedi_right.gltf")
		return Oculus._vrRCtl
	else
		Oculus._vrLCtl=loadGltf("oculus/jedi","jedi_left.gltf")
		return Oculus._vrLCtl
	end
end

function Oculus.updateHandTracking(e)
	if e.deviceType==32  then
		local _hand
		if (e.caps&1)==1 then _hand=Oculus._htLHand 
		elseif (e.caps&2)==2 then _hand=Oculus._htRHand
		end
		if _hand then
			local bp={}
			for n,b in ipairs(e.handBone) do
				local m=Matrix.fromSRT({r=b,t=_hand.rootPose[n].basePose.t})
				bp["bone_"..n]={{ratio=1,mat={m:getMatrix()}}}
			end
			D3Anim.setBonesPose(_hand.objs.mesh,bp)
			local ss=e.handScale
			local srt={r=e.rotation,t=e.position, s={ss,ss,ss}}
			_hand:setMatrix(Matrix.fromSRT(srt))
		end
	end
end

function Oculus.updateVRHands(e)
	if e.deviceType==4  then
		local _hand
		if (e.caps&4)==4 then _hand=Oculus._vrLHand 
		elseif (e.caps&8)==8 then _hand=Oculus._vrRHand
		end
		if _hand then
			local bp={}
			local eg=e.gripTrigger*0.3
			local ei=e.indexTrigger*0.3
			local ep=eg*.2
			local n=nil
			local rots={n,
						n,n,ep,eg,eg, --Pouce
						ei,ei,ei, --Index
						eg,eg,eg, --Majeur
						eg,eg,eg, --Annulaire
						n,eg*.6,eg,eg, --Auriculaire
						n,n,n,n,n}
			
			for n,b in pairs(rots) do
				local m=Matrix.fromSRT({r={0,0,math.sin(b),math.cos(b)},t=_hand.bones["bone_"..n].srt.t})
				bp["bone_"..n]={{ratio=1,mat={m:getMatrix()}}}
			end
			D3Anim.setBonesPose(_hand.objs.hand.objs.mesh,bp)
			local ss=1
			local srt={r=e.rotation,t=e.position, s={ss,ss,ss}}
			_hand:setMatrix(Matrix.fromSRT(srt))
		end
	end
end
