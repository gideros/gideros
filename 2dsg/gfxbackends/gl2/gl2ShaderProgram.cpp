/*
 * gl2ShaderProgram.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "gl2Shaders.h"
#include "glog.h"
#include <set>

#define FBO_MINSIZE 192
class gl2ShaderBufferCache : public ShaderBufferCache {
public:
	GLuint VBO;
	int keptCounter;
	gl2ShaderBufferCache()
	{
		VBO = 0;
		keptCounter=0;
		if (!allVBO)
			allVBO=new std::set<gl2ShaderBufferCache*>();
		allVBO->insert(this);
	}
	virtual ~gl2ShaderBufferCache()
	{
		if (VBO)
        {
            GLCALL_CHECK;
            GLCALL_INIT;
            GLCALL glDeleteBuffers(1,&VBO);
        }
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
        {
            GLCALL_INIT;
            GLCALL glDeleteBuffers(1,&VBO);
        }
		VBO=0;
	}
	bool valid()
	{
		return (VBO!=0);
	}
	static std::set<gl2ShaderBufferCache *> *allVBO;
};

std::set<gl2ShaderBufferCache *> *gl2ShaderBufferCache::allVBO=NULL;
int ogl2ShaderProgram::vboFreeze=0;
int ogl2ShaderProgram::vboUnfreeze=0;
bool ogl2ShaderProgram::supportInstances=0;
bool ogl2ShaderProgram::vboForceGeneric=false;
#ifndef GL2SHADERS_COMMON_GENVBO
GLuint ogl2ShaderProgram::genVBO[17]={0,};
#else
std::vector<GLuint> ogl2ShaderProgram::genVBOs;
std::vector<GLuint> ogl2ShaderProgram::freeVBOs;
std::vector<GLuint> ogl2ShaderProgram::usedVBOs;
std::vector<GLuint> ogl2ShaderProgram::renderedVBOs;
GLuint ogl2ShaderProgram::curGenVBO=0;
size_t ogl2ShaderProgram::genBufferOffset=0;
#endif
GLuint ogl2ShaderProgram::curAttribs[16]={0,};
GLuint ogl2ShaderProgram::curArrayBuffer=0;
GLuint ogl2ShaderProgram::curElementBuffer=0;

void ogl2ShaderProgram::bindBuffer(GLuint type,GLuint buf)
{
    GLCALL_INIT;
    switch (type) {
    case GL_ELEMENT_ARRAY_BUFFER:
        if (curElementBuffer==buf) return;
        curElementBuffer=buf;
    break;
    case GL_ARRAY_BUFFER:
        if (curArrayBuffer==buf) return;
        curArrayBuffer=buf;
    break;
    }
    GLCALL glBindBuffer(type,buf);
}

GLuint ogl2ShaderProgram::allocateVBO() {
    if (freeVBOs.empty()) {
        GLuint vbos[8];
        GLCALL_INIT;
        GLCALL glGenBuffers(8,vbos);
        for (int i=0;i<8;i++)
            genVBOs.push_back(vbos[i]);
        for (int i=0;i<7;i++)
            freeVBOs.push_back(vbos[i]);
        usedVBOs.push_back(vbos[7]);
        return vbos[7];
    }
    GLuint vbo=freeVBOs.back();
    freeVBOs.pop_back();
    usedVBOs.push_back(vbo);
    return vbo;
}

#define IDXBUFSIZE  65536
GLuint ogl2ShaderProgram::getGenericVBO(int index,int size, const void *&ptr) {
    GLCALL_INIT;
    GLuint bname=(index==0)?GL_ELEMENT_ARRAY_BUFFER:GL_ARRAY_BUFFER;
#ifndef GL2SHADERS_COMMON_GENVBO
    //With this enabled, we can mix VBO/Client memory
    if ((!vboForceGeneric)&&(size<FBO_MINSIZE)) {
        bindBuffer(bname,0);
        return 0;
    }
    if (genVBO[index] == 0){
		GLCALL glGenBuffers(1,genVBO+index);
	}
    bindBuffer(bname,genVBO[index]);
    return genVBO[index];
#else
    index=0;
    size_t psize=((size+3)&(~3));
    if (psize>(IDXBUFSIZE/2))
    {
        GLuint vbo=allocateVBO();
        bindBuffer(bname,vbo);
        GLCALL glBufferData(bname,psize,ptr,GL_DYNAMIC_DRAW);
        genBufferOffset=IDXBUFSIZE;
        ptr=NULL;
    }
    else if ((curGenVBO==0)||((genBufferOffset+psize)>IDXBUFSIZE))
    {
        curGenVBO=allocateVBO();
        bindBuffer(bname,curGenVBO);
        GLCALL glBufferData(bname,IDXBUFSIZE,NULL,GL_DYNAMIC_DRAW);
        GLCALL glBufferSubData(bname,0,size,ptr);
        ptr=NULL;
        genBufferOffset=0;
    }
    else {
        bindBuffer(bname,curGenVBO);
        GLCALL glBufferSubData(bname,genBufferOffset,size,ptr);
        ptr=(void *)genBufferOffset;
    }
    genBufferOffset+=psize;
#endif
    return 0;
}

GLuint ogl2ShaderProgram::getCachedVBO(ShaderBufferCache **cache,bool &modified,GLuint type) {
    GLCALL_INIT;
    if (!cache) {
        bindBuffer(type,0);
        return 0; //XXX: Could we check for VBO availability ?
    }
	if (!*cache)
		*cache = new gl2ShaderBufferCache();
	gl2ShaderBufferCache *dc = static_cast<gl2ShaderBufferCache*> (*cache);
	bool useVBO=dc->valid();
#ifdef EMSCRIPTEN
	useVBO=true;
#else
	if (ogl2ShaderProgram::vboFreeze>1)
	{
		if (modified)
		{
			dc->keptCounter=(dc->keptCounter>0)?0:dc->keptCounter-1;
			if (dc->keptCounter<=(-ogl2ShaderProgram::vboUnfreeze))
			{
				useVBO=false;
				dc->keptCounter=-ogl2ShaderProgram::vboUnfreeze;
			}
		}
		else
		{
			dc->keptCounter=(dc->keptCounter>0)?dc->keptCounter+1:1;
			if (dc->keptCounter>=(ogl2ShaderProgram::vboFreeze))
			{
				useVBO=false;
				dc->keptCounter=ogl2ShaderProgram::vboFreeze;
			}
		}
	}
	else if (ogl2ShaderProgram::vboFreeze==1)
		useVBO=true;
	else
		useVBO=false;
#endif
	if (useVBO)
	{
		if (!dc->valid())
		{
			GLCALL glGenBuffers(1,&dc->VBO);
			modified=true;
		}
	}
	else
		dc->recreate();
    bindBuffer(type,dc->VBO);
    return dc->VBO;
}

GLuint ogl2ShaderProgram::curProg =0;
ShaderProgram *ogl2ShaderProgram::current = NULL;
std::vector<ogl2ShaderProgram *> ogl2ShaderProgram::shaders;

void ogl2ShaderProgram::resetAll()
{
	  for (std::vector<ogl2ShaderProgram *>::iterator it = shaders.begin() ; it != shaders.end(); ++it)
		  (*it)->recreate();
	  if (gl2ShaderBufferCache::allVBO)
		  for (std::set<gl2ShaderBufferCache *>::iterator it = gl2ShaderBufferCache::allVBO->begin() ; it != gl2ShaderBufferCache::allVBO->end(); ++it)
			  (*it)->recreate();
      GLCALL_INIT;
      GLCALL glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
      GLCALL glBindBuffer(GL_ARRAY_BUFFER,0);
      curProg=0;
      curArrayBuffer=curElementBuffer=0;
      for (int i=0;i<16;i++)
      {
        curAttribs[i]=(GLuint)-1;
        GLCALL glDisableVertexAttribArray(i);
      }
}

void ogl2ShaderProgram::resetAllUniforms()
{
#ifndef GL2SHADERS_COMMON_GENVBO
    GLCALL_INIT;
    int nvbo=17;
    for (int k = 0; k < nvbo; k++) {
        if (genVBO[k]) {
            GLCALL glDeleteBuffers(1,genVBO+k);
            genVBO[k]=0;
        }
    }
#else
    freeVBOs.insert(freeVBOs.end(),renderedVBOs.begin(),renderedVBOs.end());
    renderedVBOs.clear();
    renderedVBOs.assign(usedVBOs.begin(),usedVBOs.end());
    usedVBOs.clear();
    curGenVBO=0;
#endif
    for (std::vector<ogl2ShaderProgram *>::iterator it = shaders.begin() ; it != shaders.end(); ++it)
        (*it)->resetUniforms();
}


const char *hdrShaderCode_DK=
    "#version 120\n"
    "#define highp\n"
    "#define mediump\n"
    "#define lowp\n";

const char *hdrShaderCode_ES=
    "#version 100\n"
    "#define GLES2\n";


GLuint ogl2LoadShader(GLuint type, const char *code, std::string &log) {
	GLCALL_INIT;
	GLuint shader = GLCALL glCreateShader(type);
	GLCALL glShaderSource(shader, 1, &code, NULL);
	GLCALL glCompileShader(shader);

	GLint isCompiled = 0;
	GLCALL glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		GLCALL glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		if (maxLength > 0) {
			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			GLCALL glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
			log.append((type==GL_FRAGMENT_SHADER)?"FragmentShader:\n":"VertexShader:\n");
			log.append(&infoLog[0]);
			log.append("\n");
			glog_e("Shader Compile: %s\n", &infoLog[0]);
		}
		GLCALL glDeleteShader(shader);
		shader = 0;
	}
	//glog_i("Loaded shader:%d\n", shader);
	return shader;
}

GLuint ogl2BuildProgram(GLuint vertexShader, GLuint fragmentShader, std::string log) {
	GLCALL_INIT;

	if (!((GLCALL glIsShader(vertexShader))&&(GLCALL glIsShader(fragmentShader))))
	{
		//Invalid shaders
		log.append("Shader Program: invalid shader(s)\n");
		return 0;
	}

	GLuint program = GLCALL glCreateProgram();
	GLCALL glAttachShader(program, vertexShader);
	GLCALL glAttachShader(program, fragmentShader);
	GLCALL glBindAttribLocation(program, 0, "vVertex"); //Ensure vertex is at 0
	GLCALL glLinkProgram(program);

	GLint maxLength = 0;
	GLCALL glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
	if (maxLength > 0) {
		std::vector<GLchar> infoLog(maxLength);
		GLCALL glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
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
    /*
	GLCALL_INIT;
	for (std::vector<GLint>::iterator it = glattributes.begin();
			it != glattributes.end(); ++it) {
		GLint att = *it;
		if (att >= 0)
            GLCALL glDisableVertexAttribArray(at);
    }*/
	current = NULL;
}

void ogl2ShaderProgram::activate() {
	GLCALL_INIT;
	GLECALL_INIT;
	useProgram();
	if (current == this)
		return;
	if (current)
		current->deactivate();
	current = this;
    GLuint catt[16];
    for (int i=0;i<16;i++) catt[i]=(GLuint)-1;
    for (size_t k=0;k<glattributes.size();k++) {
		GLint att = glattributes[k];
		if (att >= 0) {
            catt[att]=(supportInstances)?attributes[k].instances:0;
		}
	}
    for (int i=0;i<16;i++) {
        if (catt[i]!=curAttribs[i]) {
            if (catt[i]==((GLuint)-1)) {
                GLCALL glDisableVertexAttribArray(i);
                if (supportInstances&&curAttribs[i])
                    GLECALL glVertexAttribDivisor(i,0);
            }
            else {
                if (curAttribs[i]==((GLuint)-1))
                    GLCALL glEnableVertexAttribArray(i);
                if (supportInstances&&catt[i])
                    GLECALL glVertexAttribDivisor(i,catt[i]);
            }
            curAttribs[i]=catt[i];
        }
    }
}

void ogl2ShaderProgram::useProgram() {
	if (curProg != program) {
		GLCALL_INIT;
		GLCALL glUseProgram(program);
		curProg = program;
	}
}

void ogl2ShaderProgram::setData(int index, DataType type, int mult,
		const void *ptr, unsigned int count, bool modified,
		ShaderBufferCache **cache,int stride,int offset) {
	GLCALL_INIT;
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
	GLCALL glVertexPointer(mult,gltype, stride, ((char *)ptr)+offset);
#else
    size_t fbo_sz=elmSize * mult * count;
    if (fbo_sz<FBO_MINSIZE)
        cache=NULL;
    GLuint vbo=cache?getCachedVBO(cache,modified,GL_ARRAY_BUFFER):getGenericVBO(index+1,fbo_sz,ptr);
    if (vbo)
	{
		if (modified||(!cache))
            GLCALL glBufferData(GL_ARRAY_BUFFER,fbo_sz,ptr,GL_DYNAMIC_DRAW);
		ptr=NULL;
	}
	if ((index<glattributes.size())&&(glattributes[index]>=0))
		GLCALL glVertexAttribPointer(glattributes[index], mult, gltype, normalize, stride, ((char *)ptr)+offset);
#endif

}

void ogl2ShaderProgram::setConstant(int index, ConstantType type, int mult,
		const void *ptr) {
	if ((!updateConstant(index, type, mult, ptr))&&(!(uninit_uniforms&(1<<index))))
		return;
	GLCALL_INIT;
	useProgram();
	uninit_uniforms&=~(1<<index);
	switch (type) {
	case CINT:
	case CTEXTURE:
		GLCALL glUniform1iv(gluniforms[index], mult,((GLint *) ptr));
		break;
	case CFLOAT:
		GLCALL glUniform1fv(gluniforms[index],mult, ((GLfloat *) ptr));
		break;
	case CFLOAT2:
		GLCALL glUniform2fv(gluniforms[index], mult, ((GLfloat *) ptr));
		break;
	case CFLOAT3:
		GLCALL glUniform3fv(gluniforms[index], mult, ((GLfloat *) ptr));
		break;
	case CFLOAT4:
		GLCALL glUniform4fv(gluniforms[index], mult, ((GLfloat *) ptr));
		break;
	case CMATRIX:
		GLCALL glUniformMatrix4fv(gluniforms[index], mult, false, ((GLfloat *) ptr));
		break;
	}
	/*
#ifdef GIDEROS_GL1
	GLCALL glColor4f(r,g,b,a);
#endif
*/
}

ogl2ShaderProgram::ogl2ShaderProgram(const char *vshader, const char *fshader,int flags,
		const ConstantDesc *uniforms, const DataDesc *attributes,bool isGLES) {
	bool fromCode=(flags&ShaderProgram::Flag_FromCode);
	void *vs = fromCode?(void *)vshader:LoadShaderFile(vshader, "glsl", NULL);
	void *fs = fromCode?(void *)fshader:LoadShaderFile(fshader, "glsl", NULL);
	const char *hdr=(flags&ShaderProgram::Flag_NoDefaultHeader)?"":(isGLES?hdrShaderCode_ES:hdrShaderCode_DK);
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
	for (size_t iu = 0; iu < this->uniforms.size(); iu++)
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
	GLCALL_INIT;
	errorLog="";
    uninit_uniforms=-1;
    if (GLCALL glIsProgram(program))
    {
    	if (current==this)
    		deactivate();
    	if (curProg == program) {
    		GLCALL glUseProgram(0);
    		curProg = 0;
    	}
    	if (GLCALL glIsShader(vertexShader))
        	GLCALL glDetachShader(program, vertexShader);
    	if (GLCALL glIsShader(fragmentShader))
    		GLCALL glDetachShader(program, fragmentShader);
    	GLCALL glDeleteProgram(program);
    }
	if (GLCALL glIsShader(vertexShader))
    	GLCALL glDeleteShader(vertexShader);
	if (GLCALL glIsShader(fragmentShader))
		GLCALL glDeleteShader(fragmentShader);
	vertexShader = ogl2LoadShader(GL_VERTEX_SHADER, vshadercode.c_str(),errorLog);
	fragmentShader = ogl2LoadShader(GL_FRAGMENT_SHADER, fshadercode.c_str(),errorLog);
	program = ogl2BuildProgram(vertexShader, fragmentShader,errorLog);
	gluniforms.clear();
	glattributes.clear();
	if (GLCALL glIsProgram(program))
	{
		useProgram();
		GLint ntex = 0;
		for (size_t k=0;k<uniforms.size();k++) {
			ConstantDesc cd=uniforms[k];
			this->gluniforms.push_back(GLCALL glGetUniformLocation(program, cd.name.c_str()));
			switch (cd.type) {
			case CTEXTURE:
				GLCALL glUniform1i(gluniforms[gluniforms.size() - 1], ntex++);
				break;
			default:
				break;
			}
		}
		for (size_t k=0;k<attributes.size();k++) {
			glattributes.push_back(GLCALL glGetAttribLocation(program, attributes[k].name.c_str()));
		}
	}
}

void ogl2ShaderProgram::resetUniforms() {
    uninit_uniforms=-1;
}

ogl2ShaderProgram::~ogl2ShaderProgram() {
	GLCALL_INIT;
	 for (std::vector<ogl2ShaderProgram *>::iterator it = shaders.begin() ; it != shaders.end(); )
		if (*it==this)
			it=shaders.erase(it);
		else
			it++;

	if (current==this)
		deactivate();
	if (curProg == program) {
		GLCALL glUseProgram(0);
		curProg = 0;
	}
    if (GLCALL glIsProgram(program))
    {
    	if (GLCALL glIsShader(vertexShader))
        	GLCALL glDetachShader(program, vertexShader);
    	if (GLCALL glIsShader(fragmentShader))
    		GLCALL glDetachShader(program, fragmentShader);
    	GLCALL glDeleteProgram(program);
    }
	if (GLCALL glIsShader(vertexShader))
    	GLCALL glDeleteShader(vertexShader);
	if (GLCALL glIsShader(fragmentShader))
		GLCALL glDeleteShader(fragmentShader);
	glog_i("Deleted program:%d", program);
	free(cbData);
}

void ogl2ShaderProgram::drawArrays(ShapeType shape, int first,
		unsigned int count,unsigned int instances) {
	GLCALL_INIT;
	GLECALL_INIT;
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
	case TriangleStrip:
		mode = GL_TRIANGLE_STRIP;
		break;
	}
	if (instances) {
		if (supportInstances)
			GLECALL glDrawArraysInstanced(mode, first, count, instances);
		else
		{
			int ciid=getConstantByName("gl_InstanceID");
			for (size_t i=0;i<instances;i++) {
				if (ciid>=0)
					GLCALL glUniform1iv(gluniforms[ciid], 1,((GLint *) &i));
				GLCALL glDrawArrays(mode, first, count);
			}
		}
	}
	else
		GLCALL glDrawArrays(mode, first, count);

}
void ogl2ShaderProgram::drawElements(ShapeType shape, unsigned int count,
		DataType type, const void *indices, bool modified, ShaderBufferCache **cache,unsigned int first,unsigned int dcount,unsigned int instances) {
	GLCALL_INIT;
	GLECALL_INIT;
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
		dtype = GL_UNSIGNED_INT; //Indices are unsigned
		elmSize=4;
		break;
	}
    size_t fbo_sz=elmSize * count;
    if (fbo_sz<FBO_MINSIZE)
        cache=NULL;
    GLuint vbo=cache?getCachedVBO(cache,modified,GL_ELEMENT_ARRAY_BUFFER):getGenericVBO(0,fbo_sz,indices);
	if (vbo)
	{
		if (modified||(!cache))
			GLCALL glBufferData(GL_ELEMENT_ARRAY_BUFFER,elmSize * count,indices,GL_DYNAMIC_DRAW);
		indices=NULL;
	}
	if (instances) {
		if (supportInstances)
			GLECALL glDrawElementsInstanced(mode, dcount?dcount:count, dtype, ((char *)indices)+elmSize*first, instances);
		else
		{
			int ciid=getConstantByName("gl_InstanceID");
			for (size_t i=0;i<instances;i++) {
				if (ciid>=0)
					GLCALL glUniform1iv(gluniforms[ciid], 1,((GLint *) &i));
				GLCALL glDrawElements(mode, dcount?dcount:count, dtype, ((char *)indices)+elmSize*first);
			}
		}
	}
	else
		GLCALL glDrawElements(mode, dcount?dcount:count, dtype, ((char *)indices)+elmSize*first);
}
