#include "pixel.h"
#include "ogl.h"
#include "color.h"

VertexBuffer<unsigned short> Pixel::quad;
VertexBuffer<unsigned short> Pixel::ninepatch;

Pixel::Pixel(Application *application) : Sprite(application)
{
    r_ = 1, g_ = 1, b_ = 1, a_ = 1;
    width_ = 1, height_ = 1;
    sx_ = 1, sy_ = 1;
    x_ = 0, y_ = 0;
    anchorx_ = 0, anchory_ = 0;
    tx_=0, ty_=0, tw_=0, th_=0;
    isStretching_ = false;
    isNinePatch_=false;
    tmatrix_.identity();
    insetv_t_=0; insetv_b_=0; insetv_l_=0; insetv_r_=0;
    insett_t_=0; insett_b_=0; insett_l_=0; insett_r_=0;
    c1_=c2_=c3_=c4_=0xFFFFFF;
    a1_=a2_=a3_=a4_=1.0;
    isWhite_=true;
    minw_=minh_=0;
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
	if (ninepatch.empty())
	{
		ninepatch.resize(22);
		ninepatch[0]=0;
		ninepatch[1]=1;

		ninepatch[2]=4;
		ninepatch[3]=5;
		ninepatch[4]=8;
		ninepatch[5]=9;
		ninepatch[6]=12;
		ninepatch[7]=13;

		ninepatch[8]=14;

		ninepatch[9]=9;
		ninepatch[10]=10;
		ninepatch[11]=5;
		ninepatch[12]=6;
		ninepatch[13]=1;
		ninepatch[14]=2;

		ninepatch[15]=3;

		ninepatch[16]=6;
		ninepatch[17]=7;
		ninepatch[18]=10;
		ninepatch[19]=11;
		ninepatch[20]=14;
		ninepatch[21]=15;
		ninepatch.Update();
	}
    vertices.resize(4);
}

void Pixel::cloneFrom(Pixel *s) {
    Sprite::cloneFrom(s);
    r_ = s->r_, g_ = s->g_, b_ = s->b_, a_ = s->a_;
    width_ = s->width_, height_ = s->height_;
    sx_ = s->sx_, sy_ = s->sy_;
    x_ = s->x_, y_ = s->y_;
    anchorx_ = s->anchorx_, anchory_ = s->anchory_;
    tx_=s->tx_, ty_=s->ty_, tw_=s->tw_, th_=s->th_;
    isStretching_ = s->isStretching_;
    isNinePatch_=s->isNinePatch_;
    tmatrix_=s->tmatrix_;
    insetv_t_=s->insetv_t_; insetv_b_=s->insetv_b_; insetv_l_=s->insetv_l_; insetv_r_=s->insetv_r_;
    insett_t_=s->insett_t_; insett_b_=s->insett_b_; insett_l_=s->insett_l_; insett_r_=s->insett_r_;
    c1_=s->c1_; c2_=s->c2_; c3_=s->c3_; c4_=s->c4_;
    a1_=s->a1_; a2_=s->a2_; a3_=s->a3_; a4_=s->a4_;
    isWhite_=s->isWhite_;
    minw_=s->minw_; minh_=s->minh_;
    for (int t=0;t<PIXEL_MAX_TEXTURES;t++)
        if ((texture_[t]=s->texture_[t])!=NULL)
                texture_[t]->ref();
    texcoords.assign(s->texcoords.cbegin(),s->texcoords.cend());
    texcoords.Update();
    vertices.assign(s->vertices.cbegin(),s->vertices.cend());
    vertices.Update();
    colors_.assign(s->colors_.cbegin(),s->colors_.cend());
    colors_.Update();
}

Pixel::~Pixel()
{
    for (int t=0;t<PIXEL_MAX_TEXTURES;t++)
        if (texture_[t])
            texture_[t]->unref();
}

void Pixel::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
    G_UNUSED(sx); G_UNUSED(sy); G_UNUSED(ex); G_UNUSED(ey);
    if (!a_ ) return;
	if (isWhite_ == false)
	{
		glPushColor();
		glMultColor(r_, g_, b_, a_);
	}

	for (int t=0;t<PIXEL_MAX_TEXTURES;t++)
		if (texture_[t])
			ShaderEngine::Engine->bindTexture(t,texture_[t]->data->id());
    ShaderProgram *shp=getShader((texture_[0])?ShaderEngine::STDP_TEXTURE:(
        colors_.empty()?ShaderEngine::STDP_BASIC:ShaderEngine::STDP_COLOR));
	int sc=shp->getSystemConstant(ShaderProgram::SysConst_TextureInfo);
	if ((sc>=0)&&texture_[0])
	{
    	float textureInfo[4]={0,0,0,0};
   		textureInfo[0]=(float)texture_[0]->data->width / (float)texture_[0]->data->exwidth;
    	textureInfo[1]=(float)texture_[0]->data->height / (float)texture_[0]->data->exheight;
    	textureInfo[2]=1.0/texture_[0]->data->exwidth;
    	textureInfo[3]=1.0/texture_[0]->data->exheight;
		shp->setConstant(sc,ShaderProgram::CFLOAT4,1,textureInfo);
	}

    shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size(),vertices.modified,&vertices.bufferCache);
    shp->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2,&texcoords[0],texcoords.size(),texcoords.modified,&texcoords.bufferCache);
    if (!colors_.empty())
    {
        shp->setData(ShaderProgram::DataColor,ShaderProgram::DUBYTE,4,&colors_[0],colors_.size()/4,colors_.modified,&colors_.bufferCache);
        colors_.modified=false;
    }
    if (isNinePatch_) {
        shp->drawElements(ShaderProgram::TriangleStrip, ninepatch.size(), ShaderProgram::DUSHORT, &ninepatch[0], ninepatch.modified, &ninepatch.bufferCache);
    	ninepatch.modified = false;
    }
    else
    {
    	shp->drawElements(ShaderProgram::TriangleStrip, quad.size(), ShaderProgram::DUSHORT, &quad[0], quad.modified, &quad.bufferCache);
    	quad.modified = false;
    }
    vertices.modified = false;
	texcoords.modified = false;


	if (isWhite_ == false)
	{
		glPopColor();
	}
}

void Pixel::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
    if (minx)
        *minx = -width_*anchorx_;
    if (miny)
        *miny = -height_*anchory_;
    if (maxx)
        *maxx = width_-width_*anchorx_;
    if (maxy)
        *maxy = height_-height_*anchory_;
}

void Pixel::updateVertices() {
	float dx=-width_*anchorx_;
	float dy=-height_*anchory_;
    if (isNinePatch_) {
		float vt=insetv_t_;
		float vb=insetv_b_;
		float vr=insetv_r_;
		float vl=insetv_l_;
		float dw=width_-vr-vl;
		float dh=height_-vb-vt;
		if (dw<0) {
			float r=width_/(vr+vl); dw=0;
			vr*=r; vl*=r;
		}
		if (dh<0) {
			float r=height_/(vt+vb); dh=0;
			vt*=r; vb*=r;
		}
		vertices.resize(16);
		vertices[0] = Point2f(0+dx,0+dy);
		vertices[1] = Point2f(vl+dx,0+dy);
		vertices[2] = Point2f(width_-vr+dx,0+dy);
		vertices[3] = Point2f(width_+dx,0+dy);
		vertices[4] = Point2f(0+dx,vt+dy);
		vertices[5] = Point2f(vl+dx,vt+dy);
		vertices[6] = Point2f(width_-vr+dx,vt+dy);
		vertices[7] = Point2f(width_+dx,vt+dy);
		vertices[8] = Point2f(0+dx,height_-vb+dy);
		vertices[9] = Point2f(vl+dx,height_-vb+dy);
		vertices[10] = Point2f(width_-vr+dx,height_-vb+dy);
		vertices[11] = Point2f(width_+dx,height_-vb+dy);
		vertices[12] = Point2f(0+dx,height_+dy);
		vertices[13] = Point2f(vl+dx,height_+dy);
		vertices[14] = Point2f(width_-vr+dx,height_+dy);
		vertices[15] = Point2f(width_+dx,height_+dy);
	}
	else {
		vertices.resize(4);
		vertices[0] = Point2f(0+dx,0+dy);
		vertices[1] = Point2f(width_+dx,0+dy);
		vertices[2] = Point2f(width_+dx,height_+dy);
		vertices[3] = Point2f(0+dx,height_+dy);
    }
	vertices.Update();
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void Pixel::updateTexture()
{
    TextureBase* texture = texture_[0];

    float tx = tx_;
    float ty = ty_;
    float tw = tw_;
    float th = th_;

    float etw = texture->data->exwidth;
    float eth = texture->data->exheight;
    float u0 = tx / etw;
    float v0 = ty / eth;

    if (isStretching_ || isNinePatch_ || texture->data->parameters.wrap == eRepeat) {
        float w, h, x, y;
        float pfx,pfy;
        if (isStretching_|| isNinePatch_) {
        	pfx=1.0 / (etw * sx_);
        	pfy=1.0 / (eth * sy_);
            w = tw * pfx;
            h = th * pfy;
            x = 0.5 * w * (sx_ - 1) - x_ * w;
            y = 0.5 * h * (sy_ - 1) - y_ * h;
        } else {
            float tsx = texture->uvscalex;
            float tsy = texture->uvscaley;
            w = width_*tsx / (etw * sx_);
            h = height_*tsy / (eth * sy_);
            x = -x_*tsx / etw;
            y = -y_*tsy / eth;
        }
        x+= u0;
        y+= v0;

        if (isNinePatch_) {
    		float vt=insett_t_*pfy;
    		float vb=insett_b_*pfy;
    		float vr=insett_r_*pfx;
    		float vl=insett_l_*pfx;
    		float dw=w-vr-vl;
    		float dh=h-vb-vt;
    		if (dw<0) {
    			float r=w/(vr+vl); dw=0;
    			vr*=r; vl*=r;
    		}
    		if (dh<0) {
    			float r=h/(vt+vb); dh=0;
    			vt*=r; vb*=r;
    		}
        	texcoords.resize(16);
    		texcoords[0] = Point2f(x,y);
    		texcoords[1] = Point2f(x+vl,y);
    		texcoords[2] = Point2f(x+w-vr,y);
    		texcoords[3] = Point2f(x+w,y);
    		texcoords[4] = Point2f(x,y+vt);
    		texcoords[5] = Point2f(x+vl,y+vt);
    		texcoords[6] = Point2f(x+w-vr,y+vt);
    		texcoords[7] = Point2f(x+w,y+vt);
    		texcoords[8] = Point2f(x,y+h-vb);
    		texcoords[9] = Point2f(x+vl,y+h-vb);
    		texcoords[10] = Point2f(x+w-vr,y+h-vb);
    		texcoords[11] = Point2f(x+w,y+h-vb);
    		texcoords[12] = Point2f(x,y+h);
    		texcoords[13] = Point2f(x+vl,y+h);
    		texcoords[14] = Point2f(x+w-vr,y+h);
    		texcoords[15] = Point2f(x+w,y+h);
        }
        else {
        	texcoords.resize(4);
            texcoords[0] = Point2f(x,y);
            texcoords[1] = Point2f(x+w,y);
            texcoords[2] = Point2f(x+w,y+h);
            texcoords[3] = Point2f(x,y+h);
        }

        for (size_t tc=0;tc<texcoords.size();tc++)
			tmatrix_.transformPoint(texcoords[tc].x, texcoords[tc].y, &texcoords[tc].x,&texcoords[tc].y);
 		texcoords.Update();
 		invalidate(INV_GRAPHICS);
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

    float dx=-width_*anchorx_;
    float dy=-height_*anchory_;
    vertices[0] = Point2f(x1+dx,y1+dy);
    vertices[1] = Point2f(x2+dx,y1+dy);
    vertices[2] = Point2f(x2+dx,y2+dy);
    vertices[3] = Point2f(x1+dx,y2+dy);
    vertices.Update();

    float tx1, ty1, tx2, ty2;

    float u = tw / etw;
    float v = th / eth;

    float rw = x2 - x1;
    float rh = y2 - y1;

    float rx = 0.5 - (0.5 + x_) * rw / w;
    float ry = 0.5 - (0.5 + y_) * rh / h;

    tx1 = u0 + u * rx;
    ty1 = v0 + v * ry;
    tx2 = u0 + u * (rw / w + rx);
    ty2 = v0 + v * (rh / h + ry);

    texcoords[0] = Point2f(tx1,ty1);
    texcoords[1] = Point2f(tx2,ty1);
    texcoords[2] = Point2f(tx2,ty2);
    texcoords[3] = Point2f(tx1,ty2);
    for (size_t tc=0;tc<texcoords.size();tc++)
		tmatrix_.transformPoint(texcoords[tc].x, texcoords[tc].y, &texcoords[tc].x,&texcoords[tc].y);
    texcoords.Update();
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void Pixel::setAnchorPoint(float x, float y)
{
	anchorx_ = x;
	anchory_ = y;

	updateVertices();
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void Pixel::getAnchorPoint(float* x, float* y) const
{
	if (x)
		*x = anchorx_;
	if (y)
		*y = anchory_;
}

bool Pixel::setDimensions(float width,float height,bool forLayout)
{
    bool changed=(width_!=width)||(height_!=height);
	width_=width;
	height_=height;
	if (!forLayout) {
		minw_=width;
		minh_=height;
	}
    if (changed) {
        updateVertices();
        invalidate(INV_GRAPHICS|INV_BOUNDS);
        if ((!(isStretching_|| isNinePatch_))&&texture_[0]) updateTexture();
    }
    return Sprite::setDimensions(width, height);
}

void Pixel::setTextureRegion(BitmapData *bitmapdata,int slot)
{
	setTexture(bitmapdata->texture(),slot,NULL);

    if (slot==0)
    {
    	int x,y,width,height;
    	bitmapdata->getRegion(&x, &y, &width, &height, NULL,NULL,NULL,NULL);
        tx_=x;
        ty_=y;
        tw_=width;
        th_=height;
        updateTexture();
 	}
	invalidate(INV_GRAPHICS);
}

void Pixel::setTexture(TextureBase *texture,int slot, const Matrix4* matrix)
{
    if (texture)
        texture->ref();
    if (texture_[slot])
        texture_[slot]->unref();
    texture_[slot] = texture;
    if (matrix) tmatrix_=*matrix;

    if (slot==0)
    {
        if (texture) {
            tx_=0;
            ty_=0;
            tw_=texture->data->width;
            th_=texture->data->height;
        	updateTexture();
        }
 	}
	invalidate(INV_GRAPHICS);
}

void Pixel::setNinePatch(float vl,float vr,float vt,float vb,float tl,float tr,float tt,float tb)
{
	insetv_t_=vt;
	insetv_b_=vb;
	insetv_r_=vr;
	insetv_l_=vl;
	insett_t_=tt;
	insett_b_=tb;
	insett_r_=tr;
	insett_l_=tl;
	isNinePatch_=(vt||vb||vr||vl);
	updateVertices();
    if (texture_[0]) updateTexture();
}

void Pixel::setTextureMatrix(const Matrix4* matrix)
{
	tmatrix_=*matrix;
    if (texture_[0]) updateTexture();
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
	invalidate(INV_GRAPHICS);
}

int Pixel::getMixedColor(int c1, int c2, float a1,float a2,float a,float &am)
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
    am= a1*a+a2*(1-a);
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

    float ao1,ao2,ao3,ao4;
    float c1 = getMixedColor(co1,co2,a1,a2,f1,ao1);
    float c2 = getMixedColor(co1,co2,a1,a2,f2,ao2);
    float c3 = getMixedColor(co1,co2,a1,a2,f3,ao3);
    float c4 = getMixedColor(co1,co2,a1,a2,f4,ao4);

    setGradient(c1, ao1, c2, ao2, c3, ao3, c4, ao4);
}
