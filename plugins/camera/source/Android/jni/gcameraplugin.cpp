#include "camerabinder.h"
#include <jni.h>
#include "Shaders.h"
#include "graphicsbase.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define GL_TEXTURE_EXTERNAL_OES 0x8D65
extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static const char *VShaderCode = "attribute highp vec3 vVertex;\n"
		"attribute mediump vec2 vTexCoord;\n"
		"uniform highp mat4 vMatrix;\n"
		"uniform highp mat4 tMatrix;\n"
		"varying highp vec2 fTexCoord;\n"
		"\n"
		"void main() {\n"
		"  highp vec4 texc = tMatrix*vec4(vTexCoord,0.0,1.0);\n"
		"  highp vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fTexCoord=texc.xy;\n"
		"}\n";
static const char *FShaderCode =
		"#extension GL_OES_EGL_image_external : require\n"
				"uniform samplerExternalOES fTexture;\n"
				"varying highp vec2 fTexCoord;\n"
				"void main() {\n"
				" gl_FragColor=texture2D(fTexture, fTexCoord);\n"
				"}\n";

static const ShaderProgram::ConstantDesc camUniforms[] =
		{ { "tMatrix", ShaderProgram::CMATRIX, 1, ShaderProgram::SysConst_None,
				true, 0, NULL },
		 { "vMatrix", ShaderProgram::CMATRIX, 1,
						ShaderProgram::SysConst_WorldViewProjectionMatrix, true,
						0, NULL }, { "fTexture", ShaderProgram::CTEXTURE, 1,
						ShaderProgram::SysConst_None, false, 0, NULL }, { "",
						ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,
						false, 0, NULL } };
static const ShaderProgram::DataDesc camAttributes[] = { { "vVertex",
		ShaderProgram::DFLOAT, 3, 0, 0 }, { "vColor", ShaderProgram::DUBYTE, 4,
		1, 0 }, { "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0 }, { "",
		ShaderProgram::DFLOAT, 0, 0, 0 } };

class GCAMERA {
	ShaderBuffer *rdrTgt;
	ShaderProgram *shader;
	TextureData *tex;
	VertexBuffer<unsigned short> indices;
	VertexBuffer<Point2f> vertices;
	VertexBuffer<Point2f> texcoords;
	bool running;
public:
	GCAMERA() {
		running=false;

		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass(
				"com/giderosmobile/android/plugins/camera/GCamera");
		cls_ = (jclass) env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);

		shader = ShaderEngine::Engine->createShaderProgram(VShaderCode,
				FShaderCode, ShaderProgram::Flag_FromCode, camUniforms,
				camAttributes);
		indices.resize(4);
		vertices.resize(4);
		texcoords.resize(4);

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 3;
		indices[3] = 2;
		indices.Update();

		texcoords[0] = Point2f(0, 0);
		texcoords[1] = Point2f(1, 0);
		texcoords[2] = Point2f(1, 1);
		texcoords[3] = Point2f(0, 1);
		texcoords.Update();

	}

	~GCAMERA() {
		stop();

		JNIEnv *env = g_getJNIEnv();
		env->DeleteGlobalRef(cls_);

		delete shader;
	}

	bool isCameraAvailable() {
		JNIEnv *env = g_getJNIEnv();
		return (bool) env->CallStaticBooleanMethod(cls_,
				env->GetStaticMethodID(cls_, "isCameraAvailable", "()Z"));
	}

	void stop() {
		if (running)
		{
		running=false;
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_,
				env->GetStaticMethodID(cls_, "stop", "()V"));
		delete rdrTgt;
		}
	}

	void start(TextureData *texture,int orientation,int *camwidth,int *camheight) {
		tex = texture;
		rdrTgt = ShaderEngine::Engine->createRenderTarget(tex->id());
		vertices[0] = Point2f(0, 0);
		vertices[1] = Point2f(tex->width, 0);
		vertices[2] = Point2f(tex->width, tex->height);
		vertices[3] = Point2f(0, tex->height);
		vertices.Update();
		switch (orientation)
		{
		case 0: //Portrait
			texcoords[3] = Point2f(1, 0);
			texcoords[0] = Point2f(0, 0);
			texcoords[1] = Point2f(0, 1);
			texcoords[2] = Point2f(1, 1);
			break;
		case 90: //Landscape left
			texcoords[0] = Point2f(1, 0);
			texcoords[1] = Point2f(0, 0);
			texcoords[2] = Point2f(0, 1);
			texcoords[3] = Point2f(1, 1);
			break;
		case 180: //Portrait upside down
			texcoords[1] = Point2f(1, 0);
			texcoords[2] = Point2f(0, 0);
			texcoords[3] = Point2f(0, 1);
			texcoords[0] = Point2f(1, 1);
			break;
		case 270: //Landscape right
			texcoords[2] = Point2f(1, 0);
			texcoords[3] = Point2f(0, 0);
			texcoords[0] = Point2f(0, 1);
			texcoords[1] = Point2f(1, 1);
			break;
		}
		texcoords.Update();

		JNIEnv *env = g_getJNIEnv();
		jintArray ret=(jintArray) env->CallStaticObjectMethod(cls_,
				env->GetStaticMethodID(cls_, "start", "(III)[I"),tex->width, tex->height,orientation);
		jboolean isCopy;
		jint *rvals = env->GetIntArrayElements(ret, &isCopy);
		*camwidth=rvals[0];
		*camheight=rvals[1];
		running=true;
	}

	void nativeRender(int camtex, float *mat) {
		if (!running) return;
		ShaderEngine::Engine->reset();
		ShaderBuffer *oldfbo = ShaderEngine::Engine->setFramebuffer(rdrTgt);
		ShaderEngine::Engine->setViewport(0, 0, tex->width, tex->height);
		Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0,
				tex->baseWidth, 0, tex->baseHeight, -1, 1);
		ShaderEngine::Engine->setProjection(projection);
		Matrix4 model;
		ShaderEngine::Engine->setModel(model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, camtex);

		shader->setConstant(shader->getConstantByName("tMatrix"),
				ShaderProgram::CMATRIX, 1, mat);
		shader->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,
				&vertices[0], vertices.size(), vertices.modified,
				&vertices.bufferCache);
		shader->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT, 2,
				&texcoords[0], texcoords.size(), texcoords.modified,
				&texcoords.bufferCache);
		shader->drawElements(ShaderProgram::TriangleStrip, indices.size(),
				ShaderProgram::DUSHORT, &indices[0], indices.modified,
				&indices.bufferCache);
		vertices.modified = false;
		texcoords.modified = false;
		indices.modified = false;

		ShaderEngine::Engine->setFramebuffer(oldfbo);
	}

private:
	jclass cls_;
};

static GCAMERA *s_gcamera = NULL;

void cameraplugin::init() {
	if (!s_gcamera)
		s_gcamera = new GCAMERA;
}

void cameraplugin::deinit() {
	if (s_gcamera)
	{
		s_gcamera->stop();
		delete s_gcamera;
		s_gcamera = NULL;
	}
}

void cameraplugin::start(Orientation orientation,int *camwidth,int *camheight) {
	int o=0;
	switch (orientation)
	{
	case ePortrait:
		o=0;
		break;
	case eLandscapeLeft:
		o=90;
		break;
	case ePortraitUpsideDown:
		o=180;
		break;
	case eLandscapeRight:
		o=270;
		break;
	}
	s_gcamera->start(cameraplugin::cameraTexture->data,o,camwidth,camheight);
}

void cameraplugin::stop() {
	if (s_gcamera)
		s_gcamera->stop();
}

extern "C" {

void Java_com_giderosmobile_android_plugins_camera_GCamera_nativeRender(
		JNIEnv *env, jclass clz, jint camtex, jfloatArray mat) {
	if (s_gcamera) {
		jboolean isCopy;
		jfloat *fmat = env->GetFloatArrayElements(mat, &isCopy);
		s_gcamera->nativeRender((int) camtex, fmat);
	}
}

}
