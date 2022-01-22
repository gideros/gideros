--!NEEDS:luashader/luashader.lua

local function makeEffect(name,vshader,fshader)
	local s=Shader.lua(vshader,fshader,0,
	{
	{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
	{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
	{name="fTexture",type=Shader.CTEXTURE,vertex=false},
	{name="fTextureInfo",type=Shader.CFLOAT4,sys=Shader.SYS_TEXTUREINFO,vertex=false},
	{name="fTime",type=Shader.CFLOAT,sys=Shader.SYS_TIMER,vertex=false},
	},
	{
	{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
	{name="vColor",type=Shader.DUBYTE,mult=4,slot=1,offset=0},
	{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
	},
	{
	{name="fTexCoord",type=Shader.CFLOAT2},
	}
	)
	s.name=name
	return s
end

Effect={}

Effect.none=makeEffect("None",
	function (vVertex,vColor,vTexCoord) : Shader
		local vertex = hF4(vVertex,0.0,1.0)
		fTexCoord=vTexCoord
		return vMatrix*vertex
	end,
	function () : Shader
	 local frag=lF4(fColor)*texture2D(fTexture, fTexCoord)
	 if (frag.a==0.0) then discard() end
	 return frag
	end)	
	
Effect.blur=makeEffect("Blur",
	function (vVertex,vColor,vTexCoord) : Shader
		local vertex = hF4(vVertex,0.0,1.0)
		fTexCoord=vTexCoord
		return vMatrix*vertex
	end,
	function () : Shader
	 local frag=lF4(0,0,0,0)
	 local frad=floor((1+sin(fTime))*4)%9 --For the demo use time for rad
	 local ext=2*frad+1
	 local tc=fTexCoord-fTextureInfo.zw*frad
	 for v=0,19 do
		if v<ext then
			frag=frag+lF4(fColor)*texture2D(fTexture, tc)
		end
		tc+=fTextureInfo.zw
	 end
	 frag=frag/ext
	 if (frag.a==0.0) then discard() end
	 return frag
	end)

Effect.grayscale=makeEffect("Grayscale",
	function (vVertex,vColor,vTexCoord) : Shader
		local vertex = hF4(vVertex,0.0,1.0)
		fTexCoord=vTexCoord
		return vMatrix*vertex
	end,
	function () : Shader
	 local frag=lF4(fColor)*texture2D(fTexture, fTexCoord)
	 local coef=lF3(0.2125, 0.7154, 0.0721)
	 local gray=dot(frag.rgb,coef)
	 frag.rgb=lF3(gray,gray,gray)
	 if (frag.a==0.0) then discard() end
	 return frag
	end)
Effect.saturate=makeEffect("Saturate",
	function (vVertex,vColor,vTexCoord) : Shader
		local vertex = hF4(vVertex,0.0,1.0)
		fTexCoord=vTexCoord
		return vMatrix*vertex
	end,
	function () : Shader
	 local frad=(1+sin(fTime))*0.5
	 local frag=lF4(fColor)*texture2D(fTexture, fTexCoord)
	 local coef=lF3(0.2125, 0.7154, 0.0721)
	 local dp=dot(frag.rgb,coef)
	 frag.rgb=mix(frag.rgb,frag.rgb/dp,frad)
	 if (frag.a==0.0) then discard() end
	 return frag
	end)

Effect.emphasize=makeEffect("Emphasize",
	function (vVertex,vColor,vTexCoord) : Shader
		local vertex = hF4(vVertex,0.0,1.0)
		fTexCoord=vTexCoord
		return vMatrix*vertex
	end,
	function () : Shader
	 local frag=lF4(fColor)*texture2D(fTexture, fTexCoord)
	 local e=lF1(2+sin(fTime))
	 frag.rgb=lF3(frag.r^e,frag.g^e,frag.b^e)
	 if (frag.a==0.0) then discard() end
	 return frag
	end)

Effect.waves=makeEffect("Waves",
	function (vVertex,vColor,vTexCoord) : Shader
		local vertex = hF4(vVertex,0.0,1.0)
		fTexCoord=vTexCoord
		return vMatrix*vertex
	end,
	function () : Shader
	 local tc=hF2(fTexCoord.x+(1+sin(fTexCoord.x*10+fTime*2))*0.05,fTexCoord.y)*0.9
	 local frag=lF4(fColor)*texture2D(fTexture, tc)
	 if (frag.a==0.0) then discard() end
	 return frag
	end)

Effect.bloom=makeEffect("Bloom",
	function (vVertex,vColor,vTexCoord) : Shader
		local vertex = hF4(vVertex,0.0,1.0)
		fTexCoord=vTexCoord
		return vMatrix*vertex
	end,
	function () : Shader
	 local frag=lF4(0,0,0,0)
	 local amount=0.5*(1+sin(fTime))
	 local frad=floor(amount*8)%9 --For the demo use time for rad
	 local ext=2*frad+1
	 local tc=fTexCoord-fTextureInfo.zw*frad
	 for v=0,19 do
		if v<ext then
			frag=frag+lF4(fColor)*texture2D(fTexture, tc)
		end
		tc+=fTextureInfo.zw
	 end
	 frag=frag/ext
	 
	 local bfrag=lF4(fColor)*texture2D(fTexture, fTexCoord)
 	 local coef=lF3(0.2125, 0.7154, 0.0721)
	 local dp=dot(bfrag.rgb,coef)
	 if dp<0.5 then bfrag.rgb=lF3(0,0,0) end
	 frag.rgb=frag.rgb+bfrag.rgb*amount	

	 if (frag.a==0.0) then discard() end
	 return frag
	end)
