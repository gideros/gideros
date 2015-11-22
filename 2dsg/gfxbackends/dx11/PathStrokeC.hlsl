Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);

struct VOut
{
	float4 position : SV_POSITION;
	float4 d0: TEXCOORD0;
	float4 d1: TEXCOORD1;
	float4 d2: TEXCOORD2;
};

cbuffer cbv : register(b0)
{
	float4x4 mvp;
	float4x4 xform;
};

VOut VShader(float4 d0 : dataA, float4 d1: dataB, float4 d2: dataC)
{
	VOut output;

	output.position = mul(mvp, mul(xform, float4(d0.xy,0.0f,1.0f)));
	output.d0 = d0; 
	output.d1 = d1;
	output.d2 = d2;

	return output;
}


cbuffer cbp : register(b1)
{
	float4 fColor;
	float feather;
};

#define M_PI 3.1415926535897932384626433832795                                

float2 evaluateQuadratic(float t, float2 a, float2 b, float2 c)
{
	return a * t * t + b * t + c;
}

float check(float t, float2 pos, float2 ow, float2 a, float2 b, float2 c)
{
	if (0.0f <= t && t <= 1.0f)
	{
		float2 q = evaluateQuadratic(t,a,b,c) - pos;
		return sqrt(dot(q, q)) / ow.y;
	}

	return 2.0;
}

float cbrt(float x)
{
	return sign(x) * pow(abs(x), 1.0f / 3.0f);
}

float4 PShader(float4 position : SV_POSITION, float4 d0: TEXCOORD0, float4 d1: TEXCOORD1, float4 d2 : TEXCOORD2) : SV_TARGET
{
	float2 pos = d0.xy;
	float2 pq = d0.zw;
	float2 a = d1.xy;
	float2 b = d1.zw;
	float2 c = d2.xy;
	float2 ow = d2.zw;

	float d = pq.y * pq.y / 4.0f + pq.x * pq.x * pq.x / 27.0f;
	float4 f = fColor;

	if (d >= 0.0f)
	{
		float c1 = -pq.y / 2.0f;
		float c2 = sqrt(d);
		d = check(cbrt(c1 + c2) + cbrt(c1 - c2) + ow.x, pos, ow, a, b, c);
	}
	else
	{
		float cos_3_theta = 3.0f * pq.y * sqrt(-3.0f / pq.x) / (2.0f * pq.x);
		float theta = acos(cos_3_theta) / 3.0f;
		float r = 2.0f * sqrt(-pq.x / 3.0f);

		d = min(check(r * cos(theta) + ow.x, pos, ow, a, b, c), min(
			check(r * cos(theta + 2.0f * M_PI / 3.0f) + ow.x, pos, ow, a, b, c),
			check(r * cos(theta + 4.0f * M_PI / 3.0f) + ow.x, pos, ow, a, b, c)));
	}
	float alpha = 1.0f - smoothstep(0.5f - feather/2, 0.5f + feather/2, d);
	if (alpha <= 0.0)
		discard;
	f *= alpha;

	return f;
}
