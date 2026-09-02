Gltf=Core.class()

function Gltf:init(path,name)
	if path then
		self.path = string.gsub(path, "\\", "/")
	else
		self.path = path
	end
	if name then
		local fn = name
		if self.path and self.path ~= "" then
			fn = self.path .. "/" .. name
		end
		local f = io.open(fn, "rb")
		if f then
			self.desc = json.decode(f:read("*a"))
			f:close()
		else
			print("Gltf:init cannot open file:", fn)
		end
	end
end

function Gltf:getScene(i)
	if not self.desc then return nil end

	local root = {}
	root.type = "group"
	root.parts = {}
	root.bones = {}
	self.collectBones = root.bones

	if self.desc.scenes and #self.desc.scenes > 0 then
		i = i or (self.desc.scene and (self.desc.scene + 1)) or 1
		local ns = self.desc.scenes[i]
		root.name = "s" .. i
		if ns and ns.nodes then
			for ni, n in ipairs(ns.nodes) do
				root.parts["n" .. n] = self:getNode(n + 1)
			end
		end
	elseif self.desc.nodes then
		root.name = "default_scene"
		for n = 0, #self.desc.nodes - 1 do
			root.parts["n" .. n] = self:getNode(n + 1)
		end
	end
	self.collectBones = nil

	if self.desc.animations then
		local PATHLEN = { scale = 3, rotation = 4, translation = 3 }
		root.animations = {}
		for an, ad in ipairs(self.desc.animations) do
			local abones = {}
			local btab = {}
			for _, c in ipairs(ad.channels or {}) do
				local bone = btab[c.target.node]
				local samp = ad.samplers[c.sampler + 1]
				local path = c.target.path
				if not bone then
					bone = { boneId = "n" .. c.target.node, keyframes = {}, keytimes = {} }
					btab[c.target.node] = bone
					abones[#abones + 1] = bone
				end
				local pl = PATHLEN[path]
				local ksmpout = self:getBuffer(samp.output + 1, false, pl) -- vector
				local ksmpin = self:getBuffer(samp.input + 1, false, 1) -- Time points
				-- Populate keyframes, possibly with partial data
				if ksmpin and ksmpout then
					for ki, kv in ipairs(ksmpin) do
						local b = (ki - 1) * pl
						local v = {}
						for n = 1, pl do v[n] = ksmpout[n + b] end
						local ktm = (kv * 1000) // 1
						local kf = bone.keyframes[ktm] 
						if not kf then
							kf = { keytime = ktm }
							bone.keyframes[ktm] = kf
							bone.keytimes[#bone.keytimes + 1] = ktm
						end
						kf[path] = v
					end
				end
			end
			
			-- Reassemble keyframes			
			for _, bone in ipairs(abones) do
				-- Sort timeline
				table.sort(bone.keytimes)
				local kfirst = bone.keytimes[1] or 0
				-- Reassembled and sorted key frames vector
				local nkf = {}
				-- Missing vectors recording
				local otm = {}
				local ohisto = {}
				-- Go through all partial keyframes in order
				for i, ktm in ipairs(bone.keytimes) do
					-- Assign to ordered vector and fix keyframe time
					local kf = bone.keyframes[ktm]
					kf.keytime -= kfirst
					nkf[i] = kf
					-- Check each vector
					for path, pl in pairs(PATHLEN) do
						local vp = kf[path]
						if vp then
							-- Vector is present: check for holes
							local op = ohisto[path]
							ohisto[path] = nil
							if op and otm[path] then
								-- We need to interpolate and fill holes
								local ltm = otm[path]
								local vst = bone.keyframes[ltm][path]
								local vsr = (ktm - ltm)
								for _, hktm in ipairs(op) do
									local v = {}
									for n = 1, pl do 
										v[n] = vst[n] + (vp[n] - vst[n]) * (hktm - ltm) / vsr
									end
									bone.keyframes[hktm][path] = v
								end
							elseif op then
								-- Starting point hole
							end
							-- Record last time point with a value for this vector
							otm[path] = ktm							
						else
							-- We don't have a value, record it as a hole
							local op = ohisto[path] or {}
							ohisto[path] = op
							op[#op + 1] = ktm
						end
					end
				end
				for path, pl in pairs(PATHLEN) do
					ohisto[path] = nil
				end
				bone.keyframes = nkf
				bone.keytimes = nil
			end
			root.animations[an] = { bones = abones, name = ad.name }
		end
	end

	return root
end

function Gltf:getNode(i)
	local nd = self.desc.nodes[i]
	if not nd then return nil end

	local root = {}
	root.type = "group"
	root.parts = {}
	root.name = nd.name or ("n" .. (i - 1))
	if nd.mesh ~= nil and self.desc.meshes then
		local md = self.desc.meshes[nd.mesh + 1]
		local bones
		if nd.skin and self.desc.skins then
			local sd = self.desc.skins[nd.skin + 1]
			if sd then
				bones = {}
				local mats = sd.inverseBindMatrices and self:getBuffer(sd.inverseBindMatrices + 1, false, 16)
				for k, v in ipairs(sd.joints or {}) do
					local n = (k - 1) * 16 + 1
					local rm = Matrix.new()
					if mats then
						rm:setMatrix(
							mats[n + 0], mats[n + 1], mats[n + 2], mats[n + 3],
							mats[n + 4], mats[n + 5], mats[n + 6], mats[n + 7],
							mats[n + 8], mats[n + 9], mats[n + 10], mats[n + 11],
							mats[n + 12], mats[n + 13], mats[n + 14], mats[n + 15]
						)
						rm:invert()
					end
					bones[k] = { node = "n" .. v }
					if self.collectBones then self.collectBones["n" .. v] = true end
				end
			end
		end
		if md and md.primitives then
			for pi, prim in ipairs(md.primitives) do
				local function bufferIndex(str)
					if not prim.attributes then return 0 end

					if prim.attributes[str] ~= nil then

						return prim.attributes[str] + 1
					end
					if prim.attributes[str .. "_0"] ~= nil then

						return prim.attributes[str .. "_0"] + 1
					end
					for n, id in pairs(prim.attributes) do
						if n == str or n:sub(1, #str) == str then

							return id + 1
						end
					end

					return 0
				end
				local animi = self:getBuffer(bufferIndex("JOINTS_0"), false, 4)
				local animw = self:getBuffer(bufferIndex("WEIGHTS_0"), false, 4)
				local verts = self:getBuffer(bufferIndex("POSITION"), false, 3)
				local indices = nil
				if prim.indices ~= nil then
					indices = self:getBuffer(prim.indices + 1, true)
				elseif verts then
					indices = {}
					for vi = 1, #verts // 3 do
						indices[vi] = vi
					end
				end
				local m = {
					vertices = verts,
					texcoords = self:getBuffer(bufferIndex("TEXCOORD")),
					normals = self:getBuffer(bufferIndex("NORMAL")),
					colors = self:getBuffer(bufferIndex("COLOR_0"), false, 4),
					indices = indices,
					animdata = animi and animw and { bi = animi, bw = animw },
					type = "mesh",
					material = self:getMaterial((prim.material or -1) + 1),
					bones = bones,
				}
				root.parts["p" .. pi] = m
			end
		end
	end
	if nd.children then
		for ni, n in ipairs(nd.children) do
			root.parts["n" .. n] = self:getNode(n + 1)
		end
	end
	if nd.matrix then 
		root.transform = nd.matrix
	else
		root.srt = { s = nd.scale, r = nd.rotation, t = nd.translation }
	end

	return root
end

function Gltf:getBuffer(i,indices,vlen)
	local bd = self.desc.accessors and self.desc.accessors[i]
	if bd == nil then return nil end

	if bd._array then return bd._array end

	local t = {}
	local buf, stride, bname = self:getBufferView(bd.bufferView + 1)
	local bc = bd.count or 0
	local bm = 1
	if bd.type == "SCALAR" then bm = 1
	elseif bd.type == "VEC2" then bm = 2
	elseif bd.type == "VEC3" then bm = 3
	elseif bd.type == "VEC4" then bm = 4
	elseif bd.type == "MAT4" then bm = 16
	else assert(false, "Unhandled type: " .. tostring(bd.type))
	end

	local cl, dv = 0, ""
	if bd.componentType == 5126 then
		cl = 4
		dv = "f"
	elseif bd.componentType == 5123 then
		cl = 2
		dv = "S"
	elseif bd.componentType == 5121 then
		cl = 1
		dv = "B"
	elseif bd.componentType == 5125 then
		cl = 4
		dv = "I"
	elseif bd.componentType == 5120 then
		cl = 1
		dv = "b"
	elseif bd.componentType == 5122 then
		cl = 2
		dv = "s"
	elseif bd.componentType == 5124 then
		cl = 4
		dv = "i"
	else
		assert(false, "Unhandled componentType: " .. tostring(bd.componentType))
	end
	if stride > 0 then stride = stride - cl * bm end
	local br = bd.byteOffset or 0
	local ii = 1
	for ci = 1, bc do
		for mi = 1, bm do
			if bd.componentType == 5126 then
				t[ii] = buf:get(br, 4):decodeValue("f")
				br += 4
			elseif bd.componentType == 5123 then
				t[ii] = buf:get(br, 2):decodeValue(dv)
				if indices then 
					t[ii] += 1 
				elseif bd.normalized then
					t[ii] /= 65535
				end
				br += 2
			elseif bd.componentType == 5121 then
				t[ii] = buf:get(br, 1):decodeValue(dv)
				if indices then 
					t[ii] += 1 
				elseif bd.normalized then
					t[ii] /= 255
				end
				br += 1
			elseif bd.componentType == 5125 then
				t[ii] = buf:get(br, 4):decodeValue(dv)
				if indices then 
					t[ii] += 1 
				elseif bd.normalized then
					t[ii] /= 4294967295
				end
				br += 4
			elseif bd.componentType == 5120 then
				t[ii] = buf:get(br, 1):decodeValue(dv)
				if indices then
					t[ii] += 1
				elseif bd.normalized then
					t[ii] = math.max(t[ii] / 127, -1.0)
				end
				br += 1
			elseif bd.componentType == 5122 then
				t[ii] = buf:get(br, 2):decodeValue(dv)
				if indices then
					t[ii] += 1
				elseif bd.normalized then
					t[ii] = math.max(t[ii] / 32767, -1.0)
				end
				br += 2
			elseif bd.componentType == 5124 then
				t[ii] = buf:get(br, 4):decodeValue(dv)
				if indices then
					t[ii] += 1
				elseif bd.normalized then
					t[ii] = math.max(t[ii] / 2147483647, -1.0)
				end
				br += 4
			else
				assert(false, "Unhandled componentType: " .. tostring(bd.componentType))
			end
			ii += 1
		end
		if vlen == 4 and bm == 3 then
			t[ii] = 1
			ii += 1
		elseif vlen == 3 and bm == 4 then
			t[ii] = nil
			ii -= 1
		end
		br += stride
	end
	bd._array = t
	if bname then
		pcall(os.remove, "|B|" .. bname)
	end

	return t
end

local gltfNum=0
function Gltf:getBufferView(n,ext)
	local bd = self.desc.bufferViews[n]
	local buf = self.desc.buffers[bd.buffer + 1]
	if not buf.data then
		if buf.uri then
			if buf.uri:match("^data:") then
				local b64 = buf.uri:match(";base64,(.+)$")
				if b64 then
					buf.data = Cryptography.unb64(b64)
				end
			end
		end
		if not buf.data then buf.data = self:loadBuffer(bd.buffer + 1, buf) end
	end
	gltfNum += 1
	local bname = "_gltf_" .. gltfNum .. (ext or "")
	local bb = Buffer.new(bname)
	bb:set(buf.data:sub((bd.byteOffset or 0) + 1, (bd.byteOffset or 0) + bd.byteLength))

	return bb, bd.byteStride or 0, bname
end

function Gltf:getImage(n)
	local bd = self.desc.images and self.desc.images[n]
	if not bd then return nil end

	if bd.uri then
		if bd.uri:match("^data:") then
			local b64 = bd.uri:match(";base64,(.+)$")
			if b64 then
				local iext = ".png"
				if bd.uri:match("^data:image/jpe?g") then iext = ".jpg" end
				local data = Cryptography.unb64(b64)
				gltfNum += 1
				local bname = "_gltf_img_" .. gltfNum .. iext
				local bb = Buffer.new(bname)
				bb:set(data)

				return "|B|" .. bname
			end
		end
		local uri = string.gsub(bd.uri, "\\", "/")
		if self.path and self.path ~= "" then

			return self.path .. "/" .. uri
		else

			return uri
		end
	end
	if bd.bufferView then
		local iext = nil
		if bd.mimeType == "image/jpeg" then iext = ".jpg"
		elseif bd.mimeType == "image/png" then iext = ".png"
		end
		assert(iext, "Unsupported image type: " .. tostring(bd.mimeType))
		local _, _, bname = self:getBufferView(bd.bufferView + 1, iext)

		return "|B|" .. bname
	end
end

function Gltf:getTextureImage(texIndex)
	if not texIndex or not self.desc then return nil end
	local imgIndex = texIndex
	if self.desc.textures and self.desc.textures[texIndex] then
		local tex = self.desc.textures[texIndex]
		if tex.source ~= nil then
			imgIndex = tex.source + 1
		end
	end

	return self:getImage(imgIndex)
end

function Gltf:loadBuffer(i,buf)
	local uri = string.gsub(buf.uri, "\\", "/")
	local fn = uri
	if self.path and self.path ~= "" then
		fn = self.path .. "/" .. uri
	end
	local f = io.open(fn, "rb")
	assert(f, "Buffer file not found: " .. tostring(fn))
	local data = f:read("*a")
	f:close()

	return data
end

function Gltf:getMaterial(i)
	if i == nil then return nil end

	local bd = nil
	if self.desc and self.desc.materials then
		bd = self.desc.materials[i]
	end
	if not bd then return nil end

	if bd._mat then return bd._mat end
	local mat = {}
	if bd.pbrMetallicRoughness then
		local baseCol = bd.pbrMetallicRoughness.baseColorFactor
		if baseCol then
			mat.kd = {
				(baseCol[1] or 1)^.3,
				(baseCol[2] or 1)^.3,
				(baseCol[3] or 1)^.3,
				baseCol[4] or 1
			}
		end
		local td = bd.pbrMetallicRoughness.baseColorTexture
		if td and td.index ~= nil then
			local embedded = self:getTextureImage(td.index + 1)
			if embedded then
				local res, val = pcall(Texture.new, embedded, true, { wrap = TextureBase.REPEAT, extend = false })
				if res then
					mat.embeddedtexture = val
				else
					print("Warning: failed to load gltf texture:", tostring(embedded))
				end
				if type(embedded) == "string" and embedded:sub(1, 3) == "|B|" then
					pcall(os.remove, embedded)
				end
			end
		end
	end
	bd._mat = mat

	return mat
end

-- ***********************************************************
Glb=Core.class(Gltf,function (path,name) return path,nil end)

function Glb:init(path,name)
	local fn = name
	if path and path ~= "" then 
		fn = string.gsub(path, "\\", "/") .. "/" .. name 
	end
	local f = io.open(fn, "rb")
	assert(f, "File not found: " .. tostring(fn))
	self.binData = f:read("*a")
	f:close()

	local hdr = self.binData:decodeValue("iii")
	assert(hdr[1] == 0x46546c67, "Not a glb file " .. tostring(name))
	local length = hdr[3] - 12
	local l = 13

	local chunks = {}
	while length >= 8 do
		local chdr = self.binData:sub(l, l + 7):decodeValue("ii")
		local cl, _ct = chdr[1], chdr[2]
		table.insert(chunks, { type = chdr[2], length = chdr[1], start = l + 8 })
		l += 8 + cl
		length -= (8 + cl)
	end
	assert(chunks[1].type == 0x4E4F534A, "GLB: first buffer should be JSON")
	self.binChunks = chunks
	self.desc = json.decode(self.binData:sub(chunks[1].start, chunks[1].start + chunks[1].length - 1))
end

function Glb:loadBuffer(i,buf)

	return self.binData:sub(self.binChunks[i+1].start, self.binChunks[i+1].start + self.binChunks[i+1].length - 1)
end
