struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD0;
	float2 steprot : TEXCOORD1;
};

cbuffer cbv : register(b0)
{
	float4x4 vWorldMatrix;
	float4x4 vViewMatrix;
	float4x4 vProjMatrix;
	float4 vfColor;
};

VOut VShader(float4 position : vVertex, float4 color : vColor, float4 texcoord : vTexCoord)
{
	VOut output;

	float psizen = texcoord.z;
	float psize = length(mul(vWorldMatrix, float4(texcoord.z, 0.0, 0.0, 0.0)));
	float angle = texcoord.w*3.141592654 / 180.0;
	output.steprot = float2(sign(psizen) / 100.0, texcoord.w);
	position.w = 1.0f;
	float2 rad = (texcoord.xy - float2(0.5, 0.5));
	float ca=cos(angle);
	float sa=sin(angle);
	float2x2 rot=float2x2(ca,sa,-sa,ca);
	rad=mul(rad,rot);
	float4 vertex=mul(vViewMatrix,mul(vWorldMatrix,position));
	vertex.xy+=rad*psize;
	output.position = mul(vProjMatrix, vertex);
	output.color = color*vfColor;
	output.texcoord = texcoord.xy;

	return output;
}


Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);


cbuffer cbp : register(b1)
{
	float4 fTexInfo;
};

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD0, float2 steprot : TEXCOORD1) : SV_TARGET
{
	if (steprot.x == 0.0)
		discard;
	if (steprot.x < 0.0)
		return color;
	float2 rad = float2(-0.5,-0.5) + texcoord;
	if (fTexInfo.x <= 0.0)
	{
		float4 frag = color;
		float alpha = 1.0 - smoothstep(0.5 - steprot.x,0.5 + steprot.x,length(rad));
		frag *= alpha;
		return frag;
	}
	else
	{
		if ((rad.x<-0.5) || (rad.y<-0.5) || (rad.x>0.5) || (rad.y>0.5))
			discard;
		return myTexture.Sample(samLinear, (rad + float2(0.5,0.5))*fTexInfo.xy)*color;
	}
}
