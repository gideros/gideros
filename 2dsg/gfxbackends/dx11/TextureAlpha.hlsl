Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);

struct VOut
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

cbuffer cbv : register(b0)
{
	float4x4 vMatrix;
};

VOut VShader(float4 position : vVertex, float2 texcoord : vTexCoord)
{
	VOut output;

	position.w = 1.0f;

	output.position = mul(vMatrix, position);
	output.texcoord = texcoord;

	return output;
}

cbuffer cbp : register(b1)
{
	float4 fColor;
};

float4 PShader(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 frag = myTexture.Sample(samLinear, texcoord);
	frag=frag.aaaa*fColor;
	if (frag.a == 0.0) discard;
	return frag;
}
