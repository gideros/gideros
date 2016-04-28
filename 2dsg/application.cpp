#include "application.h"
#include "touchevent.h"
#include "ogl.h"
#include <time.h>
#include "stageorientationevent.h"
#include "font.h"
#include "stage.h"
#include <ghttp.h>
#include <timer.h>
#include <platform.h>
#include <gui.h>
#include <glog.h>
#include <gtexture.h>
#include <memory.h>
#include <gstdio.h>
#include "Shaders.h"

#if 0 && defined(QT_CORE_LIB)
#include <QDebug>
#endif

#if _WIN32
#include <windows.h>
static double PCFreq = 0.0;
static __int64 CounterStart = 0;
#else
static double CounterStart = 0;
#endif

static void StartCounter() {
#if _WIN32
	LARGE_INTEGER li;
	if(!QueryPerformanceFrequency(&li))
	printf("QueryPerformanceFrequency failed!\n");

	PCFreq = double(li.QuadPart)/1000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
#else
	CounterStart = iclock();
#endif
}
static double GetCounter() {
#if _WIN32
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart-CounterStart)/PCFreq;
#else
	return (iclock() - CounterStart) * 1000.0;
#endif
}

Application::Application() :
		textureManager_(this), timerContainer_(this) {
	stage_ = 0;

	orientation_ = ePortrait;
	hardwareOrientation_ = ePortrait;
	deviceOrientation_ = ePortrait;

	nframe_ = -1;	// uninitialized yet
	time_ = -1;

	clearColorBuffer_ = true;

	width_ = 320;
	height_ = 480;

	logicalWidth_ = 320;
	logicalHeight_ = 480;
	scaleMode_ = eNoScale;

	calculateLogicalTransformation();

	defaultFont_ = NULL;

	scale_ = 1;
	fov_ = 0;
	farplane_ = 0;
	nearplane_ = 0;
}

Application::~Application() {
	for (std::size_t i = 0; i < unrefPool_.size(); ++i)
		delete unrefPool_[i];
	for (std::size_t i = 0; i < unrefPoolTrash_.size(); ++i)
		delete unrefPoolTrash_[i];
	oglCleanup(); //Only release on destroy, so that alls refs are discarded
}

Stage* Application::stage() const {
	return stage_;
}

void Application::initView() {
	oglInitialize(width_, height_);
	backr_ = 1.f;
	backg_ = 1.f;
	backb_ = 1.f;

	stage_ = new Stage(this);
}

void Application::releaseView() {
	timerContainer_.removeAllTimers();
	timerContainer_.resumeAllTimers();

	ghttp_CloseAll();

	if (defaultFont_) {
		defaultFont_->unref();
		defaultFont_ = NULL;
	}

	stage_->unref();
	stage_ = NULL;

//	Referenced::emptyPool();
}

void Application::enterFrame() {
	timerContainer_.tick();

	tickersIteratorInvalid_ = false;
	std::set<Ticker*>::iterator iter, e = tickers_.end();
	for (iter = tickers_.begin(); iter != e; ++iter) {
		(*iter)->tick();
		if (tickersIteratorInvalid_)
			break;
	}

	stage_->enterFrame(1, lastFrameRenderTime_);
}

void Application::clearBuffers() {
	if (clearColorBuffer_ == true)
		ShaderEngine::Engine->clearColor(backr_, backg_, backb_, 1.f);
}

void Application::configureFrustum(float fov, float farplane, float nearplane) {
	if (fov > 179) //Do not allow 180°, this would cause infinite width
		fov = 179;
	if (fov < 0)
		fov = 0;
	if (farplane <= 0)
		farplane = 0;
	if (nearplane <= 0)
		nearplane = 0;
	farplane_ = farplane;
	nearplane_ = nearplane;
	fov_ = fov;
}

void Application::renderScene(int deltaFrameCount) {
	if (nframe_ < 0 || time_ < 0) {
		nframe_ = 0;
		time_ = iclock();
	}

	double time = iclock();
	nframe_++;
	if (nframe_ == 60) {
		double dtime = time - time_;
		time_ = time;

		glog_v("fps: %g %d", nframe_ / dtime, GReferenced::instanceCount);

		nframe_ = 0;
	}

	if (!ShaderEngine::Engine)
		return;
	ShaderEngine *gfx = ShaderEngine::Engine;
	//oglTextureReset();
	gfx->reset();

	gtexture_tick();

	if (scale_ == 0)
		scale_ = 1;

	switch (hardwareOrientation_) {
	case eFixed:
	case ePortrait:
	case ePortraitUpsideDown:
		gfx->setViewport(0, 0, width_ / scale_, height_ / scale_);
		break;
	case eLandscapeLeft:
	case eLandscapeRight:
		gfx->setViewport(0, 0, height_ / scale_, width_ / scale_);
		break;
	}

	if (projectionDirty_) {
		projectionDirty_ = false;
		Matrix4 projection, frustum;
		//glMatrixMode(GL_PROJECTION);
		//glLoadIdentity();

		//glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

		projection.scale(logicalScaleX_, logicalScaleY_, fov_?logicalScaleY_:1.0);
		projection.translate(logicalTranslateX_, logicalTranslateY_, 0);
		projection.scale(1.f / scale_, 1.f / scale_, fov_?1.f/scale_:1.f);

		if (hardwareOrientation_ == eFixed) {
		} else {
			switch (orientation_) {
			case ePortrait:
				break;
			case ePortraitUpsideDown:
				projection.translate(-(width_ / scale_) / 2,
						-(height_ / scale_) / 2, 0);
				projection.rotate(180, 0, 0, 1);
				projection.translate((width_ / scale_) / 2,
						(height_ / scale_) / 2, 0);
				break;
			case eLandscapeLeft:
				projection.translate(-(width_ / scale_) / 2,
						-(width_ / scale_) / 2, 0);
				projection.rotate(90, 0, 0, 1);
				projection.translate((width_ / scale_) / 2,
						(width_ / scale_) / 2, 0);
				break;
			case eLandscapeRight:
				projection.translate(-(height_ / scale_) / 2,
						-(height_ / scale_) / 2, 0);
				projection.rotate(270, 0, 0, 1);
				projection.translate((height_ / scale_) / 2,
						(height_ / scale_) / 2, 0);
				break;
			}

			switch (hardwareOrientation_) {
			case ePortrait:
				break;
			case ePortraitUpsideDown:
				projection.translate(-(width_ / scale_) / 2,
						-(height_ / scale_) / 2, 0);
				projection.rotate(180, 0, 0, 1);
				projection.translate((width_ / scale_) / 2,
						(height_ / scale_) / 2, 0);
				break;
			case eLandscapeLeft:
				projection.translate(-(width_ / scale_) / 2,
						-(width_ / scale_) / 2, 0);
				projection.rotate(-90, 0, 0, 1);
				projection.translate((width_ / scale_) / 2,
						(width_ / scale_) / 2, 0);
				break;
			case eLandscapeRight:
				projection.translate(-(height_ / scale_) / 2,
						-(height_ / scale_) / 2, 0);
				projection.rotate(-270, 0, 0, 1);
				projection.translate((height_ / scale_) / 2,
						(height_ / scale_) / 2, 0);
				break;
			}
		}

		Matrix4 vpProjection = projection;
		switch (hardwareOrientation_) {
		case eFixed:
		case ePortrait:
		case ePortraitUpsideDown:
			if (fov_ > 0) {
				float hw = width_ * 0.5 / scale_;
				float hh = height_ * 0.5 / scale_;
				float fp = (farplane_ > 0) ? farplane_ : height_ * 100;
				float np = (nearplane_ > 0) ? nearplane_ : 1;
				float fd=hh / tan(fov_ * M_PI / 360.0);
				float nps = np/fd;
				frustum = gfx->setFrustum(-hw * nps, hw * nps, hh * nps,
						-hh * nps, np, fp);
				projection.translate(-hw, -hh, -fd-0.001);
			} else {
				float fp = (farplane_ > 0) ? farplane_ : 1; //Conservative default
				frustum = gfx->setOrthoFrustum(0, width_ / scale_,
						height_ / scale_, 0, -fp, fp);
			}
			break;
		case eLandscapeLeft:
		case eLandscapeRight:
			if (fov_ > 0) {
				float hw = width_ * 0.5 / scale_;
				float hh = height_ * 0.5 / scale_;
				float fp = (farplane_ > 0) ? farplane_ : height_ * 100;
				float np = (nearplane_ > 0) ? nearplane_ : 1;
				float fd=hh / tan(fov_ * M_PI / 360.0);
				float nps = np/fd;
				frustum = gfx->setFrustum(-hh * nps, hh * nps, hw * nps,
						-hw * nps, np, fp);
				projection.translate(-hh, -hw, -fd-0.001);
			} else {
				float fp = (farplane_ > 0) ? farplane_ : 1; //Conservative default
				frustum = gfx->setOrthoFrustum(0, height_ / scale_,
						width_ / scale_, 0, -fp, fp);
			}
			break;
		}
		projectionMatrix_ = frustum * projection;
		vpProjectionMatrix_ = vpProjection;
	}
	if ((hardwareOrientation_==eLandscapeRight)||(hardwareOrientation_==eLandscapeLeft))
		gfx->setViewportProjection(vpProjectionMatrix_,height_ / scale_,width_ / scale_);
	else
		gfx->setViewportProjection(vpProjectionMatrix_,width_ / scale_,	height_ / scale_);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	float ltx = this->getLogicalTranslateX();
	float lty = this->getLogicalTranslateY();
	float lsx = this->getLogicalScaleX();
	float lsy = this->getLogicalScaleY();
	int hw = this->getHardwareWidth();
	int hh = this->getHardwareHeight();
	
	if (hardwareOrientation_!=eFixed) {
		Orientation orientation = this->orientation();

		if (orientation == eLandscapeLeft || orientation == eLandscapeRight)
			std::swap(hw, hh);
	}
	gfx->setProjection(projectionMatrix_);

	// hardware start/end x/y
	//if(lsx == 0) lsx = 1;
	//if(lsy == 0) lsy = 1;
	float sx = (0 - ltx) / lsx;
	float sy = (0 - lty) / lsy;
	float ex = (hw - ltx) / lsx;
	float ey = (hh - lty) / lsy;

	CurrentTransform currentTransform;
	stage_->draw(currentTransform, sx, sy, ex, ey);

#if 0 && defined(QT_CORE_LIB)
	qDebug() << "bindTextureCount: " << getBindTextureCount();
	qDebug() << "clientStateCount: " << getClientStateCount();
	qDebug() << "texture2DStateCount: " << getTexture2DStateCount();

	qDebug() << getVertexArrayCount() << getTextureCoordArrayCount();
#endif

//	Referenced::emptyPool();
	lastFrameRenderTime_ = iclock() - time;
}

void Application::mouseDown(int x, int y, int button) {
	correctTouchPositionHardware(&x, &y);
	correctTouchPosition(&x, &y);
	correctTouchPositionLogical(&x, &y);
	stage_->mouseDown(x, y, button, logicalScaleX_, logicalScaleY_,
			logicalTranslateX_, logicalTranslateY_);
}

void Application::mouseUp(int x, int y, int button) {
	correctTouchPositionHardware(&x, &y);
	correctTouchPosition(&x, &y);
	correctTouchPositionLogical(&x, &y);
	stage_->mouseUp(x, y, button, logicalScaleX_, logicalScaleY_,
			logicalTranslateX_, logicalTranslateY_);
}

void Application::mouseMove(int x, int y, int button) {
	correctTouchPositionHardware(&x, &y);
	correctTouchPosition(&x, &y);
	correctTouchPositionLogical(&x, &y);
	stage_->mouseMove(x, y, button, logicalScaleX_, logicalScaleY_,
			logicalTranslateX_, logicalTranslateY_);
}

void Application::mouseHover(int x, int y, int button) {
	correctTouchPositionHardware(&x, &y);
	correctTouchPosition(&x, &y);
	correctTouchPositionLogical(&x, &y);
	stage_->mouseHover(x, y, button, logicalScaleX_, logicalScaleY_,
			logicalTranslateX_, logicalTranslateY_);
}

void Application::mouseWheel(int x, int y, int wheel) {
	correctTouchPositionHardware(&x, &y);
	correctTouchPosition(&x, &y);
	correctTouchPositionLogical(&x, &y);
	stage_->mouseWheel(x, y, logicalScaleX_, logicalScaleY_, logicalTranslateX_,
			logicalTranslateY_, wheel);
}

void Application::keyDown(int keyCode, int realCode) {
	stage_->keyDown(keyCode, realCode);
}

void Application::keyUp(int keyCode, int realCode) {
	stage_->keyUp(keyCode, realCode);
}

void Application::keyChar(const char *code) {
	stage_->keyChar(code);
}

void Application::correctTouchPositions(ginput_TouchEvent *event) {
	int &x = event->touch.x;
	int &y = event->touch.y;
	correctTouchPositionHardware(&x, &y);
	correctTouchPosition(&x, &y);
	correctTouchPositionLogical(&x, &y);

	for (int i = 0; i < event->allTouchesCount; ++i) {
		int &x = event->allTouches[i].x;
		int &y = event->allTouches[i].y;
		correctTouchPositionHardware(&x, &y);
		correctTouchPosition(&x, &y);
		correctTouchPositionLogical(&x, &y);
	}
}

void Application::touchesBegin(ginput_TouchEvent *event) {
	correctTouchPositions(event);
	stage_->touchesBegin(event, logicalScaleX_, logicalScaleY_,
			logicalTranslateX_, logicalTranslateY_);
}

void Application::touchesMove(ginput_TouchEvent *event) {
	correctTouchPositions(event);
	stage_->touchesMove(event, logicalScaleX_, logicalScaleY_,
			logicalTranslateX_, logicalTranslateY_);
}

void Application::touchesEnd(ginput_TouchEvent *event) {
	correctTouchPositions(event);
	stage_->touchesEnd(event, logicalScaleX_, logicalScaleY_,
			logicalTranslateX_, logicalTranslateY_);
}

void Application::touchesCancel(ginput_TouchEvent *event) {
	correctTouchPositions(event);
	stage_->touchesCancel(event, logicalScaleX_, logicalScaleY_,
			logicalTranslateX_, logicalTranslateY_);
}

void Application::setOrientation(Orientation orientation) {
	orientation_ = orientation;

	calculateLogicalTransformation();
}

Orientation Application::orientation() const {
	return orientation_;
}

void Application::setHardwareOrientation(Orientation orientation) {
	hardwareOrientation_ = orientation;
	projectionDirty_ = true;
}

Orientation Application::hardwareOrientation() const {
	return hardwareOrientation_;
}

void Application::setDeviceOrientation(Orientation orientation) {
	deviceOrientation_ = orientation;
	projectionDirty_ = true;
}

Orientation Application::getDeviceOrientation() const {
	return deviceOrientation_;
}

void Application::setResolution(int width, int height) {
	width_ = width;
	height_ = height;

	if (ShaderEngine::Engine)
		ShaderEngine::Engine->resizeFramebuffer(width_, height_);

	calculateLogicalTransformation();
}

void Application::resolution(int* width, int* height) {
	if (width != 0)
		*width = width_;

	if (height != 0)
		*height = height_;
}

void Application::correctTouchPosition(int* x, int* y) {
	if (hardwareOrientation_==eFixed) return;
	switch (orientation_) {
	case ePortrait:
		break;
	case ePortraitUpsideDown:
		*x = width_ - *x - 1;
		*y = height_ - *y - 1;
		break;
	case eLandscapeLeft:
		std::swap(*x, *y);
		*y = width_ - *y - 1;
		break;
	case eLandscapeRight:
		std::swap(*x, *y);
		*x = height_ - *x - 1;
		break;
	}
}

void Application::correctTouchPositionHardware(int* x, int* y) {
	switch (hardwareOrientation_) {
	case eFixed:
	case ePortrait:
		break;
	case ePortraitUpsideDown:
		*x = width_ - *x - 1;
		*y = height_ - *y - 1;
		break;
	case eLandscapeLeft:
		std::swap(*x, *y);
		*x = width_ - *x - 1;
		break;
	case eLandscapeRight:
		std::swap(*x, *y);
		*y = height_ - *y - 1;
		break;
	}
}

void Application::broadcastEvent(Event* event) {
	EventDispatcher::broadcastEvent(event);
}

Font* Application::getDefaultFont() {
	if (defaultFont_ == NULL)
		defaultFont_ = new Font(this);

	return defaultFont_;
}

void Application::calculateLogicalTransformation() {
	int width = this->width_;
	int height = this->height_;
	int logicalWidth = this->logicalWidth_;
	int logicalHeight = this->logicalHeight_;


	if (hardwareOrientation_==eFixed)
	{
		//scaleMode_=eLetterBox;
		if (orientation_ == eLandscapeLeft || orientation_ == eLandscapeRight) {
			std::swap(logicalWidth, logicalHeight);
		}
	}
	else
	{
		if (orientation_ == eLandscapeLeft || orientation_ == eLandscapeRight) {
			std::swap(width, height);
			std::swap(logicalWidth, logicalHeight);
		}
	}

	if (scaleMode_ == eNoScale) {
		logicalScaleX_ = 1;
		logicalScaleY_ = 1;

		logicalTranslateX_ = 0;
		logicalTranslateY_ = 0;
	} else if (scaleMode_ == eCenter) {
		logicalScaleX_ = 1;
		logicalScaleY_ = 1;

		logicalTranslateX_ = (double) (width - logicalWidth) / 2;
		logicalTranslateY_ = (double) (height - logicalHeight) / 2;
	} else if (scaleMode_ == ePixelPerfect) {
		double scale1 = (double) logicalWidth / (double) logicalHeight;
		double scale2 = (double) width / (double) height;

		if (scale1 > scale2) {
			double scale = (double) width / (double) logicalWidth;

			if (scale >= 1 - 1e-5)
				scale = floor(scale + 1e-5);
			else
				scale = 1 / ceil(1 / scale - 1e-5);

			logicalScaleX_ = scale;
			logicalScaleY_ = scale;

			logicalTranslateX_ = (width - logicalWidth * scale) / 2;
			logicalTranslateY_ = (height - logicalHeight * scale) / 2;
		} else {
			double scale = (double) height / (double) logicalHeight;

			if (scale >= 1 - 1e-5)
				scale = floor(scale + 1e-5);
			else
				scale = 1 / ceil(1 / scale - 1e-5);

			logicalScaleX_ = scale;
			logicalScaleY_ = scale;

			logicalTranslateX_ = (width - logicalWidth * scale) / 2;
			logicalTranslateY_ = (height - logicalHeight * scale) / 2;
		}
	} else if (scaleMode_ == eLetterBox) {
		double scale1 = (double) logicalWidth / (double) logicalHeight;
		double scale2 = (double) width / (double) height;

		if (scale1 > scale2) {
			double scale = (double) width / (double) logicalWidth;

			logicalScaleX_ = scale;
			logicalScaleY_ = scale;

			logicalTranslateX_ = 0;
			logicalTranslateY_ = (height - logicalHeight * scale) / 2;
		} else {
			double scale = (double) height / (double) logicalHeight;

			logicalScaleX_ = scale;
			logicalScaleY_ = scale;

			logicalTranslateX_ = (width - logicalWidth * scale) / 2;
			logicalTranslateY_ = 0;
		}
	} else if (scaleMode_ == eCrop) {
		double scale1 = (double) logicalWidth / (double) logicalHeight;
		double scale2 = (double) width / (double) height;

		if (scale1 > scale2) {
			double scale = (double) height / (double) logicalHeight;

			logicalScaleX_ = scale;
			logicalScaleY_ = scale;

			logicalTranslateX_ = (width - logicalWidth * scale) / 2;
			logicalTranslateY_ = 0;
		} else {
			double scale = (double) width / (double) logicalWidth;

			logicalScaleX_ = scale;
			logicalScaleY_ = scale;

			logicalTranslateX_ = 0;
			logicalTranslateY_ = (height - logicalHeight * scale) / 2;
		}
	} else if (scaleMode_ == eStretch) {
		logicalScaleX_ = (double) width / (double) logicalWidth;
		logicalScaleY_ = (double) height / (double) logicalHeight;

		logicalTranslateX_ = 0;
		logicalTranslateY_ = 0;
	} else if (scaleMode_ == eFitWidth) {
		double scale = (double) width / (double) logicalWidth;

		logicalScaleX_ = scale;
		logicalScaleY_ = scale;

		logicalTranslateX_ = 0;
		logicalTranslateY_ = (height - logicalHeight * scale) / 2;
	} else if (scaleMode_ == eFitHeight) {
		double scale = (double) height / (double) logicalHeight;

		logicalScaleX_ = scale;
		logicalScaleY_ = scale;

		logicalTranslateX_ = (width - logicalWidth * scale) / 2;
		logicalTranslateY_ = 0;
	}
	projectionDirty_ = true;
}

void Application::correctTouchPositionLogical(int* x, int* y) {
#if 0
	float newx = *x - logicalTranslateX_;
	float newy = *y - logicalTranslateY_;

	newx /= logicalScaleX_;
	newy /= logicalScaleY_;

	*x = newx;
	*y = newy;
#endif
}

void Application::setLogicalDimensions(int width, int height) {
	logicalWidth_ = width;
	logicalHeight_ = height;

	calculateLogicalTransformation();
}

void Application::setLogicalScaleMode(LogicalScaleMode mode) {
	scaleMode_ = mode;
	calculateLogicalTransformation();
}

LogicalScaleMode Application::getLogicalScaleMode() const {
	return scaleMode_;
}

int Application::getLogicalWidth() const {
	return logicalWidth_;
}
int Application::getLogicalHeight() const {
	return logicalHeight_;
}
int Application::getHardwareWidth() const {
	return width_;
}
int Application::getHardwareHeight() const {
	return height_;
}

void Application::setImageScales(
		const std::vector<std::pair<std::string, float> >& imageScales) {
	imageScales_ = imageScales;

	imageScales2_.clear();

	for (std::size_t i = 0; i < imageScales_.size(); ++i)
		imageScales2_.push_back(
				ImageScale(imageScales_[i].first.c_str(),
						imageScales_[i].second));
	imageScales2_.push_back(ImageScale(NULL, 1.f));

	std::sort(imageScales2_.begin(), imageScales2_.end());

	for (std::size_t i = 0; i < imageScales2_.size() - 1; ++i)
		imageScales2_[i].midscale = (imageScales2_[i].scale
				+ imageScales2_[i + 1].scale) / 2;
}

const std::vector<std::pair<std::string, float> >& Application::getImageScales() const {
	return imageScales_;
}

const char *Application::getImageSuffix(float *pscale) const {
	const char *result = NULL;
	float nearestscale = 1;

	float scale = (logicalScaleX_ + logicalScaleY_) / 2;

	float mindelta = 1e30f;

	for (size_t i = 0; i < imageScales_.size(); ++i) {
		float delta = fabs(scale - imageScales_[i].second);
		if (delta < mindelta) {
			mindelta = delta;
			result = imageScales_[i].first.c_str();
			nearestscale = imageScales_[i].second;
		}
	}

	float delta = fabs(scale - 1);
	if (delta < mindelta) {
		result = NULL;
		nearestscale = 1;
	}

	if (pscale)
		*pscale = nearestscale;

	return result;
}

const char *Application::getImageSuffix(const char *file, float *pscale) const {
	float scale = (logicalScaleX_ + logicalScaleY_) / 2;

	const char *ext = strrchr(file, '.');

	if (ext == NULL)
		ext = file + strlen(file);

	for (std::size_t i = 0; i < imageScales2_.size(); ++i) {
		if (scale < imageScales2_[i].midscale)
			continue;

		const char *suffix = imageScales2_[i].suffix;

		std::string filewithsuffix = std::string(file, ext - file)
				+ (suffix ? suffix : "") + ext;

		G_FILE *fis = g_fopen(filewithsuffix.c_str(), "rb");

		if (fis != NULL) {
			g_fclose(fis);
			if (pscale)
				*pscale = imageScales2_[i].scale;
			return suffix;
		}
	}

	if (pscale)
		*pscale = 1;
	return NULL;
}

void Application::addTicker(Ticker* ticker) {
	tickers_.insert(ticker);
	tickersIteratorInvalid_ = true;
}

void Application::removeTicker(Ticker* ticker) {
	tickers_.erase(ticker);
	tickersIteratorInvalid_ = true;
}

void Application::setBackgroundColor(float r, float g, float b) {
	backr_ = r;
	backg_ = g;
	backb_ = b;
}

void Application::getBackgroundColor(float* r, float* g, float* b) {
	if (r)
		*r = backr_;
	if (g)
		*g = backg_;
	if (b)
		*b = backb_;
}

float Application::getLogicalTranslateX() const {
	return logicalTranslateX_;
}

float Application::getLogicalTranslateY() const {
	return logicalTranslateY_;
}

float Application::getLogicalScaleX() const {
	return logicalScaleX_;
}

float Application::getLogicalScaleY() const {
	return logicalScaleY_;
}

void Application::autounref(GReferenced *referenced) {
	unrefPool_.back()->push_back(referenced);
}

void *Application::createAutounrefPool() {
	std::vector<GReferenced*> *pool;
	if (unrefPoolTrash_.empty())
		pool = new std::vector<GReferenced*>;
	else {
		pool = unrefPoolTrash_.back();
		unrefPoolTrash_.pop_back();
	}

	unrefPool_.push_back(pool);
	return reinterpret_cast<void*>(pool);
}

void Application::deleteAutounrefPool(void *pool) {
	while (!unrefPool_.empty()) {
		std::vector<GReferenced*> *pool2 = unrefPool_.back();
		unrefPool_.pop_back();

		for (std::size_t i = 0; i < pool2->size(); ++i)
			(*pool2)[i]->unref();
		pool2->clear();
		unrefPoolTrash_.push_back(pool2);

		if (pool == pool2)
			break;
	}
}

