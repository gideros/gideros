require "json"

local function buildPart(gdx,mparts,bones)
	local mp=mparts[gdx.meshpartid]
	assert(mp,"No such part id:"..gdx.meshpartid)
	print("NodeMesh:",gdx.meshpartid)
	if gdx.bones then
		for _,b in ipairs(gdx.bones) do
			bones[b.node]=true
			b.poseSrt={ s=b.scale, r=b.rotation, t=b.translation }
			b.scale=nil
			b.rotation=nil
			b.translation=nil
		end
	end
	local m={ vertices=mp.v, texcoords=mp.t, normals=mp.n, indices=mp.i, animdata={ bw=mp.bw, bi=mp.bi}, type="mesh",
		material=gdx.materialid,bones=gdx.bones }
			--[[treeDesc.material=mtl
			treeDesc.color=m.color
			treeDesc.material=m.material]]
	return m
end

local function buildNode(gdx,mparts,bones,all)
	local root={}
	root.type="group"
	root.parts={}
	root.name=gdx.id
	all[gdx.id]=true
	if gdx.children then
		for _,node in ipairs(gdx.children) do
			root.parts[node.id]=buildNode(node,mparts,bones,all)
		end
	end
	if gdx.parts then
		for _,node in ipairs(gdx.parts) do
			root.parts[node.meshpartid]=buildPart(node,mparts,bones)
		end
	end
	root.srt={ s=gdx.scale, r=gdx.rotation, t=gdx.translation }
	return root
end

function loadGdx(file,imtls)
	local mtls=imtls or {}
	local fd=io.open(file,"rb")
	assert(fd,"Can't open file:"..file)
	local js=fd:read("*all")
	fd:close()
	gdx=json.decode(js)
	
	local root={}
	root.type="group"
	root.parts={}

	local mparts={}
	for n,gm in ipairs(gdx.meshes) do
		--Compute attrs info
		local attrs={}
		local ds=0
		for _,a in ipairs(gm.attributes) do		
			if a=="POSITION" then as=3 an="v"
			elseif a=="NORMAL" then as=3 an="n"
			elseif a=="COLOR" then as=4 an="c"
			elseif a=="TEXCOORD0" then as=2 an="t"
			elseif a:sub(1,11)=="BLENDWEIGHT" then as=2 an="bw"..a:sub(12,12)
			else assert(false, "Attribute not handled:"..a) end
			attrs[an]={ ab=ds, al=as, d={} }
			ds+=as
		end
		local dc=#gm.vertices//ds
		assert(dc*ds==#gm.vertices,"Vertice array size "..dc*ds.." mismatch with attributes desc "..ds.." ("..#gm.vertices..")")
		-- Parse attributes data
		for _,a in pairs(attrs) do
			for i=0,dc-1 do
				for j=1,a.al do
					a.d[i*a.al+j]=gm.vertices[i*ds+a.ab+j]
				end			
			end
		end
		-- Reformulate anim (bw)
		if attrs.bw0 then
			local dw={}
			local di={}
			for i=1,dc do
				local bd=attrs.bw0.d
				di[i*4-3]=bd[i*2-1]
				dw[i*4-3]=bd[i*2]
				if attrs.bw1 then
					local bd=attrs.bw1.d
					di[i*4-2]=bd[i*2-1]
					dw[i*4-2]=bd[i*2]
				else di[i*4-2]=0 dw[i*4-2]=0
				end
				if attrs.bw2 then
					local bd=attrs.bw2.d
					di[i*4-1]=bd[i*2-1]
					dw[i*4-1]=bd[i*2]
				else di[i*4-1]=0 dw[i*4-1]=0
				end
				if attrs.bw3 then
					local bd=attrs.bw3.d
					di[i*4]=bd[i*2-1]
					dw[i*4]=bd[i*2]
				else di[i*4]=0 dw[i*4]=0
				end
			end
			attrs.bw={ d=dw, al=4}
			attrs.bi={ d=di, al=4}
			attrs.bw0=nil attrs.bw1=nil attrs.bw2=nil attrs.bw3=nil
		end
		-- Build parts
		for _,part in ipairs(gm.parts) do
			--TODO handle type
			local ia=part.indices
			local ian=#part.indices
			for i=1,ian do ia[i]+=1 end
			mparts[part.id]={ v=attrs.v.d, t=attrs.t.d, n=attrs.n.d, i=part.indices, bw=attrs.bw.d, bi=attrs.bi.d }
		end
	end
	local bones,all={},{}
	for _,node in ipairs(gdx.nodes) do
		local n=buildNode(node,mparts,bones,all)
		root.parts[node.id]=n
	end
	if #bones==0 then bones=all end
	root.bones=bones
	root.animations=gdx.animations
	G3DFormat.computeG3DSizes(root)
	return root,mtls
end

function buildGdx(file,imtls)
 local root,mtls=loadGdx(file,imtls)
 return G3DFormat.buildG3D(root,mtls)
end