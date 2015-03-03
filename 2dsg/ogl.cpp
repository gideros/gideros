#include "ogl.h"
#include <cassert>
#include <vector>

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
GLuint matrixVS=0;
GLuint colorFS=0;
GLuint colorSelFS=0;
GLuint textureFS=0;

/* Vertex shader*/
const char *xformVShaderCode=
"attribute vec2 vTexCoord;\n"
"attribute vec4 vVertex;\n"
"attribute vec4 vColor;\n"
"uniform mat4 vMatrix;\n"
"varying vec2 fTexCoord;\n"
"varying vec4 fInColor; "
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
uniform vec4 fColor;\
uniform sampler2D fTexture;\
varying mediump vec2 fTexCoord;\
varying vec4 fInColor;\
void main() {\
 vec4 col=mix(fColor,fInColor,fColorSel);\
 gl_FragColor = texture2D(fTexture, fTexCoord) * col;\
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

		//printf(&infoLog[0]);
		glDeleteShader(shader);
		shader=0;
	}
	//printf("Loaded shader:%d\n",shader);
	return shader;
}

void oglSetupShaders()
{
	if (xformVShader||colorFShader) return;
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
    colorFS=glGetUniformLocation(shaderProgram, "fColor");
    textureFS=glGetUniformLocation(shaderProgram, "fTexture");

    //printf("VIndices: %d,%d,%d,%d\n", vertexVS,textureVS,colorVS,matrixVS);
    //printf("FIndices: %d,%d\n", colorFS,textureFS);

    glUniform1i(textureFS, 0);

	GLint maxLength = 0;
	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
	std::vector<GLchar> infoLog(maxLength);
	glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
	//printf(&infoLog[0]);

}

Matrix4 oglProjection;
void oglLoadMatrixf(const Matrix4 m)
{
	Matrix4 xform=oglProjection*m; //Maybe the other way ??

	glUniformMatrix4fv(matrixVS, 1, false, xform.data());
}

void oglSetProjection(const Matrix4 m)
{
	oglProjection=m;
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
	//printf("glColor: %f,%f,%f,%f\n",r,g,b,a);
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
			glVertexAttribPointer(colorVS, mult,type, true,0, ptr);
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
	/*printf("OglArrays: %d:%d,%d:%d,%d:%d\n",
			s_VERTEX_ARRAY,s_VERTEX_ARRAY_enabled,
			s_TEXTURE_COORD_ARRAY,s_TEXTURE_COORD_ARRAY_enabled,
			s_COLOR_ARRAY,s_COLOR_ARRAY_enabled
	);*/
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
		    glUniform1f(colorSelFS, 1);
        }
    }
    else
    {
        if (s_COLOR_ARRAY_enabled == true)
        {
            s_COLOR_ARRAY_enabled = false;
            s_clientStateCount++;
		    glDisableVertexAttribArray(colorVS);
		    glUniform1f(colorSelFS, 0);
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
	oglSetupShaders();

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
    glDisableVertexAttribArray(vertexVS);
    glDisableVertexAttribArray(textureVS);
    glDisableVertexAttribArray(colorVS);

	resetBindTextureCount();
	resetClientStateCount();
	resetTexture2DStateCount();

	glClearColor(0.5, 0.1, 0.2, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

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
