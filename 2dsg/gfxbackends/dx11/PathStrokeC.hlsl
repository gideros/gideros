Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);

struct VOut
{
	float4 position : SV_POSITION;
	float2 pos : TEXCOORD1;
	float2 pq: TEXCOORD2;
	float2 a: TEXCOORD3,b : TEXCOORD4, c : TEXCOORD5;
	float2 ow : TEXCOORD6;
};

cbuffer cbv : register(b0)
{
	float4x4 mvp;
};

VOut VShader(float4 d0 : dataA, float4 d1: dataB, float4 d2: dataC)
{
	VOut output;

	output.position = mul(mvp, float4(d0.xy,0.0f,1.0f));
	output.pos = d0.xy;
	output.pq = d0.zw;
	output.a = d1.xy;
	output.b = d1.zw;
	output.c = d2.xy;
	output.ow = d2.zw;

	return output;
}


cbuffer cbp : register(b1)
{
	float4 fColor;
};

#define M_PI 3.1415926535897932384626433832795                                

float2 evaluateQuadratic(float t, float2 a: TEXCOORD3, float2 b : TEXCOORD4, float2 c : TEXCOORD5)
{
	return a * t * t + b * t + c;
}

bool check(float t, float2 pos: TEXCOORD1, float2 ow : TEXCOORD6, float2 a : TEXCOORD3, float2 b : TEXCOORD4, float2 c : TEXCOORD5)
{
	if (0.0f <= t && t <= 1.0f)
	{
		float2 q = evaluateQuadratic(t,a,b,c) - pos;
		if (dot(q, q) <= ow.y)
			return false;
	}

	return true;
}

float cbrt(float x)
{
	return sign(x) * pow(abs(x), 1.0f / 3.0f);
}

float4 PShader(float2 pq: TEXCOORD2, float2 ow : TEXCOORD6, float2 pos : TEXCOORD1, float2 a : TEXCOORD3, float2 b : TEXCOORD4, float2 c : TEXCOORD5) : SV_TARGET
{
	float d = pq.y * pq.y / 4.0f + pq.x * pq.x * pq.x / 27.0f;

	if (d >= 0.0f)
	{
		float c1 = -pq.y / 2.0f;
		float c2 = sqrt(d);
		if (check(cbrt(c1 + c2) + cbrt(c1 - c2) + ow.x,pos,ow,a,b,c)) discard;
	}
	else
	{
		float cos_3_theta = 3.0f * pq.y * sqrt(-3.0f / pq.x) / (2.0f * pq.x);
		float theta = acos(cos_3_theta) / 3.0f;
		float r = 2.0f * sqrt(-pq.x / 3.0f);

		if (check(r * cos(theta) + ow.x, pos, ow, a,b,c) &&
			check(r * cos(theta + 2.0f * M_PI / 3.0f) + ow.x, pos, ow, a,b,c) &&
			check(r * cos(theta + 4.0f * M_PI / 3.0f) + ow.x, pos, ow, a,b,c)) discard;
	}
	return fColor;
}
