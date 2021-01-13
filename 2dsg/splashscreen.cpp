#include "splashscreen.h"
#include "ogl.h"
#include "dib.h"
#include "application.h"
#include <zlib.h>
#include <platform.h>

#include "logo.inc"

#undef min
#undef max

static Dib logo2dib(Application* application, unsigned char* logo, size_t logoSize, int width, int height)
{
	uLong imageLen = width * height * 4;
	unsigned char* image = (unsigned char*)malloc(imageLen);
	uncompress(image, &imageLen, logo, logoSize);
	
	Dib dib(application, width, height, true);

	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
		{
			int index = (x + y * width) * 4;

			unsigned int r = image[index + 0];
			unsigned int g = image[index + 1];
			unsigned int b = image[index + 2];
			unsigned int a = image[index + 3];
			
			unsigned char rgba[] = {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
			dib.setPixel(x, y, rgba);
		}

	free(image);

	return dib;
}

static GraphicsBase createGraphicsBase(TextureData* data, int x, int y)
{
	GraphicsBase graphicsBase;
	
	graphicsBase.mode = ShaderProgram::TriangleStrip;
	
	graphicsBase.data = data;
	
	graphicsBase.vertices.resize(4);
	graphicsBase.vertices[0] = Point2f(x, y);
	graphicsBase.vertices[1] = Point2f(data->width + x, y);
	graphicsBase.vertices[2] = Point2f(data->width + x, data->height + y);
	graphicsBase.vertices[3] = Point2f(x, data->height + y);
	graphicsBase.vertices.Update();
	
	float u = (float)data->width / (float)data->exwidth;
	float v = (float)data->height / (float)data->exheight;
	
	graphicsBase.texcoords.resize(4);
	graphicsBase.texcoords[0] = Point2f(0, 0);
	graphicsBase.texcoords[1] = Point2f(u, 0);
	graphicsBase.texcoords[2] = Point2f(u, v);
	graphicsBase.texcoords[3] = Point2f(0, v);
	graphicsBase.texcoords.Update();
	
	graphicsBase.indices.resize(4);
	graphicsBase.indices[0] = 0;
	graphicsBase.indices[1] = 1;
	graphicsBase.indices[2] = 3;
	graphicsBase.indices[3] = 2;
	graphicsBase.indices.Update();
	
	return graphicsBase;
}

SplashScreen::SplashScreen(Application* application) : Sprite(application)
{
	Dib dib1 = logo2dib(application, logo1, sizeof(logo1), 320, 44);
	Dib dib2 = logo2dib(application, logo2, sizeof(logo2), 320, 96);

	TextureParameters parameters;
	data1_ = application_->getTextureManager()->createTextureFromDib(dib1, parameters);
	data2_ = application_->getTextureManager()->createTextureFromDib(dib2, parameters);
	
	graphicsBase1_ = createGraphicsBase(data1_, 0, 0);
	graphicsBase2_ = createGraphicsBase(data2_, 0, 30);

	startTime_ = iclock();

	Orientation orientation = application->orientation();
	float width = application->getHardwareWidth();
	float height = application->getHardwareHeight();
	if (orientation == eLandscapeLeft || orientation == eLandscapeRight)
		std::swap(width, height);	
	float dx = (width - 320) / 2;
	float dy = (height - 140) / 2; 

	float sx = application->getLogicalScaleX();
	float sy = application->getLogicalScaleY();
	float tx = application->getLogicalTranslateX();
	float ty = application->getLogicalTranslateY();
	setScaleXY(1/sx, 1/sy);
	setXY((-tx + dx) / sx, (-ty + dy) / sy);
}

SplashScreen::~SplashScreen()
{
	application_->getTextureManager()->destroyTexture(data1_);
	application_->getTextureManager()->destroyTexture(data2_);
}

static float alphaFunction(float t)
{
	float a = 1;

	if (t < 1)
		a = t;
	else if (t < 3)
		a = 1;
	else
		a = 4 - t;
	
	return std::min(std::max(a, 0.f), 1.f);
}

void SplashScreen::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
	double t = iclock() - startTime_;
	
	float a1 = alphaFunction(t);
	float a2 = alphaFunction(t - 0.25);
	
	graphicsBase1_.setColor(1, 1, 1, a1);
	graphicsBase1_.draw(getShader(graphicsBase1_.getShaderType()));

	graphicsBase2_.setColor(1, 1, 1, a2);
	graphicsBase2_.draw(getShader(graphicsBase2_.getShaderType()));
}

bool SplashScreen::isFinished() const
{
	double t = iclock() - startTime_;

	return t > 4.25;
}
