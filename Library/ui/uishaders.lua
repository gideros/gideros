--!NEEDS:uiinit.lua
--!NEEDS:luashader.lua

UI.Shader=Core.class(Object)
function UI.Shader:init(params)
	self.applied={}
    local weak = { __mode="k" }
    setmetatable(self.applied, weak)
	self.params=params or {}
end

function UI.Shader:apply(sprite,mode)
	if self.applyStackTo then self:applyStackTo(sprite) end
	if self.stack then
		sprite:setEffectStack(self.stack,mode)
	end
	if self.shader then
		sprite:setShader(self.shader)
	end
	self.applied[sprite]=true
	self:applyParametersTo(sprite)
end

function UI.Shader:remove(sprite)
	if not self.applied[sprite] then return end
	if self.stack then
		sprite:setEffectStack()
	end
	if self.shader then
		sprite:setShader()
	end
	self.applied[sprite]=nil
end

function UI.Shader:applyParameters()
	for k,_ in pairs(self.applied) do
		self:applyParametersTo(k)
	end
end

function UI.Shader:applyParametersTo(sprite)
	for n,v in pairs(self.params) do
		if n:sub(1,3)=="col" then
			--A color
			local c=UI.Utils.colorVector(v,sprite._style)
			sprite:setShaderConstant(n,Shader.CFLOAT4,1,c.r,c.g,c.b,c.a)
		else
			if type(v)=="string" then v=sprite:resolveStyle(v) end
			--A simple value
			sprite:setShaderConstant(n,Shader.CFLOAT,1,tonumber(v) or 0)
		end
	end
end


local shaderCache={}
local function getShader(ref,vf,ff,uni,att,var)
	if not shaderCache[ref] then 
		shaderCache[ref]=Shader.lua(vf,ff,0,uni,att,var)
	end
	return shaderCache[ref] 
end

local function vertexShader(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,0.0,1.0)
	fTexCoord=vTexCoord
	return vMatrix*vertex
end

-- A few predefined shaders
--A shader to turn an image grayscale
UI.Shader.Grayscale=Core.class(UI.Shader)
function UI.Shader.Grayscale:init(params)
	self.shader=getShader(UI.Shader.Grayscale,
		vertexShader,
		function () : Shader
			local t=texture2D(fTexture, fTexCoord)
			local fc=t.r*0.4+t.g*0.5+t.b*0.1
			local frag=hF4(fc,fc,fc,t.a)
			frag*=fColor
			if (frag.a==0.0) then discard() end
			return lF4(frag)
		end,
		{
			{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
			{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
			{name="fTexture",type=Shader.CTEXTURE,vertex=false},
		},
		{
			{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
			{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
			{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
		},
		{
			{name="fTexCoord",type=Shader.CFLOAT2},
		}
	)
end

--A shader to recolor each channel of an image
UI.Shader.MultiLayer=Core.class(UI.Shader)
function UI.Shader.MultiLayer:init(params)
	self.shader=getShader(UI.Shader.MultiLayer,
		vertexShader,
		function () : Shader
			local t=texture2D(fTexture, fTexCoord)
			local frag=colLayer1*t.r*colLayer1.a
			local f2=colLayer2*t.g*colLayer2.a
			frag=frag*(1-f2.a)+f2
			f2=colLayer3*t.b*colLayer3.a
			frag=frag*(1-f2.a)+f2
			f2=colLayer4*t.a*colLayer4.a
			frag=frag*(1-f2.a)+f2
			frag*=fColor
			if (frag.a==0.0) then discard() end
			return lF4(frag)
		end,
		{
			{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
			{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
			{name="fTexture",type=Shader.CTEXTURE,vertex=false},
			{name="colLayer1",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer2",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer3",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer4",type=Shader.CFLOAT4,vertex=false},
		},
		{
			{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
			{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
			{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
		},
		{
			{name="fTexCoord",type=Shader.CFLOAT2},
		}
	)
end

--A shader to tint a specific sector of an image
UI.Shader.SectorPainter=Core.class(UI.Shader)
function UI.Shader.SectorPainter:init(params)
	self.shader=getShader(UI.Shader.SectorPainter,
		vertexShader,
		function () : Shader
			local frag=lF4(0,0,0,0)
			local tc=(fTexCoord/fTextureInfo.xy)*2-1
			local r=length(tc)
			local bcol=lF4(colOut)
			if r>=numRadiusStart and r<=numRadiusEnd then
				local ang=0.25-atan2(-tc.y,tc.x)/6.282
				if ang<0 then ang+=1 end
				local inTest=BOOL(ang>=numAngleStart and ang<=numAngleEnd)
				if numAngleStart>numAngleEnd then 
					inTest=BOOL(ang<numAngleEnd or ang>numAngleStart)
				end
				if inTest then
					bcol=lF4(colIn)
				end
			end
			bcol.rgb*=bcol.a
			frag=lF4(fColor)*bcol*texture2D(fTexture, fTexCoord)
			return frag
		end,
		{
			{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
			{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
			{name="fTexture",type=Shader.CTEXTURE,vertex=false},
			{name="fTextureInfo",type=Shader.CFLOAT4,sys=Shader.SYS_TEXTUREINFO,vertex=false},
			{name="colIn",type=Shader.CFLOAT4,vertex=false},
			{name="colOut",type=Shader.CFLOAT4,vertex=false},
			{name="numAngleStart",type=Shader.CFLOAT,vertex=false},
			{name="numAngleEnd",type=Shader.CFLOAT,vertex=false},
			{name="numRadiusStart",type=Shader.CFLOAT,vertex=false},
			{name="numRadiusEnd",type=Shader.CFLOAT,vertex=false},
		},
		{
			{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
			{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
			{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
		},
		{
			{name="fTexCoord",type=Shader.CFLOAT2},
		}
	)
end

--A Shadow shader
UI.Shader.Shadow=Core.class(UI.Shader)

function UI.Shader.Shadow:init(params)
	
	self.shaderBlur=getShader("blur",
		vertexShader,
		function () : Shader
		 local frag=lF4(0,0,0,0)
		 local frad=fAmount.x
		 local ext=2*frad+1
		 local dir=fTextureInfo.zw*fDirection
		 local tc=fTexCoord-dir*frad
		 for v=0,19 do
			if v<hI1(ext) then
				frag=frag+texture2D(fTexture, tc)
			end
			tc+=dir
		 end
		 frag=(frag/ext)*lF4(fColor)
		 if (frag.a==0.0) then discard() end
		 return frag
		end,
		{
			{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
			{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
			{name="fTexture",type=Shader.CTEXTURE,vertex=false},
			{name="fAmount",type=Shader.CFLOAT2,vertex=false},
			{name="fDirection",type=Shader.CFLOAT2,vertex=false},
			{name="fTextureInfo",type=Shader.CFLOAT4,sys=Shader.SYS_TEXTUREINFO,vertex=false},
		},
		{
			{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
			{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
			{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
		},
		{
			{name="fTexCoord",type=Shader.CFLOAT2},
		}
	)

	self.shaderExtract=getShader(UI.Shader.Shadow,
		function (vVertex,vColor,vTexCoord) : Shader
			local vertex = hF4(vVertex,0.0,1.0)
			vertex.xy+=vDirection*vRtScale
			fTexCoord=vTexCoord
			return vMatrix*vertex
		end,
		function () : Shader
		 local c=texture2D(fTexture, fTexCoord)
		 c.a*=colShadow.a
		 c.rgb=lF3(colShadow.r*colShadow.a,colShadow.g*colShadow.a,colShadow.b*colShadow.a)
		 return c
		end,
		{
			{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
			{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
			{name="fTexture",type=Shader.CTEXTURE,vertex=false},
			{name="colShadow",type=Shader.CFLOAT4,vertex=false},
			{name="vDirection",type=Shader.CFLOAT2,vertex=true},
			{name="vRtScale",type=Shader.CFLOAT2,sys=Shader.SYS_RTSCALE,vertex=true},
		},
		{
			{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
			{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
			{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
		},
		{
			{name="fTexCoord",type=Shader.CFLOAT2},
		}
	)

	
	self.shaderCombine=getShader("shadowCombine",
		function (vVertex,vColor,vTexCoord) : Shader
			local vertex = hF4(vVertex,0.0,1.0)
			vertex.xy+=vDirection*vRtScale
			fTexCoord=vTexCoord
			return vMatrix*vertex
		end,
		function () : Shader
		 local base=texture2D(fTexture, fTexCoord)
		 local ctr=fTexInfo.xy*fZoom.xy
		 local shtc=ctr+(fTexCoord-ctr)*fZoom.z
		 local sh=texture2D(fTexture2, shtc)
		 return mix(base,sh,1-base.a)
		end,
		{
			{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
			{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
			{name="fTexture",type=Shader.CTEXTURE,vertex=false},
			{name="fTexture2",type=Shader.CTEXTURE,vertex=false},
			{name="fTexInfo",type=Shader.CFLOAT4,vertex=false,sys=Shader.SYS_TEXTUREINFO},
			{name="fZoom",type=Shader.CFLOAT4,vertex=false},
			{name="vDirection",type=Shader.CFLOAT2,vertex=true},
			{name="vRtScale",type=Shader.CFLOAT2,sys=Shader.SYS_RTSCALE,vertex=true},
		},
		{
			{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
			{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
			{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
		},
		{
			{name="fTexCoord",type=Shader.CFLOAT2},
		}
	)

	local rt1=RenderTarget.new(1,1)
	local rt2=RenderTarget.new(1,1)
	local rt3=RenderTarget.new(1,1)

	self.stack={
		{ buffer=rt3, shader=self.shaderExtract, transform=Matrix.new(), autoBuffer=true, autoTransform=Matrix.new()},
		{ buffer=rt1, shader=self.shaderBlur, clear=true, autoBuffer=true},
		{ buffer=rt2, shader=self.shaderBlur, clear=true, autoBuffer=true},
		{ buffer=rt1, shader=self.shaderCombine,textures={rt3,rt1}, postTransform=Matrix.new()},
	}
	params=params or {}
	self.displace=params.displace or 0
	self.angle=params.angle or 215
	self.radius=params.radius or 7
	self.shadow=params.color or {0,0,0,1} --Color and alpha
	self.expand=params.expand or 1
end

function UI.Shader.Shadow:applyParametersTo(sprite)
	local disx=(math.sin(^<self.angle)*self.displace)><0
	local disy=(-math.cos(^<self.angle)*self.displace)><0
	--Work buffer shall include room for twice the radius and the displacement
	self.stack[1].autoTransform:setPosition(2*self.radius*self.expand+self.displace,2*self.radius*self.expand+self.displace)
	--Draw so that shadow can fit on the surface
	self.stack[1].transform:setPosition(self.radius*self.expand-disx,self.radius*self.expand-disy)
	local cx,cy=.5,.5
	sprite:setEffectStack(self.stack)
	disx=(math.sin(^<self.angle)*self.displace)
	disy=(-math.cos(^<self.angle)*self.displace)
	sprite:setEffectConstant(1,"vDirection",Shader.CFLOAT2,1,disx,disy)
	sprite:setEffectConstant(1,"colShadow",Shader.CFLOAT4,1,self.shadow[1],self.shadow[2],self.shadow[3],self.shadow[4])
	sprite:setEffectConstant(2,"fDirection",Shader.CFLOAT2,1,1,0)
	sprite:setEffectConstant(3,"fDirection",Shader.CFLOAT2,1,0,1)
	sprite:setEffectConstant(2,"fAmount",Shader.CFLOAT2,1,self.radius,0)
	sprite:setEffectConstant(3,"fAmount",Shader.CFLOAT2,1,self.radius,0)
	sprite:setEffectConstant(4,"fAmount",Shader.CFLOAT2,1,self.angle,self.displace)
	sprite:setEffectConstant(4,"fZoom",Shader.CFLOAT4,1,cx,cy,1/self.expand,0)
	--Correct vertex position to account for work space offset
	sprite:setEffectConstant(4,"vDirection",Shader.CFLOAT2,1,-self.radius+(disx><0),-self.radius+(disy><0))
end


--A shader to recolor each channel of an image and apply it to a MeshLine
UI.Shader.MeshLineMultiLayer=Core.class(UI.Shader)
function UI.Shader.MeshLineMultiLayer:init(params)
	self.shader=getShader(UI.Shader.MeshLineMultiLayer,
		vertexShader,
		function () : Shader
			local tc=fTexCoord.xy
			local tb1=fCornersT.xy*fTexInfo.xy
			if tc.x<0 then
				tc.x=(tc.x+1)*tb1.s
			elseif tc.x<1 then
				tc.x=tb1.s+tc.x*(1-tb1.s-tb1.t)
			else
				tc.x=1-tb1.t+(tc.x-1)*tb1.t
			end
			local tb=fCornersT.zw*fTexInfo.xy
			local vb=fCornersV.zw*fTexInfo.xy
			if tc.y<vb.s then
				tc.y=tc.y*(tb.s/vb.s)
			elseif tc.y>(1-vb.t) then
				tc.y=((tc.y-1+vb.t)*(tb.t/vb.t))+1-tb.t
			else
				tc.y=((tc.y-vb.s)*((1-tb.t-tb.s)/(1-vb.t-vb.s)))+tb.s
			end
			local t=texture2D(fTexture, tc.xy)
			local frag=colLayer1*t.r
			frag=frag*frag.a
			local f2=colLayer2*t.g
			frag=frag*(1-f2.a)+f2*f2.a
			f2=(if (fTexCoord.x>=fRatio.y or fTexCoord.x<fRatio.x) then colLayer3a else colLayer3)*t.b
			
			frag=frag*(1-f2.a)+f2*f2.a
			f2=colLayer4*t.a
			frag=frag*(1-f2.a)+f2*f2.a
			frag*=fColor
			return lF4(frag)
		end,
		{
			{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
			{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
			{name="fTexture",type=Shader.CTEXTURE,vertex=false},
			{name="fTexInfo",type=Shader.CFLOAT4,vertex=false,sys=Shader.SYS_TEXTUREINFO},
			{name="fCornersT",type=Shader.CFLOAT4,vertex=false},
			{name="fCornersV",type=Shader.CFLOAT4,vertex=false},
			{name="fRatio",type=Shader.CFLOAT2,vertex=false},
			{name="colLayer1",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer2",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer3",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer3a",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer4",type=Shader.CFLOAT4,vertex=false},
		},
		{
			{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
			{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
			{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
		},
		{
			{name="fTexCoord",type=Shader.CFLOAT2},
		}
	)
end

--A shader to recolor each channel of an image and apply it to a Progress bar
UI.Shader.ProgressMultiLayer=Core.class(UI.Shader)
function UI.Shader.ProgressMultiLayer:init(params)
	self.shader=getShader(UI.Shader.ProgressMultiLayer,
		function (vVertex,vColor,vTexCoord) : Shader
			local vertex = hF4(vVertex,0.0,1.0)
			fTexCoord=vTexCoord
			fXPos=vVertex.x/vBounds.z
			return vMatrix*vertex
		end,
		function () : Shader
			local t=texture2D(fTexture, fTexCoord.xy)
			local frag=colLayer1*t.r
			frag=frag*frag.a
			local f2=colLayer2*t.g
			frag=frag*(1-f2.a)+f2
			f2=(if fXPos<fRatio.x or fXPos>fRatio.y then colLayer3a else colLayer3)*t.b
			
			frag=frag*(1-f2.a)+f2			
			f2=colLayer4*t.a
			frag=frag*(1-f2.a)+f2
			frag*=fColor
			return lF4(frag)
		end,
		{
			{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
			{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
			{name="fTexture",type=Shader.CTEXTURE,vertex=false},
			{name="fRatio",type=Shader.CFLOAT2,vertex=false},
			{name="vBounds",type=Shader.CFLOAT4,sys=Shader.SYS_BOUNDS,vertex=true},
			{name="colLayer1",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer2",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer3",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer3a",type=Shader.CFLOAT4,vertex=false},
			{name="colLayer4",type=Shader.CFLOAT4,vertex=false},
		},
		{
			{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
			{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
			{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
		},
		{
			{name="fTexCoord",type=Shader.CFLOAT2},
			{name="fXPos",type=Shader.CFLOAT},
		}
	)
end
