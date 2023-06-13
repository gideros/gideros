StandardShaders={}

--Constants/Uniforms:
--PROGRAM-CODES: B,C,T,TC, B3,C3,T3,TC3, TA,TAC,P,PS, PS3,FC,SC,SL
StandardShaders.stdUniforms={
	{cat=0b1110011111111111, name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
	{cat=0b1110000000000000, name="vXform",type=Shader.CMATRIX,sys=Shader.SYS_NONE, vertex=true}, --Positional, should always be second uniform
	{cat=0b1110000101010101, name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
	{cat=0b1100000000000000, name="fFeather",type=Shader.CFLOAT,sys=Shader.SYS_NONE, vertex=false}, --Positional, should always be fourth uniform
	{cat=0b0001111010101010, name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=true},
	{cat=0b0001111111001100, name="fTexture",type=Shader.CTEXTURE,vertex=false},
	{cat=0b0001100000000000, name="vWorldMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WORLD,vertex=true},
	{cat=0b0001000000000000, name="vViewMatrix",type=Shader.CMATRIX,sys=Shader.SYS_VIEW,vertex=true},
	{cat=0b0001000000000000, name="vProjMatrix",type=Shader.CMATRIX,sys=Shader.SYS_PROJECTION,vertex=true},
	{cat=0b0000010000000000, name="vPSize",type=Shader.CFLOAT,sys=Shader.SYS_PARTICLESIZE,vertex=true},
}

StandardShaders.stdAttributes={
	{cat=0b0000011100001111, used=0b0001111100001111, name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
	{cat=0b0000011111110000, used=0b0001111111110000, name="vVertex",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
	{cat=0b0001111111111111, used=0b0001111010101010, name="vColor",type=Shader.DUBYTE,mult=4,slot=1,offset=0},
	{cat=0b0000011111111111, used=0b0001111111001100, name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
	-- Particle shader variants
	{cat=0b0001100000000000, used=0b0001111111110000, name="vVertex",type=Shader.DFLOAT,mult=4,slot=0,offset=0},
	{cat=0b0001100000000000, used=0b0001111111110000, name="vTexCoord",type=Shader.DFLOAT,mult=4,slot=2,offset=0},
	-- Line variants
	{cat=0b0010000000000000, used=0b0010000000000000, name="dataa",type=Shader.DFLOAT,mult=4,slot=0,offset=0},
	{cat=0b0100000000000000, used=0b0100000000000000, name="dataa",type=Shader.DFLOAT,mult=4,slot=0,offset=0,stride=64},
	{cat=0b0100000000000000, used=0b0100000000000000, name="datab",type=Shader.DFLOAT,mult=4,slot=1,offset=0,stride=64},
	{cat=0b0100000000000000, used=0b0100000000000000, name="datac",type=Shader.DFLOAT,mult=4,slot=2,offset=0,stride=64},
	{cat=0b0100000000000000, used=0b0100000000000000, name="linepos",type=Shader.DFLOAT,mult=4,slot=3,offset=0,stride=64},
	{cat=0b1000000000000000, used=0b1000000000000000, name="dataa",type=Shader.DFLOAT,mult=4,slot=0,offset=0,stride=32},
	{cat=0b1000000000000000, used=0b1000000000000000, name="linepos",type=Shader.DFLOAT,mult=4,slot=1,offset=0,stride=32},
}

StandardShaders.stdVarying={
	{cat=0b1011111111001100, name="fTexCoord",type=Shader.CFLOAT2},
	{cat=0b0001111010101010, name="fInColor",type=Shader.CFLOAT4},
	{cat=0b0001100000000000, name="fStepRot",type=Shader.CFLOAT2},
		--For Line path shaders
	{cat=0b0100000000000000, name="fPos",type=Shader.CFLOAT2},
	{cat=0b0100000000000000, name="fPQ",type=Shader.CFLOAT2},
	{cat=0b0100000000000000, name="fA",type=Shader.CFLOAT2},
	{cat=0b0100000000000000, name="fB",type=Shader.CFLOAT2},
	{cat=0b0100000000000000, name="fC",type=Shader.CFLOAT2},
	{cat=0b0100000000000000, name="fOffsetWidth",type=Shader.CFLOAT2},
	{cat=0b1100000000000000, name="fLinePos",type=Shader.CFLOAT4},
}

StandardShaders.stdFunctions={
	{cat=0b0100000000000000, name="evaluateQuadratic", rtype="hF2", acount=4,	args={ {name="a", type="hF2"},{name="b", type="hF2"},{name="c", type="hF2"}, {name="t", type="hF1"} }},
	{cat=0b0100000000000000, name="check", rtype="hF1", acount=1,	args={ {name="a", type="hF2"},{name="b", type="hF2"},{name="c", type="hF2"},{name="pos", type="hF2"},{name="w", type="hF1"},{name="t", type="hF1"} }},
	{cat=0b0100000000000000, name="cbrt", rtype="hF1", acount=1,	args={ {name="x", type="hF1"} }},
}
--VERTEX shaders
--BASICS
function StandardShaders.vBasic(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,0.0,1.0)
	return vMatrix*vertex
end
function StandardShaders.vColor(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,0.0,1.0)
	fInColor=vColor*fColor
	return vMatrix*vertex
end
function StandardShaders.vTexture(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,0.0,1.0)
	fTexCoord=vTexCoord
	return vMatrix*vertex
end
function StandardShaders.vTextureColor(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,0.0,1.0)
	fTexCoord=vTexCoord
	fInColor=vColor*fColor
	return vMatrix*vertex
end
function StandardShaders.vBasic3(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,1.0)
	return vMatrix*vertex
end
function StandardShaders.vColor3(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,1.0)
	fInColor=vColor*fColor
	return vMatrix*vertex
end
function StandardShaders.vTexture3(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,1.0)
	fTexCoord=vTexCoord
	return vMatrix*vertex
end
function StandardShaders.vTextureColor3(vVertex,vColor,vTexCoord) : Shader
	local vertex = hF4(vVertex,1.0)
	fTexCoord=vTexCoord
	fInColor=vColor*fColor
	return vMatrix*vertex
end

StandardShaders.vTextureAlpha=StandardShaders.vTexture
StandardShaders.vTextureAlphaColor=StandardShaders.vTextureColor
--PARTICLES
function StandardShaders.vParticles(vVertex,vColor,vTexCoord) : Shader
    local rad=(hF2(-0.5,-0.5)+vTexCoord.xy)*vTexCoord.z
    local angle=^<vTexCoord.w
    local ca=cos(angle)
    local sa=sin(angle)
    local rot=hF22(ca,sa,-sa,ca)
    rad=rad*rot
    fInColor=hF4(vColor)*fColor
    local vertex = hF4(vVertex.xy+rad,0,1.0)
    local xpsize=vWorldMatrix*hF4(vTexCoord.z,0.0,0.0,0.0)
    local xpl=length(xpsize.xyz)
    if (xpl<20.0) then xpl=20.0 end
    fStepRot=hF2(sign(vTexCoord.z)/xpl,vTexCoord.w)
    fTexCoord=vTexCoord.xy
	return vMatrix*vertex
end
function StandardShaders.vParticles3(vVertex,vColor,vTexCoord) : Shader
    local rad=hF2(-0.5,-0.5)+vTexCoord.xy
    local angle=^<vTexCoord.w
    local ca=cos(angle)
    local sa=sin(angle)
    local rot=hF22(ca,sa,-sa,ca)
    rad=rad*rot
    fInColor=hF4(vColor)*fColor
    local xpsize=vWorldMatrix*hF4(vTexCoord.z,0.0,0.0,0.0)
    local xpl=length(xpsize.xyz)
    if (xpl<20.0) then xpl=20.0 end
    local vertex = vViewMatrix*(vWorldMatrix*hF4(vVertex.xyz,1.0))
    vertex.xy+=rad*xpl
    fStepRot=hF2(sign(vTexCoord.z)/xpl,vTexCoord.w)
    fTexCoord=vTexCoord.xy
	return vProjMatrix*vertex
end
--PATHS
function StandardShaders.vPathFC(dataa) : Shader
	fTexCoord=dataa.zw
	return vMatrix*vXform*hF4(dataa.xy,0.0,1.0)
end
function StandardShaders.vPathSC(dataa,datab,datac,linepos) : Shader
	fPos=dataa.xy
	fPQ=dataa.zw
	fA=datab.xy
	fB=datab.zw
	fC=datac.xy
	fOffsetWidth=datac.zw
	fLinePos=linepos
	return vMatrix*vXform*hF4(dataa.xy,0.0,1.0)
end
function StandardShaders.vPathSL(dataa,linepos) : Shader
	fTexCoord=normalize(dataa.zw)
	fLinePos=linepos
	return vMatrix*vXform*hF4(dataa.xy+dataa.zw,0.0,1.0)
end

--FRAGMENT Shaders
--BASICS
function StandardShaders.fBasic() : Shader
	return fColor
end
function StandardShaders.fColor() : Shader
	return fInColor
end
function StandardShaders.fTexture() : Shader
	local frag=lF4(fColor)*texture2D(fTexture, fTexCoord)
	if (frag.a==0.0) then discard() end
	return frag
end
function StandardShaders.fTextureColor() : Shader
	local frag=lF4(fInColor)*texture2D(fTexture, fTexCoord)
	if (frag.a==0.0) then discard() end
	return frag
end
StandardShaders.fBasic3=StandardShaders.fBasic
StandardShaders.fColor3=StandardShaders.fColor
StandardShaders.fTexture3=StandardShaders.fTexture
StandardShaders.fTextureColor3=StandardShaders.fTextureColor

function StandardShaders.fTextureAlpha() : Shader
	local frag=lF4(fColor)*texture2D(fTexture, fTexCoord).aaaa
	if (frag.a==0.0) then discard() end
	return frag
end
function StandardShaders.fTextureAlphaColor() : Shader
	local frag=lF4(fInColor)*texture2D(fTexture, fTexCoord).aaaa
	if (frag.a==0.0) then discard() end
	return frag
end

--PARTICLES
function StandardShaders.fParticles() : Shader
	if fStepRot.x==0.0 then 
		discard()
	elseif fStepRot.x<0.0 then
	else
		local rad=hF2(-0.5,-0.5)+fTexCoord
		if fTexInfo.x<=0.0 then
			local alpha=lF1(1.0-smoothstep(0.5-fStepRot.x,0.5+fStepRot.x,length(rad)))
			return lF4(fInColor*alpha)
		else
			if (rad.x<-0.5) or (rad.y<-0.5) or (rad.x>0.5) or (rad.y>0.5) then
				discard()
			end		
			return lF4(fInColor)*texture2D(fTexture, (rad+hF2(0.5,0.5))*fTexInfo.xy)
		end
	end
	return lF4(fInColor)
end
StandardShaders.fParticles3=StandardShaders.fParticles

--PATHS
function StandardShaders.fPathFC() : Shader
	if (fTexCoord.x*fTexCoord.x > fTexCoord.y) then discard() end
	return lF4(fColor)
end
function StandardShaders.fPathSC() : Shader
	--vec2 evaluateQuadratic(float t)
	function evaluateQuadratic(a,b,c,t)
		return a*t*t+b*t+c
	end
	--float check(float t)
	function check(a,b,c,pos,w,t)
		if 0<=t and t<=1 then
			local q=evaluateQuadratic(a,b,c,t)-pos
			return length(q)/w
		end
		return 2
	end
	--float cbrt(float x)
	function cbrt(x)
		return sign(x)*pow(abs(x),1/3)
	end
	local p=fPQ.x
	local q=fPQ.y
	--main
	local d=q*q/4+p*p*p/27
	if d>0 then
		local c1=-q/2
		local c2=sqrt(d)
		d=check(fA,fB,fC,fPos,fOffsetWidth.y,cbrt(c1+c2)+cbrt(c1-c2)+fOffsetWidth.x)
	else
		local cos3t=3*q*sqrt(-3/p)/(2*p)
		local theta=acos(cos3t)/3
		local r=2*sqrt(-p/3)
		d=check(fA,fB,fC,fPos,fOffsetWidth.y,r*cos(theta)+fOffsetWidth.x)><
			check(fA,fB,fC,fPos,fOffsetWidth.y,r*cos(theta+2*3.141592/3)+fOffsetWidth.x)><
			check(fA,fB,fC,fPos,fOffsetWidth.y,r*cos(theta+4*3.141592/3)+fOffsetWidth.x)
	end
	local alpha=1-smoothstep(.5-fFeather/2,.5+fFeather/2,d)
	if alpha<=0 then discard() end
	return lF4(fColor*alpha)
end
function StandardShaders.fPathSL() : Shader
	local l=length(fTexCoord)
	local alpha=1-smoothstep(.5-fFeather/2,.5+fFeather/2,l)
	if alpha<=0 then discard() end
	return lF4(fColor*alpha)
end


StandardShaders.stdSpecs={
	[Shader.SHADER_PROGRAM_BASIC]={ cat=0, name="Basic", 
		variants={ [Shader.SHADER_VARIANT_3D]={ cat=4, name="Basic3", }}},
	[Shader.SHADER_PROGRAM_COLOR]={ cat=1, name="Color", 
		variants={ [Shader.SHADER_VARIANT_3D]={ cat=5, name="Color3", }}},
	[Shader.SHADER_PROGRAM_TEXTURE]={ cat=2, name="Texture",
		variants={ [Shader.SHADER_VARIANT_3D]={ cat=6, name="Texture3", }}},
	[Shader.SHADER_PROGRAM_TEXTURECOLOR]={ cat=3, name="TextureColor", 
		variants={ [Shader.SHADER_VARIANT_3D]={ cat=7, name="TextureColor3", }}},
	[Shader.SHADER_PROGRAM_TEXTUREALPHA]={ cat=8, name="TextureAlpha" },
	[Shader.SHADER_PROGRAM_TEXTUREALPHACOLOR]={ cat=9, name="TextureAlphaColor" },
	--[Shader.SHADER_PROGRAM_PARTICLE]={ cat=10, name="Basic" },
	[Shader.SHADER_PROGRAM_PARTICLES]={ cat=11, name="Particles", 
		variants={ [Shader.SHADER_VARIANT_3D]={ cat=12, name="Particles3", }}},
	[Shader.SHADER_PROGRAM_PATHFILLCURVE]={ cat=13, name="PathFC" },
	[Shader.SHADER_PROGRAM_PATHSTROKECURVE]={ cat=14, name="PathSC" },
	[Shader.SHADER_PROGRAM_PATHSTROKELINE]={ cat=15, name="PathSL" },
}

function StandardShaders:build(catchErrors)
	local function buildShader()
		return Shader.lua(
			self.vertexShader,
			self.fragmentShader,
			self.options or 0,
			self.uniforms,
			self.attributes,
			self.varying,
			self.functions,
			self.constants)
	end
	if catchErrors then
		local ok,ret=pcall(buildShader)
		if ok then return ret end
		print("Failed to build shader: "..ret.." at:\n"..debug.traceback())
		return nil,ret
	else
		return buildShader()
	end
end

function StandardShaders:getShaderSpecification(type,variant)
	local lang=Shader.getShaderLanguage()
	if lang=="hlsl" then		
		-- For HLSL always use 3D versions of basic shaders
		if type==Shader.SHADER_PROGRAM_BASIC 
			or type==Shader.SHADER_PROGRAM_COLOR
			or type==Shader.SHADER_PROGRAM_TEXTURE
			or type==Shader.SHADER_PROGRAM_TEXTURECOLOR then
			variant=(variant or 0)|Shader.SHADER_VARIANT_3D
		end
	end
	
	local st=self.stdSpecs[type]
	if not st then return end
	st=st.variants and st.variants[variant] or st
	local cmask=1<<st.cat
	local function filter(t,c,attr)
		local m={}
		for _,v in ipairs(t) do
			if (v.cat&c)==c then
				if attr then
					local vc=table.clone(v)
					if (vc.used&c)==0 then
						vc.mult=0
					end
					m[#m+1]=vc
				else
					m[#m+1]=v
				end
			end
		end
		return m
	end
	local spec={
		vertexShader=self["v"..st.name],
		fragmentShader=self["f"..st.name],
		build=self.build,
		uniforms=filter(self.stdUniforms,cmask),
		attributes=filter(self.stdAttributes,cmask,true),
		varying=filter(self.stdVarying,cmask),
		functions=filter(self.stdFunctions,cmask),
	}
	return spec
end
