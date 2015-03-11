#include "ogl.h"
#include <cassert>
#include <vector>
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

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

		glog_e("Shader Compile: %s\n",&infoLog[0]);
		glDeleteShader(shader);
		shader=0;
	}
	glog_i("Loaded shader:%d\n",shader);
	return shader;
}


void oglSetupShaders()
{
	xformVShader=oglLoadShader(GL_VERTEX_SHADER,xformVShaderCode);
	colorFShader=oglLoadShader(GL_FRAGMENT_SHADER,colorFShaderCode);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, xformVShader);
    glAttachShader(shaderProgram, colorFShader);
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

    //glog_i("VIndices: %d,%d,%d,%d\n", vertexVS,textureVS,colorVS,matrixVS);
    //glog_i("FIndices: %d,%d,%d,%d\n", colorSelFS,textureSelFS,colorFS,textureFS);

    glUniform1i(textureFS, 0);

	GLint maxLength = 0;
	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
	std::vector<GLchar> infoLog(maxLength);
	glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
	glog_i("GL Program log:%s\n",&infoLog[0]);

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


 glGenRenderbuffers(1, &_depthRenderBuffer);
 glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
 glRenderbufferStorage(GL_RENDERBUFFER, depthfmt, sw,sh);
 glBindRenderbuffer(GL_RENDERBUFFER, 0);
 glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
 glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
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
	glDeleteRenderbuffers(1,&_depthRenderBuffer);
}

Matrix4 oglProjection;
void oglLoadMatrixf(const Matrix4 m)
{
#ifdef GIDEROS_GL1
	 glMatrixMode(GL_MODELVIEW);
	 glLoadMatrixf(m.data());
#else
	Matrix4 xform=oglProjection*m;
	glUniformMatrix4fv(matrixVS, 1, false, xform.data());
#endif
}

void oglSetProjection(const Matrix4 m)
{
#ifdef GIDEROS_GL1
	 glMatrixMode(GL_PROJECTION);
	 glLoadMatrixf(m.data());
#else
	oglProjection=m;
#endif
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
	resetBindTextureCount();
	resetClientStateCount();
	resetTexture2DStateCount();

	//glClearColor(0.5, 0.1, 0.2, 1.f);
    //glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);

#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
}
