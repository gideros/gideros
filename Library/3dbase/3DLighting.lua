Lighting={}
local glversion=Shader.getEngineVersion()
local isES3Level=(glversion~="GLES2")

print(glversion)
print(Shader.getProperties().version)
print(json.encode(Shader.getProperties().extensions))

local LightingVShader=
[[
attribute vec4 POSITION0;
#ifdef TEXTURED
attribute vec2 TEXCOORD0;
#endif
attribute vec3 NORMAL0;
#ifdef ANIMATED
attribute vec4 ANIMIDX;
attribute vec4 ANIMWEIGHT;
#endif

uniform mat4 g_MVPMatrix;
uniform mat4 g_MVMatrix;
uniform mat4 g_NMatrix;
uniform mat4 g_LMatrix;

varying highp vec3 position;
#ifdef TEXTURED
varying mediump vec2 texCoord;
#endif
varying mediump vec3 normalCoord;
#ifdef SHADOWS
varying highp vec4 lightSpace;
#endif
#ifdef ANIMATED
uniform mat4 bones[16];
#endif
#ifdef INSTANCED_TEST
uniform highp sampler2D g_InstanceMap;
uniform float InstanceMapWidth;
#endif
#ifdef INSTANCED
uniform mat4 InstanceMatrix;
attribute vec4 INSTMAT1;
attribute vec4 INSTMAT2;
attribute vec4 INSTMAT3;
attribute vec4 INSTMAT4;
#endif

void main()
{
#ifdef TEXTURED
	texCoord = TEXCOORD0;
#endif
	highp vec4 pos=POSITION0;
	mediump vec4 norm=vec4(NORMAL0,0.0);
#ifdef INSTANCED_TEST
	vec2 tc=vec2(((gl_InstanceID*2+1)/InstanceMapWidth)/2.0,0.5);
	vec4 itex=texture2D(g_InstanceMap, tc);
	pos.x-=itex.r*512.0;
	pos.y+=itex.g*512.0;
	pos.z-=itex.b*512.0;
#endif
#ifdef INSTANCED
	highp mat4 imat=mat4(INSTMAT1,INSTMAT2,INSTMAT3,INSTMAT4);
	pos=imat*InstanceMatrix*pos;
	norm = imat*InstanceMatrix*vec4(norm.xyz, 0.0);
#endif

#ifdef ANIMATED
	mat4 skinning=mat4(0.0);
	skinning+=bones[int(ANIMIDX.x)]*ANIMWEIGHT.x;
	skinning+=bones[int(ANIMIDX.y)]*ANIMWEIGHT.y;
	skinning+=bones[int(ANIMIDX.z)]*ANIMWEIGHT.z;
	skinning+=bones[int(ANIMIDX.w)]*ANIMWEIGHT.w;
    vec4 npos = skinning * pos;
    vec4 nnorm = skinning * vec4(norm.xyz, 0.0);
	pos=vec4(npos.xyz,1.0);
	norm=nnorm;
#endif
	position = (g_MVMatrix*pos).xyz;
	normalCoord = normalize((g_NMatrix*vec4(norm.xyz,0)).xyz);
#ifdef SHADOWS
	lightSpace = g_LMatrix*vec4(position,1.0);
#endif
	gl_Position = g_MVPMatrix * pos;
}
]]

local LightingFShader=[[
#ifdef GLES
#extension GL_OES_standard_derivatives : enable
#endif
uniform lowp vec4 g_Color;
uniform highp vec4 lightPos;
uniform lowp float ambient;
uniform highp vec4 cameraPos;
#ifdef TEXTURED
uniform lowp sampler2D g_Texture;
#endif
#ifdef NORMMAP
uniform lowp sampler2D g_NormalMap;
#endif
#ifdef SHADOWS
uniform highp sampler2DShadow g_ShadowMap;
//uniform highp sampler2D g_ShadowMap;
#endif

#ifdef TEXTURED
varying mediump vec2 texCoord;
#endif
varying mediump vec3 normalCoord;
varying highp vec3 position;
#ifdef SHADOWS
varying highp vec4 lightSpace;
#endif

#ifdef GLES2
precision mediump float;
#endif

#ifdef NORMMAP
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // récupère les vecteurs du triangle composant le pixel
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );

    // résout le système linéaire
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construit une trame invariante à l'échelle 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
{
    // N, la normale interpolée et
    // V, le vecteur vue (vertex dirigé vers l'œil)
    vec3 map = texture2D(g_NormalMap, texcoord ).xyz;
    map = map * 255./127. - 128./127.;
    mat3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}
#endif

#ifdef SHADOWS
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
	if ((projCoords.x<0.0)||(projCoords.y<0.0)||(projCoords.x>=1.0)||(projCoords.y>=1.0))
		return 1.0;
	projCoords.z-=0.001; //BIAS
#ifdef GLES2	
	float shadow=texture2D(g_ShadowMap, projCoords.xyz); 
#else
	//float shadow=shadow2D(g_ShadowMap, projCoords.xyz).r; 	
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = shadow2D(g_ShadowMap, projCoords.xyz).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float shadow = currentDepth > closestDepth  ? 0.0 : 1.0;  	
#endif
	if(projCoords.z >= 0.999)
        shadow = 1.0;
	return shadow;
}  
#endif

void main()
{
#ifdef TEXTURED
	lowp vec4 color0 = g_Color*texture2D(g_Texture, texCoord);
#else
	lowp vec4 color0 = g_Color;
#endif
	lowp vec3 color1 = vec3(0.5, 0.5, 0.5);
	highp vec3 normal = normalize(normalCoord);
	
	highp vec3 lightDir = normalize(lightPos.xyz - position.xyz);
	highp vec3 viewDir = normalize(cameraPos.xyz-position.xyz);
#ifdef NORMMAP	
	normal=perturb_normal(normal, viewDir, texCoord);
#endif	
	
	mediump float diff = max(0.0, dot(normal, lightDir));
	mediump float spec =0.0;
    // calculate shadow
	float shadow=1.0;
#ifdef SHADOWS
    shadow = ShadowCalculation(lightSpace);       
#endif	
	if (diff>0.0)
	{
		mediump float nh = max(0.0, dot(reflect(-lightDir,normal),viewDir));
		spec = pow(nh, 32.0)*shadow;
	}

	diff=max(diff*shadow,ambient);
	gl_FragColor = vec4(color0.rgb * diff + color1 * spec, color0.a);
}
]]

local LightingShaderAttrs=
{
{name="POSITION0",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="TEXCOORD0",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
{name="NORMAL0",type=Shader.DFLOAT,mult=3,slot=3,offset=0},
{name="ANIMIDX",type=Shader.DFLOAT,mult=4,slot=4,offset=0},
{name="ANIMWEIGHT",type=Shader.DFLOAT,mult=4,slot=5,offset=0},
{name="INSTMAT1",type=Shader.DFLOAT,mult=4,slot=6,offset=0,instances=1},
{name="INSTMAT2",type=Shader.DFLOAT,mult=4,slot=7,offset=0,instances=1},
{name="INSTMAT3",type=Shader.DFLOAT,mult=4,slot=8,offset=0,instances=1},
{name="INSTMAT4",type=Shader.DFLOAT,mult=4,slot=9,offset=0,instances=1},
}

local LightingShaderConstants={
{name="g_MVPMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP, vertex=true},
{name="g_MVMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WORLD, vertex=true},
{name="g_NMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WIT3, vertex=true},
{name="g_LMatrix",type=Shader.CMATRIX, vertex=true},
{name="g_Color",type=Shader.CFLOAT4,mult=1,sys=Shader.SYS_COLOR},
{name="lightPos",type=Shader.CFLOAT4,mult=1,vertex=false},
{name="cameraPos",type=Shader.CFLOAT4,mult=1,vertex=false},
{name="ambient",type=Shader.CFLOAT,mult=1,vertex=false}}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_Texture",type=Shader.CTEXTURE,mult=1,vertex=false}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_NormalMap",type=Shader.CTEXTURE,mult=1,vertex=false}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_ShadowMap",type=Shader.CTEXTURE,mult=1,vertex=false}
--[[	
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_InstanceMap",type=Shader.CTEXTURE,mult=1,vertex=true}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="InstanceMapWidth",type=Shader.CFLOAT,mult=1,vertex=true}
	]]
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="bones",type=Shader.CMATRIX,mult=16,vertex=true}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="InstanceMatrix",type=Shader.CMATRIX,mult=1,vertex=true}

-- Shaders defs
Lighting._shaders={}
Lighting.getShader=function(code)
	local cmap={
		{"t","TEXTURED",true},
		{"s","SHADOWS",isES3Level},
		{"n","NORMMAP",true},
		{"i","INSTANCED",true},
		{"a","ANIMATED",true},
	}	
	local lcode,ccode="",""
	for _,k in ipairs(cmap) do
		if code:find(k[1]) then
			lcode=lcode..k[1]
			if k[3] then
				ccode=ccode.."#define "..k[2].."\n"
			end
		end
	end
	if lcode=="" then return nil,nil end
	if application:getDeviceInfo()~="WinRT" then
		if not Lighting._shaders[lcode] then
			v=Shader.new(
				ccode..LightingVShader,
				ccode..LightingFShader,
				Shader.FLAG_FROM_CODE,
				LightingShaderConstants,LightingShaderAttrs)
			v:setConstant("lightPos",Shader.CFLOAT4,1,Lighting.light[1],Lighting.light[2],Lighting.light[3],1)
			v:setConstant("ambient",Shader.CFLOAT,1,Lighting.light[4])
			v:setConstant("cameraPos",Shader.CFLOAT4,1,Lighting.camera[1],Lighting.camera[2],Lighting.camera[3],1)
			Lighting._shaders[lcode]=v
		end
	end
	return Lighting._shaders[lcode],lcode
end

function Lighting.setLight(x,y,z,a)
	Lighting.light={x,y,z,a}
	for k,v in pairs(Lighting._shaders) do
		v:setConstant("lightPos",Shader.CFLOAT4,1,x,y,z,1)
		v:setConstant("ambient",Shader.CFLOAT,1,a)
	end
end
function Lighting.setLightTarget(x,y,z,d,f)
	Lighting.lightTarget={x,y,z,d or 50,f or 120}
end

function Lighting.setCamera(x,y,z)
	Lighting.camera={x,y,z}
	for k,v in pairs(Lighting._shaders) do
		v:setConstant("cameraPos",Shader.CFLOAT4,1,x,y,z,1)
	end
end

Lighting._sprites={}
Lighting._shadowed={}
function Lighting.setSpriteMode(sprite,mode)
	local sh,sc=Lighting.getShader(mode)
	sprite:setShader(sh)
	local lsc=sprite._lighting_Mode
	if lsc then
		Lighting._sprites[lsc]=Lighting._sprites[lsc] or {}
		Lighting._sprites[lsc][sprite]=nil
		Lighting._shadowed[sprite]=nil
		D3Anim._animated[sprite]=nil
	end
	sprite._lighting_Mode=sc
	if sc then
		Lighting._sprites[sc]=Lighting._sprites[sc] or {}
		Lighting._sprites[sc][sprite]=sprite
		if sc:find("s") then
			sprite:setTexture(Lighting.getShadowMap(),2)
			Lighting._shadowed[sprite]=sprite
		end
		if sc:find("a") then
			D3Anim._addMesh(sprite)
		end
	end
end

Lighting.shadowrt=nil
function Lighting.getShadowMap(swap)
	if not Lighting.shadowrt then
		local ssz=1024
		local rt1=RenderTarget.new(ssz,ssz,true,false,false,true)
		local rt2=RenderTarget.new(ssz,ssz,true,false,false,true)
		local view=Viewport.new()
		view:setScale(ssz/2,ssz/2)
		view:setPosition(ssz/2,ssz/2)
		Lighting.shadowview=view
		Lighting.shadowrt=rt1
		Lighting.shadowrt1=rt1
		Lighting.shadowrt2=rt2
	end
	if swap then
		if Lighting.shadowrt==Lighting.shadowrt1 then
			Lighting.shadowrt=Lighting.shadowrt2
		else
			Lighting.shadowrt=Lighting.shadowrt1
		end
	end
	return Lighting.shadowrt
end

function Lighting.computeShadows(scene)
	local p=Lighting.lightProj or Matrix.new()
	Lighting.lightProj=p
	local lz=Lighting.lightTarget[4]
	p:perspectiveProjection(Lighting.lightTarget[5],1,lz/16,lz)
	local srt=Lighting.getShadowMap(true)
	local view=Lighting.shadowview
	view:setContent(scene)
	view:setProjection(p)
	view:lookAt(Lighting.light[1],Lighting.light[2],Lighting.light[3],Lighting.lightTarget[1],Lighting.lightTarget[2],Lighting.lightTarget[3],0,1,0)
	p:multiply(view:getTransform())
	for k,v in pairs(Lighting._shaders) do
		v:setConstant("g_LMatrix",Shader.CMATRIX,1,p:getMatrix())
	end
	srt:clear(0,0)
	srt:draw(view)
	for _,v in pairs(Lighting._shadowed) do
		v:setTexture(srt,2)
	end
	--stage:addChild(Bitmap.new(srt))
	--stage:addChild(Lighting.shadowview)
end

Lighting.setCamera(0,0,0)
Lighting.setLight(0,10,0,0.3)
Lighting.setLightTarget(0,0,0) --X,Y,Z, DIST, FOV
