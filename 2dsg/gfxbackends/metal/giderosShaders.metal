//
//  giderosShaders.metal
//  iosplayer
//
//  Created by nico on 31/01/2019.
//  Copyright © 2019 Gideros Mobile. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;


struct InVertex
{
    float2 vVertex [[attribute(0)]];
};
struct InVertexT
{
    float2 vVertex [[attribute(0)]];
    float2 vTexcoord [[attribute(2)]];
};
struct InVertexC
{
    float2 vVertex [[attribute(0)]];
    uchar4 vColor [[attribute(1)]];
};
struct InVertexTC
{
    float2 vVertex [[attribute(0)]];
    float2 vTexcoord [[attribute(2)]];
    uchar4 vColor [[attribute(1)]];
};
struct InVertexPS
{
    float4 vVertex [[attribute(0)]];
    float4 vTexcoord [[attribute(2)]];
    uchar4 vColor [[attribute(1)]];
};
struct InVertex3
{
    float3 vVertex [[attribute(0)]];
};
struct InVertexT3
{
    float3 vVertex [[attribute(0)]];
    float2 vTexcoord [[attribute(2)]];
};
struct InVertexC3
{
    float3 vVertex [[attribute(0)]];
    uchar4 vColor [[attribute(1)]];
};
struct InVertexTC3
{
    float3 vVertex [[attribute(0)]];
    float2 vTexcoord [[attribute(2)]];
    uchar4 vColor [[attribute(1)]];
};

struct PVertex
{
    float4 position [[position]];
};
struct PVertexC
{
    float4 position [[position]];
    half4 color [[user(color)]];
};
struct PVertexT
{
    float4 position [[position]];
    float2 texcoord [[user(texcoord)]];
};
struct PVertexTC
{
    float4 position [[position]];
    float2 texcoord [[user(texcoord)]];
    half4 color [[user(color)]];
};

struct PVertexP
{
    float4 position [[position]];
    half4 color [[user(color)]];
    float pointsize[[point_size]];
};

struct PVertexPS
{
    float4 position [[position]];
    half4 color [[user(color)]];
    float2 texcoord [[user(texcoord)]];
    float2 steprot [[user(steprot)]];
};

struct Uniforms
{
    float4x4 vMatrix;
    float4 fColor;
};

struct UniformsP
{
    float4x4 vMatrix;
    float4x4 vWorldMatrix;
    float4 fColor;
    float4 fTexInfo;
    float vPSize;
};

struct UniformsPS
{
    float4x4 vMatrix;
    float4x4 vWorldMatrix;
    float4 fColor;
    float4 fTexInfo;
};

struct UniformsPS3
{
    float4x4 vWorldMatrix;
    float4x4 vViewMatrix;
    float4x4 vProjMatrix;
    float4 fColor;
    float4 fTexInfo;
};

//Solid color shader
vertex PVertex gidV(InVertex inVertex [[stage_in]],
           constant Uniforms &uniforms [[buffer(0)]])
{
    PVertex outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,0.0,1.0);
    return outVert;
}

vertex PVertex gidV3(InVertex3 inVertex [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]])
{
    PVertex outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,1.0);
    return outVert;
}

fragment half4 gidF(PVertex vert [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]])
{
    return half4(uniforms.fColor);
}

//Per vertex color shader
vertex PVertexC gidCV(InVertexC inVertex [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexC outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,0.0,1.0);
    outVert.color = half4(uniforms.fColor*float4(inVertex.vColor)/255.0);
    return outVert;
}

vertex PVertexC gidCV3(InVertexC3 inVertex [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexC outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,1.0);
    outVert.color = half4(uniforms.fColor*float4(inVertex.vColor)/255.0);
    return outVert;
}

fragment half4 gidCF(PVertexC vert [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]])
{
    return half4(vert.color);
}

//Texture shader
vertex PVertexT gidTV(InVertexT inVertex [[stage_in]],
                      constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexT outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,0.0,1.0);
    outVert.texcoord = inVertex.vTexcoord;
    return outVert;
}

vertex PVertexT gidTV3(InVertexT3 inVertex [[stage_in]],
                      constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexT outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,1.0);
    outVert.texcoord = inVertex.vTexcoord;
    return outVert;
}

fragment half4 gidTF(PVertexT vert [[stage_in]],
                     constant Uniforms &uniforms [[buffer(0)]],
                     texture2d<float> tex [[texture(0)]],
                     sampler smp [[sampler(0)]])
{
    float4 frag=uniforms.fColor*tex.sample(smp, vert.texcoord);
    if (frag.a==0.0) discard_fragment();
    return half4(frag);
}

fragment half4 gidTAF(PVertexT vert [[stage_in]],
                     constant Uniforms &uniforms [[buffer(0)]],
                     texture2d<float> tex [[texture(0)]],
                     sampler smp [[sampler(0)]])
{
    float4 frag=tex.sample(smp, vert.texcoord);
    frag=uniforms.fColor*frag.aaaa;
    if (frag.a==0.0) discard_fragment();
    return half4(frag);
}

//Texture+Color shader
vertex PVertexTC gidCTV(InVertexTC inVertex [[stage_in]],
                        constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexTC outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,0.0,1.0);
    outVert.texcoord = inVertex.vTexcoord;
    outVert.color = half4(uniforms.fColor*float4(inVertex.vColor)/255.0);
    return outVert;
}

vertex PVertexTC gidCTV3(InVertexTC3 inVertex [[stage_in]],
                        constant Uniforms &uniforms [[buffer(0)]])
{
    PVertexTC outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,1.0);
    outVert.texcoord = inVertex.vTexcoord;
    outVert.color = half4(uniforms.fColor*float4(inVertex.vColor)/255.0);
    return outVert;
}

fragment half4 gidCTF(PVertexTC vert [[stage_in]],
                     constant Uniforms &uniforms [[buffer(0)]],
                     texture2d<half> tex [[texture(0)]],
                     sampler smp [[sampler(0)]])
{
    half4 frag=vert.color*tex.sample(smp, vert.texcoord);
    if (frag.a==0.0) discard_fragment();
    return frag;
}

fragment half4 gidCTAF(PVertexTC vert [[stage_in]],
                      constant Uniforms &uniforms [[buffer(0)]],
                      texture2d<half> tex [[texture(0)]],
                      sampler smp [[sampler(0)]])
{
    half4 frag=tex.sample(smp, vert.texcoord);
    frag=vert.color*frag.aaaa;
    if (frag.a==0.0) discard_fragment();
    return half4(frag);
}

//Point particle shader (box2D particles)
vertex PVertexP gidPV(InVertexC inVertex [[stage_in]],
                        constant UniformsP &uniforms [[buffer(0)]])
{
    PVertexP outVert;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex,0.0,1.0);
    outVert.color = half4(uniforms.fColor*float4(inVertex.vColor)/255.0);
    float4 xpsize=(uniforms.vWorldMatrix*float4(uniforms.vPSize,0.0,0.0,1.0))-(uniforms.vWorldMatrix*float4(0.0,0.0,0.0,1.0));
    outVert.pointsize=length(xpsize.xyz);
    return outVert;
}

fragment half4 gidPF(PVertexP vert [[stage_in]],
                      float2 pc [[point_coord]],
                      constant UniformsP &uniforms [[buffer(0)]])
{
    float2 rad=pc+float2(-0.5,-0.5);
    float alpha=1.0-step(0.5,length(rad));
    return half4(vert.color*alpha);
}

fragment half4 gidPTF(PVertexP vert [[stage_in]],
                     float2 pc [[point_coord]],
                     constant UniformsP &uniforms [[buffer(0)]],
                     texture2d<half> tex [[texture(0)]],
                     sampler smp [[sampler(0)]])
{
    return vert.color*tex.sample(smp,pc*uniforms.fTexInfo.xy);
}

//Mesh particle shader (Gideros particles)
vertex PVertexPS gidPSV(InVertexPS inVertex [[stage_in]],
                      constant UniformsPS &uniforms [[buffer(0)]])
{
    PVertexPS outVert;
    float2 rad=(float2(-0.5,-0.5)+inVertex.vTexcoord.xy)*inVertex.vTexcoord.z;
    float angle=inVertex.vTexcoord.w*3.141592654/180.0;
    float ca=cos(angle);
    float sa=sin(angle);
    float2x2 rot=float2x2(float2(ca,sa),float2(-sa,ca));
    rad=rad*rot;
    outVert.position = uniforms.vMatrix * float4(inVertex.vVertex.xy+rad,inVertex.vVertex.z,1.0);
    outVert.color = half4(uniforms.fColor*float4(inVertex.vColor)/255.0);
    outVert.texcoord = inVertex.vTexcoord.xy;
    float4 xpsize=uniforms.vWorldMatrix*float4(inVertex.vVertex.z,0.0,0.0,0.0);
    float xpl=length(xpsize.xyz);
    if (xpl==0.0) xpl=1.0;
    outVert.steprot=float2(sign(inVertex.vTexcoord.z)/xpl,inVertex.vTexcoord.w);
    return outVert;
}

vertex PVertexPS gidPS3V(InVertexPS inVertex [[stage_in]],
                      constant UniformsPS3 &uniforms [[buffer(0)]])
{
    PVertexPS outVert;
    float2 rad=(float2(-0.5,-0.5)+inVertex.vTexcoord.xy);
    float angle=inVertex.vTexcoord.w*3.141592654/180.0;
    float ca=cos(angle);
    float sa=sin(angle);
    float2x2 rot=float2x2(float2(ca,sa),float2(-sa,ca));
    rad=rad*rot;
    outVert.color = half4(uniforms.fColor*float4(inVertex.vColor)/255.0);
    outVert.texcoord = inVertex.vTexcoord.xy;
    float4 xpsize=uniforms.vWorldMatrix*float4(inVertex.vVertex.z,0.0,0.0,0.0);
    float xpl=length(xpsize.xyz);
    if (xpl==0.0) xpl=1.0;
    outVert.steprot=float2(sign(inVertex.vTexcoord.z)/100.0,inVertex.vTexcoord.w);
    float4 vpos = uniforms.vViewMatrix*(uniforms.vWorldMatrix*float4(inVertex.vVertex.xyz,1.0));
    vpos.xy+=rad*xpl;
    outVert.position = uniforms.vProjMatrix *vpos;
    return outVert;
}

fragment half4 gidPSF(PVertexPS vert [[stage_in]],
                     constant UniformsPS &uniforms [[buffer(0)]])
{
    if (vert.steprot.x==0.0) discard_fragment();
    if (vert.steprot.x<=0.0) return half4(vert.color);

    float2 rad=vert.texcoord+float2(-0.5,-0.5);
    float alpha=1.0-smoothstep(0.5-vert.steprot.x,0.5+vert.steprot.x,length(rad));
    return half4(vert.color*alpha);
}

fragment half4 gidPSTF(PVertexPS vert [[stage_in]],
                      constant UniformsPS &uniforms [[buffer(0)]],
                      texture2d<half> tex [[texture(0)]],
                      sampler smp [[sampler(0)]])
{
    if (vert.steprot.x==0.0) discard_fragment();
    if (vert.steprot.x<=0.0) return half4(vert.color);
    
    float2 rad=vert.texcoord+float2(-0.5,-0.5);
    if ((rad.x<-0.5)||(rad.y<-0.5)||(rad.x>0.5)||(rad.y>0.5)) discard_fragment();
    return vert.color*tex.sample(smp,vert.texcoord*uniforms.fTexInfo.xy);
}
