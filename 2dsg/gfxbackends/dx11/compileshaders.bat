fxc /T vs_4_0_level_9_3 /E VShader /Fo vBasic.cso Basic.hlsl
fxc /T ps_4_0_level_9_3 /E PShader /Fo pBasic.cso Basic.hlsl

fxc /T vs_4_0_level_9_3 /E VShader /Fo vColor.cso Color.hlsl
fxc /T ps_4_0_level_9_3 /E PShader /Fo pColor.cso Color.hlsl

fxc /T vs_4_0_level_9_3 /E VShader /Fo vTexture.cso Texture.hlsl
fxc /T ps_4_0_level_9_3 /E PShader /Fo pTexture.cso Texture.hlsl

fxc /T vs_4_0_level_9_3 /E VShader /Fo vTextureColor.cso TextureColor.hlsl
fxc /T ps_4_0_level_9_3 /E PShader /Fo pTextureColor.cso TextureColor.hlsl

fxc /T vs_4_0_level_9_3 /E VShader /Fo vParticle.cso Particle.hlsl
fxc /T ps_4_0_level_9_3 /E PShader /Fo pParticle.cso Particle.hlsl

bin2c -o dx11_shaders.c vBasic.cso pBasic.cso ^
vColor.cso pColor.cso ^
vTexture.cso pTexture.cso ^
vTextureColor.cso pTextureColor.cso ^
vParticle.cso pParticle.cso
