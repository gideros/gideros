Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);

cbuffer cbp : register(b1)
{
	float4 fColor;
	float4 fTexelSize;
	int fRad;
};

float4 PShader(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 frag=float4(0,0,0,0);
	int ext=(2*fRad+1);
	float2 tc=texcoord-fTexelSize.xy*fRad;
	[unroll(21)]
	for (int v=0;v<ext;v++)	
	{
		frag=frag+myTexture.Sample(samLinear, tc)/ext;
		tc+=fTexelSize.xy;
	}
	if (frag.a == 0.0) discard;
	return frag;
}
