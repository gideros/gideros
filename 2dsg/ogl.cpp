#include "ogl.h"
#include <cassert>

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

GLuint shaderProgram=0;
GLuint xformVShader=0;
GLuint colorFShader=0;
GLuint vertexVS=0;
GLuint textureVS=0;
GLuint colorVS=0;
GLuint colorFS=0;

/* Vertex shader*/
const char *xformVShaderCode="\
attribute vec4 vVertex;\
attribute vec4 vColor;\
attribute vec2 vTexCoord;\
varying vec2 fTexCoord;\
\
void main() {\
  gl_position = vVertex;\
  fTexCoord=vTexCoord;\
}";

/* Fragment shader*/
const char *colorFShaderCode="\
precision mediump float;\
uniform vec4 fColor;\
void main() {\
 gl_FragColor = fColor;\
}";

GLuint oglLoadShader(GLuint type,const char *code)
{
	return 0; //TODO
}

void oglSetupShaders()
{
	xformVShader=oglLoadShader(GL_VERTEX_SHADER,xformVShaderCode);
	colorFShader=oglLoadShader(GL_FRAGMENT_SHADER,colorFShaderCode);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, xformVShader);
    glAttachShader(shaderProgram, colorFShader);
    glLinkProgram(shaderProgram);

	vertexVS=glGetAttribLocation(shaderProgram, "vPosition");
	textureVS=glGetAttribLocation(shaderProgram, "vTexture");
	colorVS=glGetAttribLocation(shaderProgram, "vColor");
    colorFS=glGetUniformLocation(shaderProgram, "fColor");
}

void oglLoadMatrixf(const Matrix4 m)
{
 //TODO
}

void oglEnable(GLenum cap)
{
	switch (cap)
	{
	case GL_TEXTURE_2D:
		if (s_Texture2DEnabled == false)
		{
			glEnable(GL_TEXTURE_2D);
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
			glDisable(GL_TEXTURE_2D);
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
		glBindTexture(target, texture);
		s_BindTextureCount++;
	}
}

void oglForceBindTexture(GLenum target, GLuint texture)
{
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
	glUniform4f(colorFS,r,g,b,a);
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
	switch (array)
	{
		case VertexArray:
			glVertexAttribPointer(vertexVS, mult,type, false,0, ptr);
			break;
		case TextureArray:
			glVertexAttribPointer(textureVS, mult,type, false,0, ptr);
			break;
        case ColorArray:
			glVertexAttribPointer(colorVS, mult,type, false,0, ptr);
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
	if (s_VERTEX_ARRAY)
	{
		if (s_VERTEX_ARRAY_enabled == false)
		{
			s_VERTEX_ARRAY_enabled = true;
			s_clientStateCount++;
		    glEnableVertexAttribArray(vertexVS);
		}
	}
	else
	{
		if (s_VERTEX_ARRAY_enabled == true)
		{
			s_VERTEX_ARRAY_enabled = false;
			s_clientStateCount++;
		    glDisableVertexAttribArray(vertexVS);
		}
	}

	if (s_TEXTURE_COORD_ARRAY)
	{
		if (s_TEXTURE_COORD_ARRAY_enabled == false)
		{
			s_TEXTURE_COORD_ARRAY_enabled = true;
			s_clientStateCount++;
		    glEnableVertexAttribArray(textureVS);
		}
	}
	else
	{
		if (s_TEXTURE_COORD_ARRAY_enabled == true)
		{
			s_TEXTURE_COORD_ARRAY_enabled = false;
			s_clientStateCount++;
		    glDisableVertexAttribArray(textureVS);
		}
	}

    if (s_COLOR_ARRAY)
    {
        if (s_COLOR_ARRAY_enabled == false)
        {
            s_COLOR_ARRAY_enabled = true;
            s_clientStateCount++;
		    glEnableVertexAttribArray(colorVS);
        }
    }
    else
    {
        if (s_COLOR_ARRAY_enabled == true)
        {
            s_COLOR_ARRAY_enabled = false;
            s_clientStateCount++;
		    glDisableVertexAttribArray(colorVS);
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

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	s_VERTEX_ARRAY_enabled = false;
	s_TEXTURE_COORD_ARRAY_enabled = false;
    s_COLOR_ARRAY_enabled = false;
	s_VERTEX_ARRAY = 0;
	s_TEXTURE_COORD_ARRAY = 0;
    s_COLOR_ARRAY = 0;
    oglSetupArrays();

	resetBindTextureCount();
	resetClientStateCount();
	resetTexture2DStateCount();

    glEnable(GL_BLEND);

#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	/* sanity set */ XXX
}
