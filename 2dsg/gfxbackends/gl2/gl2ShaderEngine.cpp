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
const char *hdrVShaderCode =
#ifdef OPENGL_ES
		"#version 100\n"
		"#define GLES2\n"
#else
		"#version 120\n"
				"#define highp\n"
				"#define mediump\n"
				"#define lowp\n"
#endif
		"attribute highp vec3 vVertex;\n";

const char *stdVShaderCode = "uniform highp mat4 vMatrix;\n"
		"\n"
		"void main() {\n"
		"  vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"}\n";
const char *stdCVShaderCode = "attribute lowp vec4 vColor;\n"
		"uniform highp mat4 vMatrix;\n"
		"varying lowp vec4 fInColor; "
		"\n"
		"void main() {\n"
		"  vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fInColor=vColor;\n"
		"}\n";
const char *stdTVShaderCode = "attribute mediump vec2 vTexCoord;\n"
		"uniform highp mat4 vMatrix;\n"
		"varying mediump vec2 fTexCoord;\n"
		"\n"
		"void main() {\n"
		"  vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fTexCoord=vTexCoord;\n"
		"}\n";
const char *stdCTVShaderCode = "attribute mediump vec2 vTexCoord;\n"
		"attribute lowp vec4 vColor;\n"
		"uniform highp mat4 vMatrix;\n"
		"varying mediump vec2 fTexCoord;\n"
		"varying lowp vec4 fInColor; "
		"\n"
		"void main() {\n"
		"  vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fTexCoord=vTexCoord;\n"
		"  fInColor=vColor;\n"
		"}\n";
const char *stdPVShaderCode = "attribute lowp vec4 vColor;\n"
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

/* Fragment shader*/
const char *hdrFShaderCode =
#ifdef OPENGL_ES
		"#version 100\n"
		"#define GLES2\n";
#else
		"#version 120\n"
				"#define highp\n"
				"#define mediump\n"
				"#define lowp\n";
#endif

const char *stdFShaderCode = "uniform lowp vec4 fColor;\n"
		"void main() {\n"
		" gl_FragColor = fColor;\n"
		"}\n";
const char *stdCFShaderCode = "varying lowp vec4 fInColor;\n"
		"void main() {\n"
		" gl_FragColor = fInColor;\n"
		"}\n";
const char *stdTFShaderCode = "uniform lowp vec4 fColor;\n"
		"uniform lowp sampler2D fTexture;\n"
		"varying mediump vec2 fTexCoord;\n"
		"void main() {\n"
		" lowp vec4 frag=fColor*texture2D(fTexture, fTexCoord);\n"
		" if (frag.a==0.0) discard;\n"
		" gl_FragColor = frag;\n"
		"}\n";
const char *stdCTFShaderCode = "varying lowp vec4 fInColor;\n"
		"uniform lowp sampler2D fTexture;\n"
		"varying mediump vec2 fTexCoord;\n"
		"void main() {\n"
		" lowp vec4 frag=fInColor*texture2D(fTexture, fTexCoord);\n"
		" if (frag.a==0.0) discard;\n"
		" gl_FragColor = frag;\n"
		"}\n";
const char *stdPFShaderCode =
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
#endif

const char *ogl2ShaderEngine::getVersion() {
#ifdef OPENGL_ES
	return "GLES2";
#else
	return "GL2";
#endif
}

void ogl2ShaderEngine::resizeFramebuffer(int width,int height)
{
	devWidth = width;
	devHeight = height;
	int depthfmt = 0;
#ifdef GL_DEPTH24_STENCIL8_OES
	depthfmt=GL_DEPTH24_STENCIL8_OES;
#else
	depthfmt = GL_DEPTH24_STENCIL8;
#endif

#ifdef OPENGL_ES
	glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, depthfmt, devWidth,devHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
#endif
}


void ogl2ShaderEngine::reset(bool reinit) {
	if (reinit) {
		s_texture = 0;
		s_depthEnable = 0;
		s_depthBufferCleared = false;

		currentBuffer = NULL;
		ogl2ShaderProgram::current = NULL;

		int depthfmt = 0;
#ifdef GL_DEPTH24_STENCIL8_OES
		depthfmt=GL_DEPTH24_STENCIL8_OES;
#else
		depthfmt = GL_DEPTH24_STENCIL8;
#endif

#ifdef OPENGL_ES
		if (!glIsRenderbuffer(_depthRenderBuffer))
		{
			glGenRenderbuffers(1, &_depthRenderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, depthfmt, devWidth,devHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
		}
#endif
#ifdef GL_POINT_SPRITE_OES
		glEnable(GL_POINT_SPRITE_OES);
#else
#ifdef GL_POINT_SPRITE
		glEnable(GL_POINT_SPRITE);
#endif
#endif
#ifdef GL_VERTEX_PROGRAM_POINT_SIZE
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
		ogl2ShaderProgram::resetAll();
	}
	ShaderEngine::reset(reinit);
	s_texture = 0;
	s_depthEnable = 0;
	s_depthBufferCleared = false;

#ifdef GIDEROS_GL1
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); /* sanity set */
#endif

#ifdef GIDEROS_GL1
	glDisable(GL_TEXTURE_2D);
#endif

	glBindTexture(GL_TEXTURE_2D, 0);
	//glClearColor(0.5, 0.1, 0.2, 1.f);
	//glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glDepthFunc(GL_LEQUAL);

#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
}

void ogl2SetupShaders() {
	glog_i("GL_VERSION:%s\n", glGetString(GL_VERSION));
	glog_i("GLSL_VERSION:%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

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
	ShaderProgram::stdBasic = new ogl2ShaderProgram(hdrVShaderCode,
			stdVShaderCode, hdrFShaderCode, stdFShaderCode, stdUniforms,
			stdAttributes);
	ShaderProgram::stdColor = new ogl2ShaderProgram(hdrVShaderCode,
			stdCVShaderCode, hdrFShaderCode, stdCFShaderCode, stdUniforms,
			stdAttributes);
	ShaderProgram::stdTexture = new ogl2ShaderProgram(hdrVShaderCode,
			stdTVShaderCode, hdrFShaderCode, stdTFShaderCode, stdUniforms,
			stdAttributes);
	ShaderProgram::stdTextureColor = new ogl2ShaderProgram(hdrVShaderCode,
			stdCTVShaderCode, hdrFShaderCode, stdCTFShaderCode, stdUniforms,
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
}

ShaderProgram *ogl2ShaderEngine::createShaderProgram(const char *vshader,
		const char *pshader, int flags,
		const ShaderProgram::ConstantDesc *uniforms,
		const ShaderProgram::DataDesc *attributes) {
	return new ogl2ShaderProgram(vshader, pshader, flags, uniforms, attributes);
}

ogl2ShaderEngine::ogl2ShaderEngine(int sw, int sh) {
	devWidth = sw;
	devHeight = sh;
	_depthRenderBuffer = 0;

#ifndef GIDEROS_GL1
	ogl2SetupShaders();
#endif

	reset(true);
}

ogl2ShaderEngine::~ogl2ShaderEngine() {
	if (currentBuffer)
		setFramebuffer(NULL);

	delete ShaderProgram::stdBasic;
	delete ShaderProgram::stdColor;
	delete ShaderProgram::stdTexture;
	delete ShaderProgram::stdTextureColor;
	delete ShaderProgram::stdParticle;
#ifdef OPENGL_ES
	glDeleteRenderbuffers(1,&_depthRenderBuffer);
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
	ShaderBuffer *previous = currentBuffer;
	GLint oldFBO = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
#ifdef OPENGL_DESKTOP
	if (GLEW_ARB_framebuffer_object)
#endif
		glBindFramebuffer(GL_FRAMEBUFFER,
				fbo ? ((ogl2ShaderBuffer *) fbo)->glid : 0);
#ifdef OPENGL_DESKTOP
	else
		glBindFramebufferEXT(GL_FRAMEBUFFER,
				fbo ? ((ogl2ShaderBuffer *) fbo)->glid : 0);
#endif
	currentBuffer = fbo;
	return previous;

}

void ogl2ShaderEngine::setViewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
}

void ogl2ShaderEngine::setModel(const Matrix4 m) {
	ShaderEngine::setModel(m);
#ifdef GIDEROS_GL1
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m.data());
#endif
}

void ogl2ShaderEngine::setProjection(const Matrix4 p) {
	ShaderEngine::setProjection(p);
#ifdef GIDEROS_GL1
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(p.data());
#endif
}

void ogl2ShaderEngine::clearColor(float r, float g, float b, float a) {
	glClearColor(r * a, g * a, b * a, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void ogl2ShaderEngine::bindTexture(int num, ShaderTexture *texture) {
	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_2D, ((ogl2ShaderTexture *) texture)->glid);
}

void ogl2ShaderEngine::setClip(int x, int y, int w, int h) {
	if ((w < 0) || (h < 0))
		glDisable(GL_SCISSOR_TEST);
	else {
		glEnable(GL_SCISSOR_TEST);
		glScissor(x, y, w, h);
	}
}

void ogl2ShaderEngine::setDepthTest(bool enable) {
	if (enable) {
		if (!(s_depthEnable++)) {
			if (!s_depthBufferCleared) {
#ifdef OPENGL_ES
				glClearDepthf(1);
#endif
				glClear(GL_DEPTH_BUFFER_BIT);
				s_depthBufferCleared = true;
			}
			glEnable(GL_DEPTH_TEST);
		}
	} else {
		if (!(--s_depthEnable))
			glDisable(GL_DEPTH_TEST);
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
	}

	return GL_ZERO;
}

void ogl2ShaderEngine::setBlendFunc(BlendFactor sfactor, BlendFactor dfactor) {
	glBlendFunc(blendFactor2GLenum(sfactor), blendFactor2GLenum(dfactor));
}

