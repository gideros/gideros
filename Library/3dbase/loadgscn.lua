local Files={}

function Files.loadJson(file)
	local js=Files.load(file)
	local ok,ret=pcall(json.decode,js)
	if not ok then return end
	return ret
end

function Files.load(file)
	local fd=io.open(file,"rb")
	if not fd then return end
	local js=fd:read("*all")
	fd:close()
	return js
end

function LoadGScene(libpath,file)
	local loadGroup
	local scene=Sprite.new()
	scene.refs={}
	function loadGroup(g,p)
		p.name=g.name
		p.tag=g.tag
		if p.name then scene.refs[p.name]=p end
		if g.transform then
			if g.transform.position then
				p:setPosition(g.transform.position[1],g.transform.position[2],g.transform.position[3])
			end
			if g.transform.rotation then
				p:setRotationX(g.transform.rotation[1])
				p:setRotationY(g.transform.rotation[2])
				p:setRotation(g.transform.rotation[3])
			end
			if g.transform.scale then
				p:setScale(g.transform.scale[1],g.transform.scale[2],g.transform.scale[3])
			end
		end
		for _,s in ipairs(g.children) do
			if s.asset then	
				local data=Files.loadJson(libpath.."/"..s.asset.lib.."/"..s.asset.name..".g3d")
				G3DFormat.computeG3DSizes(data)
				local m=G3DFormat.buildG3D(data)
				m.name=s.name
				m.tag=s.tag
				if s.transform then
					local t=m
					if s.transform.position then
						t:setPosition(s.transform.position[1],s.transform.position[2],s.transform.position[3])
					end
					if s.transform.rotation then
						t:setRotationX(s.transform.rotation[1])
						t:setRotationY(s.transform.rotation[2])
						t:setRotation(s.transform.rotation[3])
					end
					if s.transform.scale then
						t:setScale(s.transform.scale[1],s.transform.scale[2],s.transform.scale[3])
					end
				end
				if m.name then scene.refs[m.name]=m end
				p:addChild(m)
			else
				local m=Sprite.new()
				loadGroup(s,m)
				p:addChild(m)
			end
		end
	end
	local project=Files.loadJson(file)
	loadGroup(project.scene,scene)
	return scene
end
