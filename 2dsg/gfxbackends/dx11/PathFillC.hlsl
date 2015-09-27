Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);

struct VOut
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer cbv : register(b0)
{
	float4x4 mvp;
};

VOut VShader(float4 position : vVertex)
{
	VOut output;

	position.w = 1.0f;
	position.z = 0.0f;

	output.position = mul(mvp, position);
	output.uv = position.zw;

	return output;
}


cbuffer cbp : register(b1)
{
	float4 fColor;
};

float4 PShader(float4 position : SV_POSITION, float2 uv: TEXCOORD) : SV_TARGET
{
	float u = uv.x;
	float v = uv.x;
	float4 frag = fColor;
	if (u*u > v) discard;
	return frag;
}
