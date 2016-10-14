struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD0;
	float2 steprot : TEXCOORD1;
};

cbuffer cbv : register(b0)
{
	float4x4 vMatrix;
	float4x4 vWorldMatrix;
};

VOut VShader(float4 position : vVertex, float4 color : vColor, float2 texcoord : vTexCoord)
{
	VOut output;

	float psizen = position.z;
	float psize = length(mul(vWorldMatrix, float4(position.z, 0.0, 0.0, 0.0)));
	float angle = position.w*3.141592654 / 180.0;
	output.steprot = float2(sign(psizen) / psize, position.w);
	position.w = 1.0f;
	position.z = 0.0f;
	float2 rad = (texcoord - float2(0.5, 0.5))*psizen;
	float ca=cos(angle);
	float sa=sin(angle);
	float2x2 rot=float2x2(ca,sa,-sa,ca);
	rad=mul(rad,rot);
	output.position = mul(vMatrix, position + float4(rad, 0.0, 0.0));
	output.color = color;
	output.texcoord = texcoord;

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
