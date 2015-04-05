#include "ogl.h"
#include <cassert>
#include <vector>
#include <stack>
#include "glog.h"

static GLuint s_texture = 0;
static int s_BindTextureCount = 0;
static bool s_Texture2DEnabled = false;
static int s_Texture2DStateCount = 0;

static int s_VERTEX_ARRAY = 0;
static int s_TEXTURE_COORD_ARRAY = 0;
static int s_COLOR_ARRAY = 0;

static bool s_VERTEX_ARRAY_enabled = false;
static bool s_TEXTURE_COORD_ARRAY_enabled = false;
static bool s_COLOR_ARRAY_enabled = false;

static int s_clientStateCount = 0;
static int s_depthEnable=0;
static bool s_depthBufferCleared=false;

static bool oglInitialized=false;

#ifndef GIDEROS_GL1
GLuint shaderProgram=0;
GLuint xformVShader=0;
GLuint colorFShader=0;
GLuint vertexVS=0;
GLuint textureVS=0;
GLuint colorVS=0;
GLuint matrixVS=0;
GLuint colorFS=0;
GLuint colorSelFS=0;
GLuint textureSelFS=0;
GLuint textureFS=0;
GLuint _depthRenderBuffer=0;

#ifdef OPENGL_ES
/* Vertex shader*/
const char *xformVShaderCode=
"attribute mediump vec2 vTexCoord;\n"
"attribute mediump vec4 vVertex;\n"
"attribute mediump vec4 vColor;\n"
"uniform mediump mat4 vMatrix;\n"
"varying mediump vec2 fTexCoord;\n"
"varying mediump vec4 fInColor; "
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
uniform mediump vec4 fColor;\
uniform sampler2D fTexture;\
varying mediump vec2 fTexCoord;\
varying mediump vec4 fInColor;\
void main() {\
 mediump vec4 col=mix(fColor,fInColor,fColorSel);\
 mediump vec4 tex=mix(vec4(1,1,1,1),texture2D(fTexture, fTexCoord),fTextureSel);\
 gl_FragColor = tex * col;\
}";
#else
/* Vertex shader*/
const char *xformVShaderCode=
#ifdef OPENGL_ES
    "#version 100\n"
    "#define GLES2\n"
#else
    "#version 120\n"
#endif
"attribute vec2 vTexCoord;\n"
"attribute vec4 vColor;\n"
"attribute vec3 vVertex;\n"
"uniform mat4 vMatrix;\n"
"varying vec2 fTexCoord;\n"
"varying vec4 fInColor; "
"\n"
"void main() {\n"
"  vec4 vertex = vec4(vVertex,1.0f);\n"
"  gl_Position = vMatrix*vertex;\n"
"  fTexCoord=vTexCoord;\n"
"  fInColor=vColor;\n"
"}\n";

/* Fragment shader*/
const char *colorFShaderCode=
#ifdef OPENGL_ES
    "#version 100\n"
    "#define GLES2\n"
#else
    "#version 120\n"
#endif
"uniform float fColorSel;"
"uniform float fTextureSel;\n"
"uniform vec4 fColor;\n"
"uniform sampler2D fTexture;\n"
"varying vec2 fTexCoord;\n"
"varying vec4 fInColor;\n"
"void main() {\n"
" vec4 col=mix(fColor,fInColor,fColorSel);\n"
" vec4 tex=mix(vec4(1.0f,1.0f,1.0f,1.0f),texture2D(fTexture, fTexCoord),fTextureSel);\n"
" gl_FragColor = tex * col;\n"
"}\n";
#endif

GLuint oglLoadShader(GLuint type,const char *code)
{
	GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code,NULL);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		if (maxLength>0)
		{
			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

			glog_e("Shader Compile: %s\n",&infoLog[0]);
		}
		glDeleteShader(shader);
		shader=0;
	}
	glog_i("Loaded shader:%d\n",shader);
	return shader;
}


void oglSetupShaders()
{
	glog_i("GL_VERSION:%s\n",glGetString(GL_VERSION));
	glog_i("GLSL_VERSION:%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	xformVShader=oglLoadShader(GL_VERTEX_SHADER,xformVShaderCode);
	colorFShader=oglLoadShader(GL_FRAGMENT_SHADER,colorFShaderCode);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, xformVShader);
    glAttachShader(shaderProgram, colorFShader);

    glBindAttribLocation(shaderProgram, 0, "vVertex"); //Ensure vertex is at 0

    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);

	vertexVS=glGetAttribLocation(shaderProgram, "vVertex");
	textureVS=glGetAttribLocation(shaderProgram, "vTexCoord");
	colorVS=glGetAttribLocation(shaderProgram, "vColor");
    matrixVS=glGetUniformLocation(shaderProgram, "vMatrix");
    colorSelFS=glGetUniformLocation(shaderProgram, "fColorSel");
    textureSelFS=glGetUniformLocation(shaderProgram, "fTextureSel");
    colorFS=glGetUniformLocation(shaderProgram, "fColor");
    textureFS=glGetUniformLocation(shaderProgram, "fTexture");

    glog_i("VIndices: %d,%d,%d,%d\n", vertexVS,textureVS,colorVS,matrixVS);
    glog_i("FIndices: %d,%d,%d,%d\n", colorSelFS,textureSelFS,colorFS,textureFS);

    glUniform1i(textureFS, 0);

	GLint maxLength = 0;
	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
	if (maxLength>0)
	{
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
		glog_i("GL Program log:%s\n",&infoLog[0]);
	}
}
#endif

void oglInitialize(unsigned int sw,unsigned int sh)
{
    if (oglInitialized) return;
#ifndef GIDEROS_GL1
 oglSetupShaders();
 glActiveTexture(GL_TEXTURE0);
#endif

 int depthfmt=0;
#ifdef GL_DEPTH24_STENCIL8_OES
 depthfmt=GL_DEPTH24_STENCIL8_OES;
#else
 depthfmt=GL_DEPTH24_STENCIL8;
#endif

#ifdef OPENGL_ES
 glGenRenderbuffers(1, &_depthRenderBuffer);
 glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
 glRenderbufferStorage(GL_RENDERBUFFER, depthfmt, sw,sh);
 glBindRenderbuffer(GL_RENDERBUFFER, 0);
 glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
 glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
#endif

 oglInitialized=true;
}

void oglCleanup()
{
    oglInitialized=false;
#ifndef GIDEROS_GL1
	glUseProgram(0);
	glDeleteProgram(shaderProgram);
	glDeleteShader(xformVShader);
	glDeleteShader(colorFShader);
#endif
#ifdef OPENGL_ES
	glDeleteRenderbuffers(1,&_depthRenderBuffer);
#endif
}

Matrix4 oglProjection;
Matrix4 oglVPProjection;
Matrix4 oglModel;
Matrix4 oglCombined;

void oglViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	glViewport(x,y,width,height);
}

void oglLoadMatrixf(const Matrix4 m)
{
	oglModel=m;
	oglCombined=oglProjection*oglModel;
#ifdef GIDEROS_GL1
	 glMatrixMode(GL_MODELVIEW);
	 glLoadMatrixf(m.data());
#else
	glUniformMatrix4fv(matrixVS, 1, false, oglCombined.data());
#endif
}

Matrix4 oglGetModelMatrix()
{
	return oglModel;
}

void oglViewportProjection(const Matrix4 m)
{
	oglVPProjection=m;
}

void oglSetProjection(const Matrix4 m)
{
#ifdef GIDEROS_GL1
	 glMatrixMode(GL_PROJECTION);
	 glLoadMatrixf(m.data());
#endif
	oglProjection=m;
}

void oglEnable(GLenum cap)
{
	switch (cap)
	{
	case GL_TEXTURE_2D:
		if (s_Texture2DEnabled == false)
		{
#ifdef GIDEROS_GL1
			glEnable(GL_TEXTURE_2D);
#else
		    glUniform1f(textureSelFS, 1);
		    //glog_d("TextureSelFS:%d\n",1);
#endif
			s_Texture2DEnabled = true;
			s_Texture2DStateCount++;
		}
		break;
	case GL_DEPTH_TEST:
		if (!(s_depthEnable++))
		{
			if (!s_depthBufferCleared)
			{
#ifdef OPENGL_ES
				glClearDepthf(1);
#endif
    			glClear(GL_DEPTH_BUFFER_BIT);
    			s_depthBufferCleared=true;
			}
			glEnable(cap);
		}
		break;
	default:
		glEnable(cap);
		break;
	}
}

void oglDisable(GLenum cap)
{
	switch (cap)
	{
	case GL_TEXTURE_2D:
		if (s_Texture2DEnabled == true)
		{
#ifdef GIDEROS_GL1
			glDisable(GL_TEXTURE_2D);
#else
		    glUniform1f(textureSelFS, 0);
		    //glog_d("TextureSelFS:%d\n",0);
#endif
			s_Texture2DEnabled = false;
			s_Texture2DStateCount++;
		}
		break;
	case GL_DEPTH_TEST:
		if (!(--s_depthEnable))
			glDisable(cap);
		break;
	default:
		glDisable(cap);
		break;
	}
}

void resetTexture2DStateCount()
{
	s_Texture2DStateCount = 0;
}

int getTexture2DStateCount()
{
	return s_Texture2DStateCount;
}
void oglBindTexture(GLenum target, GLuint texture)
{
	if (texture != s_texture)
	{
		s_texture = texture;
		//glog_d("BindTexture:%d\n",s_texture);
#ifndef GIDEROS_GL1
		glActiveTexture(GL_TEXTURE0);
#endif
		glBindTexture(target, texture);

		s_BindTextureCount++;
	}
}

void oglForceBindTexture(GLenum target, GLuint texture)
{
#ifndef GIDEROS_GL1
	glActiveTexture(GL_TEXTURE0);
#endif
	glBindTexture(target, texture);
	s_texture = texture;
	s_BindTextureCount++;
}

void resetBindTextureCount()
{
	s_BindTextureCount = 0;
}

int getBindTextureCount()
{
	return s_BindTextureCount;
}

void oglColor4f(float r,float g,float b,float a)
{
	//printf("glColor: %f,%f,%f,%f\n",r,g,b,a);
#ifdef GIDEROS_GL1
	glColor4f(r,g,b,a);
#else
	glUniform4f(colorFS,r,g,b,a);
#endif
}

void oglEnableClientState(enum OGLClientState array)
{
	switch (array)
	{
		case VertexArray:
			assert(s_VERTEX_ARRAY == 0);
			s_VERTEX_ARRAY++;
			break;
		case TextureArray:
			assert(s_TEXTURE_COORD_ARRAY == 0);
			s_TEXTURE_COORD_ARRAY++;
			break;
        case ColorArray:
            assert(s_COLOR_ARRAY == 0);
            s_COLOR_ARRAY++;
            break;
		default:
			assert(1);
			break;
	}
}

void oglArrayPointer(enum OGLClientState array,int mult,GLenum type,const void *ptr)
{
	//glog_d("OglArrayPtr: %d:%p[%f,%f,%f,%f]\n",array,ptr,((const float *)ptr)[0],((const float *)ptr)[1],((const float *)ptr)[2],((const float *)ptr)[3]);
	switch (array)
	{
		case VertexArray:
#ifdef GIDEROS_GL1
			glVertexPointer(mult,type, 0,ptr);
#else
			glVertexAttribPointer(vertexVS, mult,type, false,0, ptr);
#endif
			break;
		case TextureArray:
#ifdef GIDEROS_GL1
			glTexCoordPointer(mult,type, 0,ptr);
#else
			glVertexAttribPointer(textureVS, mult,type, false,0, ptr);
#endif
			break;
        case ColorArray:
#ifdef GIDEROS_GL1
			glColorPointer(mult,type, 0,ptr);
#else
			glVertexAttribPointer(colorVS, mult,type, true,0, ptr);
#endif
            break;
		default:
			assert(1);
			break;
	}
}

void oglDisableClientState(enum OGLClientState array)
{
	switch (array)
	{
		case VertexArray:
			assert(s_VERTEX_ARRAY == 1);
			s_VERTEX_ARRAY--;
			break;
		case TextureArray:
			assert(s_TEXTURE_COORD_ARRAY == 1);
			s_TEXTURE_COORD_ARRAY--;
			break;
        case ColorArray:
            assert(s_COLOR_ARRAY == 1);
            s_COLOR_ARRAY--;
            break;
		default:
			assert(1);
			break;
	}
}

void oglSetupArrays()
{
	/*glog_d("OglArrays: %d:%d,%d:%d,%d:%d\n",
			s_VERTEX_ARRAY,s_VERTEX_ARRAY_enabled,
			s_TEXTURE_COORD_ARRAY,s_TEXTURE_COORD_ARRAY_enabled,
			s_COLOR_ARRAY,s_COLOR_ARRAY_enabled	);*/
	if (s_VERTEX_ARRAY)
	{
		if (s_VERTEX_ARRAY_enabled == false)
		{
			s_VERTEX_ARRAY_enabled = true;
			s_clientStateCount++;
#ifdef GIDEROS_GL1
			glEnableClientState(GL_VERTEX_ARRAY);
#else
		    glEnableVertexAttribArray(vertexVS);
#endif
		}
	}
	else
	{
		if (s_VERTEX_ARRAY_enabled == true)
		{
			s_VERTEX_ARRAY_enabled = false;
			s_clientStateCount++;
#ifdef GIDEROS_GL1
			glDisableClientState(GL_VERTEX_ARRAY);
#else
		    glDisableVertexAttribArray(vertexVS);
#endif
		}
	}

	if (s_TEXTURE_COORD_ARRAY)
	{
		if (s_TEXTURE_COORD_ARRAY_enabled == false)
		{
			s_TEXTURE_COORD_ARRAY_enabled = true;
			s_clientStateCount++;
#ifdef GIDEROS_GL1
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#else
		    glEnableVertexAttribArray(textureVS);
#endif
		}
	}
	else
	{
		if (s_TEXTURE_COORD_ARRAY_enabled == true)
		{
			s_TEXTURE_COORD_ARRAY_enabled = false;
			s_clientStateCount++;
#ifdef GIDEROS_GL1
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else
		    glDisableVertexAttribArray(textureVS);
#endif
		}
	}

    if (s_COLOR_ARRAY)
    {
        if (s_COLOR_ARRAY_enabled == false)
        {
            s_COLOR_ARRAY_enabled = true;
            s_clientStateCount++;
#ifdef GIDEROS_GL1
			glEnableClientState(GL_COLOR_ARRAY);
#else
		    glEnableVertexAttribArray(colorVS);
		    glUniform1f(colorSelFS, 1);
#endif
        }
    }
    else
    {
        if (s_COLOR_ARRAY_enabled == true)
        {
            s_COLOR_ARRAY_enabled = false;
            s_clientStateCount++;
#ifdef GIDEROS_GL1
			glDisableClientState(GL_COLOR_ARRAY);
#else
		    glDisableVertexAttribArray(colorVS);
		    glUniform1f(colorSelFS, 0);
#endif
        }
    }
}

void oglDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	oglSetupArrays();
	glDrawArrays(mode, first, count);
}



void oglDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
	oglSetupArrays();
	glDrawElements(mode, count, type, indices);
}


int getVertexArrayCount()
{
	return s_VERTEX_ARRAY;
}

int getTextureCoordArrayCount()
{
	return s_TEXTURE_COORD_ARRAY;
}

void resetClientStateCount()
{
	s_clientStateCount = 0;
}

int getClientStateCount()
{
	return s_clientStateCount;
}



void oglReset()
{
	s_texture = 0;
	s_Texture2DEnabled = false;
	s_depthEnable=0;
	s_depthBufferCleared=false;

#ifdef GIDEROS_GL1
	glDisable(GL_TEXTURE_2D);
#endif
	oglColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, 0);

	s_VERTEX_ARRAY_enabled = false;
	s_TEXTURE_COORD_ARRAY_enabled = false;
    s_COLOR_ARRAY_enabled = false;
	s_VERTEX_ARRAY = 0;
	s_TEXTURE_COORD_ARRAY = 0;
    s_COLOR_ARRAY = 0;
#ifdef GIDEROS_GL1
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	/* sanity set */
#else
    glDisableVertexAttribArray(vertexVS);
    glDisableVertexAttribArray(textureVS);
    glDisableVertexAttribArray(colorVS);
    glUniform1f(colorSelFS, 0);
    glUniform1f(textureSelFS, 0);
#endif
    oglProjection.identity();
    oglVPProjection.identity();
    oglModel.identity();
    oglCombined.identity();

	resetBindTextureCount();
	resetClientStateCount();
	resetTexture2DStateCount();

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

struct Scissor
{
	Scissor() {}
	Scissor(int x,int y,int w,int h) : x(x), y(y), w(w), h(h) {}
	Scissor(const Scissor &p,int nx,int ny,int nw,int nh) : x(nx), y(ny), w(nw), h(nh)
	{
		int x2=x+w-1;
		int y2=y+h-1;
		int px2=p.x+p.w-1;
		int py2=p.y+p.h-1;
		if (p.x>x)
			x=p.x;
		if (p.y>y)
			y=p.y;
		if (px2<x2)
			x2=px2;
		if (py2<y2)
			y2=py2;
		w=x2+1-x;
		h=y2+1-y;
	}

	int x,y,w,h;
};

static std::stack<Scissor> scissorStack;

//Coordinates are untransformed coordinates (ie, cuurent Sprite local)
void oglPushScissor(float x,float y,float w,float h)
{
	Vector4 v1(x,y,0,1);
	Vector4 v2(x+w,y+h,0,1);
	Matrix4 xform=oglVPProjection*oglModel;
	v1=xform*v1;
	v2=xform*v2;
	//glog_d("Scissor: [%f,%f->%f,%f] -> [%f,%f->%f,%f]",x,y,x+w,y+h,v1.x,v1.y,v2.x,v2.y);
	x=v1.x;
	w=v2.x-v1.x;
	if (w<0)
	{
		x=v2.x;
		w=-w;
	}
	y=v1.y;
	h=v2.y-v1.y;
	if (h<0)
	{
		y=v2.y;
		h=-h;
	}
	if (scissorStack.empty())
	{
		Scissor s(x,y,w,h);
		scissorStack.push(s);
		glEnable(GL_SCISSOR_TEST);
		glScissor(s.x,s.y,s.w,s.h);
	}
	else
	{
		Scissor s(scissorStack.top(),x,y,w,h);
		scissorStack.push(s);
		glScissor(s.x,s.y,s.w,s.h);
	}
}

void oglPopScissor()
{
	if (scissorStack.empty()) return; //Probably a code issue
	scissorStack.pop();
	if (scissorStack.empty())
	{
		glDisable(GL_SCISSOR_TEST);
	}
	else
	{
		Scissor s=scissorStack.top();
		glScissor(s.x,s.y,s.w,s.h);
	}
}
