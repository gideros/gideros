/*
 * gl2PathShaders.cpp
 *
 *  Created on: 1 sept. 2015
 *      Author: Nico
 */

#include "gl2Shaders.h"

static const char *hdrShaderCode =
#ifdef OPENGL_ES
		"#version 100\n"
		"#define GLES2\n";
#else
		"#version 120\n"
				"#define highp\n"
				"#define mediump\n"
				"#define lowp\n";
#endif

static const char *vs_code1 =
"uniform highp mat4 mvp;                                                      \n"
"                                                                       \n"
"attribute highp vec4 data0;                                                  \n"
"                                                                       \n"
"varying mediump vec2 uv;                                                       \n"
"                                                                       \n"
"void main(void)                                                        \n"
"{                                                                      \n"
"  gl_Position = mvp * vec4(data0.xy, 0, 1);   \n"
"  uv = data0.zw;                                                          \n"
"}                                                                      \n";

static const char *fs_code1 =
"uniform lowp vec4 fColor;\n"
"varying mediump vec2 uv;                           \n"
"                                           \n"
"void main(void)                            \n"
"{                                          \n"
"		// Gradients  						\n"
"		mediump vec2 px = dFdx(uv);				\n"
"		mediump vec2 py = dFdy(uv);				\n"
"		// Chain rule  						\n"
"		mediump float fx = (2.0f * uv.x)*px.x - px.y;	\n"
"		mediump float fy = (2.0f * uv.x)*py.x - py.y;	\n"
"		// Signed distance  				\n"
"		mediump float sd = (uv.x*uv.x - uv.y) / sqrt(fx*fx + fy*fy);	\n"
"		// Linear alpha  					\n"
"		mediump float alpha = 0.5f - sd;				\n"
"		alpha = min(alpha, 1.0f);			\n"
"		lowp vec4 frag = fColor;				\n"
"		 if (alpha <= 0.0f)  // Outside  		\n"
"			discard;						\n"
"		frag*= alpha;						\n"
"  gl_FragColor = frag;				         \n"
"}                                          \n";

static const char *vs_code2 =
"uniform highp mat4 mvp;                                                   \n"
"uniform mediump float width;                                                      \n"
"                                                                       \n"
"attribute highp vec4 data0;                                                  \n"
"attribute highp vec4 data1;                                                  \n"
"attribute highp vec4 data2;                                                  \n"
"                                                                       \n"
"varying highp vec2 pos;                                                      \n"
"varying highp float p, q;                                                    \n"
"varying highp vec2 a, b, c;                                                  \n"
"varying mediump float offset, strokeWidth;                                     \n"
"                                                                       \n"
"void main()                                                            \n"
"{                                                                      \n"
"  gl_Position = mvp * vec4(data0.xy, 0, 1);   \n"
"  pos = data0.xy;                                                  \n"
"  p = data0.z;                                                         \n"
"  q = data0.w;                                                         \n"
"  a = data1.xy;                                                        \n"
"  b = data1.zw;                                                        \n"
"  c = data2.xy;                                                        \n"
"  offset = data2.z;                                                    \n"
"  strokeWidth = width;                                               \n"
"}                                                                      \n";

static const char *fs_code2 =
"uniform lowp vec4 fColor;\n"
"uniform mediump float feather;                                                      \n"
"varying highp vec2 pos;                                                            \n"
"varying highp float p, q;                                                        \n"
"varying highp vec2 a, b, c;                                                        \n"
"varying mediump float offset, strokeWidth;                                            \n"
"                                                                            \n"
"#define M_PI 3.1415926535897932384626433832795                                \n"
"                                                                            \n"
"highp vec2 evaluateQuadratic(highp float t)                                            \n"
"{                                                                            \n"
"   return a * t * t + b * t + c;                                            \n"
"}                                                                            \n"
"                                                                            \n"
"highp float check(highp float t)                                                        \n"
"{                                                                            \n"
"   if (0.0 <= t && t <= 1.0)                                                \n"
"   {                                                                        \n"
"      highp vec2 q = evaluateQuadratic(t) - pos;                                    \n"
"      return length(q)/strokeWidth;                                        \n"
"   }                                                                        \n"
"                                                                            \n"
"   return 2.0;                                                            \n"
"}                                                                            \n"
"                                                                            \n"
"highp float cbrt(highp float x)                                                        \n"
"{                                                                            \n"
"   return sign(x) * pow(abs(x), 1.0 / 3.0);                                \n"
"}                                                                            \n"
"                                                                            \n"
"void main()                                                                \n"
"{                                                                            \n"
"   highp float d = q * q / 4.0 + p * p * p / 27.0;                                \n"
"                                                                            \n"
"   if (d >= 0.0)                                                            \n"
"   {                                                                        \n"
"      highp float c1 = -q / 2.0;                                                    \n"
"      highp float c2 = sqrt(d);                                                    \n"
"      d=check(cbrt(c1 + c2) + cbrt(c1 - c2) + offset);            \n"
"   }                                                                        \n"
"   else                                                                    \n"
"   {                                                                        \n"
"      highp float cos_3_theta = 3.0 * q * sqrt(-3.0 / p) / (2.0 * p);            \n"
"      highp float theta = acos(cos_3_theta) / 3.0;                                \n"
"      highp float r = 2.0 * sqrt(-p / 3.0);                                        \n"
"                                                                            \n"
"      d=min(check(r * cos(theta) + offset),min(                                \n"
"          check(r * cos(theta + 2.0 * M_PI / 3.0) + offset),                \n"
"          check(r * cos(theta + 4.0 * M_PI / 3.0) + offset)));        \n"
"   }                                                                        \n"
"	lowp vec4 frag=fColor; \n"
"		mediump float alpha= 1.0f-smoothstep(0.5f - feather/2, 0.5f + feather/2, d); \n"
"		 if (alpha <= 0.0)  // Outside  		\n"
"			discard;						\n"
"		frag*= alpha;						\n"
"  gl_FragColor = frag;                                         \n"
"}                                                                            \n";

static const char *vs_codeSL =
"uniform highp mat4 mvp;                                                      \n"
"uniform mediump float width;                                                      \n"
"                                                                       \n"
"attribute highp vec4 data0;                                                  \n"
"                                                                       \n"
"varying mediump vec2 uv;                                                       \n"
"                                                                       \n"
"void main(void)                                                        \n"
"{                                                                      \n"
"  gl_Position = mvp*vec4(data0.zw*width+data0.xy, 0, 1);   \n"
"  uv = data0.zw;                                                          \n"
"}                                                                      \n";

static const char *fs_codeSL =
"uniform lowp vec4 fColor;\n"
"uniform mediump float feather;                                                      \n"
"varying mediump vec2 uv;                           \n"
"                                           \n"
"void main(void)                            \n"
"{                                          \n"
"		lowp vec4 frag = fColor;				\n"
"		mediump float l=length(uv);			\n"
"		mediump float alpha= 1.0f-smoothstep(0.5f - feather/2, 0.5f + feather/2, l); \n"
"		 if (alpha <= 0.0)  // Outside  		\n"
"			discard;						\n"
"		frag*= alpha;						\n"
"  gl_FragColor = frag;				         \n"
"}                                          \n";

void pathShadersInit()
{
	const ShaderProgram::ConstantDesc pathUniforms[] = {
			{ "mvp",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
			{ "fColor", ShaderProgram::CFLOAT4, 1,	ShaderProgram::SysConst_Color, false, 0, NULL },
			{ "fTexture", ShaderProgram::CTEXTURE, 1, ShaderProgram::SysConst_None, false, 0, NULL },
			{ "", ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,false, 0, NULL } };

	const ShaderProgram::ConstantDesc pathUniformsStrokeSL[] = {
			{ "mvp",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
			{ "width", ShaderProgram::CFLOAT, 1,	ShaderProgram::SysConst_None, true, 0, NULL },
			{ "fColor", ShaderProgram::CFLOAT4, 1,	ShaderProgram::SysConst_Color, false, 0, NULL },
			{ "feather", ShaderProgram::CFLOAT, 1, ShaderProgram::SysConst_None, false, 0, NULL },
			{ "", ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,false, 0, NULL } };

	const ShaderProgram::DataDesc pathAttributesFillC[] = {
			{ "data0",ShaderProgram::DFLOAT, 4, 0, 0 },
			{ "", ShaderProgram::DFLOAT, 0, 0, 0 } };

	const ShaderProgram::DataDesc pathAttributesStrokeC[] = {
			{ "data0",ShaderProgram::DFLOAT, 4, 0, 0 },
			{ "data1", ShaderProgram::DFLOAT, 4, 1, 0 },
			{ "data2", ShaderProgram::DFLOAT, 4, 2, 0 },
			{ "", ShaderProgram::DFLOAT, 0, 0, 0 } };

	const ShaderProgram::DataDesc pathAttributesStrokeSL[] = {
			{ "data0",ShaderProgram::DFLOAT, 4, 0, 0 },
			{ "", ShaderProgram::DFLOAT, 0, 0, 0 } };

	ShaderProgram::pathShaderFillC = new ogl2ShaderProgram(hdrShaderCode,
			vs_code1, hdrShaderCode, fs_code1, pathUniforms,
			pathAttributesFillC);
	ShaderProgram::pathShaderStrokeC = new ogl2ShaderProgram(hdrShaderCode,
			vs_code2, hdrShaderCode, fs_code2, pathUniformsStrokeSL,
			pathAttributesStrokeC);
	ShaderProgram::pathShaderStrokeLC = new ogl2ShaderProgram(hdrShaderCode,
			vs_codeSL, hdrShaderCode, fs_codeSL, pathUniformsStrokeSL,
			pathAttributesStrokeSL);
}

void pathShadersRelease()
{
	delete ShaderProgram::pathShaderFillC;
	delete ShaderProgram::pathShaderStrokeC;
	delete ShaderProgram::pathShaderStrokeLC;
}
