/*
 * CameraPlugin.cpp
 *
 *  Created on: 5 ao�t 2017
 *      Author: Nicolas
 */
#include <QVideoFrame>
#include <QVideoSink>
#include <QList>
#include <QCamera>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QtOpenGL>
#include <mutex>
#include "Shaders.h"
#include "graphicsbase.h"
#include "screen.h"
#include "ticker.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#define GLCALL_INIT QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
#define GLCALL glFuncs->

#include "../common/camerabinder.h"

static const char *VShaderCode = "attribute highp vec3 vVertex;\n"
		"attribute mediump vec2 vTexCoord;\n"
		"uniform highp mat4 vMatrix;\n"
		"varying highp vec2 fTexCoord;\n"
		"\n"
		"void main() {\n"
		"  fTexCoord = vTexCoord;\n"
		"  highp vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"}\n";
static const char *FShaderCode =
				"uniform lowp sampler2D fTexture;\n"
				"varying highp vec2 fTexCoord;\n"
				"void main() {\n"
				" gl_FragColor=vec4(texture2D(fTexture, fTexCoord).bgr,1.0);\n"
				"}\n";

static const ShaderProgram::ConstantDesc camUniforms[] =
		{ { "vMatrix", ShaderProgram::CMATRIX, 1,
						ShaderProgram::SysConst_WorldViewProjectionMatrix, true,
						0, NULL }, { "fTexture", ShaderProgram::CTEXTURE, 1,
						ShaderProgram::SysConst_None, false, 0, NULL }, { "",
						ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,
						false, 0, NULL } };
static const ShaderProgram::DataDesc camAttributes[] = { { "vVertex",
        ShaderProgram::DFLOAT, 3, 0, 0, 0 }, { "vColor", ShaderProgram::DUBYTE, 4,
        1, 0, 0 }, { "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0, 0 }, { "",
        ShaderProgram::DFLOAT, 0, 0, 0, 0 } };


class VideoFrameSurface : public Ticker {
	TextureData *gtex;
	ShaderBuffer *rdrTgt;
	ShaderTexture *camtex;
	ShaderProgram *shader;
	VertexBuffer<unsigned short> indices;
	VertexBuffer<Point2f> vertices;
	VertexBuffer<Point2f> texcoords;
	int cw,ch;
	std::mutex g_mutex;
	QVideoFrame g_frame;
    QVideoSink *sink;
	void tick();
public:
    VideoFrameSurface(TextureData *tex,int w,int h, int o,QVideoSink *sink);
	~VideoFrameSurface();
    void setVideoFrame(const QVideoFrame& frame);
    void render();
};

static QCamera *camera=NULL;
static VideoFrameSurface *camerasurface=NULL;
static QMediaCaptureSession *captureSession=NULL;

VideoFrameSurface::VideoFrameSurface(TextureData *tex,int w,int h, int o,QVideoSink *sink) {
	G_UNUSED(o);
    this->sink=sink;
	gtex=tex;
	cw=w;
	ch=h;
	shader = gtexture_get_engine()->createShaderProgram(VShaderCode,
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
/*
	switch (o)
	{
	case 0: //Portrait
		texcoords[3] = Point2f(0, 1);
		texcoords[0] = Point2f(0, 0);
		texcoords[1] = Point2f(1, 0);
		texcoords[2] = Point2f(1, 1);
		break;
	case 90: //Landscape left
		texcoords[0] = Point2f(0, 1);
		texcoords[1] = Point2f(0, 0);
		texcoords[2] = Point2f(1, 0);
		texcoords[3] = Point2f(1, 1);
		break;
	case 180: //Portrait upside down
		texcoords[1] = Point2f(0, 1);
		texcoords[2] = Point2f(0, 0);
		texcoords[3] = Point2f(1, 0);
		texcoords[0] = Point2f(1, 1);
		break;
	case 270: //Landscape right
		texcoords[2] = Point2f(0, 1);
		texcoords[3] = Point2f(0, 0);
		texcoords[0] = Point2f(1, 0);
		texcoords[1] = Point2f(1, 1);
		break;
	}*/
	texcoords[0] = Point2f(0, 0);
	texcoords[1] = Point2f(1, 0);
	texcoords[2] = Point2f(1, 1);
	texcoords[3] = Point2f(0, 1);
	texcoords.Update();

//#ifdef Q_OS_MACX
	vertices[0] = Point2f(0, 0);
	vertices[1] = Point2f(gtex->width, 0);
	vertices[2] = Point2f(gtex->width, gtex->height);
	vertices[3] = Point2f(0, gtex->height);
/*#else
	vertices[0] = Point2f(0, gtex->height);
	vertices[1] = Point2f(gtex->width, gtex->height);
	vertices[2] = Point2f(gtex->width, 0);
	vertices[3] = Point2f(0, 0);
#endif*/
	vertices.Update();

	rdrTgt = gtexture_get_engine()->createRenderTarget(gtexture_getInternalTexture(gtex->gid));
	camtex = gtexture_get_engine()->createTexture(ShaderTexture::FMT_RGBA,
			ShaderTexture::PK_UBYTE,w,h,NULL,
			ShaderTexture::WRAP_CLAMP,ShaderTexture::FILT_LINEAR);
}
VideoFrameSurface::~VideoFrameSurface() {
	delete camtex;
	delete rdrTgt;
	delete shader;
}

void VideoFrameSurface::render() {
   g_frame= sink->videoFrame();
    if (!g_frame.isValid()) return;
	GLCALL_INIT;
	{
		GLuint tid=*((GLuint *)camtex->getNative());
		GLCALL glBindTexture(GL_TEXTURE_2D, tid);
		//qDebug() << "Render:" << g_frame.width()<< g_frame.height() << tid;
        QImage frame=g_frame.toImage();
        frame.convertTo(QImage::Format_ARGB32);
        GLCALL glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cw, ch,0, GL_RGBA, GL_UNSIGNED_BYTE, frame.bits());
		ShaderEngine *engine=gtexture_get_engine();
		engine->reset();
		ShaderBuffer *oldfbo = engine->setFramebuffer(rdrTgt);
		engine->setViewport(0, 0, gtex->width, gtex->height);
		Matrix4 projection = engine->setOrthoFrustum(0,
				gtex->baseWidth, gtex->baseHeight, 0, -1, 1,true);
		engine->setProjection(projection);
		Matrix4 model;
		engine->setModel(model);
		engine->bindTexture(0,camtex);
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

		engine->setFramebuffer(oldfbo);
	}

}

void VideoFrameSurface::tick()
{
	g_mutex.lock();
    ScreenManager *sm=gtexture_get_screenmanager();
    if (sm) sm->screenDestroyed();
	render();
	g_mutex.unlock();
}

void VideoFrameSurface::setVideoFrame(const QVideoFrame& frame)
{
	if(frame.isValid())
	{
		g_mutex.lock();
		g_frame=QVideoFrame(frame);
		g_mutex.unlock();
	}
}

void cameraplugin::init()
{
}

std::vector<cameraplugin::CameraDesc> cameraplugin::availableDevices()
{
	std::vector<cameraplugin::CameraDesc> cams;
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : cameras) {
		cameraplugin::CameraDesc cam;
        cam.name=cameraDevice.description().toStdString();
        cam.description=cameraDevice.description().toStdString();
		cam.pos=cameraplugin::CameraDesc::POS_UNKNOWN;
        if (cameraDevice.position()==QCameraDevice::FrontFace)
			cam.pos=cameraplugin::CameraDesc::POS_FRONTFACING;
        if (cameraDevice.position()==QCameraDevice::BackFace)
			cam.pos=cameraplugin::CameraDesc::POS_BACKFACING;
		cams.push_back(cam);
	}
	return cams;
}

void cameraplugin::deinit()
{
	stop();
}

void cameraplugin::start(Orientation orientation,int *camwidth,int *camheight,const char *cam, int *picwidth, int *picheight)
{
    const QCameraDevice *camdev=nullptr;
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cam) {
        for (const QCameraDevice &cameraDevice : cameras)
            if (cameraDevice.description()==QString(cam))
                camdev=&cameraDevice;
    }
    if (camdev==nullptr) {
        if (cameras.size()>0) {
            camdev=&cameras[0];
        }
    }
	*camwidth=0; *camheight=0;
    if (camdev==nullptr) return;
    camera = new QCamera(*camdev);
	if (!camera) return;
	int o=0;
	switch (orientation)
	{
	case eFixed:
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

    QList<QCameraFormat> reslist=camdev->videoFormats();
    int best=0,opt=-1,bestw=0,optw=0;

    int expw=cameraplugin::cameraTexture->data->width;
    int exph=cameraplugin::cameraTexture->data->height;
    for (int k=0;k<reslist.size();k++)
    {
        int weight=reslist[k].resolution().width()*reslist[k].resolution().height();
    	if (weight>bestw) { best=k; bestw=weight; }
        if ((reslist[k].resolution().width()>=expw)&&(reslist[k].resolution().height()>=exph))
    	{
    		if ((opt<0)||(optw>weight))
    		{
    			opt=k;
    			optw=weight;
    		}
    	}
    }
    if (opt<0) opt=best;


    int cw=reslist[opt].resolution().width();
    int ch=reslist[opt].resolution().height();
    camera->setCameraFormat(reslist[opt]);
    *camwidth=reslist[best].resolution().width();
    *camheight=reslist[best].resolution().height();

    if (!captureSession)
        captureSession=new QMediaCaptureSession;
    captureSession->setCamera(camera);

    QVideoSink *sink=new QVideoSink();
    camerasurface=new VideoFrameSurface(cameraplugin::cameraTexture->data,cw,ch,o,sink);
    captureSession->setVideoSink(sink);
    camera->start();
   	cameraplugin::application->addTicker(camerasurface);

   	*picwidth=0;
   	*picheight=0;
}

void cameraplugin::stop()
{
	if (!camera) return;
    cameraplugin::application->removeTicker(camerasurface);
	camera->stop();
	delete camerasurface;
	camerasurface=NULL;
	delete camera;
	camera=NULL;
    if (captureSession) {
        delete captureSession->videoSink();
        delete captureSession;
    }
    captureSession=NULL;
}

bool cameraplugin::isAvailable()
{
    return true;
}

bool cameraplugin::setFlash(int mode) {
    G_UNUSED(mode);
    return false;
}

bool cameraplugin::takePicture() {
    return false;
}

cameraplugin::CameraInfo cameraplugin::queyCamera(const char *device, Orientation orientation)
{
    G_UNUSED(device);
    G_UNUSED(orientation);
    cameraplugin::CameraInfo ci; //Dummy info, not supported
    return ci;
}

void cameraplugin::setOrientation(Orientation orientation) {
    G_UNUSED(orientation);
}

