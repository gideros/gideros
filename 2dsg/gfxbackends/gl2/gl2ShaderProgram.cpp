/*
 * gl2ShaderProgram.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "gl2Shaders.h"
#include "glog.h"
#include <set>

class gl2ShaderBufferCache : public ShaderBufferCache {
public:
	GLuint VBO;
	gl2ShaderBufferCache()
	{
		VBO = 0;
		if (!allVBO)
			allVBO=new std::set<gl2ShaderBufferCache*>();
		allVBO->insert(this);
	}
	virtual ~gl2ShaderBufferCache()
	{
		if (VBO)
			glDeleteBuffers(1,&VBO);
		allVBO->erase(this);
		if (allVBO->empty())
		{
			delete allVBO;
			allVBO=NULL;
		}
	}
	void recreate()
	{
		if (VBO)
			glDeleteBuffers(1,&VBO);
		VBO=0;
	}
	bool valid()
	{
		return (VBO!=0);
	}
	static std::set<gl2ShaderBufferCache *> *allVBO;
};

std::set<gl2ShaderBufferCache *> *gl2ShaderBufferCache::allVBO=NULL;

GLuint getCachedVBO(ShaderBufferCache **cache,bool &modified) {
	if (!cache) return 0; //XXX: Could we check for VBO availability ?
	if (!*cache)
		*cache = new gl2ShaderBufferCache();
	gl2ShaderBufferCache *dc = static_cast<gl2ShaderBufferCache*> (*cache);
	if (!dc->valid())
	{
		glGenBuffers(1,&dc->VBO);
		modified=true;
	}
	return dc->VBO;
}

GLint ogl2ShaderProgram::curProg =0;
ShaderProgram *ogl2ShaderProgram::current = NULL;
std::vector<ogl2ShaderProgram *> ogl2ShaderProgram::shaders;

void ogl2ShaderProgram::resetAll()
{
	  for (std::vector<ogl2ShaderProgram *>::iterator it = shaders.begin() ; it != shaders.end(); ++it)
		  (*it)->recreate();
	  if (gl2ShaderBufferCache::allVBO)
		  for (std::set<gl2ShaderBufferCache *>::iterator it = gl2ShaderBufferCache::allVBO->begin() ; it != gl2ShaderBufferCache::allVBO->end(); ++it)
			  (*it)->recreate();
}


const char *hdrShaderCode=
#ifdef OPENGL_ES
    "#version 100\n"
    "#define GLES2\n";
#else
    "#version 120\n"
    "#define highp\n"
    "#define mediump\n"
    "#define lowp\n";
#endif


GLuint ogl2LoadShader(GLuint type, const char *code, std::string &log) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		if (maxLength > 0) {
			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
			log.append((type==GL_FRAGMENT_SHADER)?"FragmentShader:\n":"VertexShader:\n");
			log.append(&infoLog[0]);
			log.append("\n");
			glog_e("Shader Compile: %s\n", &infoLog[0]);
		}
		glDeleteShader(shader);
		shader = 0;
	}
	//glog_i("Loaded shader:%d\n", shader);
	return shader;
}

GLuint ogl2BuildProgram(GLuint vertexShader, GLuint fragmentShader, std::string log) {
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glBindAttribLocation(program, 0, "vVertex"); //Ensure vertex is at 0
	glLinkProgram(program);

	GLint maxLength = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
	if (maxLength > 0) {
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
		log.append("Shader Program:\n");
		log.append(&infoLog[0]);
		log.append("\n");
		glog_v("GL Program log:%s\n", &infoLog[0]);
	}
	//glog_i("Loaded program:%d", program);
	return program;
}

bool ogl2ShaderProgram::isValid()
{
	return vertexShader&&fragmentShader&&program;
}

const char *ogl2ShaderProgram::compilationLog()
{
	return errorLog.c_str();
}

void ogl2ShaderProgram::deactivate() {
	for (std::vector<GLint>::iterator it = glattributes.begin();
			it != glattributes.end(); ++it) {
		GLint att = *it;
		if (att >= 0)
			glDisableVertexAttribArray(*it);
	}
	current = NULL;
}

void ogl2ShaderProgram::activate() {
	useProgram();
	if (current == this)
		return;
	if (current)
		current->deactivate();
	current = this;
	for (std::vector<GLint>::iterator it = glattributes.begin();
			it != glattributes.end(); ++it) {
		GLint att = *it;
		if (att >= 0)
			glEnableVertexAttribArray(*it);
	}

}

void ogl2ShaderProgram::useProgram() {
	if (curProg != program) {
		glUseProgram(program);
		curProg = program;
	}
}

void ogl2ShaderProgram::setData(int index, DataType type, int mult,
		const void *ptr, unsigned int count, bool modified,
		ShaderBufferCache **cache,int stride,int offset) {
	useProgram();
	GLenum gltype = GL_FLOAT;
	bool normalize = false;
	int elmSize = 1;
	switch (type) {
	case DINT:
		gltype = GL_INT;
		elmSize = 4;
		break;
	case DBYTE:
		gltype = GL_BYTE;
		break;
	case DUBYTE:
		gltype = GL_UNSIGNED_BYTE;
		normalize = true; //TODO check vs actual shader type to see if normalization is required
		break;
	case DSHORT:
		gltype = GL_SHORT;
		elmSize = 2;
		break;
	case DUSHORT:
		gltype = GL_UNSIGNED_SHORT;
		elmSize = 2;
		break;
	case DFLOAT:
		gltype = GL_FLOAT;
		elmSize = 4;
		break;
	}
#ifdef GIDEROS_GL1
	glVertexPointer(mult,gltype, stride, ((char *)ptr)+offset);
#else
	GLuint vbo=getCachedVBO(cache,modified);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	if (vbo)
	{
		if (modified)
			glBufferData(GL_ARRAY_BUFFER,elmSize * mult * count,ptr,GL_DYNAMIC_DRAW);
		ptr=NULL;
	}
	if ((index<glattributes.size())&&(glattributes[index]>=0))
	glVertexAttribPointer(glattributes[index], mult, gltype, normalize, stride, ((char *)ptr)+offset);
#endif

}

void ogl2ShaderProgram::setConstant(int index, ConstantType type, int mult,
		const void *ptr) {
	if ((!updateConstant(index, type, mult, ptr))&&(!(uninit_uniforms&(1<<index))))
		return;
	useProgram();
	uninit_uniforms&=~(1<<index);
	switch (type) {
	case CINT:
	case CTEXTURE:
		glUniform1iv(gluniforms[index], mult,((GLint *) ptr));
		break;
	case CFLOAT:
		glUniform1fv(gluniforms[index],mult, ((GLfloat *) ptr));
		break;
	case CFLOAT2:
		glUniform2fv(gluniforms[index], mult, ((GLfloat *) ptr));
		break;
	case CFLOAT3:
		glUniform3fv(gluniforms[index], mult, ((GLfloat *) ptr));
		break;
	case CFLOAT4:
		glUniform4fv(gluniforms[index], mult, ((GLfloat *) ptr));
		break;
	case CMATRIX:
		glUniformMatrix4fv(gluniforms[index], mult, false, ((GLfloat *) ptr));
		break;
	}
	/*
#ifdef GIDEROS_GL1
	glColor4f(r,g,b,a);
#endif
*/
}

ogl2ShaderProgram::ogl2ShaderProgram(const char *vshader, const char *fshader,int flags,
		const ConstantDesc *uniforms, const DataDesc *attributes) {
	bool fromCode=(flags&ShaderProgram::Flag_FromCode);
	void *vs = fromCode?(void *)vshader:LoadShaderFile(vshader, "glsl", NULL);
	void *fs = fromCode?(void *)fshader:LoadShaderFile(fshader, "glsl", NULL);
	const char *hdr=(flags&ShaderProgram::Flag_NoDefaultHeader)?"":hdrShaderCode;
	program=0;
	if (vs&&fs)
		buildProgram(hdr,(char *) vs, hdr, (char *) fs, uniforms, attributes);
	else if (vs==NULL)
		errorLog="Vertex shader code not found";
	else
		errorLog="Fragment shader code not found";
	if (!fromCode)
	{
		if (vs) free(vs);
		if (fs) free(fs);
	}
	shaders.push_back(this);
}

ogl2ShaderProgram::ogl2ShaderProgram(const char *vshader1, const char *vshader2,
		const char *fshader1, const char *fshader2,
		const ConstantDesc *uniforms, const DataDesc *attributes) {
	program=0;
	buildProgram(vshader1, vshader2, fshader1, fshader2, uniforms, attributes);
	shaders.push_back(this);
}

void ogl2ShaderProgram::buildProgram(const char *vshader1, const char *vshader2,
		const char *fshader1, const char *fshader2,
		const ConstantDesc *uniforms, const DataDesc *attributes) {
	cbsData=0;
    vshadercode=vshader1;
    if (vshader2)
    	vshadercode.append(vshader2);
    fshadercode=fshader1;
    if (fshader2)
    	fshadercode.append(fshader2);
	GLint ntex = 0;
	while (!uniforms->name.empty()) {
		int usz = 0, ual = 4;
		ConstantDesc cd;
		cd = *(uniforms++);
		switch (cd.type) {
		case CTEXTURE:
			usz = 4;
			ual = 4;
			break;
		case CINT:
			usz = 4;
			ual = 4;
			break;
		case CFLOAT:
			usz = 4;
			ual = 4;
			break;
		case CFLOAT2:
			usz = 8;
			ual = 4;
			break;
		case CFLOAT3:
			usz = 12;
			ual = 4;
			break;
		case CFLOAT4:
			usz = 16;
			ual = 16;
			break;
		case CMATRIX:
			usz = 64;
			ual = 16;
			break;
		}
		if (cbsData & (ual - 1))
			cbsData += ual - (cbsData & (ual - 1));
		cd.offset = cbsData;
		cbsData += usz*cd.mult;
		this->uniforms.push_back(cd);
	}
	cbData = malloc(cbsData);
	for (int iu = 0; iu < this->uniforms.size(); iu++)
		this->uniforms[iu]._localPtr = ((char *) cbData)
				+ this->uniforms[iu].offset;

	while (!attributes->name.empty()) {
		this->attributes.push_back(*attributes);
		attributes++;
	}
	recreate();
	shaderInitialized();
}

void ogl2ShaderProgram::recreate() {
	errorLog="";
    uninit_uniforms=-1;
    if (glIsProgram(program))
    {
    	if (current==this)
    		deactivate();
    	if (curProg == program) {
    		glUseProgram(0);
    		curProg = 0;
    	}
    	glDetachShader(program, vertexShader);
    	glDetachShader(program, fragmentShader);
    	glDeleteShader(vertexShader);
    	glDeleteShader(fragmentShader);
    	glDeleteProgram(program);
    }
	vertexShader = ogl2LoadShader(GL_VERTEX_SHADER, vshadercode.c_str(),errorLog);
	fragmentShader = ogl2LoadShader(GL_FRAGMENT_SHADER, fshadercode.c_str(),errorLog);
	program = ogl2BuildProgram(vertexShader, fragmentShader,errorLog);
	gluniforms.clear();
	glattributes.clear();
	glUseProgram(program);
	GLint ntex = 0;
	for (int k=0;k<uniforms.size();k++) {
		ConstantDesc cd=uniforms[k];
		this->gluniforms.push_back(glGetUniformLocation(program, cd.name.c_str()));
		switch (cd.type) {
		case CTEXTURE:
			glUniform1i(gluniforms[gluniforms.size() - 1], ntex++);
			break;
		}
	}
	for (int k=0;k<attributes.size();k++) {
		glattributes.push_back(glGetAttribLocation(program, attributes[k].name.c_str()));
	}
}

ogl2ShaderProgram::~ogl2ShaderProgram() {
	 for (std::vector<ogl2ShaderProgram *>::iterator it = shaders.begin() ; it != shaders.end(); )
		if (*it==this)
			it=shaders.erase(it);
		else
			it++;

	if (current==this)
		deactivate();
	if (curProg == program) {
		glUseProgram(0);
		curProg = 0;
	}
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);
	glog_i("Deleted program:%d", program);
	free(cbData);
}

void ogl2ShaderProgram::drawArrays(ShapeType shape, int first,
		unsigned int count) {
	ShaderEngine::Engine->prepareDraw(this);
	activate();
	GLenum mode = GL_POINTS;
	switch (shape) {
	case Point:
		mode = GL_POINTS;
		break;
	case Lines:
		mode = GL_LINES;
		break;
	case LineLoop:
		mode = GL_LINE_LOOP;
		break;
	case Triangles:
		mode = GL_TRIANGLES;
		break;
	case TriangleFan:
		mode = GL_TRIANGLE_FAN;
		break;
	case TriangleStrip:
		mode = GL_TRIANGLE_STRIP;
		break;
	}
	glDrawArrays(mode, first, count);

}
void ogl2ShaderProgram::drawElements(ShapeType shape, unsigned int count,
		DataType type, const void *indices, bool modified, ShaderBufferCache **cache,unsigned int first,unsigned int dcount) {
	ShaderEngine::Engine->prepareDraw(this);
	activate();

	GLenum mode = GL_POINTS;
	switch (shape) {
	case Point:
		mode = GL_POINTS;
		break;
	case Lines:
		mode = GL_LINES;
		break;
	case LineLoop:
		mode = GL_LINE_LOOP;
		break;
	case Triangles:
		mode = GL_TRIANGLES;
		break;
	case TriangleFan:
		mode = GL_TRIANGLE_FAN;
		break;
	case TriangleStrip:
		mode = GL_TRIANGLE_STRIP;
		break;
	}

	GLenum dtype = GL_INT;
	int elmSize=1;
	switch (type) {
	case DFLOAT:
		dtype = GL_FLOAT;
		elmSize=4;
		break;
	case DBYTE:
		dtype = GL_BYTE;
		break;
	case DUBYTE:
		dtype = GL_UNSIGNED_BYTE;
		break;
	case DSHORT:
		dtype = GL_SHORT;
		elmSize=2;
		break;
	case DUSHORT:
		dtype = GL_UNSIGNED_SHORT;
		elmSize=2;
		break;
	case DINT:
		dtype = GL_INT;
		elmSize=4;
		break;
	}
	GLuint vbo=getCachedVBO(cache,modified);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo);
	if (vbo)
	{
		if (modified)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,elmSize * count,indices,GL_DYNAMIC_DRAW);
		indices=NULL;
	}
	glDrawElements(mode, dcount?dcount:count, dtype, ((char *)indices)+elmSize*first);
}
