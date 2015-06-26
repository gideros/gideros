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
	float4x4 vMatrix;
	float vPSize;
};

VOut VShader(float4 position : vVertex, float4 color : vColor, float2 texcoord : vTexCoord)
{
	VOut output;

	position.w = 1.0f;

	output.position = mul(vMatrix, position);
	output.color = color;
	output.texcoord = texcoord;

	return output;
}

cbuffer cbp : register(b1)
{
	float4 fTexInfo;
};

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	if (fTexInfo.x<=0.0)
	{
		float4 frag = color;
		float alpha = 1.0 - step(0.5, length(texcoord - float2(0.5, 0.5)));
		frag *= alpha;
		return frag;
	}
	else
	 	return myTexture.Sample(samLinear, texcoord*fTexInfo.xy)*color;
}
