Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer cbv : register(b0)
{
	float4x4 vMatrix;
	float4 vfColor;
};

VOut VShader(float4 position : vVertex, float4 color : vColor)
{
	VOut output;

	position.w = 1.0f;

	output.position = mul(vMatrix, position);
	output.color = color*vfColor;

	return output;
}

cbuffer cbp : register(b1)
{
};

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	float4 frag = color;
	return frag;
}
