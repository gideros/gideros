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

GLint curProg=-1;
ShaderProgram *current=NULL;
GLuint _depthRenderBuffer=0;
bool matrixDirty=true;
float constColR,constColG,constColB,constColA;
bool colorDirty=true;


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
const char *hdrVShaderCode=
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

const char *stdVShaderCode=
"uniform highp mat4 vMatrix;\n"
"\n"
"void main() {\n"
"  vec4 vertex = vec4(vVertex,1.0);\n"
"  gl_Position = vMatrix*vertex;\n"
"}\n";
const char *stdCVShaderCode=
"attribute lowp vec4 vColor;\n"
"uniform highp mat4 vMatrix;\n"
"varying lowp vec4 fInColor; "
"\n"
"void main() {\n"
"  vec4 vertex = vec4(vVertex,1.0);\n"
"  gl_Position = vMatrix*vertex;\n"
"  fInColor=vColor;\n"
"}\n";
const char *stdTVShaderCode=
"attribute mediump vec2 vTexCoord;\n"
"uniform highp mat4 vMatrix;\n"
"varying mediump vec2 fTexCoord;\n"
"\n"
"void main() {\n"
"  vec4 vertex = vec4(vVertex,1.0);\n"
"  gl_Position = vMatrix*vertex;\n"
"  fTexCoord=vTexCoord;\n"
"}\n";
const char *stdCTVShaderCode=
"attribute mediump vec2 vTexCoord;\n"
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

/* Fragment shader*/
const char *hdrFShaderCode=
#ifdef OPENGL_ES
    "#version 100\n"
    "#define GLES2\n";
#else
    "#version 120\n"
    "#define highp\n"
    "#define mediump\n"
    "#define lowp\n";
#endif

const char *stdFShaderCode=
"uniform lowp vec4 fColor;\n"
"void main() {\n"
" gl_FragColor = fColor;\n"
"}\n";
const char *stdCFShaderCode=
"varying lowp vec4 fInColor;\n"
"void main() {\n"
" gl_FragColor = fInColor;\n"
"}\n";
const char *stdTFShaderCode=
"uniform lowp vec4 fColor;\n"
"uniform lowp sampler2D fTexture;\n"
"varying mediump vec2 fTexCoord;\n"
"void main() {\n"
" lowp vec4 frag=fColor*texture2D(fTexture, fTexCoord);\n"
" if (frag.a==0.0) discard;\n"
" gl_FragColor = frag;\n"
"}\n";
const char *stdCTFShaderCode=
"varying lowp vec4 fInColor;\n"
"uniform lowp sampler2D fTexture;\n"
"varying mediump vec2 fTexCoord;\n"
"void main() {\n"
" lowp vec4 frag=fInColor*texture2D(fTexture, fTexCoord);\n"
" if (frag.a==0.0) discard;\n"
" gl_FragColor = frag;\n"
"}\n";
#endif



class oglShaderProgram : public ShaderProgram
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    std::vector<GLint> attributes;
    std::vector<GLint> uniforms;
    
public:
    virtual void activate();
    virtual void deactivate();
    virtual void setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, BufferCache **cache);
    virtual void setConstant(int index,ConstantType type,const void *ptr);
    virtual void drawArrays(ShapeType shape, int first, unsigned int count);
    virtual void drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, BufferCache *cache);

    oglShaderProgram(const char *vshader1,const char *vshader2,
                     const char *fshader1, const char *fshader2,
                     const char **uniforms, const char **attributes);
    virtual ~oglShaderProgram();
    void useProgram();
};

GLuint oglLoadShader(GLuint type,const char *hdr,const char *code)
{
	GLuint shader = glCreateShader(type);
    const char *lines[2]={hdr,code};
    glShaderSource(shader, 2, lines,NULL);
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

GLuint oglBuildProgram(GLuint vertexShader,GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glBindAttribLocation(program, 0, "vVertex"); //Ensure vertex is at 0
    glLinkProgram(program);
    
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
    if (maxLength>0)
    {
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
        glog_i("GL Program log:%s\n",&infoLog[0]);
    }
    return program;
}

void oglShaderProgram::deactivate()
{
	for(std::vector<GLint>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
		glDisableVertexAttribArray(*it);
	}
    current=NULL;
}

void oglShaderProgram::activate()
{
	useProgram();
	if (current == this) return;
	if (current) current->deactivate();
    current=this;
	for(std::vector<GLint>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
		glEnableVertexAttribArray(*it);
	}

}

void oglShaderProgram::useProgram()
{
    if (curProg!=program)
    {
        glUseProgram(program);
        curProg=program;
    }
}

void oglShaderProgram::setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, BufferCache **cache)
{
	useProgram();
	GLenum gltype=GL_FLOAT;
	bool normalize=false;
	switch (type)
	{
	case DINT:
		gltype=GL_INT;
		break;
	case DBYTE:
		gltype=GL_BYTE;
		break;
	case DUBYTE:
		gltype=GL_UNSIGNED_BYTE;
		normalize=true; //TODO check vs actual shader type to see if normalization is required
		break;
	case DSHORT:
		gltype=GL_SHORT;
		break;
	case DUSHORT:
		gltype=GL_UNSIGNED_SHORT;
		break;
	}
#ifdef GIDEROS_GL1
			glVertexPointer(mult,gltype, 0,ptr);
#else
			glVertexAttribPointer(attributes[index], mult,gltype, normalize,0, ptr
#ifdef DXCOMPAT_H
					,count,modified,(GLuint *)cache
#endif
					);
#endif

}

void oglShaderProgram::setConstant(int index,ConstantType type,const void *ptr)
{
	useProgram();
	switch (type)
	{
	case CINT:
		glUniform1i(uniforms[index],((GLint *)ptr)[0]);
		break;
	case CFLOAT:
		glUniform1f(uniforms[index],((GLfloat *)ptr)[0]);
		break;
	case CFLOAT4:
		glUniform4fv(uniforms[index],1,((GLfloat *)ptr));
		break;
	case CMATRIX:
		glUniformMatrix4fv(uniforms[index],1,false,((GLfloat *)ptr));
		break;
	}
}

oglShaderProgram::oglShaderProgram(const char *vshader1,const char *vshader2,
                 const char *fshader1, const char *fshader2,
                 const char **uniforms, const char **attributes)
{
    vertexShader=oglLoadShader(GL_VERTEX_SHADER,vshader1,vshader2);
    fragmentShader=oglLoadShader(GL_FRAGMENT_SHADER,fshader1,fshader2);
    program = oglBuildProgram(vertexShader,fragmentShader);
    while (*uniforms)
        this->uniforms.push_back(glGetUniformLocation(program, *(uniforms++)));
    while (*attributes)
        this->attributes.push_back(glGetAttribLocation(program, *(attributes++)));
}

oglShaderProgram::~oglShaderProgram()
{
    glDeleteProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

ShaderProgram *ShaderProgram::stdBasic=NULL;
ShaderProgram *ShaderProgram::stdColor=NULL;
ShaderProgram *ShaderProgram::stdTexture=NULL;
ShaderProgram *ShaderProgram::stdTextureColor=NULL;

void oglSetupShaders()
{
	glog_i("GL_VERSION:%s\n",glGetString(GL_VERSION));
	glog_i("GLSL_VERSION:%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    const char *stdUniforms[]={"vMatrix","fColor","fTexture",NULL};
    const char *stdAttributes[]={"vVertex","vColor","vTexCoord",NULL};
    ShaderProgram::stdBasic = new oglShaderProgram(hdrVShaderCode,stdVShaderCode,hdrFShaderCode,stdFShaderCode,
                                      stdUniforms,stdAttributes);
    ShaderProgram::stdColor = new oglShaderProgram(hdrVShaderCode,stdCVShaderCode,hdrFShaderCode,stdCFShaderCode,
                                      stdUniforms,stdAttributes);
    ShaderProgram::stdTexture = new oglShaderProgram(hdrVShaderCode,stdTVShaderCode,hdrFShaderCode,stdTFShaderCode,
                                      stdUniforms,stdAttributes);
    int zero=0;
    ShaderProgram::stdTexture->setConstant(2,ShaderProgram::CINT,&zero);
    ShaderProgram::stdTextureColor = new oglShaderProgram(hdrVShaderCode,stdCTVShaderCode,hdrFShaderCode,stdCTFShaderCode,
                                      stdUniforms,stdAttributes);
    ShaderProgram::stdTextureColor->setConstant(2,ShaderProgram::CINT,&zero);
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
	glUseProgram(-1);
    
    delete ShaderProgram::stdBasic;
    delete ShaderProgram::stdColor;
    delete ShaderProgram::stdTexture;
    delete ShaderProgram::stdTextureColor;
#endif
#ifdef OPENGL_ES
	glDeleteRenderbuffers(1,&_depthRenderBuffer);
#endif
}


Matrix4 setFrustum(float l, float r, float b, float t, float n, float f)
{
	Matrix4 mat;
#ifdef DXCOMPAT_H
	int df = 1, dn = 0;
#else
	int df = 1, dn = -1;
#endif
	mat[0] = 2 * n / (r - l);
	mat[5] = 2 * n / (t - b);
	mat[8] = (r + l) / (r - l);
	mat[9] = (t + b) / (t - b);
	mat[10] = -(df*f - dn*n) / (f - n);
	mat[11] = -1;
	mat[14] = -((df - dn) * f * n) / (f - n);
	mat[15] = 0;
	mat.type = Matrix4::FULL;
	return mat;
}

Matrix4 setOrthoFrustum(float l, float r, float b, float t, float n, float f)
{
	Matrix4 mat;
	mat[0] = 2 / (r - l);
	mat[5] = 2 / (t - b);
	mat[10] = -2 / (f - n);
	mat[12] = -(r + l) / (r - l);
	mat[13] = -(t + b) / (t - b);
	mat[14] = -(f + n) / (f - n);
	mat.type = Matrix4::M2D;
	return mat;
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
    matrixDirty=true;
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
    constColR=r;
    constColG=g;
    constColB=b;
    constColA=a;
    colorDirty=true;
#endif
}

void oglShaderProgram::drawArrays(ShapeType shape, int first, unsigned int count)
{
   	setConstant(ShaderProgram::ConstantMatrix,ShaderProgram::CMATRIX,oglCombined.data());
   	float constCol[4]={constColR,constColG,constColB,constColA};
   	setConstant(ShaderProgram::ConstantColor,ShaderProgram::CFLOAT4,constCol);
    activate();
    GLenum mode=GL_POINTS;
    switch (shape)
    {
    case Lines: mode=GL_LINES; break;
    case LineLoop: mode=GL_LINE_LOOP; break;
    case Triangles: mode=GL_TRIANGLES; break;
    case TriangleFan: mode=GL_TRIANGLE_FAN; break;
    case TriangleStrip: mode=GL_TRIANGLE_STRIP; break;
    }
	glDrawArrays(mode, first, count);

}
void oglShaderProgram::drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, BufferCache *cache)
{
   	setConstant(ShaderProgram::ConstantMatrix,ShaderProgram::CMATRIX,oglCombined.data());
   	float constCol[4]={constColR,constColG,constColB,constColA};
   	setConstant(ShaderProgram::ConstantColor,ShaderProgram::CFLOAT4,constCol);
    activate();

    GLenum mode=GL_POINTS;
    switch (shape)
    {
    case Lines: mode=GL_LINES; break;
    case LineLoop: mode=GL_LINE_LOOP; break;
    case Triangles: mode=GL_TRIANGLES; break;
    case TriangleFan: mode=GL_TRIANGLE_FAN; break;
    case TriangleStrip: mode=GL_TRIANGLE_STRIP; break;
    }

    GLenum dtype=GL_INT;
    switch (type)
    {
    case DBYTE: dtype=GL_BYTE; break;
    case DUBYTE: dtype=GL_UNSIGNED_BYTE; break;
    case DSHORT: dtype=GL_SHORT; break;
    case DUSHORT: dtype=GL_UNSIGNED_SHORT; break;
    }
	glDrawElements(mode, count, dtype, indices
#ifdef DXCOMPAT_H
		, modified, (GLuint *)cache
#endif
			);
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
    if (!oglInitialized) return;
	s_texture = 0;
	s_Texture2DEnabled = false;
	s_depthEnable=0;
	s_depthBufferCleared=false;

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
	resetBindTextureCount();
	resetClientStateCount();
	resetTexture2DStateCount();
#endif

#ifdef GIDEROS_GL1
	glDisable(GL_TEXTURE_2D);
#endif
	oglColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, 0);
	current=NULL;

    oglProjection.identity();
    oglVPProjection.identity();
    oglModel.identity();
    oglCombined.identity();

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
