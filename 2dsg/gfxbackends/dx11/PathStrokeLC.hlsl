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
	float width;
};

VOut VShader(float4 position : vVertex)
{
	VOut output;

	output.uv = position.zw;
	
	float4 dpos = float4(output.uv*width+position.xy,0, 1.0);
	output.position = mul(mvp, dpos);

	return output;
}


cbuffer cbp : register(b1)
{
	float4 fColor;
	float feather;
};

float4 PShader(float4 position : SV_POSITION, float2 uv: TEXCOORD) : SV_TARGET
{
	float l = length(uv);
	float4 frag = fColor;
	float alpha = 1.0f - smoothstep(0.5f - feather/2, 0.5f + feather/2, l);
	if (alpha <= 0.0)
		discard;
	frag *= alpha;
	return frag;
}
