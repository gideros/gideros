Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

cbuffer cbv : register(b0)
{
	float4x4 mTot;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
	VOut output;

	position.w = 1.0f;

	output.position = mul(mTot, position);
	output.color = color;
	output.texcoord = texcoord;

	return output;
}

cbuffer cbp : register(b0)
{
	float4 fColor;
	float fTextureSel;
	float fColorSel;
	int r1, r2; //Padding
};

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 col = lerp(fColor, color, fColorSel);
	float4 tex = lerp(float4(1.0, 1.0, 1.0, 1.0), myTexture.Sample(samLinear, texcoord), fTextureSel);
	float4 frag = col * tex;
	if (frag.a == 0.0) discard;
	return frag;
}
