/*
 * CameraPlugin.cpp
 *
 *  Created on: 5 ao�t 2017
 *      Author: Nicolas
 */
#include <QAbstractVideoSurface>
#include <QAbstractVideoBuffer>
#include <QVideoFrame>
#include <QList>
#include <QCamera>
#include <QCameraInfo>
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
		ShaderProgram::DFLOAT, 3, 0, 0 }, { "vColor", ShaderProgram::DUBYTE, 4,
		1, 0 }, { "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0 }, { "",
		ShaderProgram::DFLOAT, 0, 0, 0 } };


class VideoFrameSurface : public QAbstractVideoSurface, public Ticker {
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
	bool present(const QVideoFrame& frame);
	QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
	void tick();
public:
	VideoFrameSurface(TextureData *tex,int w,int h, int o);
	~VideoFrameSurface();
	void render();
};

static QCamera *camera=NULL;
static VideoFrameSurface *camerasurface=NULL;

VideoFrameSurface::VideoFrameSurface(TextureData *tex,int w,int h, int o) {
	G_UNUSED(o);
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

#ifdef Q_OS_MACX
	vertices[0] = Point2f(0, 0);
	vertices[1] = Point2f(gtex->width, 0);
	vertices[2] = Point2f(gtex->width, gtex->height);
	vertices[3] = Point2f(0, gtex->height);
#else
	vertices[0] = Point2f(0, gtex->height);
	vertices[1] = Point2f(gtex->width, gtex->height);
	vertices[2] = Point2f(gtex->width, 0);
	vertices[3] = Point2f(0, 0);
#endif
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
    if (!g_frame.isValid()) return;
	GLCALL_INIT;
	if(g_frame.map(QAbstractVideoBuffer::ReadOnly))
	{
		GLuint tid=*((GLuint *)camtex->getNative());
		GLCALL glBindTexture(GL_TEXTURE_2D, tid);
		//qDebug() << "Render:" << g_frame.width()<< g_frame.height() << tid;
		GLCALL glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cw, ch,0, GL_RGBA, GL_UNSIGNED_BYTE, g_frame.bits());
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
        g_frame.unmap();
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

bool VideoFrameSurface::present(const QVideoFrame& frame)
{
	if(frame.isValid())
	{
		g_mutex.lock();
		g_frame=QVideoFrame(frame);
		g_mutex.unlock();
	}

	return true;
}

QList<QVideoFrame::PixelFormat> VideoFrameSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
	G_UNUSED(handleType);
	// Return the formats you will support
	return QList<QVideoFrame::PixelFormat>()
			<< QVideoFrame::Format_ARGB32
			<< QVideoFrame::Format_RGB32
		;
}


void cameraplugin::init()
{
}

std::vector<cameraplugin::CameraDesc> cameraplugin::availableDevices()
{
	std::vector<cameraplugin::CameraDesc> cams;
	QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
	foreach (const QCameraInfo &cameraInfo, cameras) {
		cameraplugin::CameraDesc cam;
		cam.name=cameraInfo.deviceName().toStdString();
		cam.description=cameraInfo.description().toStdString();
		cam.pos=cameraplugin::CameraDesc::POS_UNKNOWN;
		if (cameraInfo.position()==QCamera::FrontFace)
			cam.pos=cameraplugin::CameraDesc::POS_FRONTFACING;
		if (cameraInfo.position()==QCamera::BackFace)
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
	QCameraInfo caminfo=QCameraInfo::defaultCamera();
	if (cam)
		caminfo=QCameraInfo(QByteArray(cam));
	*camwidth=0; *camheight=0;
	if (caminfo.isNull()) return;
	camera = new QCamera(caminfo);
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

	camera->load();

    QList<QSize> reslist=camera->supportedViewfinderResolutions();
    int best=0,opt=-1,bestw=0,optw=0;

    int expw=cameraplugin::cameraTexture->data->width;
    int exph=cameraplugin::cameraTexture->data->height;
    for (int k=0;k<reslist.size();k++)
    {
    	int weight=reslist[k].width()*reslist[k].height();
    	if (weight>bestw) { best=k; bestw=weight; }
    	if ((reslist[k].width()>=expw)&&(reslist[k].height()>=exph))
    	{
    		if ((opt<0)||(optw>weight))
    		{
    			opt=k;
    			optw=weight;
    		}
    	}
    }
    if (opt<0) opt=best;

    int cw=reslist[opt].width();
    int ch=reslist[opt].height();
    QCameraViewfinderSettings vs=camera->viewfinderSettings();
    vs.setResolution(cw,ch);
    camera->setViewfinderSettings(vs);
    *camwidth=reslist[best].width();
    *camheight=reslist[best].height();

	camerasurface=new VideoFrameSurface(cameraplugin::cameraTexture->data,cw,ch,o);
    camera->setViewfinder(camerasurface);
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
}

bool cameraplugin::isAvailable()
{
    return true;
}

bool cameraplugin::setFlash(int mode) {
    return false;
}

bool cameraplugin::takePicture() {
    return false;
}

cameraplugin::CameraInfo cameraplugin::queyCamera(const char *device, Orientation orientation)
{
    cameraplugin::CameraInfo ci; //Dummy info, not supported
    return ci;
}

