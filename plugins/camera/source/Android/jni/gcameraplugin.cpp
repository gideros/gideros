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

static void GetJStringContent(JNIEnv *AEnv, jstring AStr, std::string &ARes) {
  if (!AStr) {
    ARes.clear();
    return;
  }

  const char *s = AEnv->GetStringUTFChars(AStr,NULL);
  ARes=s;
  AEnv->ReleaseStringUTFChars(AStr,s);
}

void GetJIntArrayContent(JNIEnv *AEnv, jobject AArr, std::vector<int> &ARes)
{
	  if (!AArr) {
	    ARes.clear();
	    return;
	  }

		jboolean isCopy;
		jsize acnt=AEnv->GetArrayLength((jintArray)AArr);
		jint *rvals = AEnv->GetIntArrayElements((jintArray)AArr, &isCopy);
		for (int k=0;k<acnt;k++)
			ARes.push_back(rvals[k]);
		AEnv->ReleaseIntArrayElements((jintArray)AArr, rvals, 0);
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
		 { "rMatrix", ShaderProgram::CMATRIX, 1, ShaderProgram::SysConst_None,
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

static g_id gid = g_NextId();

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

	std::vector<cameraplugin::CameraDesc> availableDevices()
	{
		std::vector<cameraplugin::CameraDesc> cams;
		JNIEnv *env = g_getJNIEnv();
		jobject	jcams=env->CallStaticObjectMethod(cls_,
				env->GetStaticMethodID(cls_, "availableDevices", "()[Lcom/giderosmobile/android/plugins/camera/GCamera$CamInfo;"));
		jobjectArray *arr = reinterpret_cast<jobjectArray*>(&jcams);
		jsize asize=env->GetArrayLength(*arr);
		jclass ccls = env->FindClass(
				"com/giderosmobile/android/plugins/camera/GCamera$CamInfo");
		jfieldID name=env->GetFieldID(ccls,"name","Ljava/lang/String;");
		jfieldID desc=env->GetFieldID(ccls,"description","Ljava/lang/String;");
		jfieldID pos=env->GetFieldID(ccls,"position","I");
		for (int i=0;i<asize;i++)
		{
			cameraplugin::CameraDesc c;
			jobject data=env->GetObjectArrayElement(*arr,i);
			GetJStringContent(env,(jstring)env->GetObjectField(data,name),c.name);
			GetJStringContent(env,(jstring)env->GetObjectField(data,desc),c.description);
			int cp= env->GetIntField(data,pos);
			switch (cp) {
			case 0: c.pos=cameraplugin::CameraDesc::POS_UNKNOWN; break;
			case 1: c.pos=cameraplugin::CameraDesc::POS_FRONTFACING; break;
			case 2: c.pos=cameraplugin::CameraDesc::POS_BACKFACING; break;
			}
			cams.push_back(c);
		}
		return cams;
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

	bool setFlash(int mode) {
		JNIEnv *env = g_getJNIEnv();
		return env->CallStaticBooleanMethod(cls_,
				env->GetStaticMethodID(cls_, "setFlash", "(I)Z"),mode);
	}

	bool takePicture() {
		JNIEnv *env = g_getJNIEnv();
		return env->CallStaticBooleanMethod(cls_,
				env->GetStaticMethodID(cls_, "takePicture", "()Z"));
	}

	void setOrientation(int angle) {
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_,
				env->GetStaticMethodID(cls_, "setOrientation", "(I)V"),angle);
	}

	cameraplugin::CameraInfo queryCamera(const char *device, int orientation)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jdev=device?env->NewStringUTF(device):NULL;
		jobject	data=env->CallStaticObjectMethod(cls_,
				env->GetStaticMethodID(cls_, "queryCamera", "(Ljava/lang/String;I)Lcom/giderosmobile/android/plugins/camera/GCamera$CamCaps;"),
				jdev,orientation
				);

		jclass ccls = env->FindClass(
				"com/giderosmobile/android/plugins/camera/GCamera$CamCaps");

		cameraplugin::CameraInfo c;
		GetJIntArrayContent(env,(jobject)env->GetObjectField(data,env->GetFieldID(ccls,"previewSizes","[I")),c.previewSizes);
		GetJIntArrayContent(env,(jobject)env->GetObjectField(data,env->GetFieldID(ccls,"pictureSizes","[I")),c.pictureSizes);
		c.angle= env->GetIntField(data,env->GetFieldID(ccls,"angle","I"));
		GetJIntArrayContent(env,(jobject)env->GetObjectField(data,env->GetFieldID(ccls,"flashModes","[I")),c.flashModes);
		return c;
	}

	void start(TextureData *texture,int orientation,int *camwidth,int *camheight,const char *device,int *picWidth,int *picHeight) {
		tex = texture;
		rdrTgt = ShaderEngine::Engine->createRenderTarget(tex->id());
		vertices[0] = Point2f(0, 0);
		vertices[1] = Point2f(tex->width, 0);
		vertices[2] = Point2f(tex->width, tex->height);
		vertices[3] = Point2f(0, tex->height);
		vertices.Update();
		JNIEnv *env = g_getJNIEnv();
		jstring jdev=device?env->NewStringUTF(device):NULL;
		jintArray ret=(jintArray) env->CallStaticObjectMethod(cls_,
				env->GetStaticMethodID(cls_, "start", "(IIILjava/lang/String;II)[I"),tex->width, tex->height,orientation,jdev,*picWidth,*picHeight);
		if (!ret) { //Shouldn't happen really, but JNI reports it happened
			*camwidth=0;
			*camheight=0;
			*picWidth=0;
			*picHeight=0;
			delete rdrTgt;
			return;
		}
		jboolean isCopy;
		jint *rvals = env->GetIntArrayElements(ret, &isCopy);
		*camwidth=rvals[0];
		*camheight=rvals[1];
		*picWidth=rvals[4];
		*picHeight=rvals[5];
		if ((*camwidth==0)&&(*camheight==0)) {
			delete rdrTgt;
			return;
		}
	    int x0=0;
	    int x1=1;
	    if (rvals[3]) { x0=1; x1=0; }
	    switch (rvals[2])
		{
		        case 0:
		            texcoords[0] = Point2f(x0, 0);
		            texcoords[1] = Point2f(x1, 0);
		            texcoords[2] = Point2f(x1, 1);
		            texcoords[3] = Point2f(x0, 1);
		            break;
		        case 90:
		            texcoords[0] = Point2f(x1, 0);
		            texcoords[1] = Point2f(x1, 1);
		            texcoords[2] = Point2f(x0, 1);
		            texcoords[3] = Point2f(x0, 0);
		            break;
		        case 180:
		            texcoords[0] = Point2f(x1, 1);
		            texcoords[1] = Point2f(x0, 1);
		            texcoords[2] = Point2f(x0, 0);
		            texcoords[3] = Point2f(x1, 0);
		            break;
		        case 270:
		            texcoords[0] = Point2f(x0, 1);
		            texcoords[1] = Point2f(x0, 0);
		            texcoords[2] = Point2f(x1, 0);
		            texcoords[3] = Point2f(x1, 1);
		            break;
		}
		texcoords.Update();
		env->ReleaseIntArrayElements(ret, rvals, 0);

		running=true;
	}

	void nativeEvent(int code, char *data, int size) {
		char *event=(char *)malloc(sizeof(int)+size);
		memcpy(event+sizeof(int),data,size);
		*((int *)event)=size;
		gevent_EnqueueEvent(gid, cameraplugin::callback_s, code, event, 1, NULL);
	}

	void nativeRender(int camtex, float *mat) {
		if (!running) return;
		ShaderEngine::Engine->reset();
		ShaderBuffer *oldfbo = ShaderEngine::Engine->setFramebuffer(rdrTgt);
		ShaderEngine::Engine->setViewport(0, 0, tex->width, tex->height);
		Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0,
				tex->baseWidth, tex->baseHeight, 0, -1, 1, true);
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
std::vector<cameraplugin::CameraDesc> cameraplugin::availableDevices()
{
	if (s_gcamera)
		return s_gcamera->availableDevices();
	std::vector<cameraplugin::CameraDesc> cams;
	return cams;
}

bool cameraplugin::isAvailable() {
    if (!s_gcamera) return false;
    return s_gcamera->isCameraAvailable();
}

void cameraplugin::start(Orientation orientation,int *camwidth,int *camheight,const char *device,int *picWidth,int *picHeight) {
	int o=0;
	switch (orientation)
	{
	case ePortrait:
	case eFixed:
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
	s_gcamera->start(cameraplugin::cameraTexture->data,o,camwidth,camheight,device,picWidth,picHeight);
}

void cameraplugin::stop() {
	if (s_gcamera)
		s_gcamera->stop();
}

bool cameraplugin::setFlash(int mode) {
	if (s_gcamera)
		return s_gcamera->setFlash(mode);
	return false;
}

bool cameraplugin::takePicture() {
	if (s_gcamera)
		return s_gcamera->takePicture();
	return false;
}

void cameraplugin::setOrientation(Orientation orientation) {
	int o=0;
	switch (orientation)
	{
	case ePortrait:
	case eFixed:
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
	if (s_gcamera) s_gcamera->setOrientation(o);
}

cameraplugin::CameraInfo cameraplugin::queyCamera(const char *device, Orientation orientation)
{
	int o=0;
	switch (orientation)
	{
	case ePortrait:
	case eFixed:
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
	if (s_gcamera)
		return s_gcamera->queryCamera(device,o);
	cameraplugin::CameraInfo dummy;
	return dummy;
}

extern "C" {

void Java_com_giderosmobile_android_plugins_camera_GCamera_nativeRender(
		JNIEnv *env, jclass clz, jint camtex, jfloatArray mat) {
	if (s_gcamera) {
		jboolean isCopy;
		jfloat *fmat = env->GetFloatArrayElements(mat, &isCopy);
		s_gcamera->nativeRender((int) camtex, fmat);
		env->ReleaseFloatArrayElements(mat,fmat,0);
	}
}

void Java_com_giderosmobile_android_plugins_camera_GCamera_nativeEvent(
		JNIEnv *env, jclass clz, jint code, jbyteArray data) {
	if (s_gcamera) {
		if (data) {
			jsize fsz = env->GetArrayLength(data);
			jboolean isCopy;
			jbyte *fdata = env->GetByteArrayElements(data, &isCopy);
			s_gcamera->nativeEvent((int) code, (char *)fdata,fsz);
			env->ReleaseByteArrayElements(data,fdata,0);
		}
		else
			s_gcamera->nativeEvent((int) code, NULL, 0);
	}
}

}
