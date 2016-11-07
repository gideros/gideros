#include "pixel.h"
#include "ogl.h"
#include "color.h"

VertexBuffer<unsigned short> Pixel::quad;

Pixel::Pixel(Application *application) : Sprite(application)
{
    r_ = 1, g_ = 1, b_ = 1, a_ = 1;
    width_ = 1, height_ = 1;
    sx_ = 1, sy_ = 1;
    x_ = 0, y_ = 0;
    isStretching_ = false;
	for (int t=0;t<PIXEL_MAX_TEXTURES;t++)
		texture_[t]=NULL;
	texcoords.resize(4);
	texcoords[0] = Point2f(0,0);
	texcoords[1] = Point2f(1,0);
	texcoords[2] = Point2f(1,1);
	texcoords[3] = Point2f(0,1);
	texcoords.Update();
	if (quad.empty())
	{
		quad.resize(4);
		quad[0] = 0;
		quad[1] = 1;
		quad[2] = 3;
		quad[3] = 2;
		quad.Update();
	}
    vertices.resize(4);
}

void Pixel::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
    if (!a_ && !shader_) return;
	if (isWhite_ == false)
	{
		glPushColor();
		glMultColor(r_, g_, b_, a_);
	}

	for (int t=0;t<PIXEL_MAX_TEXTURES;t++)
		if (texture_[t])
			ShaderEngine::Engine->bindTexture(t,texture_[t]->data->id());
    ShaderProgram *shp=(texture_[0])?ShaderProgram::stdTexture:(
        colors_.empty()?ShaderProgram::stdBasic:ShaderProgram::stdColor);
    if (shader_) shp=shader_;

    shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size(),vertices.modified,&vertices.bufferCache);
    shp->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2,&texcoords[0],texcoords.size(),texcoords.modified,&texcoords.bufferCache);
    if (!colors_.empty())
    {
        shp->setData(ShaderProgram::DataColor,ShaderProgram::DUBYTE,4,&colors_[0],colors_.size()/4,colors_.modified,&colors_.bufferCache);
        colors_.modified=false;
    }
    shp->drawElements(ShaderProgram::TriangleStrip, quad.size(), ShaderProgram::DUSHORT, &quad[0], quad.modified, &quad.bufferCache);
    vertices.modified = false;
	texcoords.modified = false;
	quad.modified = false;

	if (isWhite_ == false)
	{
		glPopColor();
	}
}

void Pixel::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
    if (minx)
        *minx = 0;
    if (miny)
        *miny = 0;
    if (maxx)
        *maxx = width_;
    if (maxy)
        *maxy = height_;
}


void Pixel::updateTexture()
{
    TextureBase* texture = texture_[0];

    float tw = texture->data->width;
    float th = texture->data->height;

    float etw = texture->data->exwidth;
    float eth = texture->data->exheight;

    if (isStretching_ || texture->data->parameters.wrap == eRepeat) {
        float w, h, x, y;
        if (isStretching_) {
            w = tw / (etw * sx_);
            h = th / (eth * sy_);
            x = 0.5 * w * (sx_ - 1) - x_ * w;
            y = 0.5 * h * (sy_ - 1) - y_ * h;
        } else {
            w = width_ / (etw * sx_);
            h = height_ / (eth * sy_);
            x = -x_ / etw * tw;
            y = -y_ / eth * th;
        }

        texcoords[0] = Point2f(x,y);
        texcoords[1] = Point2f(x+w,y);
        texcoords[2] = Point2f(x+w,y+h);
        texcoords[3] = Point2f(x,y+h);
        texcoords.Update();
        return;
    }

    float w, h;

    if (width_ / tw < height_ / th) {
        h = sy_ * width_ * th / tw;
        w = width_ * sx_;
    } else {
        w = sx_ * height_ * tw / th;
        h = height_ * sy_;
    }

    float x1, y1, x2, y2;

    x1 = 0.5 * (width_ - w) + x_ * width_;
    y1 = 0.5 * (height_ - h) + y_ * height_;

    x2 = x1 + w;
    y2 = y1 + h;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > width_) x2 = width_;
    if (y2 > height_) y2 = height_;

    vertices[0] = Point2f(x1,y1);
    vertices[1] = Point2f(x2,y1);
    vertices[2] = Point2f(x2,y2);
    vertices[3] = Point2f(x1,y2);
    vertices.Update();

    float tx1, ty1, tx2, ty2;

    float u = tw / etw;
    float v = th / eth;

    float rw = x2 - x1;
    float rh = y2 - y1;

    float rx = 0.5 - (0.5 + x_) * rw / w;
    float ry = 0.5 - (0.5 + y_) * rh / h;

    tx1 = u * rx;
    ty1 = v * ry;
    tx2 = u * (rw / w + rx);
    ty2 = v * (rh / h + ry);

    texcoords[0] = Point2f(tx1,ty1);
    texcoords[1] = Point2f(tx2,ty1);
    texcoords[2] = Point2f(tx2,ty2);
    texcoords[3] = Point2f(tx1,ty2);
    texcoords.Update();
}

void Pixel::setDimensions(float width,float height)
{
	width_=width;
	height_=height;
    vertices[0] = Point2f(0,0);
    vertices[1] = Point2f(width_,0);
    vertices[2] = Point2f(width_,height_);
    vertices[3] = Point2f(0,height_);
	vertices.Update();
    if (texture_[0]) updateTexture();
}

void Pixel::setTexture(TextureBase *texture,int slot, const Matrix4* matrix)
{
    if (texture)
        texture->ref();
    if (texture_[slot])
        texture_[slot]->unref();
    texture_[slot] = texture;

    if (slot==0)
    {
        if (texture) updateTexture();
        if (matrix) for (int tc=0;tc<4;tc++)
			matrix->transformPoint(texcoords[tc].x, texcoords[tc].y, &texcoords[tc].x,&texcoords[tc].y);
 		texcoords.Update();
 	}
}

void Pixel::setTextureMatrix(const Matrix4* matrix)
{
    for (int tc=0;tc<4;tc++)
        matrix->transformPoint(texcoords[tc].x, texcoords[tc].y, &texcoords[tc].x, &texcoords[tc].y);
    texcoords.Update();
}

void Pixel::setTexturePosition(float x, float y)
{
    x_ = x;
    y_ = y;

    if (texture_[0]) updateTexture();
}

void Pixel::setTextureScale(float sx, float sy)
{
    sx_ = sx;
    sy_ = sy;

    if (texture_[0]) updateTexture();
}

void Pixel::setGradient(int c1, float a1, int c2, float a2, int c3, float a3, int c4, float a4)
{
    c1_ = c1, a1_ = a1, c2_ = c2, a2_ = a2, c3_ = c3, a3_ = a3, c4_ = c4, a4_ = a4;
    colors_.resize(16);
    colors_[0] = ((c1 >> 16) & 0xff) * a1;
    colors_[1] = ((c1 >> 8) & 0xff) * a1;
    colors_[2] = (c1 & 0xff) * a1;
    colors_[3] = 255 * a1;
    colors_[4] = ((c2 >> 16) & 0xff) * a2;
    colors_[5] = ((c2 >> 8) & 0xff) * a2;
    colors_[6] = (c2 & 0xff) * a2;
    colors_[7] = 255 * a2;
    colors_[8] = ((c3 >> 16) & 0xff) * a3;
    colors_[9] = ((c3 >> 8) & 0xff) * a3;
    colors_[10] = (c3 & 0xff) * a3;
    colors_[11] = 255 * a3;
    colors_[12] = ((c4 >> 16) & 0xff) * a4;
    colors_[13] = ((c4 >> 8) & 0xff) * a4;
    colors_[14] = (c4 & 0xff) * a4;
    colors_[15] = 255 * a4;
    colors_.Update();
}

int Pixel::getMixedColor(int c1, int c2, float a)
{
    int b1 = c1 % 256;
    int g1 = int(c1/256)%256;
    int r1 = int(c1/65536)%256;
    int b2 = c2 % 256;
    int g2 = int(c2/256)%256;
    int r2 = int(c2/65536)%256;
    int r = r1*a+r2*(1-a);
    int g = g1*a+g2*(1-a);
    int b = b1*a+b2*(1-a);
    return int(r)*65536+int(g)*256+int(b);
}

void Pixel::setGradientWithAngle(int co1, float a1, int co2, float a2, float angle)
{
    const float PI =3.141592653589793238463;

    float dirx = cos(angle/180*PI)/2;
    float diry = sin(angle/180*PI)/2;

    float f1 = 0.5-dirx-diry;
    float f2 = 0.5+dirx-diry;
    float f3 = 0.5+dirx+diry;
    float f4 = 0.5-dirx+diry;

    float fmin = f1 < f2 ? f1 : f2;
    fmin = fmin < f3 ? fmin : f3;
    fmin = fmin < f4 ? fmin : f4;

    float fmax = f1 > f2 ? f1 : f2;
    fmax = fmax > f3 ? fmax : f3;
    fmax = fmax > f4 ? fmax : f4;

    float fscl = 1/(fmax-fmin);
    f1 = (f1-fmin)*fscl;
    f2 = (f2-fmin)*fscl;
    f3 = (f3-fmin)*fscl;
    f4 = (f4-fmin)*fscl;

    float c1 = getMixedColor(co1,co2,f1);
    float c2 = getMixedColor(co1,co2,f2);
    float c3 = getMixedColor(co1,co2,f3);
    float c4 = getMixedColor(co1,co2,f4);

    setGradient(c1, 1.0, c2, 1.0, c3, 1.0, c4, 1.0);
}
