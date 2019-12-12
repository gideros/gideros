D3=D3 or {}

local slang=Shader.getShaderLanguage()
print(slang)

if slang=="glsl" then
D3._V_Shader=
[[
attribute vec4 POSITION;
#ifdef TEXTURED
attribute vec2 TEXCOORD;
#endif
attribute vec3 NORMAL;
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
attribute vec4 INSTMATA;
attribute vec4 INSTMATB;
attribute vec4 INSTMATC;
attribute vec4 INSTMATD;
#endif

void main()
{
#ifdef TEXTURED
	texCoord = TEXCOORD;
#endif
	highp vec4 pos=POSITION;
	mediump vec4 norm=vec4(NORMAL,0.0);
#ifdef INSTANCED_TEST
	vec2 tc=vec2(((gl_InstanceID*2+1)/InstanceMapWidth)/2.0,0.5);
	vec4 itex=texture2D(g_InstanceMap, tc);
	pos.x-=itex.r*512.0;
	pos.y+=itex.g*512.0;
	pos.z-=itex.b*512.0;
#endif
#ifdef INSTANCED
	highp mat4 imat=mat4(INSTMATA,INSTMATB,INSTMATC,INSTMATD);
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

D3._F_Shader=[[
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
elseif slang=="msl" then
D3._V_Shader=
[=[
#include <metal_stdlib>
using namespace metal;

struct InVertex
{
    float3 POSITION0 [[attribute(0)]];
    //float4 vColor [[attribute(1)]];
#ifdef TEXTURED
    float2 TEXCOORD0 [[attribute(2)]];
#endif
    float3 NORMAL0 [[attribute(3)]];
#ifdef ANIMATED
    float4 ANIMIDX [[attribute(4)]];
    float4 ANIMWEIGHT [[attribute(5)]];
#endif
#ifdef INSTANCED
    float4 INSTMAT1 [[attribute(6)]];
    float4 INSTMAT2 [[attribute(7)]];
    float4 INSTMAT3 [[attribute(8)]];
    float4 INSTMAT4 [[attribute(9)]];
#endif
};

struct PVertex
{
    float4 gposition [[position]];
    float3 position [[user(position)]];
    half4 color [[user(color)]];
    float3 normalCoord [[user(normalCoord)]];
#ifdef TEXTURED
    float2 texCoord [[user(texCoord)]];
#endif
#ifdef SHADOWS
    float4 lightSpace [[user(lightSpace)]];
#endif
};


struct Uniforms
{
    float4x4 g_MVPMatrix;
    float4x4 g_MVMatrix;
    float4x4 g_NMatrix;
    float4x4 g_LMatrix;
	float4 g_Color;
	float4 lightPos;
	float4 cameraPos;
	float ambient;
	float4x4 bones[16];
#ifdef INSTANCED_TEST
//uniform highp sampler2D g_InstanceMap;
	float InstanceMapWidth;
#endif
	float4x4 InstanceMatrix;
};


vertex PVertex vmain(InVertex inVertex [[stage_in]],
                      constant Uniforms &u [[buffer(0)]])
{
    PVertex o;
#ifdef TEXTURED
	o.texCoord = inVertex.TEXCOORD0;
#endif
	float4 pos=float4(inVertex.POSITION0,1.0);
	float4 norm=float4(inVertex.NORMAL0,0.0);
#ifdef INSTANCED_TEST
	float2 tc=float2(((gl_InstanceID*2+1)/InstanceMapWidth)/2.0,0.5);
	float4 itex=texture2D(g_InstanceMap, tc);
	pos.x-=itex.r*512.0;
	pos.y+=itex.g*512.0;
	pos.z-=itex.b*512.0;
#endif
#ifdef INSTANCED
	float4x4 imat=float4x4(inVertex.INSTMAT1,inVertex.INSTMAT2,inVertex.INSTMAT3,inVertex.INSTMAT4);
	pos=imat*u.InstanceMatrix*pos;
	norm = imat*u.InstanceMatrix*float4(norm.xyz, 0.0);
#endif

#ifdef ANIMATED
	float4x4 skinning=float4x4(0.0);
	skinning+=u.bones[int(inVertex.ANIMIDX.x)]*inVertex.ANIMWEIGHT.x;
	skinning+=u.bones[int(inVertex.ANIMIDX.y)]*inVertex.ANIMWEIGHT.y;
	skinning+=u.bones[int(inVertex.ANIMIDX.z)]*inVertex.ANIMWEIGHT.z;
	skinning+=u.bones[int(inVertex.ANIMIDX.w)]*inVertex.ANIMWEIGHT.w;
    float4 npos = skinning * pos;
    float4 nnorm = skinning * float4(norm.xyz, 0.0);
	pos=float4(npos.xyz,1.0);
	norm=nnorm;
#endif
	o.position = (u.g_MVMatrix*pos).xyz;
	o.normalCoord = normalize((u.g_NMatrix*float4(norm.xyz,0)).xyz);
#ifdef SHADOWS
	o.lightSpace = u.g_LMatrix*float4(o.position,1.0);
#endif
	o.gposition = u.g_MVPMatrix * pos;	

    return o;
}

]=]

D3._F_Shader=[=[
#include <metal_stdlib>
using namespace metal;

struct PVertex
{
    float4 gposition [[position]];
    float3 position [[user(position)]];
    half4 color [[user(color)]];
    float3 normalCoord [[user(normalCoord)]];
#ifdef TEXTURED
    float2 texCoord [[user(texCoord)]];
#endif
#ifdef SHADOWS
    float4 lightSpace [[user(lightSpace)]];
#endif
};


struct Uniforms
{
    float4x4 g_MVPMatrix;
    float4x4 g_MVMatrix;
    float4x4 g_NMatrix;
    float4x4 g_LMatrix;
	float4 g_Color;
	float4 lightPos;
	float4 cameraPos;
	float ambient;
	float4x4 bones[16];
#ifdef INSTANCED_TEST
//uniform highp sampler2D g_InstanceMap;
	float InstanceMapWidth;
#endif
	float4x4 InstanceMatrix;
};



#ifdef NORMMAP
float3x3 cotangent_frame(float3 N, float3 p, float2 uv)
{
    // récupère les vecteurs du triangle composant le pixel
    float3 dp1 = dFdx( p );
    float3 dp2 = dFdy( p );
    float2 duv1 = dFdx( uv );
    float2 duv2 = dFdy( uv );

    // résout le système linéaire
    float3 dp2perp = cross( dp2, N );
    float3 dp1perp = cross( N, dp1 );
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construit une trame invariante à l'échelle 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return float3x3( T * invmax, B * invmax, N );
}

float3 perturb_normal( float3 N, float3 V, float2 texcoord,texture2d<half> ntex [[texture(1)]], sampler nsmp [[sampler(1)]] )
{
    // N, la normale interpolée et
    // V, le vecteur vue (vertex dirigé vers l'œil)
    float3 map = ntex.sample(nsmp, texcoord ).xyz;
    map = map * 255./127. - 128./127.;
    float3x3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}
#endif

#ifdef SHADOWS
float ShadowCalculation(float4 fragPosLightSpace,depth2d<float> stex [[texture(2)]], sampler ssmp [[sampler(2)]])
{
    // perform perspective divide
    float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5 +0.5;
	projCoords.y=1-projCoords.y;
	if ((projCoords.x<0.0)||(projCoords.y<0.0)||(projCoords.x>=1.0)||(projCoords.y>=1.0))
		return 1.0;
	projCoords.z-=0.005; //BIAS
	float shadow=stex.sample_compare(ssmp, projCoords.xy, projCoords.z);
	if(projCoords.z >= 0.999)
        shadow = 1.0;
	return shadow;
}  
#endif

fragment half4 fmain(
	PVertex vert [[stage_in]],
    constant Uniforms &u [[buffer(0)]]
	#ifdef TEXTURED
	,texture2d<half> ttex [[texture(0)]], sampler tsmp [[sampler(0)]]
	#endif
	#ifdef NORMMAP
	,texture2d<half> ntex [[texture(1)]], sampler nsmp [[sampler(1)]]
	#endif
	#ifdef SHADOWS
	,depth2d<float> stex [[texture(2)]], sampler ssmp [[sampler(2)]]
	#endif
)
{
#ifdef TEXTURED
	half4 color0 = half4(u.g_Color)*ttex.sample(tsmp, vert.texCoord);
#else
	half4 color0 = half4(u.g_Color);
#endif
	half3 color1 = half3(0.5, 0.5, 0.5);
	float3 normal = normalize(vert.normalCoord);
	
	float3 lightDir = normalize(u.lightPos.xyz - vert.position.xyz);
	float3 viewDir = normalize(u.cameraPos.xyz-vert.position.xyz);
#ifdef NORMMAP	
	normal=perturb_normal(normal, viewDir, texCoord,ntex,nsmp);
#endif	
	
	float diff = max(0.0, dot(normal, lightDir));
	float spec =0.0;
    // calculate shadow
	float shadow=1.0;
#ifdef SHADOWS
    shadow = ShadowCalculation(vert.lightSpace,stex,ssmp);       
#endif	
	if (diff>0.0)
	{
		float nh = max(0.0, dot(reflect(-lightDir,normal),viewDir));
		spec = pow(nh, 32.0)*shadow;
	}

	diff=(half)max((diff*shadow),u.ambient);
	return half4(color0.rgb * diff + color1 * spec, color0.a);
}
]=]

elseif slang=="hlsl" then

D3._V_Shader=
[=[

struct PVertex
{
    float3 position : POSITION;
//    half4 color : COLOR;
#ifdef TEXTURED
    float2 texCoord : TEXCOORD;
#endif
    float3 normalCoord : NORMAL;
#ifdef SHADOWS
    float4 lightSpace : LIGHTSPACE;
#endif
    float4 gposition : SV_POSITION;
};


cbuffer cbv : register(b0)
{
    float4x4 g_MVPMatrix;
    float4x4 g_MVMatrix;
    float4x4 g_NMatrix;
    float4x4 g_LMatrix;
	float4x4 bones[16];
#ifdef INSTANCED_TEST
//uniform highp sampler2D g_InstanceMap;
	float InstanceMapWidth;
#endif
	float4x4 InstanceMatrix;
};

PVertex VShader(
float3 POSITION0 : POSITION,
#ifdef TEXTURED
float2 TEXCOORD0 : TEXCOORD,
#endif
#ifdef ANIMATED
float4 ANIMIDX : ANIMIDX,
float4 ANIMWEIGHT : ANIMWEIGHT,
#endif
#ifdef INSTANCED
float4 INSTMAT1 : INSTMATA,
float4 INSTMAT2 : INSTMATB,
float4 INSTMAT3 : INSTMATC,
float4 INSTMAT4 : INSTMATD,
#endif
float3 NORMAL0 : NORMAL)
{
    PVertex o;
#ifdef TEXTURED
	o.texCoord = TEXCOORD0;
#endif
	float4 pos=float4(POSITION0,1.0);
	float4 norm=float4(NORMAL0,0.0);
#ifdef INSTANCED_TEST
	float2 tc=float2(((gl_InstanceID*2+1)/InstanceMapWidth)/2.0,0.5);
	float4 itex=texture2D(g_InstanceMap, tc);
	pos.x-=itex.r*512.0;
	pos.y+=itex.g*512.0;
	pos.z-=itex.b*512.0;
#endif
#ifdef INSTANCED
	float4x4 imat=float4x4(INSTMAT1,INSTMAT2,INSTMAT3,INSTMAT4);
	float4x4 mmat=mul(transpose(imat),InstanceMatrix);
	pos=mul(mmat,pos);
	norm = mul(mmat,float4(norm.xyz, 0.0));
#endif

#ifdef ANIMATED
	float4x4 skinning={{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}};
	skinning+=bones[int(ANIMIDX.x)]*ANIMWEIGHT.x;
	skinning+=bones[int(ANIMIDX.y)]*ANIMWEIGHT.y;
	skinning+=bones[int(ANIMIDX.z)]*ANIMWEIGHT.z;
	skinning+=bones[int(ANIMIDX.w)]*ANIMWEIGHT.w;
    float4 npos = mul(skinning,pos);
    float4 nnorm = mul(skinning,float4(norm.xyz, 0.0));
	pos=float4(npos.xyz,1.0);
	norm=nnorm;
#endif
	o.position = mul(g_MVMatrix,pos).xyz;
	o.normalCoord = normalize(mul(g_NMatrix,float4(norm.xyz,0)).xyz);
#ifdef SHADOWS
	o.lightSpace = mul(g_LMatrix,float4(o.position,1.0));
#endif
	o.gposition = mul(g_MVPMatrix,pos);	

    return o;
}
]=]

D3._F_Shader=[=[
cbuffer cbp : register(b1)
{
	float4 g_Color;
	float4 lightPos;
	float4 cameraPos;
	float ambient;
};

#ifdef TEXTURED
Texture2D ttex : register(t0);
SamplerState tsmp : register(s0);
#endif
#ifdef NORMMAP
Texture2D ntex : register(t1);
SamplerState nsmp : register(s1);
#endif
#ifdef SHADOWS
Texture2D stex : register(t2);
SamplerComparisonState ssmp : register(s2);
#endif

#ifdef NORMMAP
float3x3 cotangent_frame(float3 N, float3 p, float2 uv)
{
    // récupère les vecteurs du triangle composant le pixel
    float3 dp1 = dFdx( p );
    float3 dp2 = dFdy( p );
    float2 duv1 = dFdx( uv );
    float2 duv2 = dFdy( uv );

    // résout le système linéaire
    float3 dp2perp = cross( dp2, N );
    float3 dp1perp = cross( N, dp1 );
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construit une trame invariante à l'échelle 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return float3x3( T * invmax, B * invmax, N );
}

float3 perturb_normal( float3 N, float3 V)
{
    // N, la normale interpolée et
    // V, le vecteur vue (vertex dirigé vers l'œil)
    float3 map = ntex.Sample(nsmp, texcoord ).xyz;
    map = map * 255./127. - 128./127.;
    float3x3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}
#endif

#ifdef SHADOWS
float ShadowCalculation(float4 fragPosLightSpace)
{
    // perform perspective divide
    float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
	//projCoords.y*=-1;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
	float shadow=1.0;
	if (!((projCoords.x<0.0)||(projCoords.y<0.0)||(projCoords.x>=1.0)||(projCoords.y>=1.0))) {
		projCoords.z-=0.001; //BIAS
		if(projCoords.z >= 0.999)
			shadow = 1.0;
		else
			shadow=stex.SampleCmp(ssmp, projCoords.xy,projCoords.z);
	}
	return shadow;
}  
#endif

float4 PShader(
	float3 position : POSITION,
#ifdef TEXTURED
	float2 texCoord : TEXCOORD,
#endif
	float3 normalCoord : NORMAL,
#ifdef SHADOWS
	float4 lightSpace : LIGHTSPACE,
#endif
	float4 gposition : SV_POSITION
) : SV_TARGET
{
#ifdef TEXTURED
	half4 color0 = half4(g_Color)*ttex.Sample(tsmp, texCoord);
#else
	half4 color0 = half4(g_Color);
#endif
	half3 color1 = half3(0.5, 0.5, 0.5);
	float3 normal = normalize(normalCoord);
	
	float3 lightDir = normalize(lightPos.xyz - position.xyz);
	float3 viewDir = normalize(cameraPos.xyz-position.xyz);
#ifdef NORMMAP	
	normal=perturb_normal(normal, viewDir, texCoord);
#endif	
	
	float diff = max(0.0, dot(normal, lightDir));
	float spec =0.0;
    // calculate shadow
	float shadow=1.0;
#ifdef SHADOWS
    shadow = ShadowCalculation(lightSpace);       
#endif	
	if (diff>0.0)
	{
		float nh = max(0.0, dot(reflect(-lightDir,normal),viewDir));
		spec = pow(nh, 32.0)*shadow;
	}

	diff=(half)max((diff*shadow),ambient);
	return half4(color0.rgb * diff + color1 * spec, color0.a);
}
]=]
end