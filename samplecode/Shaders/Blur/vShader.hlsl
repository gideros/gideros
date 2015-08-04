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

