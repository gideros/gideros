--Chroma
local ChromaVShader=[[

attribute vec4 POSITION0;
attribute vec2 TEXCOORD0;

uniform mat4 g_MVPMatrix;

varying mediump vec2 texCoord;

void main()
{
	gl_Position = g_MVPMatrix * POSITION0;
	texCoord = TEXCOORD0;
}]]


local ChromaFShader=[[
uniform lowp sampler2D gU;
uniform lowp sampler2D gV;
uniform lowp sampler2D gTexture;
varying mediump vec2 texCoord;

#ifdef GLES2
precision mediump float;
#endif

void main()
{
	lowp vec4 yb = texture2D(gTexture, texCoord);	
	lowp vec4 ub = texture2D(gU, texCoord);	
	lowp vec4 vb = texture2D(gV, texCoord);	
	lowp float B = 1.164*(yb.r - 0.125/2.0)                   + 2.018*(ub.r - 0.5);
    lowp float G = 1.164*(yb.r - 0.125/2.0) - 0.813*(vb.r - 0.5) - 0.391*(ub.r - 0.5);
    lowp float R = 1.164*(yb.r - 0.125/2.0) + 1.596*(vb.r - 0.5);

	gl_FragColor=vec4(R,G,B,1.0);
}]]


local ChromaShaderAttrs=
{
{name="POSITION0",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="TEXCOORD0",type=Shader.DFLOAT,mult=2,slot=2,offset=0}
}

local ChromaShaderConstants={
{name="g_MVPMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP, vertex=true},
{name="gTexture",type=Shader.CTEXTURE,mult=1,vertex=false},
{name="gU",type=Shader.CTEXTURE,mult=1,vertex=false},
{name="gV",type=Shader.CTEXTURE,mult=1,vertex=false},
}


local ChromaShader= Shader.new(
ChromaVShader,ChromaFShader,
Shader.FLAG_FROM_CODE,ChromaShaderConstants,ChromaShaderAttrs) 

local th=require "theora.core"
local Theora=Core.class(Sound)
Theora.__play=Theora.play

function Theora:play(startTime,looping,paused)
	local c=self:__play(startTime,looping,paused)
	c.getVideo=Theora._getVideo
	local sid=c:getStreamId()
	local thi=th.getVideoInfo(sid)
	local yp=Texture.new(nil,thi.width,thi.height, true, { extend=false})
	local up=Texture.new(nil,thi.width/2,thi.height/2, true, { extend=false})
	local vp=Texture.new(nil,thi.width/2,thi.height/2, true, { extend=false})
	th.setVideoSurface(sid,yp,up,vp)
	c.__thi=thi
	c.__yp=yp
	c.__up=up
	c.__vp=vp
	return c
end

function Theora:_getVideo(width,height)
	local sid=self:getStreamId()
	local thi=self.__thi
	local camview=Mesh.new()
	local scrw=width or thi.width
	local scrh=height or thi.height
	camview:setVertexArray(0,0,scrw,0,scrw,scrh,0,scrh)
	camview:setTextureCoordinateArray(0,0,thi.width,0,thi.width,thi.height,0,thi.height)
	camview:setIndexArray(1,2,3,1,3,4)
	camview:setTexture(self.__yp)
	camview:setTexture(self.__up,1)
	camview:setTexture(self.__vp,2)
	camview:setShader(ChromaShader)
	return camview
end

return Theora
