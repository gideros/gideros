Texture2D myTexture : register(t0);
SamplerState samLinear : register(s0);

cbuffer cb : register(b0)
{
    int use_tex;
    int b2;
    int b3;
    int b4;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
    VOut output;

    output.position = position;
    output.color = color;
    output.texcoord = texcoord;

    return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord: TEXCOORD) : SV_TARGET
{
  if (use_tex==1)
    return myTexture.Sample(samLinear,texcoord)*color;
  else
    return color;
}
