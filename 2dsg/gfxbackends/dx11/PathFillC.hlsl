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
	float4x4 xform;
};

VOut VShader(float4 position : vVertex)
{
	VOut output;

	output.uv = position.zw;

	position.w = 1.0f;
	position.z = 0.0f;

	output.position = mul(mvp, mul(xform,position));

	return output;
}


cbuffer cbp : register(b1)
{
	float4 fColor;
};

float4 PShader(float4 position : SV_POSITION, float2 uv: TEXCOORD) : SV_TARGET
{
	// Gradients  
	float2 px = ddx(uv);
	float2 py = ddy(uv);
	// Chain rule  
	float fx = (2 * uv.x)*px.x - px.y;
	float fy = (2 * uv.x)*py.x - py.y;
	// Signed distance  
	float sd = (uv.x*uv.x - uv.y) / sqrt(fx*fx + fy*fy);
	// Linear alpha  
	float alpha = 0.5 - sd;
	alpha = min(alpha, 1.0f);
	float4 frag = fColor;
	 if (alpha <= 0)  // Outside  
		discard;
	frag*= alpha;
	return frag;
}
