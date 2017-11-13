/*
 * gl2ShaderEngine.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "gl2Shaders.h"
#include "gtexture.h"
#include "glog.h"
#include "ogl.h"

#ifdef OPENGL_ES0
/* Vertex shader*/
const char *xformVShaderCode=
"attribute highp vec2 vTexCoord;\n"
"attribute highp vec4 vVertex;\n"
"attribute lowp vec4 vColor;\n"
"uniform highp mat4 vMatrix;\n"
"varying mediump vec2 fTexCoord;\n"
"varying lowp vec4 fInColor; "
"\n"
"void main() {\n"
"  gl_Position = vMatrix*vVertex;\n"
"  fTexCoord=vTexCoord;\n"
"  fInColor=vColor;\n"
"}\n";

/* Fragment shader*/
const char *colorFShaderCode="\
precision mediump float;\
uniform float fColorSel;\
uniform float fTextureSel;\
uniform lowp vec4 fColor;\
uniform lowp sampler2D fTexture;\
varying mediump vec2 fTexCoord;\
varying lowp vec4 fInColor;\
void main() {\
 lowp vec4 frag=mix(fColor,fInColor,fColorSel);\
 if (fTextureSel>0.0) \
  frag=frag*texture2D(fTexture, fTexCoord);\
 if (frag.a==0.0) discard;\
 gl_FragColor = frag;\
}";
#else
/* Vertex shader*/
static const char *hdrVShaderCode_DK =
        "#version 120\n"
                "#define highp\n"
                "#define mediump\n"
                "#define lowp\n"
        "attribute highp vec3 vVertex;\n";

static const char *hdrVShaderCode_ES =
        "#version 100\n"
        "#define GLES2\n"
        "attribute highp vec3 vVertex;\n";

static const char *hdrPSVShaderCode_DK =
        "#version 120\n"
                "#define highp\n"
                "#define mediump\n"
                "#define lowp\n"
        "attribute highp vec4 vVertex;\n";
static const char *hdrPSVShaderCode_ES =
        "#version 100\n"
        "#define GLES2\n"
        "attribute highp vec4 vVertex;\n";

static const char *stdVShaderCode = "uniform highp mat4 vMatrix;\n"
		"\n"
		"void main() {\n"
		"  vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"}\n";
static const char *stdCVShaderCode = "attribute lowp vec4 vColor;\n"
		"uniform lowp vec4 fColor;\n"
		"uniform highp mat4 vMatrix;\n"
		"varying lowp vec4 fInColor; "
		"\n"
		"void main() {\n"
		"  vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fInColor=vColor*fColor;\n"
		"}\n";
static const char *stdTVShaderCode = "attribute mediump vec2 vTexCoord;\n"
		"uniform highp mat4 vMatrix;\n"
		"varying mediump vec2 fTexCoord;\n"
		"\n"
		"void main() {\n"
		"  vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fTexCoord=vTexCoord;\n"
		"}\n";
static const char *stdCTVShaderCode = "attribute mediump vec2 vTexCoord;\n"
		"attribute lowp vec4 vColor;\n"
		"uniform highp mat4 vMatrix;\n"
		"uniform lowp vec4 fColor;\n"
		"varying mediump vec2 fTexCoord;\n"
		"varying lowp vec4 fInColor; "
		"\n"
		"void main() {\n"
		"  vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fTexCoord=vTexCoord;\n"
		"  fInColor=vColor*fColor;\n"
		"}\n";
static const char *stdPVShaderCode = "attribute lowp vec4 vColor;\n"
		"uniform highp mat4 vMatrix;\n"
		"uniform highp mat4 vWorldMatrix;\n"
		"uniform mediump float vPSize;\n"
		"varying lowp vec4 fInColor; "
		"\n"
		"void main() {\n"
		"  highp vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fInColor=vColor;\n"
		"  mediump vec4 xpsize=vWorldMatrix*vec4(vPSize,0.0,0.0,1.0);\n"
		"  gl_PointSize=length(xpsize.xyz);\n"
		"}\n";

static const char *stdPSVShaderCode = "attribute mediump vec2 vTexCoord;\n"
		"attribute lowp vec4 vColor;\n"
		"uniform highp mat4 vMatrix;\n"
		"uniform highp mat4 vWorldMatrix;\n"
		"varying lowp vec4 fInColor;\n"
		"varying mediump vec2 fStepRot;\n"
		"varying mediump vec2 fTexCoord;\n"
		"void main() {\n"
		"  mediump vec2 rad=(vec2(-0.5,-0.5)+vTexCoord)*vVertex.z;\n"
		"  mediump float angle=vVertex.w*3.141592654/180.0;\n"
		"  mediump float ca=cos(angle);\n"
		"  mediump float sa=sin(angle);\n"
		"  mediump mat2 rot=mat2(ca,sa,-sa,ca);\n"
		"  rad=rad*rot;\n"
		"  highp vec4 vertex = vec4(vVertex.xy+rad,0.0,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fInColor=vColor;\n"
		"  mediump vec4 xpsize=vWorldMatrix*vec4(vVertex.z,0.0,0.0,0.0);\n"
		"  highp float xpl=length(xpsize.xyz);\n"
		"  if (xpl==0.0) xpl=1.0;\n"
		"  fStepRot=vec2(sign(vVertex.z)/xpl,vVertex.w);\n"
		"  fTexCoord=vTexCoord;\n"
		"}\n";

/* Fragment shader*/
static const char *hdrFShaderCode_DK =
        "#version 120\n"
                "#define highp\n"
                "#define mediump\n"
                "#define lowp\n";
static const char *hdrFShaderCode_ES =
        "#version 100\n"
        "#define GLES2\n";

static const char *stdFShaderCode = "uniform lowp vec4 fColor;\n"
		"void main() {\n"
		" gl_FragColor = fColor;\n"
		"}\n";
static const char *stdCFShaderCode = "varying lowp vec4 fInColor;\n"
		"void main() {\n"
		" gl_FragColor = fInColor;\n"
		"}\n";
static const char *stdTFShaderCode = "uniform lowp vec4 fColor;\n"
		"uniform lowp sampler2D fTexture;\n"
		"varying mediump vec2 fTexCoord;\n"
		"void main() {\n"
		" lowp vec4 frag=fColor*texture2D(fTexture, fTexCoord);\n"
		" if (frag.a==0.0) discard;\n"
		" gl_FragColor = frag;\n"
		"}\n";
static const char *stdTAFShaderCode = "uniform lowp vec4 fColor;\n"
		"uniform lowp sampler2D fTexture;\n"
		"varying mediump vec2 fTexCoord;\n"
		"void main() {\n"
		" lowp vec4 frag=texture2D(fTexture, fTexCoord);\n"
		" frag=fColor*frag.aaaa;\n"
		" if (frag.a==0.0) discard;\n"
		" gl_FragColor = frag;\n"
		"}\n";
static const char *stdCTFShaderCode = "varying lowp vec4 fInColor;\n"
		"uniform lowp sampler2D fTexture;\n"
		"varying mediump vec2 fTexCoord;\n"
		"void main() {\n"
		" lowp vec4 frag=fInColor*texture2D(fTexture, fTexCoord);\n"
		" if (frag.a==0.0) discard;\n"
		" gl_FragColor = frag;\n"
		"}\n";
static const char *stdCTAFShaderCode = "varying lowp vec4 fInColor;\n"
		"uniform lowp sampler2D fTexture;\n"
		"varying mediump vec2 fTexCoord;\n"
		"void main() {\n"
		" lowp vec4 frag=texture2D(fTexture, fTexCoord);\n"
		" frag=fInColor*frag.aaaa;\n"
		" if (frag.a==0.0) discard;\n"
		" gl_FragColor = frag;\n"
		"}\n";
static const char *stdPFShaderCode =
		"varying lowp vec4 fInColor;\n"
				"uniform lowp sampler2D fTexture;\n"
				"uniform mediump vec4 fTexInfo;\n"
				"void main() {\n"
				" if (fTexInfo.x<=0.0)\n"
				" {\n"
				"  lowp vec4 frag;\n"
				"  mediump vec2 rad=vec2(-0.5,-0.5)+gl_PointCoord;\n"
				"  frag=fInColor;\n"
				"  lowp float alpha=1.0-step(0.5,length(rad));\n"
				"  frag*=alpha;\n"
				"  gl_FragColor=frag;\n"
				" }\n"
				" else\n"
				"  gl_FragColor=fInColor*texture2D(fTexture, gl_PointCoord*fTexInfo.xy);\n"
				"}\n";

static const char *stdPSFShaderCode =
		"varying lowp vec4 fInColor;\n"
		"varying mediump vec2 fStepRot;\n"
		"varying mediump vec2 fTexCoord;\n"
		"uniform lowp sampler2D fTexture;\n"
		"uniform mediump vec4 fTexInfo;\n"
		"void main() {\n"
		" if (fStepRot.x==0.0) discard;\n"
		" if (fStepRot.x<0.0)\n"
	"		 gl_FragColor=fInColor;\n"
	"	 else\n"
	"	 {\n"
	"	 mediump vec2 rad=vec2(-0.5,-0.5)+fTexCoord;\n"
	"	 if (fTexInfo.x<=0.0)\n"
	"	 {\n"
	"	  lowp vec4 frag;\n"
	"	  frag=fInColor;\n"
	"	  lowp float alpha=1.0-smoothstep(0.5-fStepRot.x,0.5+fStepRot.x,length(rad));\n"
	"	  frag*=alpha;\n"
	"	  gl_FragColor=frag;\n"
	"	 }\n"
	"	 else\n"
	"	 {\n"
	"	  if ((rad.x<-0.5)||(rad.y<-0.5)||(rad.x>0.5)||(rad.y>0.5))\n"
	"		  discard;\n"
	"	  gl_FragColor=fInColor*texture2D(fTexture, (rad+vec2(0.5,0.5))*fTexInfo.xy);\n"
	"	 }\n"
	"	 }\n"
				"}\n";
#endif

const char *ogl2ShaderEngine::getVersion() {
    return isGLES?"GLES2":"GL2";
}

void ogl2ShaderEngine::resizeFramebuffer(int width,int height)
{
	GLCALL_INIT;
    /*
    int fw=width,fh=height,crb=0;
    //XXX width and height may not match the framebuffer (reversed), get them from the current fb
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&crb);
    if (crb) {
        glBindRenderbuffer(GL_RENDERBUFFER, crb);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fw);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fh);
    }
    glog_i("FrameBuffer:(%d) %d,%d Real(%d,%d)",crb,width,height,fw,fh);*/
	devWidth = width;
	devHeight = height;
	int depthfmt = 0;
#ifdef GL_DEPTH24_STENCIL8_OES
	depthfmt=GL_DEPTH24_STENCIL8_OES;
#else
	depthfmt = GL_DEPTH24_STENCIL8;
#endif
#ifndef QT_CORE_LIB
#ifdef OPENGL_ES
    GLCALL glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
	GLCALL glRenderbufferStorage(GL_RENDERBUFFER, depthfmt, devWidth,devHeight);
	GLCALL glBindRenderbuffer(GL_RENDERBUFFER, 0);
    GLCALL glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
	GLCALL glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
#endif
#endif
}


void ogl2ShaderEngine::reset(bool reinit) {
	GLCALL_INIT;
	if (reinit) {
		s_texture = 0;
		s_depthEnable = 0;
		s_depthBufferCleared = false;

		currentBuffer = NULL;
		ogl2ShaderProgram::current = NULL;
		ogl2ShaderProgram::curProg=0;

		int depthfmt = 0;
#ifdef GL_DEPTH24_STENCIL8_OES
		depthfmt=GL_DEPTH24_STENCIL8_OES;
#else
		depthfmt = GL_DEPTH24_STENCIL8;
#endif

#ifndef QT_CORE_LIB
#ifdef OPENGL_ES
		if (!GLCALL glIsRenderbuffer(_depthRenderBuffer))
		{
			GLCALL glGenRenderbuffers(1, &_depthRenderBuffer);
			GLCALL glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
			GLCALL glRenderbufferStorage(GL_RENDERBUFFER, depthfmt, devWidth,devHeight);
			GLCALL glBindRenderbuffer(GL_RENDERBUFFER, 0);
			GLCALL glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
			GLCALL glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
		}
#endif
#endif
#ifdef GL_POINT_SPRITE_OES
		GLCALL glEnable(GL_POINT_SPRITE_OES);
#else
#ifdef GL_POINT_SPRITE
		GLCALL glEnable(GL_POINT_SPRITE);
#endif
#endif
#ifdef GL_VERTEX_PROGRAM_POINT_SIZE
		GLCALL glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
		ogl2ShaderProgram::resetAll();
	}
   /* glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);*/

	ShaderEngine::reset(reinit);
	ogl2ShaderProgram::resetAllUniforms();
	s_texture = 0;
	s_depthEnable = 0;
	s_depthBufferCleared = false;

#ifdef GIDEROS_GL1
	GLCALL glDisableClientState(GL_VERTEX_ARRAY);
	GLCALL glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	GLCALL glDisableClientState(GL_COLOR_ARRAY);
	GLCALL glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); /* sanity set */
#endif

#ifdef GIDEROS_GL1
	GLCALL glDisable(GL_TEXTURE_2D);
#endif

	GLCALL glBindTexture(GL_TEXTURE_2D, 0);
	//GLCALL glClearColor(0.5, 0.1, 0.2, 1.f);
	//GLCALL glClear(GL_COLOR_BUFFER_BIT);

	GLCALL glEnable(GL_BLEND);
	GLCALL glDisable(GL_SCISSOR_TEST);
	GLCALL glDepthFunc(GL_LEQUAL);

#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
	GLCALL glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
	GLCALL glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
}

extern void pathShadersInit(bool isGLES);
extern void pathShadersRelease();

void ogl2SetupShaders(bool isGLES) {
	GLCALL_INIT;
	glog_i("GL_VERSION:%s\n", GLCALL glGetString(GL_VERSION));
	glog_i("GLSL_VERSION:%s\n", GLCALL glGetString(GL_SHADING_LANGUAGE_VERSION));

	const ShaderProgram::ConstantDesc stdUniforms[] = { { "vMatrix",
			ShaderProgram::CMATRIX, 1,
			ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
			{ "fColor", ShaderProgram::CFLOAT4, 1,
					ShaderProgram::SysConst_Color, false, 0, NULL }, {
					"fTexture", ShaderProgram::CTEXTURE, 1,
					ShaderProgram::SysConst_None, false, 0, NULL }, { "",
					ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,
					false, 0, NULL } };
	const ShaderProgram::DataDesc stdAttributes[] = { { "vVertex",
			ShaderProgram::DFLOAT, 3, 0, 0 }, { "vColor", ShaderProgram::DUBYTE,
			4, 1, 0 }, { "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0 }, { "",
			ShaderProgram::DFLOAT, 0, 0, 0 } };
    const char *hdrVShaderCode=isGLES?hdrVShaderCode_ES:hdrVShaderCode_DK;
    const char *hdrPSVShaderCode=isGLES?hdrPSVShaderCode_ES:hdrPSVShaderCode_DK;
    const char *hdrFShaderCode=isGLES?hdrFShaderCode_ES:hdrFShaderCode_DK;
    ShaderProgram::stdBasic = new ogl2ShaderProgram(hdrVShaderCode,
			stdVShaderCode, hdrFShaderCode, stdFShaderCode, stdUniforms,
			stdAttributes);
	ShaderProgram::stdColor = new ogl2ShaderProgram(hdrVShaderCode,
			stdCVShaderCode, hdrFShaderCode, stdCFShaderCode, stdUniforms,
			stdAttributes);
	ShaderProgram::stdTexture = new ogl2ShaderProgram(hdrVShaderCode,
			stdTVShaderCode, hdrFShaderCode, stdTFShaderCode, stdUniforms,
			stdAttributes);
	ShaderProgram::stdTextureAlpha = new ogl2ShaderProgram(hdrVShaderCode,
			stdTVShaderCode, hdrFShaderCode, stdTAFShaderCode, stdUniforms,
			stdAttributes);
	ShaderProgram::stdTextureColor = new ogl2ShaderProgram(hdrVShaderCode,
			stdCTVShaderCode, hdrFShaderCode, stdCTFShaderCode, stdUniforms,
			stdAttributes);
	ShaderProgram::stdTextureAlphaColor = new ogl2ShaderProgram(hdrVShaderCode,
			stdCTVShaderCode, hdrFShaderCode, stdCTAFShaderCode, stdUniforms,
			stdAttributes);
	const ShaderProgram::ConstantDesc stdPUniforms[] = { { "vMatrix",
			ShaderProgram::CMATRIX, 1,
			ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
			{ "vWorldMatrix", ShaderProgram::CMATRIX, 1,
					ShaderProgram::SysConst_WorldMatrix, true, 0, NULL }, {
					"vPSize", ShaderProgram::CFLOAT, 1,
					ShaderProgram::SysConst_ParticleSize, true, 0, NULL }, {
					"fTexture", ShaderProgram::CTEXTURE, 1,
					ShaderProgram::SysConst_None, false, 0, NULL }, {
					"fTexInfo", ShaderProgram::CFLOAT4, 1,
					ShaderProgram::SysConst_TextureInfo, false, 0, NULL }, { "",
					ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,
					false, 0, NULL } };
	ShaderProgram::stdParticle = new ogl2ShaderProgram(hdrVShaderCode,
			stdPVShaderCode, hdrFShaderCode, stdPFShaderCode, stdPUniforms,
			stdAttributes);

	const ShaderProgram::ConstantDesc stdPSConstants[] = {
		{ "vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0,NULL },
		{ "vWorldMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldMatrix,true,0,NULL },
		{ "fTexture",ShaderProgram::CTEXTURE,1,ShaderProgram::SysConst_None,false,0,NULL },
		{ "fTexInfo",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_TextureInfo,false,0,NULL },
		{ "",ShaderProgram::CFLOAT,0,ShaderProgram::SysConst_None,false,0,NULL }
	};
	const ShaderProgram::DataDesc stdPSAttributes[] = {
		{ "vVertex", ShaderProgram::DFLOAT, 4, 0, 0 },
		{ "vColor", ShaderProgram::DUBYTE, 4, 1, 0 },
		{ "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0 },
		{ "",ShaderProgram::DFLOAT,0,0,0 }
	};

	ShaderProgram::stdParticles = new ogl2ShaderProgram(
			hdrPSVShaderCode,	stdPSVShaderCode, hdrFShaderCode, stdPSFShaderCode, stdPSConstants, stdPSAttributes);

}

ShaderProgram *ogl2ShaderEngine::createShaderProgram(const char *vshader,
		const char *pshader, int flags,
		const ShaderProgram::ConstantDesc *uniforms,
		const ShaderProgram::DataDesc *attributes) {
	return new ogl2ShaderProgram(vshader, pshader, flags, uniforms, attributes, isGLES);
}

ogl2ShaderEngine::ogl2ShaderEngine(int sw, int sh) {
    
    /*int fw=sw,fh=sh,crb=0;
    //XXX width and height may not match the framebuffer (reversed), get them from the current fb
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&crb);
    if (crb) {
        glBindRenderbuffer(GL_RENDERBUFFER, crb);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fw);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fh);
    }*/

	devWidth = sw;
	devHeight = sh;
	_depthRenderBuffer = 0;
    defaultFramebuffer=0;
#ifdef OPENGL_ES
    isGLES=true;
#else
    isGLES=false;
#endif
#ifdef QT_CORE_LIB
	GLCALL_INIT;
    isGLES=QOpenGLContext::currentContext()->isOpenGLES();
	GLCALL glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);
#endif

#ifndef GIDEROS_GL1
    ogl2SetupShaders(isGLES);
    pathShadersInit(isGLES);
#endif

	reset(true);
}

ogl2ShaderEngine::~ogl2ShaderEngine() {
	GLCALL_INIT;
	if (currentBuffer)
		setFramebuffer(NULL);

	delete ShaderProgram::stdBasic;
	delete ShaderProgram::stdColor;
	delete ShaderProgram::stdTexture;
	delete ShaderProgram::stdTextureAlpha;
	delete ShaderProgram::stdTextureColor;
	delete ShaderProgram::stdParticle;
	pathShadersRelease();
#ifndef QT_CORE_LIB
#ifdef OPENGL_ES
	GLCALL glDeleteRenderbuffers(1,&_depthRenderBuffer);
#endif
#endif
}

ShaderTexture *ogl2ShaderEngine::createTexture(ShaderTexture::Format format,
		ShaderTexture::Packing packing, int width, int height, const void *data,
		ShaderTexture::Wrap wrap, ShaderTexture::Filtering filtering) {
	return new ogl2ShaderTexture(format, packing, width, height, data, wrap,
			filtering);
}

ShaderBuffer *ogl2ShaderEngine::createRenderTarget(ShaderTexture *texture) {
	return new ogl2ShaderBuffer(texture);
}

ShaderBuffer *ogl2ShaderEngine::setFramebuffer(ShaderBuffer *fbo) {
	GLCALL_INIT;
	ShaderBuffer *previous = currentBuffer;
	GLint oldFBO = 0;
	GLCALL glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
#ifdef OPENGL_DESKTOP
	if (GLEW_ARB_framebuffer_object)
#endif
		GLCALL glBindFramebuffer(GL_FRAMEBUFFER,
				fbo ? ((ogl2ShaderBuffer *) fbo)->glid : defaultFramebuffer);
#ifdef OPENGL_DESKTOP
	else
		GLCALL glBindFramebufferEXT(GL_FRAMEBUFFER,
				fbo ? ((ogl2ShaderBuffer *) fbo)->glid : defaultFramebuffer);
#endif
	if (previous)
		previous->unbound();
	currentBuffer = fbo;
	return previous;

}

void ogl2ShaderEngine::setViewport(int x, int y, int width, int height) {
	GLCALL_INIT;
	GLCALL glViewport(x, y, width, height);
}

void ogl2ShaderEngine::setModel(const Matrix4 m) {
	ShaderEngine::setModel(m);
#ifdef GIDEROS_GL1
	GLCALL glMatrixMode(GL_MODELVIEW);
	GLCALL glLoadMatrixf(m.data());
#endif
}

void ogl2ShaderEngine::setProjection(const Matrix4 p) {
	ShaderEngine::setProjection(p);
#ifdef GIDEROS_GL1
	GLCALL glMatrixMode(GL_PROJECTION);
	GLCALL glLoadMatrixf(p.data());
#endif
}

void ogl2ShaderEngine::adjustViewportProjection(Matrix4 &vp, float width, float height) {
	vp.scale(1, -1, 1);
	vp.translate(0, height, 0);
}

static GLint stencilopToGl(ShaderEngine::StencilOp sf)
{
	switch (sf)
	{
	case ShaderEngine::STENCIL_KEEP: return GL_KEEP;
	case ShaderEngine::STENCIL_ZERO: return GL_ZERO;
	case ShaderEngine::STENCIL_REPLACE: return GL_REPLACE;
	case ShaderEngine::STENCIL_INCR: return GL_INCR;
	case ShaderEngine::STENCIL_INCR_WRAP: return GL_INCR_WRAP;
	case ShaderEngine::STENCIL_DECR: return GL_DECR;
	case ShaderEngine::STENCIL_DECR_WRAP: return GL_DECR_WRAP;
	case ShaderEngine::STENCIL_INVERT: return GL_INVERT;
	}
	return GL_KEEP;
}

void ogl2ShaderEngine::setDepthStencil(DepthStencil state)
{
	GLCALL_INIT;
	if (state.dClear)
	{
		state.dClear=false;
		s_depthBufferCleared=false;
	}
	if (state.dTest) {
		if (!s_depthEnable) {
			if (currentBuffer)
				currentBuffer->needDepthStencil();
			if ((!s_depthBufferCleared)||(state.dClear)) {
	#ifdef OPENGL_ES
				GLCALL glClearDepthf(1);
	#endif
				GLCALL glClear(GL_DEPTH_BUFFER_BIT);
				s_depthBufferCleared = true;
    			state.dClear=false;
			}
			s_depthEnable=true;
			GLCALL glEnable(GL_DEPTH_TEST);
		}
	} else {
		if (s_depthEnable)
		{
			GLCALL glDisable(GL_DEPTH_TEST);
			s_depthEnable=false;
		}
	}
	if (state.sClear)
	{
		if (currentBuffer)
			currentBuffer->needDepthStencil();
		GLCALL glClear(GL_STENCIL_BUFFER_BIT);
		state.sClear=false;
	}
	GLCALL glStencilOp(stencilopToGl(state.sFail),stencilopToGl(state.dFail),stencilopToGl(state.dPass));
	if (state.sFunc==STENCIL_DISABLE)
		GLCALL glDisable(GL_STENCIL_TEST);
	else
	{
		GLCALL glEnable(GL_STENCIL_TEST);
		GLenum sf=GL_ALWAYS;
		switch (state.sFunc)
		{
			case STENCIL_NEVER: sf=GL_NEVER; break;
			case STENCIL_LESS: sf=GL_LESS; break;
			case STENCIL_LEQUAL: sf=GL_LEQUAL; break;
			case STENCIL_GREATER: sf=GL_GREATER; break;
			case STENCIL_GEQUAL: sf=GL_GEQUAL; break;
			case STENCIL_EQUAL: sf=GL_EQUAL; break;
			case STENCIL_NOTEQUAL: sf=GL_NOTEQUAL; break;
			case STENCIL_ALWAYS:
			case STENCIL_DISABLE:
				break;
		}
		GLCALL glStencilFunc(sf,state.sRef,state.sMask);
		GLCALL glStencilMask(state.sWMask);
	}
	dsCurrent=state;
}



void ogl2ShaderEngine::clearColor(float r, float g, float b, float a) {
	GLCALL_INIT;
	GLCALL glClearColor(r * a, g * a, b * a, a);
	GLCALL glClear(GL_COLOR_BUFFER_BIT);
}

void ogl2ShaderEngine::bindTexture(int num, ShaderTexture *texture) {
	GLCALL_INIT;
	GLCALL glActiveTexture(GL_TEXTURE0 + num);
	GLCALL glBindTexture(GL_TEXTURE_2D, ((ogl2ShaderTexture *) texture)->glid);
}

void ogl2ShaderEngine::setClip(int x, int y, int w, int h) {
	GLCALL_INIT;
	if ((w < 0) || (h < 0))
		GLCALL glDisable(GL_SCISSOR_TEST);
	else {
		GLCALL glEnable(GL_SCISSOR_TEST);
		GLCALL glScissor(x, y, w, h);
	}
}

GLenum ogl2ShaderEngine::blendFactor2GLenum(BlendFactor blendFactor) {
	switch (blendFactor) {
	case ZERO:
		return GL_ZERO;
	case ONE:
		return GL_ONE;
	case SRC_COLOR:
		return GL_SRC_COLOR;
	case ONE_MINUS_SRC_COLOR:
		return GL_ONE_MINUS_SRC_COLOR;
	case DST_COLOR:
		return GL_DST_COLOR;
	case ONE_MINUS_DST_COLOR:
		return GL_ONE_MINUS_DST_COLOR;
	case SRC_ALPHA:
		return GL_SRC_ALPHA;
	case ONE_MINUS_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case DST_ALPHA:
		return GL_DST_ALPHA;
	case ONE_MINUS_DST_ALPHA:
		return GL_ONE_MINUS_DST_ALPHA;
		//case CONSTANT_COLOR:
		//   return GL_CONSTANT_COLOR;
		//case ONE_MINUS_CONSTANT_COLOR:
		//   return GL_ONE_MINUS_CONSTANT_COLOR;
		//case CONSTANT_ALPHA:
		//   return GL_CONSTANT_ALPHA;
		//case ONE_MINUS_CONSTANT_ALPHA:
		//   return GL_ONE_MINUS_CONSTANT_ALPHA;
	case SRC_ALPHA_SATURATE:
		return GL_SRC_ALPHA_SATURATE;
	default:
		break;
	}

	return GL_ZERO;
}

void ogl2ShaderEngine::setBlendFunc(BlendFactor sfactor, BlendFactor dfactor) {
	GLCALL_INIT;
	GLCALL glBlendFunc(blendFactor2GLenum(sfactor), blendFactor2GLenum(dfactor));
}

