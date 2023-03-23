//
//  giderosShaders.metal
//  iosplayer
//
//  Created by nico on 31/01/2019.
//  Copyright © 2019 Gideros Mobile. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;


struct InVertexFC
{
    float4 data0 [[attribute(0)]];
};
struct InVertexSL
{
    float4 data0 [[attribute(0)]];
    float4 linepos [[attribute(1)]];
};
struct InVertexSC
{
    float4 data0 [[attribute(0)]];
    float4 data1 [[attribute(1)]];
    float4 data2 [[attribute(2)]];
    float4 linepos [[attribute(3)]];
};

struct PVertexTC
{
    float4 position [[position]];
    float2 uv [[user(texcoord)]];
};
struct PVertexSC
{
    float4 position [[position]];
    float2 pos [[user(pos)]];
    float2 a [[user(a)]];
    float2 b [[user(b)]];
    float2 c [[user(c)]];
    float p [[user(p)]];
    float q [[user(q)]];
    half offset [[user(offset)]];
    half strokeWidth [[user(stroke)]];
};

struct Uniforms
{
    float4x4 mvp;
    float4x4 xform;
    float4 fColor;
    float feather;
};


//Fill curve
vertex PVertexTC gidPathFCV(InVertexFC inVertex [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexTC outVert;
    outVert.position = uniforms.mvp * uniforms.xform * float4(inVertex.data0.xy,0.0,1.0);
    outVert.uv=inVertex.data0.zw;
    return outVert;
}

fragment half4 gidPathFCF(PVertexTC vert [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]])
{
    if (vert.uv.x*vert.uv.x > vert.uv.y) discard_fragment();
    return half4(uniforms.fColor);
}

//Stroke curve
vertex PVertexSC gidPathSCV(InVertexSC inVertex [[stage_in]],
                           constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexSC outVert;
    outVert.position = uniforms.mvp * uniforms.xform * float4(inVertex.data0.xy,0.0,1.0);
    outVert.pos=inVertex.data0.xy;
    outVert.p=inVertex.data0.z;
    outVert.q=inVertex.data0.w;
    outVert.a=inVertex.data1.xy;
    outVert.b=inVertex.data1.zw;
    outVert.c=inVertex.data2.xy;
    outVert.offset=inVertex.data2.z;
    outVert.strokeWidth=inVertex.data2.w;
    return outVert;
}

#define M_PI 3.1415926535897932384626433832795
float2 evaluateQuadratic(float t,float2 a,float2 b,float2 c)
{
   return a * t * t + b * t + c;
}

float check(float t,float2 a,float2 b,float2 c,float2 pos,float strokeWidth)
{
   if (0.0 <= t && t <= 1.0)
   {
      float2 q = evaluateQuadratic(t,a,b,c) - pos;
      return length(q)/strokeWidth;
   }
    
   return 2.0;
}

float cbrt(float x)
{
   return sign(x) * pow(abs(x), 1.0 / 3.0);
}

fragment half4 gidPathSCF(PVertexSC vert [[stage_in]],
                         constant Uniforms &uniforms [[buffer(0)]])
{
   float d = vert.q * vert.q / 4.0 + vert.p * vert.p * vert.p / 27.0;
    
   if (d >= 0.0)
   {
      float c1 = -vert.q / 2.0;
      float c2 = sqrt(d);
      d=check(cbrt(c1 + c2) + cbrt(c1 - c2) + vert.offset,vert.a,vert.b,vert.c,vert.pos,vert.strokeWidth);
   }
   else
   {
      float cos_3_theta = 3.0 * vert.q * sqrt(-3.0 / vert.p) / (2.0 * vert.p);
      float theta = acos(cos_3_theta) / 3.0;
      float r = 2.0 * sqrt(-vert.p / 3.0);
       
       d=min(check(r * cos(theta) + vert.offset,vert.a,vert.b,vert.c,vert.pos,vert.strokeWidth),
             min(check(r * cos(theta + 2.0 * M_PI / 3.0) + vert.offset,vert.a,vert.b,vert.c,vert.pos,vert.strokeWidth),
                 check(r * cos(theta + 4.0 * M_PI / 3.0) + vert.offset,vert.a,vert.b,vert.c,vert.pos,vert.strokeWidth)));
   }
   half alpha= 1.0-smoothstep(0.5 - uniforms.feather/2.0, 0.5 + uniforms.feather/2.0, d);
    if (alpha <= 0.0)  // Outside
            discard_fragment();
    return half4(uniforms.fColor*alpha);
}


//Stroke line
vertex PVertexTC gidPathSLV(InVertexSL inVertex [[stage_in]],
                            constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexTC outVert;
    outVert.position = uniforms.mvp * uniforms.xform * float4(inVertex.data0.xy+inVertex.data0.zw,0.0,1.0);
    outVert.uv=normalize(inVertex.data0.zw);
    return outVert;
}

fragment half4 gidPathSLF(PVertexTC vert [[stage_in]],
                          constant Uniforms &uniforms [[buffer(0)]])
{
    float l=length(vert.uv);
    half alpha= 1.0-smoothstep(0.5 - uniforms.feather/2.0, 0.5 + uniforms.feather/2.0, l);
    if (alpha <= 0.0)  // Outside
        discard_fragment();
    return half4(uniforms.fColor*alpha);
}
