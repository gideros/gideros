Lighting={}

local LightingVShader=
[[
attribute vec4 POSITION0;
#ifdef TEXTURED
attribute vec2 TEXCOORD0;
#endif
attribute vec3 NORMAL0;

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
#ifdef INSTANCED
uniform highp sampler2D g_InstanceMap;
uniform float InstanceMapWidth;
#endif

void main()
{
#ifdef TEXTURED
	texCoord = TEXCOORD0;
#endif
	highp vec4 pos=POSITION0;
#ifdef INSTANCED
	vec2 tc=vec2(((gl_InstanceID*2+1)/InstanceMapWidth)/2.0,0.5);
	vec4 itex=texture2D(g_InstanceMap, tc);
	pos.x-=itex.r*512.0;
	pos.y+=itex.g*512.0;
	pos.z-=itex.b*512.0;
#endif
	position = (g_MVMatrix*pos).xyz;
	normalCoord = normalize((g_NMatrix*vec4(NORMAL0.xyz,0)).xyz);
#ifdef SHADOWS
	lightSpace = g_LMatrix*vec4(position,1.0);
#endif
	gl_Position = g_MVPMatrix * pos;
}
]]

local LightingFShader=[[
#extension GL_OES_standard_derivatives : enable
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
uniform highp sampler2D g_ShadowMap;
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
		return 0.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture2D(g_ShadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = 0.005;
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  
	if(projCoords.z > 1.0)
        shadow = 1.0;
	return 1.0-shadow;
}  
#endif

void main()
{
#ifdef TEXTURED
	lowp vec3 color0 = texture2D(g_Texture, texCoord).rgb;
#else
	lowp vec3 color0 = g_Color.xyz;
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
	gl_FragColor = vec4(color0 * diff + color1 * spec, 1);
}
]]

local LightingShaderAttrs=
{
{name="POSITION0",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="TEXCOORD0",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
{name="NORMAL0",type=Shader.DFLOAT,mult=3,slot=3,offset=0}
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
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_InstanceMap",type=Shader.CTEXTURE,mult=1,vertex=true}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="InstanceMapWidth",type=Shader.CFLOAT,mult=1,vertex=true}

-- Shaders defs
Lighting._shaders={}
Lighting.getShader=function(code)
	local cmap={
		{"t","TEXTURED"},
		{"s","SHADOWS"},
		{"n","NORMMAP"},
		{"i","INSTANCED"},
	}
	local lcode,ccode="",""
	for _,k in ipairs(cmap) do
		if code:find(k[1]) then
			lcode=lcode..k[1]
			ccode=ccode.."#define "..k[2].."\n"
		end
	end
	if application:getDeviceInfo()~="WinRT" then
		if not Lighting._shaders[lcode] then
		 Lighting._shaders[lcode]=Shader.new(
			ccode..LightingVShader,
			ccode..LightingFShader,
			Shader.FLAG_FROM_CODE,
			LightingShaderConstants,LightingShaderAttrs)
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

Lighting.lightTarget={0,0,0,1}
function Lighting.setLightTarget(x,y,z,d)
	Lighting.lightTarget={x,y,z,d}
end

function Lighting.setCamera(x,y,z)
	Lighting.camera={x,y,z}
	for k,v in pairs(Lighting._shaders) do
	 v:setConstant("cameraPos",Shader.CFLOAT4,1,x,y,z,1)
	end
end
Lighting._sprites={}
function Lighting.setSpriteMode(sprite,mode)
	local sh,sc=Lighting.getShader(mode)
	sprite:setShader(sh)
	local lsc=sprite._lighting_Mode
	if lsc then
		Lighting._sprites[lsc]=Lighting._sprites[lsc] or {}
		Lighting._sprites[lsc][sprite]=nil
	end
	sprite._lighting_Mode=sc
	Lighting._sprites[sc]=Lighting._sprites[sc] or {}
	Lighting._sprites[sc][sprite]=sprite
	if sc:find("s") then
		sprite:setTexture(Lighting.getShadowMap(),2)
	end
end

function Lighting.apply(obj)
	if #Lighting.allShaders>0 then
		for _,v in pairs(obj.objs) do
			for i=1,v:getNumChildren() do
				local m=v:getChildAt(i)
				if m.hasNormals then
					if (m.hasTexture) then				
						if m.hasNormalMap then
							m:setShader(Lighting.normal_shader_tn)			
						else
							m:setShader(Lighting.normal_shader_t)
						end
					else
						m:setShader(Lighting.normal_shader_b)
					end
				end
			end
		end
	end
end

function Lighting.getShadowMap()
	if not Lighting.shadowrt then
		local ssz=1024
		local rt=RenderTarget.new(ssz,ssz,true,false,false,true)
		local view=Viewport.new()
		view:setScale(ssz/2,ssz/2)
		view:setPosition(ssz/2,ssz/2)
		Lighting.shadowview=view
		Lighting.shadowrt=rt
	end
	return Lighting.shadowrt
end

function Lighting.computeShadows(scene)
	local p=Lighting.lightProj or Matrix.new()
	Lighting.lightProj=p
	local lz=30
	local lz=Lighting.lightTarget[4]
	p:perspectiveProjection(120,1,lz/16,lz)
	local srt=Lighting.getShadowMap()
	local view=Lighting.shadowview
	view:setContent(scene)
	view:setProjection(p)
	view:lookAt(Lighting.light[1],Lighting.light[2],Lighting.light[3],Lighting.lightTarget[1],Lighting.lightTarget[2],Lighting.lightTarget[3],0,1,0)
	p:multiply(view:getTransform())
	for k,v in pairs(Lighting._shaders) do
		v:setConstant("g_LMatrix",Shader.CMATRIX,1,p:getMatrix())
	end
	Lighting.shadowrt:clear(0)
	Lighting.shadowrt:draw(view)
	--stage:addChild(Bitmap.new(Lighting.shadowrt))
	--stage:addChild(Lighting.shadowview)
end
