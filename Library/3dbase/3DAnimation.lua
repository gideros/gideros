D3Anim={}
D3Anim._animated={}
D3Anim._animatedModel={}
function D3Anim.updateBones()
	for k,a in pairs(D3Anim._animated) do	
		if D3Anim._animatedModel[k.bonesTop].dirty then 
			local bt={}
			local bn=1
			for n,bd in ipairs(k.animBones) do
				local b=bd.bone
				local name=b.name

				local m=Matrix.new()
				m:setMatrix(bd.poseIMat:getMatrix())
				while true do
					local m1=b:getMatrix()
					m1:multiply(m) m=m1
					b=b:getParent()
					if b==k.bonesTop then break end
				end
				bt[bn],bt[bn+1],bt[bn+2],bt[bn+3],
				bt[bn+4],bt[bn+5],bt[bn+6],bt[bn+7],
				bt[bn+8],bt[bn+9],bt[bn+10],bt[bn+11],
				bt[bn+12],bt[bn+13],bt[bn+14],bt[bn+15]=m:getMatrix()
				bn=bn+16
			end
			k:setShaderConstant("bones",Shader.CMATRIX,#k.animBones,bt)
			a.dirty=false
		end
	end
end

function D3Anim.animate(m,a)
	local ta={}
	local dtm=(os:clock()-a.tm)*1000
	local hasNext=false
	for _,b in ipairs(a.anim.bones) do
		local cf=1
		while cf<#b.keyframes and b.keyframes[cf].keytime<dtm do cf+=1 end
		if cf<#b.keyframes then
			hasNext=true
		else
		end
		local f=b.keyframes[cf]
		if m.bones[b.boneId] then
			local nf=m.bones[b.boneId]
			local cm={ s=f.scale or nf.srt.s, r=f.rotation or nf.srt.r, t=f.translation or nf.srt.t}
			ta[b.boneId]=cm
		end
	end
	if not hasNext then
		if not a.loop then return nil end
		a.tm=os:clock()
	end
	return ta
end

function D3Anim.tick()
	for k,a in pairs(D3Anim._animatedModel) do	
		local ares={}
		local aend={}
		for slot,anim in pairs(a.animations) do
			local function animateIns(mvs,ratio)
				for bone,srt in pairs(mvs) do
					ares[bone]=ares[bone] or {}
					table.insert(ares[bone],{ratio=ratio,mat={G3DFormat.srtToMatrix(srt):getMatrix()}})
				end
			end
			local ao,ac,aor=nil,nil,1
			if anim.oldAnim then
				local aratio=(os:clock()-anim.oldStart)/anim.oldLen
				if aratio>=1 then
					anim.oldAnim=nil
				else
					if aratio<0 then aratio=0 end
					ao=D3Anim.animate(k,anim.oldAnim)
					if ao then aor=1-aratio end
				end
			end
			ac=D3Anim.animate(k,anim)
			if ao and ac then
				animateIns(ao,aor)
				animateIns(ac,1-aor)
			elseif ao then
				animateIns(ao,1)
			elseif ac then
				animateIns(ac,1)
			else
				aend[slot]=true
			end
		end
		for slot,_ in pairs(aend) do
			a.animations[slot]=nil
		end
		for bone,srtl in pairs(ares) do
			if #srtl>0 then
				local cm={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
				local tsr=0
				for _,srt in ipairs(srtl) do tsr+=srt.ratio end
				if tsr>0 then
					for _,srt in ipairs(srtl) do
						local rsc=srt.ratio/tsr
						for k=1,16 do cm[k]+=srt.mat[k]*rsc end
					end
					local rm=Matrix.new() rm:setMatrix(unpack(cm))
					k.bones[bone]:setMatrix(rm)
				end
			end
		end
		a.dirty=true
	end
	D3Anim.updateBones()
end

function D3Anim.setAnimation(model,anim,track,loop,transitionTime)
	assert(D3Anim._animatedModel[model],"Model not animatable")
	local an=D3Anim._animatedModel[model]
	local oldAnim,oldStart,oldLen,tm=nil,nil,nil,os:clock()
	if transitionTime and an.animations[track] then
		oldAnim=an.animations[track]
		oldStart=tm
		oldLen=transitionTime
	end
	an.animations[track]={anim=anim,tm=tm,loop=loop,oldAnim=oldAnim,oldStart=oldStart,oldLen=oldLen}
end

function D3Anim._addMesh(m)
	D3Anim._animated[m]={ dirty=true, animations={} }
	D3Anim._animatedModel[m.bonesTop]={dirty=true,animations={}}
end
